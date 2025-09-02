#!/bin/bash

# --- 配置 ---
# 包含所有漏洞CVE文件夹的基础目录
VULN_BASE_DIR="/home/ConCodeQL/LinConVul"
# 存放分析结果的目录
RESULT_DIR="/home/ConCodeQL/experimental_result/RacerF"

# --- 脚本主体 ---

# 检查基础目录是否存在
if [ ! -d "$VULN_BASE_DIR" ]; then
  echo "错误：漏洞目录 '$VULN_BASE_DIR' 不存在。"
  exit 1
fi

# 如果结果目录不存在，则创建它
mkdir -p "$RESULT_DIR"
echo "结果将保存在: $RESULT_DIR"

# 查找VULN_BASE_DIR下的所有子目录
# 使用 find 命令可以更灵活地处理各种目录结构
find "$VULN_BASE_DIR" -mindepth 1 -maxdepth 1 -type d | while read -r vuln_path; do
  # 从完整路径中提取漏洞名称 (例如: CVE-2024-43891)
  vuln_name=$(basename "$vuln_path")

  # 构建C源文件的完整路径
  c_file="${vuln_path}/${vuln_name}.c"
  
  # 构建输出结果文件的完整路径
  output_file="${RESULT_DIR}/${vuln_name}_frama-c_result.txt"

  # 检查C源文件是否存在
  if [ -f "$c_file" ]; then
    echo "----------------------------------------------------"
    echo "正在处理: $vuln_name"
    echo "源文件:   $c_file"
    echo "输出到:   $output_file"
    
    # 执行 frama-c 命令，并将所有输出 (stdout 和 stderr) 重定向到结果文件
    frama-c -racer "$c_file" &> "$output_file"
    
    echo "完成: $vuln_name"
  else
    echo "----------------------------------------------------"
    echo "警告：在目录 '$vuln_path' 中未找到源文件 '$c_file'，已跳过。"
  fi
done

echo "----------------------------------------------------"
echo "所有漏洞处理完毕！"