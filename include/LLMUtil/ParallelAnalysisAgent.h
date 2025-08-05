#pragma once

#include "LLMUtil/Conversation.h"

namespace llm_client {

    // 用于描述可并发的代码区域
struct CodeRegion {
    int thread_entry_id;
    int start_node_id;
    int end_node_id;
    // 或者使用行号
    // int start_line;
    // int end_line;
};

// 最终的分析结果
struct ParallelAnalysisResult {
    bool can_run_in_parallel;
    std::string reason;
    std::vector<CodeRegion> concurrent_regions;
};

// 描述一个线程的上下文，作为Agent的输入
struct ThreadContext {
    int fork_node_id;             // pthread_create 等调用的节点ID
    std::string thread_handle_var; // 存储线程句柄的变量名, e.g., "t1" in "pthread_t t1;"
    int entry_function_id;        // 入口函数的ID
    std::string entry_function_code; // 入口函数的源码
};

class ParallelAnalysisAgent : public Conversation {
public:
    explicit ParallelAnalysisAgent(std::shared_ptr<LLMClient> client);

    // Returns true if threads can run in parallel, false otherwise.
    bool analyze_parallelism(int function_id_1, int function_id_2);

private:
    bool last_parallel_status_ = false; // To store the result

    static std::string build_system_prompt();
    std::vector<Tool> get_available_tools() const override;
    std::string execute_tool(const std::string& tool_name, const nlohmann::json& arguments) override;
    std::string parseResult(const std::vector<ChatMessage>& history) override;
};

} // namespace llm_client
