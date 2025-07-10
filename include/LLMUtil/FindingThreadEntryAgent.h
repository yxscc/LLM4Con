#pragma once

#include "LLMUtil/Conversation.h"

class CCPGNode; // Forward declaration

namespace llm_client {

class FindingThreadEntryAgent : public Conversation {
public:
    // Constructor with specialized system prompt
    explicit FindingThreadEntryAgent(std::shared_ptr<LLMClient> client);

    // Method to find thread entry point
    int find_thread_entry(CCPGNode * node);

private:
    std::string last_entry_point_;

    static std::string build_system_prompt();

    // Override to provide specific tools for this agent
    std::vector<Tool> get_available_tools() const override;

    // Override to execute specific tools for this agent
    std::string execute_tool(const std::string& tool_name, const nlohmann::json& arguments) override;

    std::string parseResult(const std::vector<ChatMessage>& history) override;
};

} // namespace llm_client
