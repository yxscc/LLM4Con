#!/bin/bash
# Usage: ./prepare_cve.sh <CVE-ID> <fix_commit> <source_files...>
# Example: ./prepare_cve.sh CVE-2016-9806 92964c79b357 net/netlink/af_netlink.c net/netlink/af_netlink.h

set -e

CVE_ID="$1"
FIX_COMMIT="$2"
shift 2
USER_SOURCE_FILES=("$@")

LLM4CON_HOME="${LLM4CON_HOME:-/home/LLM4Con}"
KERNEL_DIR="${LINUX_REPO:-${KERNEL_DIR:-/home/ConCord/targets/linux.git}}"
EXPERIMENT_DIR="${EXPERIMENT_BASE:-${LLM4CON_HOME}/kernel_experiment}/$CVE_ID"
CLANG=${CLANG:-clang}
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
# Phase F (M7): set EXPAND_PATCH=0 to disable patch-driven file expansion.
# Default ON because every CVE we audited benefits or is unchanged
# (single-file patches have no siblings to add).
EXPAND_PATCH=${EXPAND_PATCH:-1}
SOURCE_FILES=("${USER_SOURCE_FILES[@]}")

echo "========================================="
echo "Preparing $CVE_ID"
echo "Fix commit: $FIX_COMMIT"
echo "Source files: ${SOURCE_FILES[*]}"
echo "========================================="

mkdir -p "$EXPERIMENT_DIR/src"

