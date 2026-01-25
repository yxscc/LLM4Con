#pragma once

#include "LLMUtil/Conversation.h"

class CCPGNode; // Forward declaration
class CCPG;     // Forward declaration

namespace llm_client {

class FindingThreadEntryAgent : public Conversation {
public:
    explicit FindingThreadEntryAgent(CCPG* ccpg, std::shared_ptr<LLMClient> client);

    // Method to find thread entry point
    long long find_thread_entry(CCPGNode* node);

private:
    CCPG* ccpg_; // Store CCPG context for tool execution
    bool attempted_get_function_ = false;
    bool attempted_call_graph_ = false;
    bool attempted_name_lookup_ = false;
    bool attempted_cpg_lookup_ = false;
    long long last_candidate_cpg_method_id_ = -1;

    // System prompt is now built dynamically
    static std::string build_system_prompt();

    // Overrides
    std::vector<Tool> get_available_tools() const override;
    std::string get_tool_choice() const override;
    std::string execute_tool(const std::string& tool_name, const nlohmann::json& arguments) override;
    std::string parseResult(const std::vector<ChatMessage>& history) override;
};

} // namespace llm_client
