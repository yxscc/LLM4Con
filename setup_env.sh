#!/bin/bash
# ============================================================
#  Lace / LLM4Con  环境一键 source 脚本（字节开发机版本）
#  用法:  source setup_env.sh
#
#  覆盖：byted 代理、外部工具路径、API key/endpoint、模型名、
#  LLM4CON_HOME / LINUX_REPO 等所有 batch_*.sh 读取的变量。
# ============================================================

# ------------------------------------------------------------ proxy
# 字节开发机外网代理（仅外网；mirrors.byted.org / search.bytedance.net 走 NO_PROXY）
export http_proxy=sys-proxy-rd-relay.byted.org:8118
export https_proxy=sys-proxy-rd-relay.byted.org:8118
export no_proxy=.byted.org,byted.org,.bytedance.net,bytedance.net,localhost,127.0.0.1
export HTTP_PROXY="$http_proxy"
export HTTPS_PROXY="$https_proxy"
export NO_PROXY="$no_proxy"

# ------------------------------------------------------------ paths
# 项目主目录（HANDOFF 假设是 /home/LLM4Con；这台机器迁到了 workspace 里）
export LLM4CON_HOME="/mlx_devbox/users/mayunlong.39/playground/LLM4Con"
# kernel_experiment 子目录是 prepare_cve.sh / batch_detect.sh / single-CVE
# 调用脚本读取的实际 CVE 目录根。HANDOFF 老的 setup_env.sh 漏掉了这一行，
# 单次 invoke `cd "$EXPERIMENT_BASE/CVE-..."` 时会拼成 "/CVE-..." 失败。
export EXPERIMENT_BASE="${LLM4CON_HOME}/kernel_experiment"

# Linux 内核 git（HANDOFF §4.2 假设是 /home/ConCord/targets/linux.git）
export LINUX_REPO="/mlx_devbox/users/mayunlong.39/playground/linux.git"

# CVE 报告目录（HANDOFF 默认 /home/ConCord/concurrency_cve_reports/linux_kernel）
# 如果你已经把它放到了别处，改这里：
# export REPORTS_DIR="/path/to/reports"

# 第三方工具
export EXTERNAL_DIR="/mlx_devbox/users/mayunlong.39/playground/external"
# Joern 需要 Java 17+；这台 Merlin 机器 /opt/tiger/yarn_deploy/jdk 是 1.8，会踩坑。强制指 17。
export JAVA_HOME="/usr/lib/jvm/java-17-openjdk-amd64"
export PATH="${JAVA_HOME}/bin:${EXTERNAL_DIR}/joern-cli:${EXTERNAL_DIR}/bin:${PATH}"

# Phasar install 路径（构建后填上）
export PHASAR_INSTALL_DIR="${EXTERNAL_DIR}/phasar/install"
export CMAKE_PREFIX_PATH="${PHASAR_INSTALL_DIR}:${CMAKE_PREFIX_PATH}"

# 强制 LLM4Con / Phasar 都用 LLVM-16
export LLVM_DIR="/usr/lib/llvm-16/cmake"
export CC=clang-16
export CXX=clang++-16
# Used by prepare_cve.sh / batch_prepare.sh for kernel single-TU compilation.
# clang-15 is what previous successful runs used for the kernel C → bitcode
# step; clang-19 (which we have for Phasar) miscompiles older kernels.
export CLANG=clang-15

# ------------------------------------------------------------ LLM endpoint
# 字节内网 GPT 网关；AK 通过 ?ak= 查询参数鉴权（同时也以 Bearer header 发送，目前网关接受双重）
# API_KEY 应该已经在你的环境里；如果没有就：
# export API_KEY="<your_GPT_AK>"
if [ -z "${API_KEY:-}" ]; then
    echo "[setup_env] WARN: API_KEY not set; LLM calls will fail." >&2
fi
export LLM_API_KEY="$API_KEY"
export LLM_BASE_URL="https://search.bytedance.net/gpt/openapi/online/v2/crawl?ak=${API_KEY}"
# 模型沿用：上一台机器是 claude-sonnet-4-6，本机 AK 不放行 claude；用 GPT-5.5（thinking 类，含 reasoning_tokens）
export LLM_MODEL="gpt-5.5-2026-04-24"

# ------------------------------------------------------------ cve_survey.csv
# batch_prepare.sh 需要读它（CVE,HAS_PATCH,FILES,FIX_COMMIT）。
# 不在 git 里，但能从每个 ground_truth.json 重建。这里如果不存在就自动生成。
export SURVEY_FILE="${LLM4CON_HOME}/kernel_experiment/cve_survey.csv"
if [ ! -f "$SURVEY_FILE" ] || [ "${SURVEY_FILE}" -ot "${LLM4CON_HOME}/kernel_experiment" ]; then
    python3 - "$LLM4CON_HOME/kernel_experiment" "$SURVEY_FILE" <<'PYEOF'
import json, os, glob, sys
ke, out = sys.argv[1:3]
with open(out, 'w') as fout:
    fout.write("CVE,HAS_PATCH,FILES,FIX_COMMIT\n")
    n = 0
    # Walk both CVE-* and SYZBOT-* entry dirs. Dataset v2 added syzbot
    # concurrency bugs alongside the original CVE set; without globbing
    # SYZBOT-*, this regen would silently drop them.
    candidate_dirs = []
    for pat in ('CVE-*', 'SYZBOT-*'):
        candidate_dirs.extend(glob.glob(os.path.join(ke, pat)))
    for d in sorted(candidate_dirs):
        gt_path = os.path.join(d, 'ground_truth.json')
        if not os.path.isfile(gt_path):
            continue
        with open(gt_path) as f:
            gt = json.load(f)
        bug_id = gt.get('cve_id') or os.path.basename(d)
        files = ';'.join(gt.get('files', []))
        commit = gt.get('fix_commit', '')
        if files and commit:
            fout.write(f"{bug_id},YES,{files},{commit}\n")
            n += 1
    print(f"[setup_env] regenerated {out} ({n} entries)")
PYEOF
fi

# ------------------------------------------------------------ summary
cat <<EOF
[setup_env] ready.
  LLM4CON_HOME      = ${LLM4CON_HOME}
  LINUX_REPO        = ${LINUX_REPO}
  EXTERNAL_DIR      = ${EXTERNAL_DIR}
  LLVM_DIR          = ${LLVM_DIR}
  PHASAR_INSTALL    = ${PHASAR_INSTALL_DIR}
  joern in PATH     = $(command -v joern || echo '<missing>')
  clang-16          = $(command -v clang-16 || echo '<missing>')
  proxy             = ${http_proxy}
  API_KEY           = ${API_KEY:0:7}...${API_KEY: -4}  (len=${#API_KEY})
  LLM_BASE_URL      = ${LLM_BASE_URL%%\?*}?ak=...
  LLM_MODEL         = ${LLM_MODEL}
  SURVEY_FILE       = ${SURVEY_FILE}  ($(wc -l < "$SURVEY_FILE" 2>/dev/null) lines)
EOF
