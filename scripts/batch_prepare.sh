#!/bin/bash
set -o pipefail

KERNEL_DIR="/home/ConCord/targets/linux.git"
EXPERIMENT_BASE="/home/LLM4Con/kernel_experiment"
REPORTS_DIR="/home/ConCord/concurrency_cve_reports/linux_kernel"
CLANG=${CLANG:-clang}
SURVEY_FILE="${SURVEY_FILE:-/tmp/cve_survey.csv}"
NJOBS=${NJOBS:-$(nproc)}
FORCE=${FORCE:-0}
LOGFILE="${EXPERIMENT_BASE}/batch_prepare.log"

SUCCESS=0
FAIL=0
SKIP=0
FAIL_LIST=""
LAST_PREPARED_COMMIT=""
TOTAL_PATCH=0
CURRENT=0

log() { echo "[$(date +%H:%M:%S)] $*" | tee -a "$LOGFILE"; }

checkout_and_prepare() {
    local commit="$1"

    if [ "$LAST_PREPARED_COMMIT" = "$commit" ]; then
        log "    reusing prepared tree for ${commit:0:12}"
        return 0
    fi

    cd "$KERNEL_DIR" || return 1
    rm -f .git/index.lock

    log "    checkout ${commit:0:12}~1 ..."
    if ! git checkout --force --quiet "${commit}~1" 2>/dev/null; then
        if ! git checkout --force --quiet "${commit}^" 2>/dev/null; then
            return 1
        fi
    fi

    log "    cleaning generated dirs..."
    git clean -fdxq -- include/generated/ include/config/ \
        arch/x86/include/generated/ scripts/ 2>/dev/null || true

    log "    make allyesconfig..."
    if ! make allyesconfig CC=gcc HOSTCC=gcc >/dev/null 2>&1; then
        log "    allyesconfig failed, trying defconfig..."
        make defconfig CC=gcc HOSTCC=gcc >/dev/null 2>&1 || true
    fi

    log "    make modules_prepare (j$NJOBS)..."
    make modules_prepare CC=gcc HOSTCC=gcc -j"$NJOBS" >/dev/null 2>&1 || \
    make prepare CC=gcc HOSTCC=gcc -j"$NJOBS" >/dev/null 2>&1 || true

    if [ ! -f include/generated/autoconf.h ]; then
        log "    autoconf.h missing!"
        return 1
    fi

    if [ ! -f arch/x86/include/generated/asm/asm-offsets.h ] && \
       [ ! -f include/generated/asm-offsets.h ]; then
        log "    asm-offsets.h missing, trying manual build..."
        make arch/x86/kernel/asm-offsets.s CC=gcc HOSTCC=gcc 2>/dev/null || true
    fi

    LAST_PREPARED_COMMIT="$commit"
    return 0
}

