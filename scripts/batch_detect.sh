#!/bin/bash
# Batch run llm_detector on all prepared CVE experiments
# Usage: ./batch_detect.sh --api-key <key> [--model <model>] [--timeout <seconds>]

set -o pipefail
shopt -s nullglob

DETECTOR="${DETECTOR:-/home/LLM4Con/Release-build/llm_detector}"
EXPERIMENT_BASE="/home/LLM4Con/kernel_experiment"
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

masked_key="${API_KEY:0:7}…${API_KEY: -4}"
echo "============================================"
echo " Batch Detection Run"
echo " Model:    $MODEL"
echo " Base URL: $BASE_URL"
echo " Key:      $masked_key  (len=${#API_KEY})"
echo "============================================"

# --- Preflight: 1 small chat call to fail fast on bad key/url/model -----------
echo "[Preflight] Probing $BASE_URL ..."
preflight_resp=$(curl -sS -X POST "$BASE_URL" \
    -H "Authorization: Bearer $API_KEY" \
    -H "Content-Type: application/json" \
    -d "{\"model\":\"$MODEL\",\"messages\":[{\"role\":\"user\",\"content\":\"ping\"}],\"max_tokens\":4}" \
    --max-time 20 2>&1)
if ! echo "$preflight_resp" | grep -q '"choices"'; then
    echo "[Preflight FAIL] Aborting batch. Response excerpt:"
    echo "$preflight_resp" | head -c 500
    echo ""
    exit 1
fi
echo "[Preflight OK] API reachable, key + model accepted."
echo ""

for cve_dir in "$EXPERIMENT_BASE"/CVE-*/; do
    cve_id=$(basename "$cve_dir")
    src_dir="$cve_dir/src"

    # Bitcode selection: merged.ll > snd-seq.ll (pre-linked ALSA) > largest .ll (often aggregate) > first
    bc_file=""
    if [ -f "$cve_dir/merged.ll" ]; then
        bc_file="$cve_dir/merged.ll"
    elif [ -f "$cve_dir/snd-seq.ll" ]; then
        bc_file="$cve_dir/snd-seq.ll"
    else
        ll_files=("$cve_dir"*.ll)
        if [ ${#ll_files[@]} -eq 0 ]; then
            echo "[$cve_id] SKIP (no .ll files)"
            SKIP=$((SKIP+1))
            continue
        fi
        if [ ${#ll_files[@]} -eq 1 ]; then
            bc_file="${ll_files[0]}"
        else
            # Multiple unmerged modules: largest file is usually the main / linked unit
            bc_file=$(ls -S "$cve_dir"/*.ll 2>/dev/null | head -1)
        fi
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
