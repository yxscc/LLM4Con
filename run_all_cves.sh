#!/bin/bash

# --- 全局设置 ---
# 定义一个函数，用于在接收到中断信号时执行
function handle_interrupt {
    echo -e "\n\n脚本被用户中断。正在退出..."
    exit 1
}

# 设置 trap，当接收到 SIGINT (Ctrl+C) 信号时，调用 handle_interrupt 函数
trap handle_interrupt SIGINT

# --- 脚本主要逻辑 ---

# 定义基础目录
LINCONVUL_DIR="/home/ConCodeQL/LinConVul"
LLVMBC_DIR="/home/ConCodeQL/llvmbc/llvm15/LinConVul"
DETECTOR_PATH="/home/ConCodeQL/Debug-build/llm_detector"
OUTPUT_BASE_DIR="/home/ConCodeQL/ConCodeQL-output" # 定义总输出目录

# 固定的参数
LLM_PROVIDER="openai"
LLM_MODEL="gpt-5-2025-08-07"
LLM_KEY="sk-Y5PXhElM2NobgKPelwlHFaXPeQrSzm4WJYOnHYn0QafbVRoK"

# 定义重试参数
MAX_OVERALL_RUNS=3       # 最外层大循环的次数
MAX_COMMAND_RETRIES=3    # 单个命令的重试次数
RETRY_DELAY_SECONDS=10   # 单个命令重试的间隔

# 检查 LinConVul 目录是否存在
if [ ! -d "$LINCONVUL_DIR" ]; then
    echo "错误: 目录 $LINCONVUL_DIR 不存在。"
    exit 1
fi

# =================================================================
# ===== 最外层的大循环，确保所有任务最终都能完成 =====
# =================================================================
run=1
while [ $run -le $MAX_OVERALL_RUNS ]; do
    echo "##################################################"
    echo "###   开始第 $run / $MAX_OVERALL_RUNS 轮完整遍历   ###"
    echo "##################################################"
    all_completed=true # 假设本轮能全部完成

    # 遍历 LinConVul 目录下的所有 CVE 文件夹
    for cve_dir in ${LINCONVUL_DIR}/CVE-*; do
        if [ -d "$cve_dir" ]; then
            cve_name=$(basename "$cve_dir")
            
            # --- 修正后的智能检查：使用通配符查找输出目录 ---
            report_found=false
            # 查找所有匹配 'CVE-ID_*' 格式的目录
            for output_subdir in ${OUTPUT_BASE_DIR}/${cve_name}_*/; do
                # 检查目录是否存在并且报告文件是否存在
                if [ -d "$output_subdir" ] && [ -f "${output_subdir}/llm_evaluation_report.md" ]; then
                    report_found=true
                    break # 找到了就跳出内部循环
                fi
            done

            if [ "$report_found" = true ]; then
                echo "[$cve_name] 的最终报告已存在，跳过。"
                echo "----------------------------------------"
                continue
            fi
            # --- 智能检查结束 ---

            all_completed=false

            input_src="${cve_dir}/${cve_name}.c"
            input_bc="${LLVMBC_DIR}/${cve_name}/${cve_name}.ll"

            if [ ! -f "$input_src" ]; then
                echo "警告: 源文件 $input_src 未找到，跳过 $cve_name。"
                continue
            fi
            if [ ! -f "$input_bc" ]; then
                echo "警告: LLVM bitcode 文件 $input_bc 未找到，跳过 $cve_name。"
                continue
            fi

            echo "正在处理: $cve_name (第 $run 轮)"

            # --- 第二级：单个命令的执行和重试逻辑 ---
            attempt=1
            success=false
            while [ $attempt -le $MAX_COMMAND_RETRIES ]; do
                "$DETECTOR_PATH" \
                    --input-src "$input_src" \
                    --input-bc "$input_bc" \
                    --llm-provider "$LLM_PROVIDER" \
                    --llm-model "$LLM_MODEL" \
                    --llm-key "$LLM_KEY"
                
                exit_code=$?
                if [ $exit_code -eq 0 ]; then
                    echo "处理成功: $cve_name"
                    success=true
                    break
                else
                    echo "处理 $cve_name 失败 (尝试次数 $attempt/$MAX_COMMAND_RETRIES)，退出码: $exit_code"
                    if [ $attempt -lt $MAX_COMMAND_RETRIES ]; then
                        echo "将在 $RETRY_DELAY_SECONDS 秒后重试..."
                        sleep $RETRY_DELAY_SECONDS
                    else
                        echo "已达到最大重试次数，暂时放弃处理 $cve_name。"
                    fi
                fi
                ((attempt++))
            done
            
            echo "----------------------------------------"
        fi
    done

    if [ "$all_completed" = true ]; then
        echo "所有 CVE 的最终报告均已生成，提前结束。"
        break
    fi

    if [ $run -lt $MAX_OVERALL_RUNS ]; then
        echo "第 $run 轮遍历完成，稍作等待后将开始下一轮..."
        sleep 30
    fi

    ((run++))
done

echo "##################################################"
echo "###      所有自动化处理流程已执行完毕      ###"
echo "##################################################"