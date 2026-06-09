#!/bin/bash
# Batch run llm_detector on all prepared CVE experiments
# Usage: ./batch_detect.sh --api-key <key> [--model <model>] [--timeout <seconds>]

set -o pipefail
shopt -s nullglob

LLM4CON_HOME="${LLM4CON_HOME:-/home/LLM4Con}"
DETECTOR="${DETECTOR:-${LLM4CON_HOME}/Release-build/llm_detector}"
EXPERIMENT_BASE="${EXPERIMENT_BASE:-${LLM4CON_HOME}/kernel_experiment}"
API_KEY=""
MODEL="claude-sonnet-4-6"
BASE_URL="${LLM_BASE_URL:-https://jeniya.cn/v1/chat/completions}"
MAX_CONSECUTIVE_API_ERR=3

while [[ $# -gt 0 ]]; do
    case $1 in
        --api-key) API_KEY="$2"; shift 2 ;;
        --model) MODEL="$2"; shift 2 ;;
        --base-url) BASE_URL="$2"; shift 2 ;;
        *) echo "Unknown option: $1"; exit 1 ;;
    esac
done

if [ -z "$API_KEY" ]; then
    echo "Usage: $0 --api-key <key> [--model <model>] [--base-url <url>]"
    exit 1
fi

SUCCESS=0
FAIL=0
SKIP=0
CONSECUTIVE_API_ERR=0

