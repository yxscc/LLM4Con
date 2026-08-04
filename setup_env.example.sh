#!/bin/bash
# ============================================================
#  Lace / LLM4Con environment template
#
#  Usage:  cp setup_env.example.sh setup_env.sh
#          # edit the paths below, then
#          source setup_env.sh
#
#  Defines every variable the build and the experiment scripts read.
#  If your site needs an HTTP proxy to reach the LLM endpoint, export
#  http_proxy / https_proxy / no_proxy here as well.
# ============================================================

# ------------------------------------------------------------ paths
# Repository root.
export LLM4CON_HOME="/path/to/LLM4Con"

# Root of the per-case experiment directories that prepare_cve.sh,
# batch_detect.sh and the single-case runners operate on.
export EXPERIMENT_BASE="${LLM4CON_HOME}/kernel_experiment"

# Linux kernel git clone, used to extract the vulnerable slices.
export LINUX_REPO="/path/to/linux.git"

# Third-party tools (Joern, Phasar, …).
export EXTERNAL_DIR="/path/to/external"

# Joern requires Java 17+; point JAVA_HOME at it explicitly if the system
# default is older.
export JAVA_HOME="/usr/lib/jvm/java-17-openjdk-amd64"
export PATH="${JAVA_HOME}/bin:${EXTERNAL_DIR}/joern-cli:${EXTERNAL_DIR}/bin:${PATH}"

export PHASAR_INSTALL_DIR="${EXTERNAL_DIR}/phasar/install"
export CMAKE_PREFIX_PATH="${PHASAR_INSTALL_DIR}:${CMAKE_PREFIX_PATH}"

# ------------------------------------------------------------ toolchain
# Lace and Phasar are built with LLVM 16.
export LLVM_DIR="/usr/lib/llvm-16/cmake"
export CC=clang-16
export CXX=clang++-16

# Used by prepare_cve.sh / batch_prepare.sh for kernel single-TU compilation.
# Our runs used clang-15 for the kernel C -> bitcode step; newer clang
# miscompiles some of the older kernel slices.
export CLANG=clang-15

# ------------------------------------------------------------ LLM endpoint
# Any OpenAI-compatible Chat Completions endpoint. Some gateways authenticate
# with a query parameter instead of a Bearer header; append it to the URL if so.
if [ -z "${API_KEY:-}" ]; then
    echo "[setup_env] WARN: API_KEY not set; LLM calls will fail." >&2
fi
export LLM_API_KEY="$API_KEY"
export LLM_BASE_URL="https://your-endpoint.example.com/v1/chat/completions"
export LLM_MODEL="gpt-5.5-2026-04-24"

# ------------------------------------------------------------ cve_survey.csv
# batch_prepare.sh reads this index (CVE,HAS_PATCH,FILES,FIX_COMMIT). It is not
# tracked, but can be rebuilt from the per-case ground_truth.json files.
export SURVEY_FILE="${LLM4CON_HOME}/kernel_experiment/cve_survey.csv"
if [ ! -f "$SURVEY_FILE" ] || [ "${SURVEY_FILE}" -ot "${LLM4CON_HOME}/kernel_experiment" ]; then
    python3 - "$LLM4CON_HOME/kernel_experiment" "$SURVEY_FILE" <<'PYEOF'
import json, os, glob, sys
ke, out = sys.argv[1:3]
with open(out, 'w') as fout:
    fout.write("CVE,HAS_PATCH,FILES,FIX_COMMIT\n")
    n = 0
    # Walk both CVE-* and SYZBOT-* entry dirs; the dataset contains syzbot
    # concurrency bugs alongside the CVE set.
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
  API_KEY           = $([ -n "${API_KEY:-}" ] && echo '<set>' || echo '<missing>')
  LLM_MODEL         = ${LLM_MODEL}
  SURVEY_FILE       = ${SURVEY_FILE}
EOF
