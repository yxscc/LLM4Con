#!/usr/bin/env bash
# B4 (Codex CLI + cybersecurity skill) launcher.
#
#   bash run_b4.sh shim                 # (re)start the local Responses<->gateway shim
#   bash run_b4.sh smoke               # 2-CVE smoke
#   bash run_b4.sh one CVE-2024-27019  # single CVE
#   bash run_b4.sh full [P]            # full 100-CVE batch, P parallel shards (default 1), resumable
#   bash run_b4.sh eval                # score the B4 dump with the shared judge
#
# Everything is model-controlled on GPT-5.5 via the gateway (same as B1/B2/B3).
set -o pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LLM4CON_HOME="${LLM4CON_HOME:-/mlx_devbox/users/mayunlong.39/playground/LLM4Con}"
# shellcheck disable=SC1091
source "$LLM4CON_HOME/setup_env.sh" >/dev/null 2>&1

# Proxy needs an explicit scheme for codex/semgrep's HTTP clients.
PROXY="http://sys-proxy-rd-relay.byted.org:8118"
export HTTP_PROXY="$PROXY" HTTPS_PROXY="$PROXY" http_proxy="$PROXY" https_proxy="$PROXY"
export NO_PROXY="127.0.0.1,localhost,.byted.org,byted.org,.bytedance.net,bytedance.net"
export no_proxy="$NO_PROXY"
export CODEX_HOME="$HERE/codex_home"
export GW_KEY="${GW_KEY:-dummy}"
export PATH="$LLM4CON_HOME/../external/bin:$HOME/.local/bin:$PATH"
export SHIM_PORT="${SHIM_PORT:-8799}"
LOG_DIR="$HERE/logs"; mkdir -p "$LOG_DIR"

shim_up() { curl -s --noproxy 127.0.0.1 -o /dev/null -w "%{http_code}" \
    --max-time 4 "http://127.0.0.1:${SHIM_PORT}/v1/models" 2>/dev/null | grep -q 200; }

start_shim() {
    if shim_up; then echo "[shim] already up on :$SHIM_PORT"; return 0; fi
    SHIM_LOG="$LOG_DIR/shim.log" nohup python3 "$HERE/shim.py" \
        >"$LOG_DIR/shim.out" 2>&1 &
    echo "[shim] started pid=$! port=$SHIM_PORT"
    sleep 2; shim_up && echo "[shim] healthy" || echo "[shim] WARN not healthy"
}

case "${1:-smoke}" in
    shim)  start_shim ;;
    smoke) start_shim; python3 "$HERE/run.py" --cve CVE-2024-27019 CVE-2024-26974 ;;
    one)   start_shim; shift; python3 "$HERE/run.py" --cve "$@" ;;
    full)
        start_shim
        P="${2:-1}"
        if [[ "$P" -le 1 ]]; then
            python3 "$HERE/run.py" --skip-existing
        else
            echo "[full] launching $P parallel shards"
            for ((i=0;i<P;i++)); do
                logf="$LOG_DIR/full_shard${i}_$(date +%Y%m%d_%H%M%S).log"
                nohup python3 "$HERE/run.py" --skip-existing --shard "$i/$P" \
                    >"$logf" 2>&1 &
                echo "  shard $i/$P pid=$! log=$logf"
            done
            echo "[full] $P shards running; wait + score with: bash run_b4.sh eval"
        fi
        ;;
    eval)
        DUMP_BASE="$LLM4CON_HOME/kernel_experiment/baseline_dump/B4_codex_skill" \
        EXPERIMENT_BASE="$LLM4CON_HOME/kernel_experiment" \
        python3 "$LLM4CON_HOME/scripts/evaluate_recall.py" \
            --api-key "${API_KEY}" --base-url "$LLM_BASE_URL" --model "$LLM_MODEL" \
            --output "$HERE/../baseline_eval/B4_codex_skill_eval.json"
        ;;
    *) echo "usage: $0 {shim|smoke|one <CVE...>|full [P]|eval}" >&2; exit 1 ;;
esac
