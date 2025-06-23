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


class FindingThreadEntryAgent : public Conversation {
public:
    // Constructor with specialized system prompt
    explicit FindingThreadEntryAgent(std::shared_ptr<LLMClient> client)
        : Conversation(client, build_system_prompt(), 15) // Higher max_history for analysis
    {
        // Additional initialization if needed
    }

    // Method to find thread entry point
    int find_thread_entry(CCPGNode * node);

private:
    std::string last_entry_point_;

    static std::string build_system_prompt() {
        return R"(
            You are a multithreaded program analysis expert responsible for identifying the entry function of a thread. 
            You will be provided with the source code containing a fork statement, which could be pthread_create or other similar APIs. 
            Your objective is to determine the entry function of the thread created by this fork statement. 
            The entry function refers to the first function executed by the thread, such as the third parameter of pthread_create.

            To obtain the necessary information, you may access various static data sources. The functions available to you are as follows:
            - `get_function(int node_id)`: Get the function containing the node, returning comprehensive details including the function ID, function body, etc.
            - `get_callers(int function_id)`: Get the callers of a function, returning a set of callsite nodes.
            - `confirm_thread_entry()`: When confident with the thread entry, call with the entry in the form `(function_id)`
            - `get_function_by_name(string name)`: Get the function whose name matches the argument.

            You must pass arguments to these functions strictly as required. Don't call one function with the same parameters multiple times in a single round.

            You should work in the following process:
            1. Identify the function that contains the fork statement to confirm the context. You can use `get_function()` to retrieve the function details.
            2. Determine if the thread entry is a direct function name or assigned pointer
            3. For direct names, use `get_function_by_name()`
            4. For assigned pointers, trace origin via `get_callers()`. you need to analyze the assignment to find the actual function name.
            )";
    }

    // Override to provide specific tools for this agent
    std::vector<Tool> get_available_tools() const override {
        return {
            {"get_function", "Get function details by node ID",
            {
                {"node_id", "number", "Get the function containing the node, returning comprehensive details including the function ID, function body, etc.", true}
            }},
            {"get_callers", "Get callers of a function by function ID", 
            {
                {"function_id", "number", "Get the callers of a function, returning a set of callsite nodes.", true}
            }},
            {"confirm_thread_entry", "Confirm the thread entry function by providing its ID",
            {
                {"entry_function_id", "number", "When confident with the thread entry, call with the entry in the form (function_id).", true}
            }},
            {"get_function_by_name", "Get a function by its name",
            {
                {"name", "string", "Get the function whose name matches the argument.", true}
            }}
        };
    }

    // Override to execute specific tools for this agent
    std::string execute_tool(const std::string& tool_name, const nlohmann::json& arguments) override;

    std::string parseResult(const std::vector<ChatMessage>& history) override;
};

} // namespace llm_client