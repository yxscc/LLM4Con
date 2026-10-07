#!/bin/bash
# Prepare a Lace case directory for UNKNOWN-defect hunting on a mainline kernel.
#
# Unlike scripts/prepare_cve.sh, there is no fix commit and no ground truth: the
# translation unit is chosen by subsystem, not by what a patch touched. The
# kernel is read from a dedicated git worktree so the shared LINUX_REPO checkout
# is never disturbed.
#
# Usage:
#   prepare_module.sh <target-name> <kernel-ref> <path>...
#
# Each <path> is either a .c file or a directory (all *.c directly inside it are
# taken, non-recursively). Paths are relative to the kernel tree.
#
# Example:
#   prepare_module.sh net-sched v7.3-rc1 net/sched
#   prepare_module.sh vsock v7.3-rc1 net/vmw_vsock/af_vsock.c net/vmw_vsock/virtio_transport.c
#
# Output: $HUNT_BASE/<target-name>/{src/,*.ll,merged.ll,target.json}
set -o pipefail

TARGET="$1"
KREF="$2"
shift 2
SEEDS=("$@")

if [ -z "$TARGET" ] || [ -z "$KREF" ] || [ ${#SEEDS[@]} -eq 0 ]; then
    echo "usage: $(basename "$0") <target-name> <kernel-ref> <path>..." >&2
    exit 2
fi

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LLM4CON_HOME="${LLM4CON_HOME:-$(dirname "$HERE")}"
LINUX_REPO="${LINUX_REPO:?set LINUX_REPO (source setup_env.sh)}"
HUNT_BASE="${HUNT_BASE:-$HERE/targets}"
WORKTREE_BASE="${WORKTREE_BASE:-$(dirname "$LINUX_REPO")/kernel_worktrees}"
# The detector is linked against LLVM 16, so it can only parse IR that LLVM 16
# understands. Compiling with a newer clang produces .ll that fails to load.
CLANG="${HUNT_CLANG:-clang-16}"
JOBS="${JOBS:-$(nproc)}"

KTREE="$WORKTREE_BASE/$KREF"
OUT="$HUNT_BASE/$TARGET"

echo "========================================="
echo "target : $TARGET"
echo "kernel : $KREF  ($KTREE)"
echo "clang  : $CLANG"
echo "seeds  : ${SEEDS[*]}"
echo "========================================="

# ---------------------------------------------------------------- worktree
if [ ! -d "$KTREE" ]; then
    echo "[1/5] Creating worktree for $KREF ..."
    mkdir -p "$WORKTREE_BASE"
    git -C "$LINUX_REPO" worktree add --detach "$KTREE" "$KREF" >/dev/null || {
        echo "  ! worktree add failed for ref '$KREF'" >&2; exit 1; }
else
    echo "[1/5] Reusing worktree $KTREE"
fi
echo "  kernel: $(git -C "$KTREE" log --oneline -1)"

# ------------------------------------------------------- generated headers
# A single-TU clang compile still needs kbuild's generated headers
# (autoconf.h, bounds.h, asm-offsets, ...). Done once per worktree+config.
#
# The config choice is not cosmetic: many subsystem fields are compiled out
# unless their CONFIG_ is set, e.g. `struct net` only has a `vsock` member
# under CONFIG_VSOCKETS. defconfig therefore fails to build most modules
# outside the default set, so allmodconfig is the default here.
KCONFIG="${KCONFIG:-allmodconfig}"
STAMP="$KTREE/.lace_prepared_$KCONFIG"
if [ ! -f "$STAMP" ]; then
    echo "[2/5] Running make $KCONFIG + modules_prepare (once per worktree+config)..."
    (
        cd "$KTREE" || exit 1
        make "$KCONFIG" CC=gcc HOSTCC=gcc >/dev/null 2>&1 || \
            echo "  ! make $KCONFIG failed; continuing"
        make modules_prepare CC=gcc HOSTCC=gcc KCFLAGS="-fno-PIE -fno-pic" \
            -j"$JOBS" >/dev/null 2>&1 || \
            echo "  ! modules_prepare had warnings (often ok)"
    )
    rm -f "$KTREE"/.lace_prepared_*
    touch "$STAMP"
else
    echo "[2/5] Headers already generated for this worktree ($KCONFIG)"
fi

# ------------------------------------------------------------ expand seeds
echo "[3/5] Expanding seed paths..."
SRC_FILES=()
for s in "${SEEDS[@]}"; do
    if [ -d "$KTREE/$s" ]; then
        while IFS= read -r c; do
            SRC_FILES+=("${c#"$KTREE/"}")
        done < <(find "$KTREE/$s" -maxdepth 1 -name '*.c' | sort)
    elif [ -f "$KTREE/$s" ]; then
        SRC_FILES+=("$s")
    else
        echo "  ! not found in kernel tree: $s" >&2
    fi
done
if [ ${#SRC_FILES[@]} -eq 0 ]; then
    echo "  ! no source files resolved" >&2; exit 1
fi
echo "  ${#SRC_FILES[@]} translation unit(s)"

mkdir -p "$OUT/src"

# ------------------------------------------------- collect sources+headers
# The detector's Joern pass reads --input-src, so the .c files and the headers
# they sit next to must be present. Directory layout is preserved because Joern
# resolves quoted includes relative to the file.
echo "[4/5] Collecting sources into $OUT/src ..."
for f in "${SRC_FILES[@]}"; do
    d=$(dirname "$f")
    mkdir -p "$OUT/src/$d"
    cp "$KTREE/$f" "$OUT/src/$d/"
done
for f in "${SRC_FILES[@]}"; do
    d=$(dirname "$f")
    for hdr in "$KTREE/$d"/*.h; do
        [ -f "$hdr" ] || continue
        [ -f "$OUT/src/$d/$(basename "$hdr")" ] || cp "$hdr" "$OUT/src/$d/"
    done
done
echo "  $(find "$OUT/src" -name '*.c' | wc -l) .c + $(find "$OUT/src" -name '*.h' | wc -l) .h"

# --------------------------------------------------------------- compile
# Each file is compiled as a module first and, if that fails, as built-in.
# Neither mode works for everything:
#   -DMODULE      needed by tristate files that reference THIS_MODULE, module
#                 parameters or MODULE_* macros -- most of net/ and fs/.
#   no -DMODULE   needed where <linux/init.h> collapses every *_initcall onto
#                 module_init, so a file with two of them (net/sched/sch_api.c
#                 has late_initcall + subsys_initcall) defines init_module and
#                 __inittest twice; and where __initcall is used directly, as
#                 in io_uring/io_uring.c, since that macro only exists built-in.
#
# The -D neutering below only bites in files that never include
# <linux/module.h>; where the header is included its own #define wins. That is
# why the module/built-in retry, not this list, is what resolves collisions.
NEUTER=(
  '-Dmodule_init(fn)=static int __mi_##fn(void) __attribute__((used,unused));static int __mi_##fn(void){return (fn)();}'
  '-Dearly_initcall(fn)=static int __ei_##fn(void) __attribute__((used,unused));static int __ei_##fn(void){return (fn)();}'
  '-Dcore_initcall(fn)=static int __ci_##fn(void) __attribute__((used,unused));static int __ci_##fn(void){return (fn)();}'
  '-Dpostcore_initcall(fn)=static int __pci_##fn(void) __attribute__((used,unused));static int __pci_##fn(void){return (fn)();}'
  '-Darch_initcall(fn)=static int __ai_##fn(void) __attribute__((used,unused));static int __ai_##fn(void){return (fn)();}'
  '-Dsubsys_initcall(fn)=static int __si_##fn(void) __attribute__((used,unused));static int __si_##fn(void){return (fn)();}'
  '-Dfs_initcall(fn)=static int __fi_##fn(void) __attribute__((used,unused));static int __fi_##fn(void){return (fn)();}'
  '-Ddevice_initcall(fn)=static int __di_##fn(void) __attribute__((used,unused));static int __di_##fn(void){return (fn)();}'
  '-Dlate_initcall(fn)=static int __li_##fn(void) __attribute__((used,unused));static int __li_##fn(void){return (fn)();}'
  '-Dmodule_exit(fn)=static void __mx_##fn(void) __attribute__((used,unused));static void __mx_##fn(void){(fn)();}'
)

# Flags beyond prepare_cve.sh's set, each needed for a mainline v7.x tree:
#   -std=gnu11        matches the kernel's own CC_FLAGS_DIALECT
#   -fms-extensions   v7.x embeds tagged anonymous struct members (e.g.
#                     `struct ns_tree;` inside struct ns_common); without it
#                     clang reports the injected fields as missing. GCC accepts
#                     these unconditionally, which is why kbuild passes nothing.
#   -DCC_USING_FENTRY allmodconfig turns on the function tracer, and
#                     arch/x86/include/asm/ftrace.h #errors out without it.
compile_one() {   # compile_one <file> <out.ll> <logfile> [extra defines...]
    local f="$1" ll="$2" log="$3"; shift 3
    $CLANG -S -emit-llvm -g -O0 -Wno-everything \
        -std=gnu11 -fms-extensions \
        -mfentry -DCC_USING_FENTRY \
        -D__KERNEL__ -DKBUILD_MODNAME='"lace"' \
        -DCONFIG_SMP -DCONFIG_64BIT \
        "$@" \
        "${NEUTER[@]}" \
        -nostdinc \
        -isystem "$($CLANG -print-file-name=include)" \
        -I"$KTREE/include" \
        -I"$KTREE/include/uapi" \
        -I"$KTREE/include/generated" \
        -I"$KTREE/include/generated/uapi" \
        -I"$KTREE/arch/x86/include" \
        -I"$KTREE/arch/x86/include/uapi" \
        -I"$KTREE/arch/x86/include/generated" \
        -I"$KTREE/arch/x86/include/generated/uapi" \
        -I"$KTREE/$(dirname "$f")" \
        -include "$KTREE/include/linux/kconfig.h" \
        "$KTREE/$f" -o "$ll" 2>"$log"
}

echo "[5/5] Compiling to LLVM IR with $CLANG ..."
BC_FILES=()
FAILED=()
NBUILTIN=0
for f in "${SRC_FILES[@]}"; do
    base=$(basename "$f" .c)
    ll="$OUT/${base}.ll"
    log="$OUT/${base}_compile.log"
    if compile_one "$f" "$ll" "$log" -DMODULE; then
        BC_FILES+=("$ll")
        printf '  OK   %-44s %s lines\n' "$f" "$(wc -l < "$ll")"
    elif compile_one "$f" "$ll" "$log"; then
        BC_FILES+=("$ll")
        NBUILTIN=$((NBUILTIN + 1))
        printf '  OK*  %-44s %s lines (built-in mode)\n' "$f" "$(wc -l < "$ll")"
    else
        FAILED+=("$f")
        printf '  FAIL %-44s %s errors\n' "$f" "$(grep -c 'error:' "$log")"
        rm -f "$ll"
    fi
done

echo "  compiled ${#BC_FILES[@]}/${#SRC_FILES[@]} (${NBUILTIN} needed built-in mode)"

# ------------------------------------------------------------------ link
if [ ${#BC_FILES[@]} -gt 1 ]; then
    echo "  Linking ${#BC_FILES[@]} IR files..."
    for bc in "${BC_FILES[@]}"; do
        stem=$(basename "$bc" .ll | tr -c 'A-Za-z0-9_' _)
        sed -i -E \
            -e "s/@init_module([^A-Za-z0-9_])/@init_module_${stem}\\1/g" \
            -e "s/@cleanup_module([^A-Za-z0-9_])/@cleanup_module_${stem}\\1/g" \
            "$bc"
    done
    "${LLVM_LINK:-llvm-link-16}" -S "${BC_FILES[@]}" -o "$OUT/merged.ll" \
        2>"$OUT/llvm-link.log" \
        && echo "  OK: merged.ll ($(wc -l < "$OUT/merged.ll") lines)" \
        || echo "  ! llvm-link failed, see $OUT/llvm-link.log"
fi

# -------------------------------------------------------------- metadata
python3 - "$OUT" "$TARGET" "$KREF" "$CLANG" "${SRC_FILES[@]}" <<'PY'
import json, os, subprocess, sys
out, target, kref, clang = sys.argv[1:5]
files = sys.argv[5:]
meta = {
    "target": target,
    "kernel_ref": kref,
    "clang": clang,
    "translation_units": files,
    "compiled": sorted(f[:-3] for f in os.listdir(out) if f.endswith(".ll")
                       and f != "merged.ll"),
    "has_merged": os.path.isfile(os.path.join(out, "merged.ll")),
    "note": "unknown-defect hunt target; no ground truth, entries auto-discovered",
}
with open(os.path.join(out, "target.json"), "w") as f:
    json.dump(meta, f, indent=2)
PY

echo ""
echo "Done: $OUT"
if [ ${#FAILED[@]} -gt 0 ]; then
    echo "Failed TUs (${#FAILED[@]}): ${FAILED[*]}"
fi
