#!/usr/bin/env bash
# Convenience driver: kick off B1/B2/B3 full 50-CVE batches in
# parallel under nohup, then (optionally) wait + evaluate.
#
# Usage:
#   bash run_all.sh start                    # spawn 3 background batches
#   bash run_all.sh status                   # print per-batch progress
#   bash run_all.sh eval                     # score finished batches
#   bash run_all.sh report > RESULTS.md      # produce final markdown table
#   bash run_all.sh all                      # start → wait → eval → report
set -o pipefail

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$HERE"

LLM4CON_HOME="${LLM4CON_HOME:?run \`source ../../setup_env.sh\` first}"
DUMP_BASE_ROOT="${BASELINE_DUMP_BASE:-$LLM4CON_HOME/kernel_experiment/baseline_dump}"
LOG_DIR="$HERE/logs"
EVAL_DIR="$HERE/baseline_eval"
mkdir -p "$LOG_DIR" "$EVAL_DIR"

start_batch() {
    local label="$1"
    local cmd="$2"
    local logf="$LOG_DIR/${label}_$(date +%Y-%m-%d_%H-%M-%S).log"
    local pidf="$LOG_DIR/${label}.pid"

    if [[ -f "$pidf" ]] && kill -0 "$(cat "$pidf")" 2>/dev/null; then
        echo "[$label] already running (pid $(cat "$pidf"))"
        return 0
    fi
    nohup bash -c "$cmd" > "$logf" 2>&1 &
    echo $! > "$pidf"
    echo "[$label] started, pid=$(cat "$pidf"), log=$logf"
}

cmd_start() {
    start_batch B1 \
        "python3 $HERE/B1_zeroshot/run.py --skip-existing"
    start_batch B2 \
        "python3 $HERE/B2_ccsr/run.py --skip-existing"
    start_batch B3 \
        "python3 $HERE/B3_mythos/pipeline.py --skip-existing --max-files 3"
}

cmd_status() {
    for label in B1 B2 B3; do
        local pidf="$LOG_DIR/${label}.pid"
        if [[ ! -f "$pidf" ]]; then
            echo "[$label] not yet started"
            continue
        fi
        local pid
        pid="$(cat "$pidf")"
        if kill -0 "$pid" 2>/dev/null; then
            echo "[$label] running (pid $pid)"
        else
            echo "[$label] finished (pid $pid not alive)"
        fi
        local logf
        logf="$(ls -t "$LOG_DIR"/${label}_*.log 2>/dev/null | head -1)"
        if [[ -n "$logf" ]]; then
            echo "  log: $logf"
            tail -3 "$logf" | sed 's/^/    /'
        fi
    done
}

cmd_wait() {
    for label in B1 B2 B3; do
        local pidf="$LOG_DIR/${label}.pid"
        [[ -f "$pidf" ]] || continue
        local pid
        pid="$(cat "$pidf")"
        if ! kill -0 "$pid" 2>/dev/null; then
            continue
        fi
        echo "[wait] $label pid=$pid..."
        while kill -0 "$pid" 2>/dev/null; do
            sleep 60
        done
        echo "[wait] $label done"
    done
}

cmd_eval() {
    for label in B1_zeroshot B2_ccsr B3_mythos; do
        local dump="$DUMP_BASE_ROOT/$label"
        [[ -d "$dump" ]] || { echo "[eval] $label: no dump dir, skip"; continue; }
        local out="$EVAL_DIR/${label}_eval.json"
        echo "[eval] $label → $out"
        DUMP_BASE="$dump" \
        EXPERIMENT_BASE="$LLM4CON_HOME/kernel_experiment" \
        python3 "$LLM4CON_HOME/scripts/evaluate_recall.py" \
            --api-key "${LLM_API_KEY:-$API_KEY}" \
            --base-url "$LLM_BASE_URL" \
            --model "$LLM_MODEL" \
            --output "$out" \
            2>&1 | tail -15
    done
}

cmd_report() {
    local args=()
    if [[ -f "$LLM4CON_HOME/kernel_experiment/evaluation_report.json" ]]; then
        args+=(--report "Lace_M7=$LLM4CON_HOME/kernel_experiment/evaluation_report.json")
    fi
    for label in B1_zeroshot B2_ccsr B3_mythos; do
        local p="$EVAL_DIR/${label}_eval.json"
        if [[ -f "$p" ]]; then
            args+=(--report "$label=$p")
        fi
    done
    if [[ ${#args[@]} -eq 0 ]]; then
        echo "no evaluation_report.json files found" >&2
        return 1
    fi
    python3 "$HERE/compare.py" "${args[@]}" --out "$HERE/RESULTS.md"
    echo "[report] wrote $HERE/RESULTS.md"
}

cmd_all() {
    cmd_start
    cmd_wait
    cmd_eval
    cmd_report
}

case "${1:-status}" in
    start)   cmd_start ;;
    status)  cmd_status ;;
    wait)    cmd_wait ;;
    eval)    cmd_eval ;;
    report)  cmd_report ;;
    all)     cmd_all ;;
    *)
        echo "usage: $0 {start|status|wait|eval|report|all}" >&2
        exit 1
        ;;
esac
