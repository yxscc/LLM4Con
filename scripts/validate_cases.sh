#!/bin/bash
# Validation runner for the contract-spec refactor. Runs the freshly built
# detector on a few cases WITHOUT clobbering the p4x4 comparison logs.
# Writes detection_validate_<stamp>.log per case. Read-only w.r.t. dataset.
#
# Usage: bash scripts/validate_cases.sh <stamp> CVE-... CVE-... ...
set -o pipefail
ROOT="/mlx_devbox/users/mayunlong.39/playground/LLM4Con"
source "$ROOT/setup_env.sh" >/dev/null 2>&1
export LLM4CON_HOME="$ROOT"
export EXPERIMENT_BASE="$ROOT/kernel_experiment"
DETECTOR="$ROOT/Release-build/llm_detector"
export LACE_STATIC_COMPOSE=1
export LACE_CONTRACT_PARALLELISM="${LACE_CONTRACT_PARALLELISM:-4}"
export LACE_ENABLE_FLOW_PRIOR=0

STAMP="$1"; shift

# Match resume_staticcompose_p4x4.py bitcode selection exactly (no oracle flow_annotation).
select_bitcode() {
    local d="$1"
    if [ -f "$d/merged.ll" ]; then echo "merged.ll"; return 0; fi
    if [ -f "$d/snd-seq.ll" ]; then echo "snd-seq.ll"; return 0; fi
    ls -S "$d"/*.ll 2>/dev/null | head -1 | xargs -r basename
}

for cve in "$@"; do
    d="$EXPERIMENT_BASE/$cve"
    [ -d "$d" ] || { echo "[$cve] SKIP no dir"; continue; }
    bc=$(select_bitcode "$d")
    [ -n "$bc" ] || { echo "[$cve] SKIP no bitcode"; continue; }
    log="$d/detection_validate_${STAMP}.log"
    echo "[$cve] start $(date +%H:%M:%S)  bc=$bc -> $(basename "$log")"
    # Static-compose contract pipeline = --legacy-workflow --abl-contract on (mode=legacy-static).
    ( cd "$d" && timeout 5400 "$DETECTOR" --input-bc "$bc" --input-src src \
        --legacy-workflow --abl-contract on \
        --llm-provider openai --llm-url "$LLM_BASE_URL" --llm-key "$LLM_API_KEY" \
        --llm-model "$LLM_MODEL" > "$log" 2>&1 )
    rc=$?
    echo "[$cve] done rc=$rc $(date +%H:%M:%S)"
done
echo "ALL_DONE"
