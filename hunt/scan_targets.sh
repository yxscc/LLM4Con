#!/bin/bash
# Compile each candidate target and measure its static surface, spending no
# LLM budget. This is the step that turns a paper survey into a real cost
# ranking: the survey guesses from source text, this measures what the
# detector actually derives.
#
# The detector still needs a key argument to start, but with a dummy key the
# only LLM call on this path (Phase 0 thread-API discovery) fails and is
# skipped, and LACE_EARLY_EXIT_AFTER_SURFACE stops the run before Phase A.
#
# Usage:
#   scan_targets.sh <targets-file> [kernel-ref]
#
# targets-file lines:  <target-name> <path>[ <path>...]
# '#' comments and blank lines are ignored.
set -o pipefail

TARGETS_FILE="${1:?usage: scan_targets.sh <targets-file> [kernel-ref]}"
KREF="${2:-v7.2}"

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LLM4CON_HOME="${LLM4CON_HOME:-$(dirname "$HERE")}"
DET="${DETECTOR:-$LLM4CON_HOME/Release-build/llm_detector}"
DUMPS="${HUNT_DUMPS:-$HERE/dumps}"
CSV="${SCAN_CSV:-$HERE/scan_${KREF}.csv}"
STATIC_TIMEOUT="${STATIC_TIMEOUT:-2400}"
export LLVM_LINK="${LLVM_LINK:-llvm-link-16}"

echo "target,tus_ok,tus_total,ll_kloc,threads,pairs,objects,high_risk,surface_s,status" > "$CSV"

while read -r name paths; do
    [ -z "$name" ] && continue
    case "$name" in \#*) continue ;; esac

    echo ""
    echo "################ $name ################"
    prep_log="/tmp/hunt_prep_${name}.log"
    # shellcheck disable=SC2086
    "$HERE/prepare_module.sh" "$name" "$KREF" $paths > "$prep_log" 2>&1
    ok=$(grep -cE '^  OK\*? ' "$prep_log")
    total=$(grep -oE '^  [0-9]+ translation unit' "$prep_log" | grep -oE '[0-9]+' | head -1)
    total=${total:-0}
    echo "  compiled $ok/$total  (log: $prep_log)"

    dir="$HERE/targets/$name"
    bc=""
    [ -f "$dir/merged.ll" ] && bc="merged.ll"
    if [ -z "$bc" ]; then
        # single-TU target, or link failed: fall back to the largest .ll
        bc=$(ls -S "$dir"/*.ll 2>/dev/null | head -1 | xargs -r basename)
    fi
    if [ -z "$bc" ] || [ "$ok" -eq 0 ]; then
        echo "$name,$ok,$total,,,,,,,COMPILE_FAIL" >> "$CSV"
        echo "  -> COMPILE_FAIL"
        continue
    fi
    kloc=$(( $(wc -l < "$dir/$bc") / 1000 ))

    slog="/tmp/hunt_static_${name}.log"
    t0=$(date +%s)
    ( cd "$dir" && LACE_EARLY_EXIT_AFTER_SURFACE=1 LACE_DUMP_ROOT="$DUMPS" \
        timeout "$STATIC_TIMEOUT" "$DET" \
        --input-bc "$bc" --input-src src \
        --legacy-workflow --abl-contract on \
        --llm-provider openai --llm-url http://localhost \
        --llm-key dummy --llm-model dummy ) > "$slog" 2>&1
    rc=$?
    t1=$(date +%s)
    secs=$((t1 - t0))

    line=$(grep -oE "Threads: [0-9]+, Conflicting pairs: [0-9]+, Shared objects: [0-9]+" "$slog" | tail -1)
    threads=$(echo "$line" | grep -oE 'Threads: [0-9]+' | grep -oE '[0-9]+')
    pairs=$(echo "$line" | grep -oE 'Conflicting pairs: [0-9]+' | grep -oE '[0-9]+')
    objs=$(echo "$line" | grep -oE 'Shared objects: [0-9]+' | grep -oE '[0-9]+')
    high=$(grep -oE '\(([0-9]+) high risk' "$slog" | grep -oE '[0-9]+' | tail -1)

    if [ $rc -eq 124 ]; then
        status=STATIC_TIMEOUT
    elif [ -z "$objs" ]; then
        status=NO_SURFACE
    else
        status=OK
    fi
    echo "$name,$ok,$total,$kloc,$threads,$pairs,$objs,$high,$secs,$status" >> "$CSV"
    echo "  -> $status  threads=${threads:-?} pairs=${pairs:-?} objects=${objs:-?} high=${high:-?} ${secs}s"
done < "$TARGETS_FILE"

echo ""
echo "======================================================================"
column -s, -t < "$CSV"
echo ""
echo "csv: $CSV"
