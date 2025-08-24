#pragma once

#include "LLMUtil/Conversation.h"
#include "LLMUtil/ThreadPair.h" // 使用新的头文件

namespace llm_client {

class ParallelAnalysisAgent : public Conversation {
public:
    explicit ParallelAnalysisAgent(std::shared_ptr<LLMClient> client);

    // Analyzes a pair of threads based on their contracts to determine if they can run in parallel.
    // The result is stored within the ThreadPair object.
    void analyze_parallelism(ThreadPair& pair);

private:
    static std::string build_system_prompt();
    std::vector<Tool> get_available_tools() const override;
    std::string execute_tool(const std::string& tool_name, const nlohmann::json& arguments) override;
    std::string parseResult(const std::vector<ChatMessage>& history) override;
};

} // namespace llm_client