select_bitcode() {
    local cve_dir="$1"
    local selected=""

    if [ -f "$cve_dir/flow_annotation.json" ]; then
        selected=$(python3 - "$cve_dir/flow_annotation.json" <<'PY' 2>/dev/null
import json, sys
try:
    data = json.load(open(sys.argv[1]))
    print((data.get("coverage") or {}).get("selected_bitcode") or "")
except Exception:
    print("")
PY
)
        if [ -n "$selected" ] && [ -f "$cve_dir/$selected" ]; then
            printf '%s\n' "$cve_dir/$selected"
            return 0
        fi
    fi

    if [ -f "$cve_dir/merged.ll" ]; then
        printf '%s\n' "$cve_dir/merged.ll"
    elif [ -f "$cve_dir/snd-seq.ll" ]; then
        printf '%s\n' "$cve_dir/snd-seq.ll"
    else
        local ll_files=("$cve_dir"*.ll)
        if [ ${#ll_files[@]} -eq 0 ]; then
            return 1
        fi
        if [ ${#ll_files[@]} -eq 1 ]; then
            printf '%s\n' "${ll_files[0]}"
        else
            ls -S "$cve_dir"/*.ll 2>/dev/null | head -1
        fi
    fi
}

masked_key="${API_KEY:0:7}…${API_KEY: -4}"
echo "============================================"
echo " Batch Detection Run"
echo " Model:    $MODEL"
echo " Base URL: $BASE_URL"
echo " Key:      $masked_key  (len=${#API_KEY})"
echo "============================================"

# --- Preflight: 1 small chat call to fail fast on bad key/url/model -----------
# Retry a few times so a single transient network blip doesn't kill a multi-
# hour batch (we hit exactly that on 2026-05-09: curl timed out once and
# the entire batch aborted, wasting the overnight slot).
echo "[Preflight] Probing $BASE_URL ..."
preflight_ok=0
for attempt in 1 2 3 4 5; do
    preflight_resp=$(curl -sS -X POST "$BASE_URL" \
        -H "Authorization: Bearer $API_KEY" \
        -H "Content-Type: application/json" \
        -d "{\"model\":\"$MODEL\",\"messages\":[{\"role\":\"user\",\"content\":\"ping\"}],\"max_tokens\":4}" \
        --max-time 30 2>&1)
    if echo "$preflight_resp" | grep -q '"choices"'; then
        preflight_ok=1
        break
    fi
    echo "[Preflight] attempt $attempt failed; backing off ${attempt}0s before retry..."
    sleep $((attempt * 10))
done
if [ "$preflight_ok" -ne 1 ]; then
    echo "[Preflight FAIL] 5 attempts exhausted. Aborting batch. Last response excerpt:"
    echo "$preflight_resp" | head -c 500
    echo ""
    exit 1
fi
echo "[Preflight OK] API reachable, key + model accepted."
echo ""

# Iterate both CVE-* and SYZBOT-* prepared cases.
shopt -s nullglob
all_dirs=("$EXPERIMENT_BASE"/CVE-*/ "$EXPERIMENT_BASE"/SYZBOT-*/)
for cve_dir in "${all_dirs[@]}"; do
    cve_id=$(basename "$cve_dir")
    src_dir="$cve_dir/src"

    # Prefer annotation-selected bitcode; fall back to the historical heuristic.
    bc_file=$(select_bitcode "$cve_dir")
    if [ -z "$bc_file" ]; then
        echo "[$cve_id] SKIP (no .ll files)"
        SKIP=$((SKIP+1))
        continue
    fi

    if [ ! -d "$src_dir" ] || [ -z "$(find "$src_dir" -name '*.c' -print -quit 2>/dev/null)" ]; then
        echo "[$cve_id] SKIP (no src/)"
        SKIP=$((SKIP+1))
        continue
    fi

    # Check if already detected (skip if previous run completed successfully)
    if [ -f "$cve_dir/detection_hypothesis_batch.log" ]; then
        if grep -q "hypotheses confirmed\|Bug_Detection.*COMPLETED" "$cve_dir/detection_hypothesis_batch.log" 2>/dev/null; then
            echo "[$cve_id] SKIP (already detected)"
            SKIP=$((SKIP+1))
            continue
        fi
    fi

    # Use entry_points.txt if present and non-empty, otherwise rely on auto-discovery
    entry_config="$cve_dir/entry_points.txt"
    entry_flag=""
    if [ -f "$entry_config" ] && [ -s "$entry_config" ]; then
        entry_flag="--entry-config entry_points.txt"
    fi

    bc_basename=$(basename "$bc_file")
    echo -n "[$cve_id] Detecting ($bc_basename)... "

    log_file="$cve_dir/detection_hypothesis_batch.log"

    cd "$cve_dir"
    "$DETECTOR" \
        --input-bc "$bc_basename" \
        --input-src src \
        $entry_flag \
        --agent-mode \
        --llm-provider openai \
        --llm-url "$BASE_URL" \
        --llm-key "$API_KEY" \
        --llm-model "$MODEL" \
        > "$log_file" 2>&1 \
        && {
            if grep -q "Phase 2 ERROR" "$log_file" 2>/dev/null; then
                echo "API_ERROR"
                FAIL=$((FAIL+1))
                CONSECUTIVE_API_ERR=$((CONSECUTIVE_API_ERR+1))
                if [ "$CONSECUTIVE_API_ERR" -ge "$MAX_CONSECUTIVE_API_ERR" ]; then
                    echo ""
                    echo "[ABORT] $CONSECUTIVE_API_ERR consecutive API_ERROR — bailing out to avoid wasting hours."
                    echo "============================================"
                    echo " Results: $SUCCESS done, $FAIL failed, $SKIP skipped"
                    echo "============================================"
                    exit 2
                fi
            elif grep -q "POTENTIAL.*VIOLATION" "$log_file" 2>/dev/null; then
                bugs=$(grep -c "POTENTIAL.*VIOLATION" "$log_file" 2>/dev/null || echo 0)
                hypotheses=$(grep -oP '\d+(?= hypotheses confirmed)' "$log_file" 2>/dev/null || echo "?")
                echo "FOUND $bugs bug(s) ($hypotheses hypotheses)"
                CONSECUTIVE_API_ERR=0
                SUCCESS=$((SUCCESS+1))
            elif grep -q "No bugs detected" "$log_file" 2>/dev/null; then
                echo "CLEAN (0 hypotheses)"
                CONSECUTIVE_API_ERR=0
                SUCCESS=$((SUCCESS+1))
            else
                echo "DONE"
                CONSECUTIVE_API_ERR=0
                SUCCESS=$((SUCCESS+1))
            fi
        } \
        || {
            exit_code=$?
            if [ $exit_code -eq 124 ]; then
                echo "TIMEOUT (${TIMEOUT}s)"
            else
                echo "FAIL (exit=$exit_code)"
            fi
            FAIL=$((FAIL+1))
        }

done

echo ""
echo "============================================"
echo " Results: $SUCCESS done, $FAIL failed, $SKIP skipped"
echo "============================================"
