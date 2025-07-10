#pragma once

#include "LLMUtil/Conversation.h"

namespace llm_client {

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
