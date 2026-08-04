#!/usr/bin/env bash
# B6 cross-function (DB-backed) control experiment.
#
# Single-TU B6 reduces Smatch's deepest cross-function analysis. This builds the
# proper Smatch cross-function database the way Dan Carpenter intends:
#   1. checkout fix~1 in the kernel tree,
#   2. `make defconfig` (gcc, core+net+fs subsystems built),
#   3. build_kernel_data.sh -> full defconfig build with smatch as CHECK,
#      producing smatch_db.sqlite (caller_info / return-states / lock states),
#   4. re-run smatch on the patched files via kchecker (now DB-backed),
#   5. save warnings for comparison against the single-TU B6 result.
#
# Usage: run_smatch_xfn.sh <CVE> <FIX_COMMIT> <file1.c> [file2.c ...]
set -o pipefail

CVE="$1"; FIX="$2"; shift 2; FILES=("$@")
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LLM4CON_HOME="${LLM4CON_HOME:-$(cd "$HERE/../../.." && pwd)}"
PLAY="$(dirname "$LLM4CON_HOME")"
SMATCH_DIR="$PLAY/external/baselines/smatch"
KERNEL="${LINUX_REPO:-$PLAY/linux.git}"
OUT="$PLAY/LLM4Con/kernel_experiment/baseline_dump/B6_smatch_xfn/$CVE"
mkdir -p "$OUT"
LOG="$OUT/run.log"
exec > >(tee "$LOG") 2>&1

echo "================ $CVE  (cross-function DB) ================"
echo "fix=$FIX  files=${FILES[*]}"
unset CC CXX            # setup_env.sh exports CC=clang-16; kernel must use gcc
export ARCH=x86_64

cd "$KERNEL" || { echo "no kernel tree"; exit 1; }

echo "--- [1] checkout ${FIX}~1 ---"
git -c advice.detachedHead=false checkout -f "${FIX}~1" 2>&1 | tail -2 || { echo "checkout failed"; exit 1; }
git --no-pager log -1 --format='HEAD=%h %s' | head -1
DESC=$(make -s kernelversion 2>/dev/null); echo "kernelversion=$DESC"

echo "--- [2] clean + defconfig ---"
make -s mrproper 2>/dev/null
rm -f smatch_db.sqlite smatch_warns.txt*
make -s defconfig 2>&1 | tail -3
# make sure the patched files' subsystems are enabled (best-effort)
make -s olddefconfig 2>&1 | tail -1

echo "--- [3] build_kernel_data.sh (full defconfig build + DB) ---"
t0=$(date +%s)
"$SMATCH_DIR/smatch_scripts/build_kernel_data.sh" 2>&1 | tail -8
t1=$(date +%s); echo "build+DB took $((t1-t0))s"
ls -la smatch_db.sqlite 2>/dev/null || echo "!! no smatch_db.sqlite produced"
cp -f smatch_warns.txt "$OUT/smatch_warns_wholebuild.txt" 2>/dev/null
echo "whole-build warnings: $(wc -l < smatch_warns.txt 2>/dev/null)"

echo "--- [4] DB-backed kchecker on patched files ---"
: > "$OUT/xfn_findings.txt"
for f in "${FILES[@]}"; do
    [[ "$f" == *.c ]] || continue
    echo "  kchecker $f"
    "$SMATCH_DIR/smatch_scripts/kchecker" --full-path "$f" \
        > "$OUT/$(basename "$f").xfn.txt" 2>"$OUT/$(basename "$f").xfn.err"
    n=$(grep -cE ' (warn|error|warning):' "$OUT/$(basename "$f").xfn.txt" 2>/dev/null)
    echo "    -> $n warn/error lines"
    grep -E ' (warn|error|warning):' "$OUT/$(basename "$f").xfn.txt" >> "$OUT/xfn_findings.txt" 2>/dev/null
done

echo "--- [5] grep DB-backed findings for the patched files only ---"
for f in "${FILES[@]}"; do
    b=$(basename "$f")
    echo "== $b =="
    grep -E "/$b:|(^|[ /])$b:" "$OUT/xfn_findings.txt" 2>/dev/null | sed "s#$KERNEL/##g" | head -40
done

echo "--- [6] cleanup build objects (disk) ---"
make -s clean 2>/dev/null
rm -f smatch_db.sqlite "$SMATCH_DIR/bak.smatch"
echo "done. outputs in $OUT"
