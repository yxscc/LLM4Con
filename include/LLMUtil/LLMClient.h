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

private:
    LLMProvider provider_;
    std::unique_ptr<APIHandler> api_handler_;
    utility::string_t path_;

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
