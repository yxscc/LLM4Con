#pragma once
#include <cpprest/http_client.h>
#include <string>
#include <deque>
#include <mutex>
#include <memory>
#include <fstream>
#include <optional>

#include "nlohmann/json.hpp" // Ensure you have this library for JSON handling

namespace llm_client {

using namespace web;
using namespace web::http;
using namespace web::http::client;

struct Parameter {
    std::string name;
    std::string type; // e.g., "string", "integer", "boolean"
    std::string description;
    bool required = false; // 是否为必需参数
};

struct Tool {
    std::string name;
    std::string description;
    std::vector<Parameter> parameters;
};

struct ToolCallRequest {
    std::string id;
    std::string toolname;
    nlohmann::json arguments;
};

enum class MessageRole {
    SYSTEM,
    USER,
    ASSISTANT,
    TOOL
};

struct ChatMessage {
    MessageRole role;
    std::string content;

    // For assistant messages that request one or more tool calls
    std::optional<std::vector<ToolCallRequest>> tool_calls;

    // For tool messages (responses from executing a tool)
    // This ID links the tool's response message back to the assistant's request.
    std::optional<std::string> tool_call_id;

    // Helper to convert role to string if needed for LLM API
    std::string role_to_string() const {
        switch (role) {
            case MessageRole::SYSTEM: return "system";
            case MessageRole::USER: return "user";
            case MessageRole::ASSISTANT: return "assistant";
            case MessageRole::TOOL: return "tool";
            default: return "unknown";
        }
    }
};

class LLMClient {
public:

    // 构造函数
    explicit LLMClient(
        const std::string& base_url,
        const std::string& api_key,
        const std::string& default_model = "deepseek-v3-250324",
        size_t max_context_length = 10
    );

    static std::shared_ptr<LLMClient> get_shared_instance(const std::string& base_url = "", const std::string& api_key = "") 
    {
        static std::mutex mtx;
        static std::shared_ptr<LLMClient> instance;
        
        std::lock_guard<std::mutex> lock(mtx);
        if (!instance && !base_url.empty()) {
            instance = std::make_shared<LLMClient>(base_url, api_key);
        }
        return instance;
    }

    struct LLMResponse {
        std::string assistant_content; // Standard text response from the assistant
        std::optional<std::vector<ToolCallRequest>> tool_requests; // If LLM wants to call one or more tools
    };

    // The client needs to be aware of available tools to pass them to the LLM API
    // if it supports structured tool calling.
    LLMResponse chat(const std::vector<ChatMessage>& messages, const std::vector<Tool>& available_tools = {});
    
    // 禁用拷贝和移动
    LLMClient(const LLMClient&) = delete;
    LLMClient& operator=(const LLMClient&) = delete;

    // 高级配置
    void set_timeout(long seconds);
    void set_model(const std::string& model);    


private:

    // 成员变量
    std::shared_ptr<http_client> client_;
    std::string api_key_;
    std::string default_model_;
    size_t max_context_length_;
    long timeout_seconds_;
};

} // namespace llm_client