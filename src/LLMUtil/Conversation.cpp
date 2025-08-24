#include <fstream>
#include "CCPG/ThreadCreationTree.h"
#include "LLMUtil/Conversation.h"
#include "Util/Logger.h"
#include <iostream>

namespace llm_client{

Conversation::Conversation(
        std::shared_ptr<LLMClient> client,
        const std::string& system_prompt,
        size_t max_history
    ) : client_(client),
        base_system_prompt_(system_prompt),
        max_history_messages_(max_history){}

std::string Conversation::send_message(const std::string& user_message, void* context_for_tools) {
    this->context_for_tools_ = context_for_tools; // Store context for tool execution

    std::string effective_sys_prompt = build_effective_system_prompt();
    if (history_.empty() || history_[0].role != MessageRole::SYSTEM) {
        history_.insert(history_.begin(), {MessageRole::SYSTEM, effective_sys_prompt});
    } else {
        history_[0].content = effective_sys_prompt; // Update if tools/prompt changed
    }

    history_.push_back({MessageRole::USER, user_message, std::nullopt, std::nullopt});
    std::cout << "User: " << user_message << std::endl;
    prune_history();

    while (true) {
        std::vector<Tool> current_tools = get_available_tools(); // Get tools for current context/agent
        LLMClient::LLMResponse llm_response = client_->chat(history_, current_tools);

        ChatMessage assistant_message = {MessageRole::ASSISTANT, llm_response.assistant_content};
        if (llm_response.tool_requests && !llm_response.tool_requests->empty()) {
            assistant_message.tool_calls = llm_response.tool_requests;
            for (const auto& tool_req : *llm_response.tool_requests) {
                std::cout << "LLM Tool Call: " << tool_req.toolname << " with args " << tool_req.arguments.dump() << std::endl;
            }
        }
        history_.push_back(assistant_message);
        prune_history();

        if (!llm_response.tool_requests || llm_response.tool_requests->empty()) {
            return llm_response.assistant_content;
        }

        for (const auto& tool_req : *llm_response.tool_requests) {
            std::string tool_result_content = execute_tool(tool_req.toolname, tool_req.arguments);
            std::cout << "Tool Result: " << tool_result_content << std::endl;
            history_.push_back({MessageRole::TOOL, tool_result_content, std::nullopt, tool_req.id});
            prune_history();
            if(tool_result_content == "finish"){
                return parseResult(history_);
            }
        }
        // Loop back to send tool results to LLM
    }
}

void Conversation::set_system_prompt(const std::string& prompt) {
    base_system_prompt_ = prompt;
    // Force update or insertion of system prompt
    std::string effective_sys_prompt = build_effective_system_prompt();
    if (!history_.empty() && history_[0].role == MessageRole::SYSTEM) {
        history_[0].content = effective_sys_prompt;
    } else {
            // If history is not empty but has no system prompt, or if it's empty
        history_.insert(history_.begin(), {MessageRole::SYSTEM, effective_sys_prompt});
    }
    prune_history(); // Ensure it's still within limits
}

}
