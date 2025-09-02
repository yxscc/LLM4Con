#!/bin/bash

# 批量编译LinConVul数据集中的C文件到LLVM IR
# 使用clang编译为.ll文件

SOURCE_DIR="/home/ConCodeQL/LinConVul"
TARGET_DIR="/home/ConCodeQL/llvmbc/llvm15/LinConVul"

echo "开始编译LinConVul数据集..."
echo "源目录: $SOURCE_DIR"
echo "目标目录: $TARGET_DIR"

# 计数器
total_files=0
compiled_files=0
failed_files=0

# 查找所有.c文件并编译
find "$SOURCE_DIR" -name "*.c" | while read -r c_file; do
    total_files=$((total_files + 1))
    
    # 获取相对路径
    rel_path=${c_file#$SOURCE_DIR/}
    
    # 获取目录和文件名
    cve_dir=$(dirname "$rel_path")
    c_file_name=$(basename "$c_file")
    
    # 生成.ll文件名
    ll_file_name="${c_file_name%.c}.ll"
    
    # 创建目标目录
    target_cve_dir="$TARGET_DIR/$cve_dir"
    mkdir -p "$target_cve_dir"
    
    # 目标.ll文件路径
    ll_file="$target_cve_dir/$ll_file_name"
    
    echo "编译: $rel_path -> $cve_dir/$ll_file_name"
    
    # 使用clang编译
    if clang -S -c -fno-discard-value-names -emit-llvm -g -O0 -fno-inline "$c_file" -o "$ll_file" 2>/dev/null; then
        echo "  ✓ 成功"
        compiled_files=$((compiled_files + 1))
    else
        echo "  ✗ 失败"
        failed_files=$((failed_files + 1))
        # 记录失败的文件
        echo "$c_file" >> /tmp/failed_compiles.txt
    fi
done

echo ""
echo "编译完成!"
echo "总文件数: $total_files"
echo "成功编译: $compiled_files"
echo "编译失败: $failed_files"

if [ -f /tmp/failed_compiles.txt ]; then
    echo ""
    echo "编译失败的文件:"
    cat /tmp/failed_compiles.txt
    rm /tmp/failed_compiles.txt
fi
