#pragma once
#include <cpprest/http_client.h>
#include <string>
#include <deque>
#include <mutex>
#include <memory>
#include <fstream>
#include <optional>

#ifdef U
#undef U
#endif

#include "nlohmann/json.hpp"

namespace llm_client {

using namespace web;
using namespace web::http;
using namespace web::http::client;

// --- LLM Provider Enum ---
enum class LLMProvider {
    DEEPSEEK,
    GEMINI
};

struct Parameter {
    std::string name;
    std::string type;
    std::string description;
    bool required = false;
    std::optional<std::unique_ptr<Parameter>> items;

    // Default constructor
    Parameter() = default;

    // Constructor for easy initialization of simple parameters
    Parameter(std::string n, std::string t, std::string d, bool req = false)
        : name(std::move(n)), type(std::move(t)), description(std::move(d)), required(req), items(std::nullopt) {}

    // Constructor for array parameters
    Parameter(std::string n, std::string t, std::string d, bool req, std::unique_ptr<Parameter> i)
        : name(std::move(n)), type(std::move(t)), description(std::move(d)), required(req) {
        if (i) {
            items = std::move(i);
        }
    }

    // --- FIX: Custom copy constructor and assignment to handle unique_ptr ---
    Parameter(const Parameter& other)
        : name(other.name),
          type(other.type),
          description(other.description),
          required(other.required) {
        if (other.items) {
            items = std::make_unique<Parameter>(**other.items);
        }
    }

    Parameter& operator=(const Parameter& other) {
        if (this == &other) {
            return *this;
        }
        name = other.name;
        type = other.type;
        description = other.description;
        required = other.required;
        if (other.items) {
            items = std::make_unique<Parameter>(**other.items);
        } else {
            items.reset();
        }
        return *this;
    }

    // Explicitly default the move operations
    Parameter(Parameter&& other) noexcept = default;
    Parameter& operator=(Parameter&& other) noexcept = default;
};

struct Tool {
    std::string name;
    std::string description;
    std::vector<Parameter> parameters;

    // --- START OF MODIFICATION ---
    // 添加这个缺失的 to_json() 函数
    nlohmann::json to_json() const {
        nlohmann::json j;
        j["name"] = name;
        j["description"] = description;

        nlohmann::json params_obj;
        params_obj["type"] = "object";
        nlohmann::json properties_obj;

        std::vector<std::string> required_params;
        for (const auto& p : parameters) {
            nlohmann::json param_details;
            param_details["type"] = p.type;
            if (!p.description.empty()) {
                param_details["description"] = p.description;
            }

            // 正确处理数组类型的 'items' 字段
            if (p.type == "array" && p.items) {
                nlohmann::json items_json;
                items_json["type"] = (*p.items)->type;
                if (!(*p.items)->description.empty()) {
                    items_json["description"] = (*p.items)->description;
                }
                param_details["items"] = items_json;
            }
            
            properties_obj[p.name] = param_details;

            if (p.required) {
                required_params.push_back(p.name);
            }
        }

        if (!properties_obj.empty()) {
            params_obj["properties"] = properties_obj;
        }
        
        if (!required_params.empty()) {
            params_obj["required"] = required_params;
        }

        j["parameters"] = params_obj;
        return j;
    }
    // --- END OF MODIFICATION ---
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
    std::optional<std::vector<ToolCallRequest>> tool_calls;
    std::optional<std::string> tool_call_id;
    std::optional<std::string> tool_name; 

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

// --- FIX: Moved LLMResponse to be a standalone struct ---
struct LLMResponse {
    std::string assistant_content;
    std::optional<std::vector<ToolCallRequest>> tool_requests;
    bool is_error = false; 
};


// --- API Handler abstract base class ---
class APIHandler {
public:
    virtual ~APIHandler() = default;
    virtual nlohmann::json build_request_body(const std::string& model, const std::vector<ChatMessage>& messages, const std::vector<Tool>& tools) = 0;
    // --- FIX: Use the standalone LLMResponse struct ---
    virtual LLMResponse parse_response(const nlohmann::json& response_body) = 0;
};


class LLMClient {
public:
    // --- FIX: Use the standalone LLMResponse struct ---
    using LLMResponse = llm_client::LLMResponse;

    static void initialize_shared_instance(
        LLMProvider provider,
        const std::string& base_url,
        const std::string& api_key);

    static std::shared_ptr<LLMClient> get_instance();

    LLMResponse chat(const std::vector<ChatMessage>& messages, const std::vector<Tool>& available_tools = {});

    LLMClient(const LLMClient&) = delete;
    LLMClient& operator=(const LLMClient&) = delete;

    void set_timeout(long seconds);
    void set_model(const std::string& model);
    LLMProvider get_provider() const { return provider_; }

private:
    LLMProvider provider_;
    std::unique_ptr<APIHandler> api_handler_;
    utility::string_t path_;
    std::string base_url_;
    std::shared_ptr<http_client> client_;
    std::string api_key_;
    std::string default_model_;
    size_t max_context_length_;
    long timeout_seconds_;
    static std::shared_ptr<LLMClient> instance;
    static std::mutex mutex;

    explicit LLMClient(
        LLMProvider provider,
        const std::string& base_url,
        const std::string& api_key,
        const std::string& default_model = "",
        size_t max_context_length = 10
    );


};

} // namespace llm_client
