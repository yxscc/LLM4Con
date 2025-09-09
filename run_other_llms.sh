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
# 使用新的可执行文件
DETECTOR_PATH="/home/ConCodeQL/Debug-build/llm_comparison"

# C++程序实际的输出根目录 (根据 TargetPath.h 推断)
PROGRAM_ACTUAL_OUTPUT_DIR="/home/ConCodeQL/LLM_dump"
# 我们希望统一存放所有对比试验结果的总目录
FINAL_OUTPUT_BASE_DIR="/home/ConCodeQL/ConCodeQL-comparison-output"

# =================================================================
# ===== 为本次运行创建一个唯一的总输出目录 =====
# =================================================================
TIMESTAMP=$(date +"%Y-%m-%d_%H-%M-%S")
RUN_OUTPUT_DIR="${FINAL_OUTPUT_BASE_DIR}/run_${TIMESTAMP}"
mkdir -p "$RUN_OUTPUT_DIR"
echo "所有本次运行的新结果将保存在: $RUN_OUTPUT_DIR"
echo "================================================================="


# 定义要测试的模型列表
MODELS_TO_RUN=("gemini-2.5-pro" "deepseek-v3.1")

# --- 【修改 1】: 使用关联数组为不同模型定义独立的 API Key ---
declare -A LLM_KEYS
LLM_KEYS["gemini-2.5-pro"]="sk-Y5PXhElM2NobgKPelwlHFaXPeQrSzm4WJYOnHYn0QafbVRoK"
LLM_KEYS["deepseek-v3.1"]="sk-AYyKhHyEzpzYEDdIQgQxQoWi7Fpg7B98b9YrP8IhzD6JNvzw"

# 固定的参数
LLM_PROVIDER="openai"

# 定义重试参数
MAX_OVERALL_RUNS=3      # 最外层大循环的次数
MAX_COMMAND_RETRIES=3   # 单个命令的重试次数
RETRY_DELAY_SECONDS=10  # 单个命令重试的间隔

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
    
    tasks_to_run_this_round=false # 标记本轮是否有任务需要执行

    # 遍历 LinConVul 目录下的所有 CVE 文件夹
    for cve_dir in ${LINCONVUL_DIR}/CVE-*; do
        if [ -d "$cve_dir" ]; then
            cve_name=$(basename "$cve_dir")
            
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

            # --- 模型循环 ---
            for LLM_MODEL in "${MODELS_TO_RUN[@]}"; do
                
                # --- 【修改 2】: 修复跳过逻辑 ---
                # 使用 find 检查在 FINAL_OUTPUT_BASE_DIR 下的任何子目录中是否已存在结果
                # -path "*/${LLM_MODEL}/${cve_name}_*" 确保能匹配到正确的模型和CVE目录
                # -quit 会在找到第一个匹配项后立即退出，效率更高
                if find "$FINAL_OUTPUT_BASE_DIR" -type d -path "*/${LLM_MODEL}/${cve_name}_*" -print -quit | grep -q . ; then
                    echo "模型 $LLM_MODEL 对 $cve_name 的结果已存在，跳过。"
                    continue
                fi

                # 如果有任务没有被跳过，说明本轮需要运行
                tasks_to_run_this_round=true
                echo "======== 正在处理: $cve_name | 模型: $LLM_MODEL (第 $run 轮) ========"

                # --- 【修改 3】: 获取当前模型对应的 Key ---
                CURRENT_LLM_KEY="${LLM_KEYS[$LLM_MODEL]}"
                if [ -z "$CURRENT_LLM_KEY" ]; then
                    echo "错误: 未找到模型 '$LLM_MODEL' 的 API Key，跳过此模型。"
                    continue
                fi
                
                # 为当前模型创建输出子目录
                MODEL_RUN_DIR="$RUN_OUTPUT_DIR/$LLM_MODEL"
                mkdir -p "$MODEL_RUN_DIR"

                # --- 单个命令的执行和重试逻辑 ---
                attempt=1
                success=false
                while [ $attempt -le $MAX_COMMAND_RETRIES ]; do
                    # 1. 运行前，获取程序实际输出目录下与此CVE相关的目录列表（快照）
                    dirs_before=$(find "$PROGRAM_ACTUAL_OUTPUT_DIR" -maxdepth 1 -type d -name "${cve_name}_*" 2>/dev/null | sort)

                    # 2. 运行检测器 (使用正确的 KEY)
                    "$DETECTOR_PATH" \
                        --input-src "$input_src" \
                        --input-bc "$input_bc" \
                        --llm-provider "$LLM_PROVIDER" \
                        --llm-model "$LLM_MODEL" \
                        --llm-key "$CURRENT_LLM_KEY"
                    
                    exit_code=$?
                    if [ $exit_code -eq 0 ]; then
                        echo "[$cve_name] 使用模型 [$LLM_MODEL] 处理成功"
                        success=true
                        
                        # --- 通过对比快照来找到并移动新目录 ---
                        dirs_after=$(find "$PROGRAM_ACTUAL_OUTPUT_DIR" -maxdepth 1 -type d -name "${cve_name}_*" 2>/dev/null | sort)
                        new_dir=$(comm -13 <(echo "$dirs_before") <(echo "$dirs_after"))

                        if [ -n "$new_dir" ] && [ -d "$new_dir" ]; then
                            echo "发现新生成的目录: $new_dir"
                            echo "移动结果目录到: $MODEL_RUN_DIR"
                            mv "$new_dir" "$MODEL_RUN_DIR/"
                        else
                            echo "警告: 未能找到为 $cve_name 新生成的输出目录。请检查 C++ 程序的输出路径是否为 $PROGRAM_ACTUAL_OUTPUT_DIR"
                        fi
                        # --- 移动逻辑结束 ---

                        break # 成功后退出重试循环
                    else
                        echo "处理 $cve_name (模型: $LLM_MODEL, 尝试次数 $attempt/$MAX_COMMAND_RETRIES) 失败，退出码: $exit_code"
                        if [ $attempt -lt $MAX_COMMAND_RETRIES ]; then
                            echo "将在 $RETRY_DELAY_SECONDS 秒后重试..."
                            sleep $RETRY_DELAY_SECONDS
                        else
                            echo "已达到最大重试次数，暂时放弃处理 $cve_name 的模型 $LLM_MODEL。"
                        fi
                    fi
                    ((attempt++))
                done
                echo "--- 模型: $LLM_MODEL 处理完成 ---"
            done # --- 模型循环结束 ---
            echo "-----------------------------------------------------"
        fi
    done

    if [ "$tasks_to_run_this_round" = false ]; then
        echo "所有 CVE 和模型组合的结果均已存在，提前结束脚本。"
        break
    fi

    if [ $run -lt $MAX_OVERALL_RUNS ]; then
        echo "第 $run 轮遍历完成，稍作等待后将开始下一轮..."
        sleep 30
    fi

    ((run++))
done

echo "##################################################"
echo "###       所有自动化对比试验流程已执行完毕       ###"
echo "###       所有新生成的结果已保存到以下目录:       ###"
echo "###         $RUN_OUTPUT_DIR         ###"
echo "##################################################"
