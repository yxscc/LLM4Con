#!/bin/bash
CLANG="/usr/local/llvm-15/bin/clang"
EXTRACTOR="./Debug-build/facts_extractor"
LINCONVUL="./LinConVul"
RESULTS=()
FAILED=()

for cve_dir in $LINCONVUL/CVE-*; do
    cve=$(basename $cve_dir)
    c_file=$(ls $cve_dir/*.c 2>/dev/null | head -1)
    
    if [ -z "$c_file" ]; then
        echo "[$cve] No .c file found, skipping"
        continue
    fi
    
    ll_file="${c_file%.c}.ll"
    output_file="$cve_dir/facts.json"
    
    echo "========================================"
    echo "Processing: $cve"
    echo "========================================"
    
    # Compile to LLVM IR if needed
    if [ ! -f "$ll_file" ] || [ "$c_file" -nt "$ll_file" ]; then
        echo "  Compiling $c_file -> $ll_file"
        $CLANG -S -emit-llvm -g -O0 -pthread "$c_file" -o "$ll_file" 2>/dev/null
        if [ $? -ne 0 ]; then
            echo "  [FAILED] Compilation failed"
            FAILED+=("$cve:compile")
            continue
        fi
    fi
    
    # Run extractor
    echo "  Extracting facts..."
    timeout 120 $EXTRACTOR \
        --input-src "$c_file" \
        --input-bc "$ll_file" \
        --output "$output_file" \
        --cve-id "$cve" 2>&1 | grep -E "(Shared Variables|Memory Accesses|Error|Complete)"
    
    if [ $? -eq 0 ] && [ -f "$output_file" ]; then
        shared=$(cat "$output_file" | python3 -c "import json,sys; d=json.load(sys.stdin); print(len(d['verifiable_facts']['shared_variables']))" 2>/dev/null)
        echo "  [SUCCESS] shared_vars=$shared"
        RESULTS+=("$cve:$shared")
    else
        echo "  [FAILED] Extraction failed"
        FAILED+=("$cve:extract")
    fi
    echo ""
done

echo "========================================"
echo "SUMMARY"
echo "========================================"
echo "Successful: ${#RESULTS[@]}"
for r in "${RESULTS[@]}"; do echo "  $r"; done
echo ""
echo "Failed: ${#FAILED[@]}"
for f in "${FAILED[@]}"; do echo "  $f"; done