build_compile_cmd() {
    local src_file="$1" out_ll="$2"
    local base
    base=$(basename "$src_file" .c)
    local src_dir
    src_dir=$(dirname "$src_file")

    local sysinclude
    sysinclude=$($CLANG -print-file-name=include 2>/dev/null)

    local inc="-nostdinc -isystem $sysinclude"
    inc="$inc -I$KERNEL_DIR/include -I$KERNEL_DIR/include/uapi"
    inc="$inc -I$KERNEL_DIR/arch/x86/include -I$KERNEL_DIR/arch/x86/include/uapi"
    inc="$inc -I$KERNEL_DIR/arch/x86/include/generated -I$KERNEL_DIR/arch/x86/include/generated/uapi"
    [ -d "$KERNEL_DIR/include/generated" ] && inc="$inc -I$KERNEL_DIR/include/generated"
    [ -d "$KERNEL_DIR/include/generated/uapi" ] && inc="$inc -I$KERNEL_DIR/include/generated/uapi"
    [ -d "$KERNEL_DIR/$src_dir" ] && inc="$inc -I$KERNEL_DIR/$src_dir"

    # also add parent dirs for local includes (e.g. drivers/gpu/drm/amd/amdkfd -> amdgpu)
    local parent_dir=$(dirname "$src_dir")
    if [ "$parent_dir" != "$src_dir" ] && [ -d "$KERNEL_DIR/$parent_dir" ]; then
        for sub in "$KERNEL_DIR/$parent_dir"/*/; do
            [ -d "$sub" ] && inc="$inc -I$sub"
        done
    fi

    local forced=""
    [ -f "$KERNEL_DIR/include/linux/compiler-version.h" ] && \
        forced="$forced -include $KERNEL_DIR/include/linux/compiler-version.h"
    forced="$forced -include $KERNEL_DIR/include/generated/autoconf.h"
    [ -f "$KERNEL_DIR/include/linux/kconfig.h" ] && \
        forced="$forced -include $KERNEL_DIR/include/linux/kconfig.h"
    [ -f "$KERNEL_DIR/include/linux/compiler_types.h" ] && \
        forced="$forced -include $KERNEL_DIR/include/linux/compiler_types.h"

    local defs="-D__KERNEL__ -DMODULE"
    defs="$defs -DKBUILD_MODNAME=\\\"${base}\\\""
    defs="$defs -DKBUILD_BASENAME=\\\"${base}\\\""
    defs="$defs -DCONFIG_SMP -DCONFIG_64BIT -DCONFIG_X86_64"
    defs="$defs -DCONFIG_SPARSEMEM -DCONFIG_X86_L1_CACHE_SHIFT=6"
    defs="$defs -DCONFIG_AS_CFI=1 -DCONFIG_AS_CFI_SIGNAL_FRAME=1"
    defs="$defs -DCONFIG_CC_HAS_ASM_GOTO_OUTPUT=1 -DCONFIG_FUNCTION_ALIGNMENT=16"
    defs="$defs -DCC_USING_FENTRY"
    defs="$defs -DCONFIG_FTRACE_MCOUNT_USE_OBJTOOL=1"
    defs="$defs '-D__copy(x)='"
    defs="$defs '-D__compiletime_object_size(x)=-1'"
    defs="$defs '-D__compiletime_warning(x)='"
    defs="$defs '-D__compiletime_error(x)='"
    defs="$defs '-Dearly_param(name,fn)='"
    defs="$defs -mfentry"

    echo "$CLANG -S -emit-llvm -g -O0 -Wno-everything -fno-pic -fno-PIE \
        -target x86_64-linux-gnu $defs $inc $forced \
        $KERNEL_DIR/$src_file -o $out_ll"
}

do_compile() {
    local cve="$1" commit="$2" files_str="$3"
    local exp_dir="$EXPERIMENT_BASE/$cve"

    if [ "$FORCE" != "1" ] && ls "$exp_dir"/*.ll >/dev/null 2>&1; then
        SKIP=$((SKIP+1))
        return 0
    fi

    IFS=';' read -ra SRC_FILES <<< "$files_str"
    local c_files=() seen=()
    for f in "${SRC_FILES[@]}"; do
        f=$(echo "$f" | xargs)
        [ -z "$f" ] && continue
        [[ "$f" != *.c ]] && continue
        local dup=0; for s in "${seen[@]}"; do [ "$s" = "$f" ] && dup=1; done
        [ "$dup" = "1" ] && continue
        c_files+=("$f"); seen+=("$f")
    done
    if [ ${#c_files[@]} -eq 0 ]; then
        SKIP=$((SKIP+1))
        return 0
    fi

    CURRENT=$((CURRENT + 1))
    log "[$CURRENT/$TOTAL_PATCH] $cve — ${#c_files[@]} files"

    if ! checkout_and_prepare "$commit"; then
        log "  FAIL (checkout/prepare)"
        FAIL=$((FAIL + 1)); FAIL_LIST="$FAIL_LIST $cve(checkout)"
        return 1
    fi

    for f in "${c_files[@]}"; do
        if [ ! -f "$KERNEL_DIR/$f" ]; then
            log "  FAIL (missing $f)"
            FAIL=$((FAIL + 1)); FAIL_LIST="$FAIL_LIST $cve(missing)"
            return 1
        fi
    done

    mkdir -p "$exp_dir/src"
    for f in "${SRC_FILES[@]}"; do
        f=$(echo "$f" | xargs); [ -z "$f" ] && continue
        if [ -f "$KERNEL_DIR/$f" ]; then
            mkdir -p "$exp_dir/src/$(dirname "$f")"
            cp "$KERNEL_DIR/$f" "$exp_dir/src/$(dirname "$f")/" 2>/dev/null || true
        fi
    done
    for f in "${c_files[@]}"; do
        local dir=$(dirname "$f")
        mkdir -p "$exp_dir/src/$dir"
        for hdr in "$KERNEL_DIR/$dir"/*.h; do
            [ -f "$hdr" ] || continue
            cp -n "$hdr" "$exp_dir/src/$dir/" 2>/dev/null || true
        done
    done

    local bc_files=() fail_files=()
    for f in "${c_files[@]}"; do
        local base=$(basename "$f" .c)
        local bcfile="$exp_dir/${base}.ll"
        local logf="$exp_dir/${base}_compile.log"
        local cmd=$(build_compile_cmd "$f" "$bcfile")
        if eval $cmd 2>"$logf"; then
            bc_files+=("$bcfile")
            log "    ${base}.ll OK"
        else
            fail_files+=("$f")
            log "    ${base}.ll FAIL"
        fi
    done

    if [ ${#bc_files[@]} -gt 1 ]; then
        llvm-link -S "${bc_files[@]}" -o "$exp_dir/merged.ll" 2>/dev/null || true
    fi

    if [ ${#bc_files[@]} -eq 0 ]; then
        log "  RESULT: FAIL (0/${#c_files[@]} compiled)"
        FAIL=$((FAIL + 1)); FAIL_LIST="$FAIL_LIST $cve(compile)"
        return 1
    fi

    if [ ${#fail_files[@]} -gt 0 ]; then
        log "  RESULT: PARTIAL (${#bc_files[@]}/${#c_files[@]})"
    else
        log "  RESULT: OK (${#bc_files[@]} .ll)"
    fi
    SUCCESS=$((SUCCESS + 1))
}

collect_ground_truth() {
    local cve="$1" commit="$2" files_str="$3"
    local exp_dir="$EXPERIMENT_BASE/$cve"
    local gt_file="$exp_dir/ground_truth.json"
    [ -d "$exp_dir" ] || return 0
    ls "$exp_dir"/*.ll >/dev/null 2>&1 || return 0
    [ -f "$gt_file" ] && [ "$FORCE" != "1" ] && return 0

    python3 - "$cve" "$commit" "$files_str" "$REPORTS_DIR" "$gt_file" << 'PYEOF'
import json, os, sys
cve_id, commit, files_str, reports_dir, gt_file = sys.argv[1:6]
gt = {"cve_id": cve_id, "files": [f.strip() for f in files_str.split(";") if f.strip()], "fix_commit": commit}
metadata_file = os.path.join(reports_dir, cve_id, "metadata.json")
if os.path.exists(metadata_file):
    with open(metadata_file) as f:
        meta = json.load(f)
    for d in meta.get("cve", {}).get("descriptions", []):
        if d.get("lang") == "en":
            gt["description"] = d["value"]; break
    cwes = []
    for w in meta.get("cve", {}).get("weaknesses", []):
        for dd in w.get("description", []):
            if dd.get("value", "").startswith("CWE"): cwes.append(dd["value"])
    gt["cwes"] = cwes
for fname, key, maxlen in [("summary.md","summary",2000),("analysis.md","analysis",3000)]:
    fpath = os.path.join(reports_dir, cve_id, fname)
    if os.path.exists(fpath):
        with open(fpath) as f: gt[key] = f.read()[:maxlen]
patch_file = os.path.join(reports_dir, cve_id, "patch.diff")
if os.path.exists(patch_file):
    with open(patch_file) as f: gt["patch"] = f.read()[:5000]
pctx_file = os.path.join(reports_dir, cve_id, "patch_context.json")
if os.path.exists(pctx_file):
    with open(pctx_file) as f: pctx = json.load(f)
    af = []
    for sec in pctx.get("sections",[]):
        for fi in sec.get("files",[]): p=fi.get("path",""); p and af.append(p)
    if af: gt["affected_files_from_patch"] = list(dict.fromkeys(af))
with open(gt_file, "w") as f: json.dump(gt, f, indent=2, ensure_ascii=False)
PYEOF
}

# --- main ---
mkdir -p "$EXPERIMENT_BASE"
echo "" > "$LOGFILE"

TOTAL_PATCH=$(tail -n +2 "$SURVEY_FILE" | awk -F, '$2=="YES" && $4!=""' | wc -l)

log "============================================"
log " Batch CVE Preparation"
log " Survey: $SURVEY_FILE ($TOTAL_PATCH CVEs with patch)"
log " Kernel: $KERNEL_DIR"
log " Output: $EXPERIMENT_BASE"
log "============================================"

# 按年份降序处理（优先新的 CVE）
while IFS=, read -r cve has_patch files commit; do
    [ "$cve" = "CVE" ] && continue
    [ "$has_patch" != "YES" ] && continue
    [ -z "$commit" ] && continue
    commit=$(echo "$commit" | tr -d '[:space:]')
    do_compile "$cve" "$commit" "$files" || true
done < <(tail -n +2 "$SURVEY_FILE" | sort -t'-' -k2,2nr -k3,3nr | sed '1i CVE,HAS_PATCH,FILES,FIX_COMMIT' | tail -n +2)

log ""
log "--- Collecting Ground Truth ---"
while IFS=, read -r cve has_patch files commit; do
    [ "$cve" = "CVE" ] && continue
    [ "$has_patch" != "YES" ] && continue
    [ -z "$commit" ] && continue
    commit=$(echo "$commit" | tr -d '[:space:]')
    collect_ground_truth "$cve" "$commit" "$files"
done < "$SURVEY_FILE"

log ""
log "============================================"
log " DONE: $SUCCESS OK, $FAIL failed, $SKIP skipped"
[ -n "$FAIL_LIST" ] && log " Failed:$FAIL_LIST"
log "============================================"
