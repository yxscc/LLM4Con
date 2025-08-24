#pragma once

#include "LLMUtil/Conversation.h"

class CCPGNode; // Forward declaration
class CCPG;     // Forward declaration

namespace llm_client {

class FindingThreadEntryAgent : public Conversation {
public:
    explicit FindingThreadEntryAgent(CCPG* ccpg, std::shared_ptr<LLMClient> client);

    // Method to find thread entry point
    int find_thread_entry(CCPGNode* node);

private:
    CCPG* ccpg_; // Store CCPG context for tool execution

    // System prompt is now built dynamically
    static std::string build_system_prompt();

    // Overrides
    std::vector<Tool> get_available_tools() const override;
    std::string execute_tool(const std::string& tool_name, const nlohmann::json& arguments) override;
    std::string parseResult(const std::vector<ChatMessage>& history) override;
};

} // namespace llm_client
