#include "LLMUtil/LLMClient.h"
#include <cpprest/filestream.h>
#include <cpprest/asyncrt_utils.h>
#include <algorithm>
#include <nlohmann/json.hpp>
#include <boost/asio/ssl.hpp>
#include "Util/Logger.h"
#include <cpprest/uri.h>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <array>

using namespace utility;

namespace llm_client {

std::shared_ptr<LLMClient> LLMClient::instance = nullptr;
std::mutex LLMClient::mutex;

// Helper function to execute a command and get its output
std::string exec(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

// --- DeepSeekHandler 保持不变 ---
class DeepSeekHandler : public APIHandler {
public:
    nlohmann::json build_request_body(const std::string& model, const std::vector<ChatMessage>& messages, const std::vector<Tool>& tools) override {
        // ... (这部分代码保持原样)
        nlohmann::json request_body;
        request_body["model"] = model;

        nlohmann::json messages_array = nlohmann::json::array();
        for (const auto& msg : messages) {
            nlohmann::json message_json;
            message_json["role"] = msg.role_to_string();
            message_json["content"] = msg.content;

            if (msg.tool_calls.has_value()) {
                nlohmann::json tool_calls_array = nlohmann::json::array();
                for (const auto& tool_call : *msg.tool_calls) {
                    nlohmann::json tool_call_json;
                    tool_call_json["id"] = tool_call.id;
                    tool_call_json["type"] = "function";
                    nlohmann::json function_json;
                    function_json["name"] = tool_call.toolname;
                    function_json["arguments"] = tool_call.arguments.dump();
                    tool_call_json["function"] = function_json;
                    tool_calls_array.push_back(tool_call_json);
                }
                message_json["tool_calls"] = tool_calls_array;
            }
            if (msg.tool_call_id.has_value()) {
                message_json["tool_call_id"] = *msg.tool_call_id;
            }
            messages_array.push_back(message_json);
        }
        request_body["messages"] = messages_array;

        if (!tools.empty()) {
            nlohmann::json tools_json = nlohmann::json::array();
            for (const auto& tool : tools) {
                 nlohmann::json function = nlohmann::json::object();
                function["type"] = "function";
                nlohmann::json tool_json;
                tool_json["name"] = tool.name;
                tool_json["description"] = tool.description;
                nlohmann::json parameters_json;
                parameters_json["type"] = "object";
                nlohmann::json required_params = nlohmann::json::array();
                parameters_json["properties"] = nlohmann::json::object();
                for( const auto& param : tool.parameters) {
                    nlohmann::json param_json;
                    param_json["type"] = param.type;
                    param_json["description"] = param.description;
                    if( param.required) {
                        required_params.push_back(param.name);
                    }
                    parameters_json["properties"][param.name] = param_json;
                }
                parameters_json["required"] = required_params;
                tool_json["parameters"] = parameters_json;
                function["function"] = tool_json;
                tools_json.push_back(function);
            }
            request_body["tools"] = tools_json;
        }
        return request_body;
    }
    
    LLMResponse parse_response(const nlohmann::json& response_body) override {
        // ... (这部分代码保持原样)
        LLMResponse result;
        if (response_body.contains("choices") && !response_body["choices"].empty()) {
            auto message = response_body["choices"][0]["message"];
            if (message.contains("content") && !message["content"].is_null()) {
                result.assistant_content = message["content"].get<std::string>();
            }
            if (message.contains("tool_calls")) {
                std::vector<ToolCallRequest> tool_requests;
                for (const auto& tool_call : message["tool_calls"]) {
                    ToolCallRequest req;
                    req.id = tool_call["id"].get<std::string>();
                    req.toolname = tool_call["function"]["name"].get<std::string>();
                    req.arguments = nlohmann::json::parse(tool_call["function"]["arguments"].get<std::string>());
                    tool_requests.push_back(req);
                }
                result.tool_requests = tool_requests;
            }
        }
        return result;
    }
};

class GeminiHandler : public APIHandler {
public:
    nlohmann::json build_request_body(const std::string& model, const std::vector<ChatMessage>& messages, const std::vector<Tool>& tools) override {
        nlohmann::json request_body;
        nlohmann::json contents_array = nlohmann::json::array();
        std::string system_prompt_content;

        auto it = std::find_if(messages.begin(), messages.end(), [](const ChatMessage& msg) {
            return msg.role == MessageRole::SYSTEM;
        });
        if (it != messages.end()) {
            system_prompt_content = it->content + "\n\n";
        }

        bool first_user_message = true;
        for (const auto& msg : messages) {
            if (msg.role == MessageRole::SYSTEM) continue;

            if (msg.role == MessageRole::TOOL) {
                nlohmann::json tool_content_item;
                tool_content_item["role"] = "tool";
                nlohmann::json tool_parts_array = nlohmann::json::array();
                
                if (msg.tool_calls.has_value() && !msg.tool_calls->empty()) {
                    for (const auto& tool_call : *msg.tool_calls) {
                        auto parsed_content = nlohmann::json::parse(tool_call.arguments.get<std::string>(), nullptr, false);

                        if (parsed_content.is_discarded()) {
                            parsed_content = nlohmann::json{{"raw_string_response", tool_call.arguments.get<std::string>()}};
                        } else if (parsed_content.is_array()) {
                            parsed_content = nlohmann::json{{"results", parsed_content}};
                        }

                        tool_parts_array.push_back({
                            {"functionResponse", {
                                {"name", tool_call.toolname},
                                {"response", parsed_content}
                            }}
                        });
                    }
                }
                
                if (!tool_parts_array.empty()) {
                    tool_content_item["parts"] = tool_parts_array;
                    contents_array.push_back(tool_content_item);
                }
                continue;
            }

            nlohmann::json content_item;
            content_item["role"] = (msg.role == MessageRole::ASSISTANT) ? "model" : msg.role_to_string();
            
            nlohmann::json parts_array = nlohmann::json::array();
            if (msg.role == MessageRole::USER && first_user_message && !system_prompt_content.empty()) {
                parts_array.push_back({{"text", system_prompt_content + msg.content}});
                first_user_message = false;
            } else if (msg.tool_calls.has_value()) {
                 if (!msg.content.empty()){
                     parts_array.push_back({{"text", msg.content}});
                 }
                 for (const auto& tool_call : *msg.tool_calls) {
                     parts_array.push_back({
                         {"functionCall", {
                             {"name", tool_call.toolname},
                             {"args", tool_call.arguments}
                         }}
                     });
                 }
            } else {
                 parts_array.push_back({{"text", msg.content}});
            }

            content_item["parts"] = parts_array;
            contents_array.push_back(content_item);
        }
        
        request_body["contents"] = contents_array;
        
        // --- THIS IS THE CRITICAL FIX ---
        if (!tools.empty()) {
            nlohmann::json function_declarations = nlohmann::json::array();
            for (const auto& tool : tools) {
                function_declarations.push_back(tool.to_json());
            }
            // The entire list of declarations must be wrapped in an object, which is then placed in the 'tools' array.
            request_body["tools"] = nlohmann::json::array({
                {{"functionDeclarations", function_declarations}}
            });
        }
        // --- END OF FIX ---

        return request_body;
    }

    LLMResponse parse_response(const nlohmann::json& response_body) override {
        LLMResponse response;

        if (response_body.contains("error")) {
            response.is_error = true;
            try {
                response.assistant_content = "API Error: " + response_body.dump(4);
            } catch (const nlohmann::json::exception& e) {
                response.assistant_content = "API Error: Failed to parse error response.";
            }
            return response;
        }

        if (!response_body.contains("candidates") || !response_body["candidates"].is_array() || response_body["candidates"].empty()) {
            response.is_error = true;
            response.assistant_content = "Malformed response from API - 'candidates' field is missing or empty.";
            return response;
        }

        const auto& first_candidate = response_body["candidates"][0];
        
        if (first_candidate.contains("finishReason") && first_candidate.value("finishReason", "") != "STOP" && first_candidate.value("finishReason", "") != "TOOL_CALLS") {
            response.is_error = true;
            response.assistant_content = "API call finished with reason: " + first_candidate.value("finishReason", "UNKNOWN");
            return response;
        }

        response.is_error = false;
        response.tool_requests = std::vector<ToolCallRequest>();
        
        if (first_candidate.contains("content") && first_candidate["content"].contains("parts")) {
            for (const auto& part : first_candidate["content"]["parts"]) {
                if (part.contains("text")) {
                    response.assistant_content += part["text"].get<std::string>();
                }
                if (part.contains("functionCall")) {
                    const auto& func_call = part["functionCall"];
                    response.tool_requests->push_back({
                        "", // Gemini does not use tool_call_id
                        func_call["name"].get<std::string>(),
                        func_call["args"]
                    });
                }
            }
        }
        return response;
    }
};

// --- LLMClient 的修改 ---

LLMClient::LLMClient(
    LLMProvider provider,
    const std::string& base_url,
    const std::string& api_key,
    const std::string& default_model,
    size_t max_context_length)
    : provider_(provider),
      api_key_(api_key),
      default_model_(default_model),
      max_context_length_(max_context_length),
      timeout_seconds_(30) {

    if (provider_ == LLMProvider::DEEPSEEK) {
        api_handler_ = std::make_unique<DeepSeekHandler>();
        // cpprestsdk client setup
        http_client_config config;
        config.set_timeout(std::chrono::seconds(timeout_seconds_));
        web::uri full_uri(utility::conversions::to_string_t(base_url));
        path_ = full_uri.path();
        utility::string_t base_uri_str = full_uri.scheme() + utility::conversions::to_string_t("://") + full_uri.host();
        if (full_uri.port() > 0) {
            base_uri_str += utility::conversions::to_string_t(":") + utility::conversions::to_string_t(std::to_string(full_uri.port()));
        }
        client_ = std::make_shared<http_client>(base_uri_str, config);

    } else if (provider_ == LLMProvider::GEMINI) {
        api_handler_ = std::make_unique<GeminiHandler>();
        // 对于Gemini，我们将base_url直接存储，因为curl会用到它
        base_url_ = base_url;
    } else {
        throw std::invalid_argument("Unsupported LLM provider.");
    }
}

void LLMClient::initialize_shared_instance(LLMProvider provider, const std::string& base_url, const std::string& api_key) {
    std::lock_guard<std::mutex> lock(mutex);
    if (!instance) {
        if (base_url.empty() || api_key.empty()) {
            throw std::invalid_argument("LLMClient must be initialized with a non-empty base URL and API key.");
        }
        struct MakeSharedEnabler : public LLMClient {
            MakeSharedEnabler(LLMProvider p, const std::string& bu, const std::string& ak) : LLMClient(p, bu, ak) {}
        };
        instance = std::make_shared<MakeSharedEnabler>(provider, base_url, api_key);
    }
}

std::shared_ptr<LLMClient> LLMClient::get_instance() {
    std::lock_guard<std::mutex> lock(mutex);
    if (!instance) {
        throw std::runtime_error("LLMClient has not been initialized. Call initialize_shared_instance first in your main function.");
    }
    return instance;
}

LLMClient::LLMResponse LLMClient::chat(const std::vector<ChatMessage>& messages, const std::vector<Tool>& available_tools) {
    nlohmann::json request_body = api_handler_->build_request_body(default_model_, messages, available_tools);
    std::string request_body_str = request_body.dump();
    
    Logger::getInstance()->log(std::string("--> Request (") + (provider_ == LLMProvider::GEMINI ? "Gemini" : "DeepSeek") + "):\n" + request_body.dump(4));

    if (provider_ == LLMProvider::GEMINI) {
        // 使用 curl 发送请求
        // 需要转义JSON字符串中的单引号，以防止shell命令出错
        std::string escaped_body = request_body_str;
        size_t pos = 0;
        while ((pos = escaped_body.find('\'', pos)) != std::string::npos) {
             escaped_body.replace(pos, 1, "'\\''");
             pos += 4;
        }

        std::string command = "curl -s -X POST -H \"Content-Type: application/json\" -H \"x-goog-api-key: " + api_key_ + "\" '" + base_url_ + "' -d '" + escaped_body + "'";
        
        std::string response_str = exec(command.c_str());
        
        if (response_str.empty()) {
             throw std::runtime_error("Gemini API request failed: empty response from curl. Check network or API key.");
        }

        auto response_json = nlohmann::json::parse(response_str, nullptr, false);
        if (response_json.is_discarded()) {
             throw std::runtime_error("Gemini API request failed: could not parse JSON response: " + response_str);
        }
        
        Logger::getInstance()->log(std::string("<-- Response (Gemini):\n") + response_json.dump(4));
        return api_handler_->parse_response(response_json);

    } else { // DeepSeek 的逻辑保持不变
        http_request request(methods::POST);
        request.set_request_uri(path_);
        request.headers().add(conversions::to_string_t("Authorization"), conversions::to_string_t("Bearer ") + conversions::to_string_t(api_key_));
        request.headers().add(conversions::to_string_t("Content-Type"), conversions::to_string_t("application/json"));
        request.set_body(conversions::to_string_t(request_body_str));

        try {
            http_response response = client_->request(request).get();

            if (response.status_code() == status_codes::OK) {
                auto response_body_str_utf8 = conversions::to_utf8string(response.extract_string().get());
                auto response_json = nlohmann::json::parse(response_body_str_utf8);
                
                Logger::getInstance()->log(std::string("<-- Response (DeepSeek):\n") + response_json.dump(4));
                return api_handler_->parse_response(response_json);
            } else {
                std::ostringstream error_msg;
                error_msg << "LLM API request failed with status code: " << response.status_code();
                std::string error_body_str;
                try {
                    error_body_str = conversions::to_utf8string(response.extract_string().get());
                    if (!error_body_str.empty()) {
                        error_msg << ", Error: " << error_body_str;
                    }
                } catch (...) {}
                Logger::getInstance()->log("!!! Error: " + error_msg.str());
                throw std::runtime_error(error_msg.str());
            }
        } catch (const web::http::http_exception& e) {
            std::cerr << "\n\n!!! CRITICAL HTTP LIBRARY ERROR !!!\n";
            std::cerr << "Exception caught: " << e.what() << "\n";
            std::cerr << "This often indicates a TLS/SSL handshake failure or a network issue.\n";
            std::cerr << "Please check your system's root CA certificates.\n\n";
            throw;
        }
    }
}

// 配置方法
void LLMClient::set_timeout(long seconds) {
    timeout_seconds_ = seconds;
    if (provider_ == LLMProvider::DEEPSEEK) {
        http_client_config config;
        config.set_timeout(std::chrono::seconds(timeout_seconds_));
        client_ = std::make_shared<http_client>(client_->base_uri(), config);
    }
    // curl的超时可以在命令中设置，但为了简化，这里暂时忽略
}

void LLMClient::set_model(const std::string& model) {
    default_model_ = model;
}

} // namespace llm_client