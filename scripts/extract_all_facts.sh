#!/bin/bash
# extract_all_facts.sh - 批量提取 LinConVul 数据集中所有 CVE 样本的事实

set -e

# 配置
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
LINCONVUL_DIR="${PROJECT_ROOT}/LinConVul"
BUILD_DIR="${PROJECT_ROOT}/Debug-build"
FACTS_EXTRACTOR="${BUILD_DIR}/facts_extractor"
OUTPUT_DIR="${PROJECT_ROOT}/extracted_facts"

# 颜色输出
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

log_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

log_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# 检查 facts_extractor 是否存在
check_extractor() {
    if [ ! -f "$FACTS_EXTRACTOR" ]; then
        log_error "facts_extractor not found at $FACTS_EXTRACTOR"
        log_info "Please build the project first:"
        log_info "  cd $PROJECT_ROOT && ./build.sh debug"
        exit 1
    fi
}

# 编译单个 CVE 样本的 bitcode
compile_bitcode() {
    local cve_dir="$1"
    local cve_id=$(basename "$cve_dir")
    local c_file="${cve_dir}/${cve_id}.c"
    local bc_file="${cve_dir}/${cve_id}.bc"
    local ll_file="${cve_dir}/${cve_id}.ll"
    
    if [ ! -f "$c_file" ]; then
        # 尝试查找其他 .c 文件
        c_file=$(find "$cve_dir" -maxdepth 1 -name "*.c" ! -name "*_clean.c" | head -1)
        if [ -z "$c_file" ]; then
            log_warn "No source file found in $cve_dir"
            return 1
        fi
    fi
    
    # 检查是否已存在 bitcode
    if [ -f "$ll_file" ] || [ -f "$bc_file" ]; then
        log_info "Bitcode already exists for $cve_id"
        return 0
    fi
    
    log_info "Compiling bitcode for $cve_id..."
    
    # 尝试编译
    if clang -g -O0 -S -emit-llvm -fno-discard-value-names "$c_file" -o "$ll_file" -pthread 2>/dev/null; then
        log_info "  -> Compiled successfully"
        return 0
    elif clang -g -O0 -S -emit-llvm -fno-discard-value-names "$c_file" -o "$ll_file" 2>/dev/null; then
        log_info "  -> Compiled successfully (without pthread)"
        return 0
    else
        log_warn "  -> Compilation failed for $cve_id"
        return 1
    fi
}

# 提取单个 CVE 样本的事实
extract_facts() {
    local cve_dir="$1"
    local cve_id=$(basename "$cve_dir")
    local c_file="${cve_dir}/${cve_id}.c"
    local ll_file="${cve_dir}/${cve_id}.ll"
    local bc_file="${cve_dir}/${cve_id}.bc"
    local output_file="${OUTPUT_DIR}/${cve_id}_facts.json"
    
    # 确定 bitcode 文件
    local input_bc=""
    if [ -f "$ll_file" ]; then
        input_bc="$ll_file"
    elif [ -f "$bc_file" ]; then
        input_bc="$bc_file"
    else
        log_warn "No bitcode found for $cve_id, skipping"
        return 1
    fi
    
    # 确定源文件
    if [ ! -f "$c_file" ]; then
        c_file=$(find "$cve_dir" -maxdepth 1 -name "*.c" ! -name "*_clean.c" | head -1)
    fi
    
    if [ -z "$c_file" ] || [ ! -f "$c_file" ]; then
        log_warn "No source file found for $cve_id, skipping"
        return 1
    fi
    
    log_info "Extracting facts for $cve_id..."
    
    # 运行提取器
    if "$FACTS_EXTRACTOR" \
        --input-src "$cve_dir" \
        --input-bc "$input_bc" \
        --output "$output_file" \
        --cve-id "$cve_id" 2>&1 | tee "${OUTPUT_DIR}/${cve_id}.log"; then
        
        if [ -f "$output_file" ]; then
            log_info "  -> Facts extracted to $output_file"
            return 0
        fi
    fi
    
    log_error "  -> Extraction failed for $cve_id"
    return 1
}

# 主函数
main() {
    log_info "=========================================="
    log_info "LinConVul Facts Extraction Pipeline"
    log_info "=========================================="
    
    # 检查必要条件
    check_extractor
    
    # 创建输出目录
    mkdir -p "$OUTPUT_DIR"
    
    # 统计
    local total=0
    local compiled=0
    local extracted=0
    local failed=0
    
    # 遍历所有 CVE 目录
    for cve_dir in "$LINCONVUL_DIR"/CVE-*/; do
        if [ -d "$cve_dir" ]; then
            total=$((total + 1))
            cve_id=$(basename "$cve_dir")
            
            log_info ""
            log_info "Processing [$total]: $cve_id"
            log_info "----------------------------------------"
            
            # Step 1: 编译 bitcode
            if compile_bitcode "$cve_dir"; then
                compiled=$((compiled + 1))
                
                # Step 2: 提取事实
                if extract_facts "$cve_dir"; then
                    extracted=$((extracted + 1))
                else
                    failed=$((failed + 1))
                fi
            else
                failed=$((failed + 1))
            fi
        fi
    done
    
    # 打印摘要
    log_info ""
    log_info "=========================================="
    log_info "Extraction Summary"
    log_info "=========================================="
    log_info "Total CVE samples: $total"
    log_info "Compiled successfully: $compiled"
    log_info "Facts extracted: $extracted"
    log_info "Failed: $failed"
    log_info ""
    log_info "Output directory: $OUTPUT_DIR"
    log_info "=========================================="
    
    # 合并所有事实到一个文件
    if [ $extracted -gt 0 ]; then
        log_info "Merging all facts into all_facts.json..."
        echo "[" > "${OUTPUT_DIR}/all_facts.json"
        first=true
        for f in "${OUTPUT_DIR}"/*_facts.json; do
            if [ -f "$f" ]; then
                if [ "$first" = true ]; then
                    first=false
                else
                    echo "," >> "${OUTPUT_DIR}/all_facts.json"
                fi
                cat "$f" >> "${OUTPUT_DIR}/all_facts.json"
            fi
        done
        echo "]" >> "${OUTPUT_DIR}/all_facts.json"
        log_info "All facts merged to ${OUTPUT_DIR}/all_facts.json"
    fi
}

# 单个 CVE 处理模式
if [ $# -eq 1 ]; then
    cve_dir="$1"
    if [ -d "$cve_dir" ]; then
        check_extractor
        mkdir -p "$OUTPUT_DIR"
        compile_bitcode "$cve_dir"
        extract_facts "$cve_dir"
    else
        log_error "Directory not found: $cve_dir"
        exit 1
    fi
else
    main
fi
