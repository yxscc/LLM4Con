#!/bin/bash
# Usage: ./prepare_cve.sh <CVE-ID> <fix_commit> <source_files...>
# Example: ./prepare_cve.sh CVE-2016-9806 92964c79b357 net/netlink/af_netlink.c net/netlink/af_netlink.h

set -e

CVE_ID="$1"
FIX_COMMIT="$2"
shift 2
SOURCE_FILES=("$@")

KERNEL_DIR="/home/ConCord/targets/linux.git"
EXPERIMENT_DIR="/home/LLM4Con/kernel_experiment/$CVE_ID"
CLANG=${CLANG:-clang}

echo "========================================="
echo "Preparing $CVE_ID"
echo "Fix commit: $FIX_COMMIT"
echo "Source files: ${SOURCE_FILES[*]}"
echo "========================================="

mkdir -p "$EXPERIMENT_DIR/src"

# Step 1: Checkout vulnerable version (parent of fix commit)
echo "[1/5] Checking out vulnerable version (${FIX_COMMIT}~1)..."
cd "$KERNEL_DIR"
rm -f .git/index.lock
git checkout "${FIX_COMMIT}~1" --quiet 2>/dev/null || git checkout "${FIX_COMMIT}^" --quiet
echo "  Kernel: $(git log --oneline -1)"

# Step 1.5: Generate headers for this version
echo "[1.5/5] Running make prepare..."
make defconfig CC=gcc KBUILD_CFLAGS="-fno-PIE -fno-pic" > /dev/null 2>&1
make prepare KBUILD_CFLAGS="-fno-PIE -fno-pic" > /dev/null 2>&1 || echo "  ! make prepare had warnings (may be ok)"

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

# Step 2.9: shim for missing generated/bounds.h on very old kernels.
# Pre-2.6.40 trees require these constants which are normally produced
# by kernel/bounds.c. When `make prepare` fails, synthesize a minimal
# header so downstream compilation can proceed.
GEN_DIR="$KERNEL_DIR/include/generated"
if [ ! -f "$GEN_DIR/bounds.h" ]; then
    mkdir -p "$GEN_DIR"
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

BC_FILES=()
for f in "${SOURCE_FILES[@]}"; do
    if [[ "$f" == *.c ]]; then
        base=$(basename "$f" .c)
        bcfile="$EXPERIMENT_DIR/${base}.ll"
        echo "  Compiling $f -> ${base}.ll"
        $CLANG -S -emit-llvm -g -O0 \
            -Wno-everything \
            -D__KERNEL__ -DMODULE -DKBUILD_MODNAME='"test"' \
            -DCONFIG_SMP -DCONFIG_64BIT \
            "${NEUTER_INITCALL[@]}" \
            -nostdinc \
            -isystem $($CLANG -print-file-name=include) \
            -I"$KERNEL_DIR/include" \
            -I"$KERNEL_DIR/include/uapi" \
            -I"$KERNEL_DIR/arch/x86/include" \
            -I"$KERNEL_DIR/arch/x86/include/uapi" \
            -I"$KERNEL_DIR/arch/x86/include/generated" \
            -I"$KERNEL_DIR/arch/x86/include/generated/uapi" \
            -include "$KERNEL_DIR/include/linux/kconfig.h" \
            "$KERNEL_DIR/$f" -o "$bcfile" 2>"$EXPERIMENT_DIR/${base}_compile.log" || {
                echo "  ! Compile failed for $f (see ${base}_compile.log)"
                continue
            }
        BC_FILES+=("$bcfile")
        echo "  OK: $(wc -l < "$bcfile") lines"
    fi
done

# Step 4: Link bitcode files if multiple
if [ ${#BC_FILES[@]} -gt 1 ]; then
    echo "[4/4] Linking ${#BC_FILES[@]} bitcode files..."

    # Rename @init_module / @cleanup_module aliases in each .ll so they
    # don't collide at llvm-link time. Kernel's <linux/module.h> emits
    # these as module-local `int init_module(void)` / `void cleanup_module(void)`
    # aliases; our -D neutering is overridden by the header's own #define,
    # so we post-process the .ll textually instead.
    for bc in "${BC_FILES[@]}"; do
        stem=$(basename "$bc" .ll | tr -c 'A-Za-z0-9_' _)
        # Replace @init_module (as symbol) with @init_module_<stem> uniquely.
        # We match the symbol followed by a non-identifier char to avoid
        # renaming inside larger symbol names (__UNIQUE_ID___addressable_init_module...).
        sed -i -E \
            -e "s/@init_module([^A-Za-z0-9_])/@init_module_${stem}\\1/g" \
            -e "s/@cleanup_module([^A-Za-z0-9_])/@cleanup_module_${stem}\\1/g" \
            "$bc"
    done

    llvm-link -S "${BC_FILES[@]}" -o "$EXPERIMENT_DIR/merged.ll" 2>"$EXPERIMENT_DIR/llvm-link.log" || {
        echo "  ! llvm-link failed, see llvm-link.log"
    }
    if [ -f "$EXPERIMENT_DIR/merged.ll" ]; then
        echo "  OK: merged.ll ($(wc -l < "$EXPERIMENT_DIR/merged.ll") lines)"
    fi
elif [ ${#BC_FILES[@]} -eq 1 ]; then
    echo "[4/4] Single file, no linking needed."
fi

echo ""
echo "Done! Experiment directory: $EXPERIMENT_DIR"
ls -la "$EXPERIMENT_DIR/"
echo ""
echo "Source files:"
ls "$EXPERIMENT_DIR/src/"