# Step 0 (Phase F, M7): expand SOURCE_FILES via the fix commit so multi-
# file kernel patches (e.g. CVE-2024-43891 across 5 files) are not
# silently truncated. patch_expander.py reads patch content via
# `git show $commit:path` so it does NOT require checking out the
# commit's worktree first.
if [ "$EXPAND_PATCH" = "1" ] && [ -x "$SCRIPT_DIR/patch_expander.py" ]; then
    echo "[0/5] Expanding source-file list from patch ${FIX_COMMIT:0:12}..."
    mapfile -t EXPANDED_FILES < <(
        "$SCRIPT_DIR/patch_expander.py" \
            --kernel-dir "$KERNEL_DIR" \
            --commit "$FIX_COMMIT" \
            --seeds "${USER_SOURCE_FILES[@]}" \
            --output "$EXPERIMENT_DIR" 2>"$EXPERIMENT_DIR/expansion.log"
    )
    if [ ${#EXPANDED_FILES[@]} -gt 0 ]; then
        SOURCE_FILES=("${EXPANDED_FILES[@]}")
        echo "  Seeds: ${#USER_SOURCE_FILES[@]} -> Expanded: ${#SOURCE_FILES[@]} files"
        echo "  Report: $EXPERIMENT_DIR/expansion_report.json"
    else
        echo "  ! Expansion produced 0 files; falling back to user seeds (${#USER_SOURCE_FILES[@]})"
        cat "$EXPERIMENT_DIR/expansion.log" 2>/dev/null | head -3
    fi
fi

# Step 1: Checkout vulnerable version (parent of fix commit)
echo "[1/5] Checking out vulnerable version (${FIX_COMMIT}~1)..."
cd "$KERNEL_DIR"
rm -f .git/index.lock
git checkout "${FIX_COMMIT}~1" --quiet 2>/dev/null || git checkout "${FIX_COMMIT}^" --quiet
echo "  Kernel: $(git log --oneline -1)"

# Step 1.5: Generate headers for this version
# NOTE: use KCFLAGS (append) not KBUILD_CFLAGS (replace). KBUILD_CFLAGS clobbers
# the kernel's own default flags, including -fcf-protection=branch, which v6.x
# kernels rely on for IBT-related attributes (__nocf_check). We add -fno-PIE/-fno-pic
# on top to work around Debian 12 GCC 12's hardened defaults that conflict with
# -mcmodel=kernel.
# All make invocations are made non-fatal: very old kernels (Linux 3.x) often
# fail to build with modern GCC/binutils. We tolerate that and rely on the
# bounds.h / autoconf.h shims further down to keep clang's compile of the
# single .c file working anyway.
echo "[1.5/5] Running make modules_prepare..."
KPREP_KCFLAGS="-fno-PIE -fno-pic"
make defconfig CC=gcc HOSTCC=gcc > /dev/null 2>&1 || \
    echo "  ! make defconfig failed (old kernel + new toolchain); continuing with shims"
make modules_prepare CC=gcc HOSTCC=gcc KCFLAGS="$KPREP_KCFLAGS" -j"$(nproc)" > /dev/null 2>&1 || \
make prepare         CC=gcc HOSTCC=gcc KCFLAGS="$KPREP_KCFLAGS" -j"$(nproc)" > /dev/null 2>&1 || \
    echo "  ! make modules_prepare had warnings (may be ok)"

# Step 2: Collect source files and their headers
echo "[2/4] Collecting source files..."
for f in "${SOURCE_FILES[@]}"; do
    if [ -f "$KERNEL_DIR/$f" ]; then
        cp "$KERNEL_DIR/$f" "$EXPERIMENT_DIR/src/"
        echo "  + $f"
    else
        echo "  ! Not found: $f"
    fi
done

# Also collect related headers in the same directory
for f in "${SOURCE_FILES[@]}"; do
    dir=$(dirname "$f")
    for hdr in "$KERNEL_DIR/$dir"/*.h; do
        if [ -f "$hdr" ]; then
            base=$(basename "$hdr")
            if [ ! -f "$EXPERIMENT_DIR/src/$base" ]; then
                cp "$hdr" "$EXPERIMENT_DIR/src/"
                echo "  + $dir/$base (header)"
            fi
        fi
    done
done

# Step 2.9: shim for missing kernel-build-system generated headers.
# When `make prepare` fails (very old kernels, missing tools, etc.) we
# synthesize the minimum set so single-TU clang compilation can proceed.
GEN_DIR="$KERNEL_DIR/include/generated"
mkdir -p "$GEN_DIR"
if [ ! -f "$GEN_DIR/bounds.h" ]; then
    cat > "$GEN_DIR/bounds.h" <<'BOUNDS_EOF'
/* Auto-generated shim - prepare_cve.sh fallback */
#ifndef __LINUX_BOUNDS_H__
#define __LINUX_BOUNDS_H__
#define NR_PAGEFLAGS 22
#define MAX_NR_ZONES 4
#define SPINLOCK_SIZE 4
#endif
BOUNDS_EOF
    echo "  ! installed fallback include/generated/bounds.h"
fi
if [ ! -f "$GEN_DIR/autoconf.h" ]; then
    cat > "$GEN_DIR/autoconf.h" <<'AUTOCONF_EOF'
/* Auto-generated shim - prepare_cve.sh fallback */
#ifndef __LINUX_AUTOCONF_H__
#define __LINUX_AUTOCONF_H__
/* Minimum CONFIG_ defines so that headers compile.
   Real kbuild fills in hundreds; we keep just enough for the single-TU
   compile to succeed. */
#define CONFIG_64BIT 1
#define CONFIG_X86_64 1
#define CONFIG_SMP 1
#define CONFIG_PRINTK 1
#define CONFIG_BUG 1
#define CONFIG_BASE_SMALL 0
#define CONFIG_LOG_BUF_SHIFT 17
#endif
AUTOCONF_EOF
    echo "  ! installed fallback include/generated/autoconf.h"
fi
if [ ! -f "$GEN_DIR/utsrelease.h" ]; then
    echo '#define UTS_RELEASE "shim"' > "$GEN_DIR/utsrelease.h"
fi
if [ ! -f "$GEN_DIR/timeconst.h" ]; then
    echo '/* shim */' > "$GEN_DIR/timeconst.h"
fi

# Step 3: Compile each .c to LLVM bitcode
echo "[3/4] Compiling to LLVM bitcode..."
cd "$KERNEL_DIR"

# Neutralise early_initcall/module_init duplication. Some kernel .c files
# contain more than one `module_init(fn)` expansion because they also
# include a header that does the same — this is legal in a real kbuild
# (the linker only keeps one) but breaks our single-TU LLVM compile
# because each expansion defines `init_module`. We redefine the macros
# to emit a uniquely-named marker function instead.
NEUTER_INITCALL=(
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

# Build subsystem-specific extra include paths. Some kernel subsystems (most
# infamously drivers/gpu/drm/amd) cross-include headers across many sibling
# directories that aren't on the default kernel include path. We probe the
# source file path against a small dispatch table and append matching -I dirs.
#
# This is a build-system workaround driven purely by the file path layout in
# the kernel tree; it does NOT affect what code clang compiles, so it cannot
# bias detection results.
build_extra_includes() {
    local src="$1"
    local -a extra=()
    case "$src" in
        drivers/gpu/drm/amd/*)
            # AMD GPU driver does aggressive cross-tree quoted-include
            # ("foo/bar.h" relative to many sibling dirs). Listing every
            # known dir is whack-a-mole; just collect every directory
            # under drivers/gpu/drm/amd that contains at least one .h
            # and add it as an -I. Cheap (~250 dirs) and fully covers
            # any future kernel-version churn.
            while IFS= read -r d; do
                extra+=("-I$d")
            done < <(find "$KERNEL_DIR/drivers/gpu/drm/amd" -type d 2>/dev/null)
            # DRM/DRM-helper headers also live one level up.
            for sub in include drm; do
                local d="$KERNEL_DIR/include/$sub"
                [ -d "$d" ] && extra+=("-I$d")
            done
            # AMD headers branch on CONFIG_DEBUG_FS for type signatures
            # (debug_evictions etc.). Without it the public
            # kfd_priv.h `extern` clashes with the static placeholder
            # in amdgpu.h. Enabling it gives a single consistent type.
            extra+=("-DCONFIG_DEBUG_FS=1" "-DCONFIG_DRM_AMD_DC=1")
            ;;
    esac
    printf '%s\n' "${extra[@]}"
}

SMATCH=${SMATCH:-smatch}
GCC_INC=$(gcc -print-file-name=include)
for f in "${SOURCE_FILES[@]}"; do
    if [[ "$f" == *.c ]]; then
        base=$(basename "$f" .c)
        out="$EXPERIMENT_DIR/${base}.smatch.txt"
        echo "  Smatching $f -> ${base}.smatch.txt"

        EXTRA_INC=()
        while IFS= read -r _inc; do
            [ -n "$_inc" ] && EXTRA_INC+=("$_inc")
        done < <(build_extra_includes "$f")

        # Smatch (sparse frontend) single-TU run with kernel headers.
        # --project=kernel enables kernel-specific checks (incl. locking);
        # --succeed makes it exit 0 even on parse errors so the batch
        # continues. We deliberately do NOT depend on the kernel build
        # system, so old-kernel-vs-new-gcc breakage cannot mask findings.
        # NOTE: sparse/smatch does not support function-like -D macros, so we
        # deliberately omit NEUTER_INITCALL (only needed for llvm-link dedup).
        $SMATCH --project=kernel --succeed --full-path \
            -D__KERNEL__ -DMODULE -DKBUILD_MODNAME='"test"' \
            -DCONFIG_SMP -DCONFIG_64BIT \
            -nostdinc \
            -isystem "$GCC_INC" \
            -I"$KERNEL_DIR/include" \
            -I"$KERNEL_DIR/include/uapi" \
            -I"$KERNEL_DIR/arch/x86/include" \
            -I"$KERNEL_DIR/arch/x86/include/uapi" \
            -I"$KERNEL_DIR/arch/x86/include/generated" \
            -I"$KERNEL_DIR/arch/x86/include/generated/uapi" \
            "${EXTRA_INC[@]}" \
            -include "$KERNEL_DIR/include/linux/kconfig.h" \
            "$KERNEL_DIR/$f" > "$out" 2>"$EXPERIMENT_DIR/${base}_smatch_compile.log" || \
                echo "  ! smatch returned nonzero for $f (see ${base}_smatch_compile.log)"
        echo "  warn/error lines: $(grep -cE ' (warn|error|warning):' "$out" 2>/dev/null)"
    fi
done

# Step 4: (smatch variant) no bitcode linking needed.
echo "[4/4] Smatch single-TU mode: no linking."

echo ""
echo "Done! Experiment directory: $EXPERIMENT_DIR"
ls -la "$EXPERIMENT_DIR/"
echo ""
echo "Source files:"
ls "$EXPERIMENT_DIR/src/"
