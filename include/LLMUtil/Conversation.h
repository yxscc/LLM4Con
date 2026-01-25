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

    void* get_context_for_tools() const {
        return context_for_tools_;
    }
    
private:
    std::shared_ptr<LLMClient> client_;
    std::string base_system_prompt_;
    std::vector<ChatMessage> history_;
    void* context_for_tools_ = nullptr;
    size_t max_history_messages_;
    std::ofstream simplified_log_file_;
    
    void prune_history() {
        if (history_.empty()) return;
        
        size_t current_size = history_.size();
        size_t limit = max_history_messages_;

        if (current_size <= limit) return;

        std::vector<ChatMessage>::iterator erase_start;
        std::vector<ChatMessage>::iterator erase_end;

        if (history_[0].role == MessageRole::SYSTEM) {
            // Keep system prompt + last (limit - 1) messages
            // We erase from index 1 to (1 + count)
            erase_start = history_.begin() + 1;
            size_t num_to_remove = current_size - limit;
            
            // Safety clamp
            if (num_to_remove > current_size - 1) num_to_remove = current_size - 1;

            erase_end = history_.begin() + 1 + num_to_remove;
        } else {
            // No system prompt, just keep last 'limit' messages
            erase_start = history_.begin();
            size_t num_to_remove = current_size - limit;
             // Safety clamp
            if (num_to_remove > current_size) num_to_remove = current_size;
            
            erase_end = history_.begin() + num_to_remove;
        }

        // CRITICAL FIX: Prevent "Orphaned Tool Response" error.
        // The OpenAI API requires that every message with role 'tool' must be immediately 
        // preceded by a message with role 'assistant' containing 'tool_calls'.
        // Since we prune from the oldest messages, we might delete an ASSISTANT message 
        // but leave its subsequent TOOL response as the new first message.
        // To fix this, if the first message we plan to KEEP (erase_end) is a TOOL message,
        // we must continue erasing until we find a non-TOOL message (or empty the history).
        while (erase_end != history_.end() && erase_end->role == MessageRole::TOOL) {
            erase_end++;
        }

        if (erase_start < erase_end) {
            history_.erase(erase_start, erase_end);
        }
    }

    // To be overridden by Agent subclasses to provide their tools
    virtual std::vector<Tool> get_available_tools() const {
        return {};
    }

    // Agent-specific tool choice for OpenAI-compatible APIs.
    // Typical values: "auto" (default) or "required" (force tool calls when tools are provided).
    virtual std::string get_tool_choice() const {
        return "auto";
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