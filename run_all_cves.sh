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

# C++程序实际的输出根目录 (根据 TargetPath.h 推断)
PROGRAM_ACTUAL_OUTPUT_DIR="/home/ConCodeQL/LLM_dump"
# 我们希望统一存放所有结果的总目录
FINAL_OUTPUT_BASE_DIR="/home/ConCodeQL/ConCodeQL-output"

# =================================================================
# ===== 为本次运行创建一个唯一的总输出目录 =====
# =================================================================
TIMESTAMP=$(date +"%Y-%m-%d_%H-%M-%S")
RUN_OUTPUT_DIR="${FINAL_OUTPUT_BASE_DIR}/run_${TIMESTAMP}"
mkdir -p "$RUN_OUTPUT_DIR"
echo "所有本次运行的新结果将保存在: $RUN_OUTPUT_DIR"
echo "================================================================="


# 固定的参数
LLM_PROVIDER="openai"
LLM_MODEL="gpt-5-2025-08-07"
LLM_KEY="sk-Y5PXhElM2NobgKPelwlHFaXPeQrSzm4WJYOnHYn0QafbVRoK"

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
    all_completed_this_run=true # 假设本轮能全部完成

    # 遍历 LinConVul 目录下的所有 CVE 文件夹
    for cve_dir in ${LINCONVUL_DIR}/CVE-*; do
        if [ -d "$cve_dir" ]; then
            cve_name=$(basename "$cve_dir")
            
            # ##################################################################
            # ### 新增/修改的检查逻辑 开始 ###
            # ##################################################################
            # 递归地在总输出目录中查找是否已存在此CVE的结果文件夹
            # 使用 -print -quit 可以在找到第一个匹配项后立即退出，提高效率
            existing_output_dir=$(find "$FINAL_OUTPUT_BASE_DIR" -type d -name "${cve_name}_*" -print -quit 2>/dev/null)

            if [ -n "$existing_output_dir" ]; then
                echo "[$cve_name] 的结果目录已存在于历史运行中，跳过。"
                # echo "  (发现于: $existing_output_dir)" # 如果需要可以取消注释此行来显示具体路径
                echo "----------------------------------------"
                continue
            fi
            # ##################################################################
            # ### 新增/修改的检查逻辑 结束 ###
            # ##################################################################

            all_completed_this_run=false # 只要有一个需要处理，就说明本轮不是空转

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
                # 1. 运行前，获取程序实际输出目录下与此CVE相关的目录列表（快照）
                dirs_before=$(find "$PROGRAM_ACTUAL_OUTPUT_DIR" -maxdepth 1 -type d -name "${cve_name}_*" 2>/dev/null | sort)

                # 2. 运行检测器
                "$DETECTOR_PATH" \
                            --input-src "$input_src" \
                            --input-bc "$input_bc" \
                            --llm-provider "$LLM_PROVIDER" \
                            --llm-model "$LLM_MODEL" \
                            --llm-key "$LLM_KEY"
                
                exit_code=$?
                if [ $exit_code -eq 0 ]; then
                    # 3. 运行后，再次获取目录列表
                    dirs_after=$(find "$PROGRAM_ACTUAL_OUTPUT_DIR" -maxdepth 1 -type d -name "${cve_name}_*" 2>/dev/null | sort)

                    # 4. 使用 comm 命令找出两个列表的差异，即新生成的目录
                    new_dir=$(comm -13 <(echo "$dirs_before") <(echo "$dirs_after"))

                    # 5. 检查是否找到了新目录
                    if [ -n "$new_dir" ] && [ -d "$new_dir" ]; then
                        echo "发现新生成的目录: $new_dir"
                        
                        report_file="$new_dir/llm_evaluation_report.md"
                        error_pattern="API request failed (curl exit"

                        # 6. 验证报告文件内容：检查文件是否存在，并且是否包含错误字符串
                        if [ -f "$report_file" ] && grep -q "$error_pattern" "$report_file"; then
                            # 如果发现错误，则打印信息，删除无效结果，并让 success 保持 false 以触发重试
                            echo "错误: 在 $report_file 中检测到API请求失败。结果无效。"
                            echo "将清理此失败的尝试 ($new_dir) 并等待下一轮重试。"
                            rm -rf "$new_dir"
                        else
                            # 如果未发现错误（或报告文件不存在），则移动目录，并将 success 设为 true
                            echo "结果验证通过，未发现API错误。正在移动结果..."
                            mv "$new_dir" "$RUN_OUTPUT_DIR/"
                            echo "处理成功: $cve_name"
                            success=true
                        fi
                    else
                        # 程序成功退出但未生成新目录，这是一个异常情况，也应重试
                        echo "警告: 未能找到为 $cve_name 新生成的输出目录。将触发重试。"
                    fi
                    
                    # 如果本次尝试最终被确认为成功，则跳出重试循环
                    if [ "$success" = true ]; then
                        break
                    fi
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

    # 注意：这里的逻辑稍微修改了一下，因为之前的 all_completed 逻辑有问题
    # 如果第一轮就发现所有东西都已存在，all_completed_this_run 会是 true，然后直接退出
    if [ "$all_completed_this_run" = true ]; then
        echo "所有需要处理的 CVE 均已存在输出目录，提前结束。"
        break
    fi

    if [ $run -lt $MAX_OVERALL_RUNS ]; then
        echo "第 $run 轮遍历完成，稍作等待后将开始下一轮..."
        sleep 30
    fi

    ((run++))
done

echo "##################################################"
echo "###       所有自动化处理流程已执行完毕       ###"
echo "###     所有新生成的结果已保存到以下目录:     ###"
echo "###       $RUN_OUTPUT_DIR       ###"
echo "##################################################"