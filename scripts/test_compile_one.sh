#!/bin/bash
set -o pipefail

CVE="${1:?Usage: $0 CVE-YYYY-NNNNN}"
KERNEL_DIR="/home/ConCord/targets/linux.git"
EXPERIMENT_BASE="/home/LLM4Con/kernel_experiment"
CLANG=${CLANG:-clang}
SURVEY_FILE="${SURVEY_FILE:-/tmp/cve_survey.csv}"

line=$(grep "^$CVE," "$SURVEY_FILE")
if [ -z "$line" ]; then
    echo "ERROR: $CVE not found in $SURVEY_FILE"
    exit 1
fi

IFS=, read -r cve has_patch files_str commit <<< "$line"
commit=$(echo "$commit" | tr -d '[:space:]')

echo "=== Testing compilation for $CVE ==="
echo "  commit: $commit"
echo "  files:  $files_str"
echo ""

IFS=';' read -ra SRC_FILES <<< "$files_str"
c_files=()
seen=()
for f in "${SRC_FILES[@]}"; do
    f=$(echo "$f" | xargs)
    [ -z "$f" ] && continue
    [[ "$f" != *.c ]] && continue
    dup=0; for s in "${seen[@]}"; do [ "$s" = "$f" ] && dup=1; done
    [ "$dup" = "1" ] && continue
    c_files+=("$f")
    seen+=("$f")
done

echo "  C files: ${c_files[*]}"
echo ""

echo "--- Step 1: checkout ${commit}~1 ---"
cd "$KERNEL_DIR" || exit 1
rm -f .git/index.lock

git checkout --force --quiet "${commit}~1" 2>&1 || \
git checkout --force --quiet "${commit}^" 2>&1 || \
{ echo "FAIL: checkout"; exit 1; }

echo "  HEAD now: $(git log --oneline -1 2>/dev/null)"

for f in "${c_files[@]}"; do
    if [ ! -f "$KERNEL_DIR/$f" ]; then
        echo "FAIL: $f does not exist at this commit"
        exit 1
    else
        echo "  exists: $f"
    fi
done

echo ""
echo "--- Step 2: make config + prepare ---"
git clean -fdxq -- include/generated/ include/config/ arch/x86/include/generated/ scripts/ 2>/dev/null

echo "  running make allyesconfig..."
if ! make allyesconfig CC=gcc HOSTCC=gcc >/dev/null 2>&1; then
    echo "  allyesconfig failed, trying defconfig..."
    make defconfig CC=gcc HOSTCC=gcc >/dev/null 2>&1
fi

echo "  running make modules_prepare..."
make modules_prepare CC=gcc HOSTCC=gcc -j$(nproc) >/dev/null 2>&1 || \
make prepare CC=gcc HOSTCC=gcc -j$(nproc) >/dev/null 2>&1 || true

echo "  checking generated headers:"
[ -f include/generated/autoconf.h ] && echo "    autoconf.h: YES" || echo "    autoconf.h: NO"
if [ -f arch/x86/include/generated/asm/asm-offsets.h ]; then
    echo "    asm-offsets.h: YES (arch/x86/include/generated/asm/)"
elif [ -f include/generated/asm-offsets.h ]; then
    echo "    asm-offsets.h: YES (include/generated/)"
else
    echo "    asm-offsets.h: NO — trying manual generation..."
    make arch/x86/kernel/asm-offsets.s CC=gcc HOSTCC=gcc 2>/dev/null && \
        echo "    asm-offsets.s generated" || echo "    asm-offsets.s FAILED"
fi

echo ""
echo "--- Step 3: compile to LLVM IR ---"
exp_dir="$EXPERIMENT_BASE/$CVE"
mkdir -p "$exp_dir/src"

sysinclude=$($CLANG -print-file-name=include 2>/dev/null)

incflags="-nostdinc -isystem $sysinclude"
incflags="$incflags -I$KERNEL_DIR/include"
incflags="$incflags -I$KERNEL_DIR/include/uapi"
incflags="$incflags -I$KERNEL_DIR/arch/x86/include"
incflags="$incflags -I$KERNEL_DIR/arch/x86/include/uapi"
incflags="$incflags -I$KERNEL_DIR/arch/x86/include/generated"
incflags="$incflags -I$KERNEL_DIR/arch/x86/include/generated/uapi"
[ -d "$KERNEL_DIR/include/generated" ] && incflags="$incflags -I$KERNEL_DIR/include/generated"
[ -d "$KERNEL_DIR/include/generated/uapi" ] && incflags="$incflags -I$KERNEL_DIR/include/generated/uapi"

forced=""
[ -f "$KERNEL_DIR/include/linux/compiler-version.h" ] && \
    forced="$forced -include $KERNEL_DIR/include/linux/compiler-version.h"
forced="$forced -include $KERNEL_DIR/include/generated/autoconf.h"
[ -f "$KERNEL_DIR/include/linux/kconfig.h" ] && \
    forced="$forced -include $KERNEL_DIR/include/linux/kconfig.h"
[ -f "$KERNEL_DIR/include/linux/compiler_types.h" ] && \
    forced="$forced -include $KERNEL_DIR/include/linux/compiler_types.h"

ok=0
fail=0
for f in "${c_files[@]}"; do
    base=$(basename "$f" .c)
    bcfile="$exp_dir/${base}.ll"
    logfile="$exp_dir/${base}_compile.log"
    src_dir=$(dirname "$f")
    extra_inc=""
    [ -d "$KERNEL_DIR/$src_dir" ] && extra_inc="-I$KERNEL_DIR/$src_dir"

    defs="-D__KERNEL__ -DMODULE -DKBUILD_MODNAME=\\\"${base}\\\" -DKBUILD_BASENAME=\\\"${base}\\\" -DCONFIG_SMP -DCONFIG_64BIT -DCONFIG_X86_64 -DCONFIG_SPARSEMEM -DCONFIG_X86_L1_CACHE_SHIFT=6 -DCONFIG_AS_CFI=1 -DCONFIG_AS_CFI_SIGNAL_FRAME=1 -DCONFIG_CC_HAS_ASM_GOTO_OUTPUT=1 -DCONFIG_FUNCTION_ALIGNMENT=16"

    echo ""
    echo "  Compiling: $f -> ${base}.ll"
    if eval $CLANG -S -emit-llvm -g -O0 -Wno-everything \
        -fno-pic -fno-PIE -target x86_64-linux-gnu \
        $defs $incflags $extra_inc $forced \
        "$KERNEL_DIR/$f" -o "$bcfile" 2>"$logfile"; then
        echo "    SUCCESS"
        ok=$((ok+1))
    else
        echo "    FAILED — last errors:"
        tail -10 "$logfile"
        fail=$((fail+1))
    fi
done

echo ""
echo "=== Result: $ok OK, $fail FAIL (out of ${#c_files[@]} files) ==="
