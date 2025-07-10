#pragma once

#include "LLMClient.h"
#include <string>
#include <vector>
#include <memory>

class CCPGNode; // Forward declaration

namespace llm_client {

class Conversation {
public:
    // 构造函数
    Conversation(std::shared_ptr<LLMClient> client, 
                const std::string& system_prompt = "",
                size_t max_history = 20);

    virtual ~Conversation() = default;

    const std::vector<ChatMessage>& get_history() const {
        return history_;
    }
    
    // 发送消息并获取回复
    std::string send_message(const std::string& user_message, void* context_for_tools = nullptr);
    
    void set_system_prompt(const std::string& prompt);

    // 重置对话（保留系统提示）
    void reset();
    
    // 获取客户端引用
    LLMClient& get_client();
    
private:
    std::shared_ptr<LLMClient> client_;
    std::string base_system_prompt_;
    std::vector<ChatMessage> history_;
    size_t max_history_messages_;
    void* tool_execution_context_;

        void prune_history() {
        if (history_.empty()) return;
        
        size_t current_size = history_.size();
        size_t limit = max_history_messages_;

        if (current_size <= limit) return;

        if (history_[0].role == MessageRole::SYSTEM) {
            // Keep system prompt + last (limit - 1) messages
            if (limit == 1) { // Only keep system prompt
                 history_.erase(history_.begin() + 1, history_.end());
            } else if (current_size > limit) {
                history_.erase(history_.begin() + 1, history_.begin() + 1 + (current_size - limit));
            }
        } else {
            // No system prompt, just keep last 'limit' messages
            history_.erase(history_.begin(), history_.begin() + (current_size - limit));
        }
    }

    // To be overridden by Agent subclasses to provide their tools
    virtual std::vector<Tool> get_available_tools() const {
        return {};
    }

    virtual std::string parseResult(const std::vector<ChatMessage>& history) {
        // Default implementation just returns the last assistant message
        if (!history.empty() && history.back().role == MessageRole::ASSISTANT) {
            return history.back().content;
        }
        return "No valid assistant response found.";
    }

    // To be overridden by Agent subclasses to execute their specific tools
    // The `tool_execution_context_` member is available for use here.
    virtual std::string execute_tool(const std::string& tool_name, const nlohmann::json& arguments) {
        nlohmann::json error_resp;
        error_resp["error"] = "Tool '" + tool_name + "' not found or not implemented by this agent.";
        error_resp["tool_name"] = tool_name;
        error_resp["arguments_received"] = arguments;
        return error_resp.dump();
    }

    // Helper to build the full system prompt including tool descriptions
    virtual std::string build_effective_system_prompt() {
        std::string effective_prompt = base_system_prompt_;
        return effective_prompt;
    }
};

} // namespace llm_client