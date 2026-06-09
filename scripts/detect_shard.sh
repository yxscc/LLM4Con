#!/bin/bash
# Per-worker detection runner. Reads a list of CVE ids from stdin (one per
# line) and invokes llm_detector for each. Mirrors batch_detect.sh's
# per-case logic exactly except for the global ConsecutiveAPI_ERR abort
# (kept per-worker) and the worker-tagged progress prefix.
#
# Usage:
#   echo -e "CVE-2013-1792\nCVE-2015-7550\n..." \
#     | WORKER_ID=0 bash detect_shard.sh
#
# Required env: LLM4CON_HOME, EXPERIMENT_BASE, LLM_API_KEY, LLM_BASE_URL,
#               LLM_MODEL, WORKER_ID.

set -o pipefail
shopt -s nullglob

LLM4CON_HOME="${LLM4CON_HOME:?LLM4CON_HOME unset}"
EXPERIMENT_BASE="${EXPERIMENT_BASE:-$LLM4CON_HOME/kernel_experiment}"
DETECTOR="${DETECTOR:-$LLM4CON_HOME/Release-build/llm_detector}"
WID="${WORKER_ID:?WORKER_ID unset}"

# Per-worker counters (NOT shared across workers).
SUCCESS=0
FAIL=0
SKIP=0
CONSECUTIVE_API_ERR=0
MAX_CONSECUTIVE_API_ERR=5

pfx() { printf '[w%d] %s\n' "$WID" "$1"; }

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
        local ll_files=("$cve_dir"/*.ll)
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

pfx "shard start: $(date)"

while IFS= read -r cve_id; do
    [ -z "$cve_id" ] && continue
    cve_dir="$EXPERIMENT_BASE/$cve_id"
    if [ ! -d "$cve_dir" ]; then
        pfx "[$cve_id] SKIP (no dir)"
        SKIP=$((SKIP+1))
        continue
    fi
    src_dir="$cve_dir/src"

    # Prefer annotation-selected bitcode; fall back to batch_detect.sh's
    # historical heuristic.
    bc_file=$(select_bitcode "$cve_dir")
    if [ -z "$bc_file" ]; then
        pfx "[$cve_id] SKIP (no .ll files)"
        SKIP=$((SKIP+1))
        continue
    fi

    if [ ! -d "$src_dir" ] || [ -z "$(find "$src_dir" -name '*.c' -print -quit 2>/dev/null)" ]; then
        pfx "[$cve_id] SKIP (no src/)"
        SKIP=$((SKIP+1))
        continue
    fi

    if [ -f "$cve_dir/detection_hypothesis_batch.log" ]; then
        if grep -q "hypotheses confirmed\|Bug_Detection.*COMPLETED" "$cve_dir/detection_hypothesis_batch.log" 2>/dev/null; then
            pfx "[$cve_id] SKIP (already detected)"
            SKIP=$((SKIP+1))
            continue
        fi
    fi

    entry_config="$cve_dir/entry_points.txt"
    entry_flag=""
    if [ -f "$entry_config" ] && [ -s "$entry_config" ]; then
        entry_flag="--entry-config entry_points.txt"
    fi

    bc_basename=$(basename "$bc_file")
    pfx "[$cve_id] Detecting ($bc_basename) ..."
    log_file="$cve_dir/detection_hypothesis_batch.log"

    (
        cd "$cve_dir" && "$DETECTOR" \
            --input-bc "$bc_basename" \
            --input-src src \
            $entry_flag \
            --agent-mode \
            --llm-provider openai \
            --llm-url "$LLM_BASE_URL" \
            --llm-key "$LLM_API_KEY" \
            --llm-model "$LLM_MODEL" \
            > "$log_file" 2>&1
    )
    rc=$?

    if [ $rc -eq 0 ]; then
        if grep -q "Phase 2 ERROR" "$log_file" 2>/dev/null; then
            pfx "[$cve_id] API_ERROR"
            FAIL=$((FAIL+1))
            CONSECUTIVE_API_ERR=$((CONSECUTIVE_API_ERR+1))
            if [ "$CONSECUTIVE_API_ERR" -ge "$MAX_CONSECUTIVE_API_ERR" ]; then
                pfx "[ABORT] $CONSECUTIVE_API_ERR consecutive API_ERROR, bailing"
                break
            fi
        elif grep -q "POTENTIAL.*VIOLATION" "$log_file" 2>/dev/null; then
            bugs=$(grep -c "POTENTIAL.*VIOLATION" "$log_file" 2>/dev/null)
            hyps=$(grep -oP '\d+(?= hypotheses confirmed)' "$log_file" 2>/dev/null | head -1)
            pfx "[$cve_id] FOUND $bugs bug(s) ($hyps hypotheses)"
            CONSECUTIVE_API_ERR=0
            SUCCESS=$((SUCCESS+1))
        elif grep -q "No bugs detected" "$log_file" 2>/dev/null; then
            pfx "[$cve_id] CLEAN"
            CONSECUTIVE_API_ERR=0
            SUCCESS=$((SUCCESS+1))
        else
            pfx "[$cve_id] DONE"
            CONSECUTIVE_API_ERR=0
            SUCCESS=$((SUCCESS+1))
        fi
    else
        pfx "[$cve_id] FAIL (exit=$rc)"
        FAIL=$((FAIL+1))
    fi
done

pfx "shard end:   $(date) — success=$SUCCESS fail=$FAIL skip=$SKIP"
