#include "LLMUtil/LLMClient.h"
#include <cpprest/filestream.h>
#include <cpprest/asyncrt_utils.h>
#include <algorithm>
#include <nlohmann/json.hpp>
#include <boost/asio/ssl.hpp>
#include "Util/Logger.h"
#include <cpprest/uri.h>

using namespace utility;

namespace llm_client {

std::shared_ptr<LLMClient> LLMClient::instance = nullptr;
std::mutex LLMClient::mutex;

class DeepSeekHandler : public APIHandler {
public:
    nlohmann::json build_request_body(const std::string& model, const std::vector<ChatMessage>& messages, const std::vector<Tool>& tools) override {
        nlohmann::json request_body;
        request_body["model"] = model;

        nlohmann::json messages_array = nlohmann::json::array();
        for (const auto& msg : messages) {
            nlohmann::json message_json;
            message_json["role"] = msg.role_to_string(); // 直接使用，会得到 "assistant"
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

        // --- 核心修正逻辑：提取系统提示 ---
        auto it = std::find_if(messages.begin(), messages.end(), [](const ChatMessage& msg) {
            return msg.role == MessageRole::SYSTEM;
        });
        if (it != messages.end()) {
            system_prompt_content = it->content + "\n\n"; // 提取内容并加换行
        }
        // --- 结束提取 ---

        bool first_user_message = true;
        for (const auto& msg : messages) {
            // 跳过原始的system message
            if (msg.role == MessageRole::SYSTEM) {
                continue; 
            }
            
            nlohmann::json content_item;
            
            if (msg.role == MessageRole::ASSISTANT) {
                content_item["role"] = "model";
            } else {
                content_item["role"] = msg.role_to_string();
            }
            
            nlohmann::json parts_array = nlohmann::json::array();

            if (msg.role == MessageRole::TOOL) {
                 parts_array.push_back({
                    {"functionResponse", {
                        {"name", msg.tool_name.value_or("unknown_tool")}, 
                        {"response", nlohmann::json::parse(msg.content, nullptr, false).is_discarded() ? nlohmann::json{{"raw_string_response", msg.content}} : nlohmann::json::parse(msg.content) }
                    }}
                });
            } else if (msg.role == MessageRole::USER && first_user_message) {
                // --- 核心修正逻辑：将系统提示合并到第一个用户消息中 ---
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
        
        // ... (tools 的定义部分保持不变) ...
        if (!tools.empty()) {
            nlohmann::json tools_json = nlohmann::json::array();
            nlohmann::json declarations = nlohmann::json::array();
            for (const auto& tool : tools) {
                nlohmann::json tool_def;
                tool_def["name"] = tool.name;
                tool_def["description"] = tool.description;
                
                nlohmann::json params;
                params["type"] = "object";
                params["properties"] = nlohmann::json::object();
                nlohmann::json required_params = nlohmann::json::array();

                for (const auto& p : tool.parameters) {
                    params["properties"][p.name] = {{"type", p.type}, {"description", p.description}};
                    if (p.required) {
                        required_params.push_back(p.name);
                    }
                }
                if (!required_params.empty()) {
                     params["required"] = required_params;
                }
                tool_def["parameters"] = params;
                declarations.push_back(tool_def);
            }
            tools_json.push_back({{"functionDeclarations", declarations}});
            request_body["tools"] = tools_json;
        }
        return request_body;
    }

    LLMResponse parse_response(const nlohmann::json& response_body) override {
        LLMResponse result;
        if (response_body.contains("candidates") && !response_body["candidates"].empty()) {
            auto content = response_body["candidates"][0]["content"];
            if (content.contains("parts") && !content["parts"].empty()) {
                for (const auto& part : content["parts"]) {
                    if (part.contains("text")) {
                        result.assistant_content += part["text"].get<std::string>();
                    }
                    if (part.contains("functionCall")) {
                        if (!result.tool_requests.has_value()) {
                            result.tool_requests = std::vector<ToolCallRequest>{};
                        }
                        ToolCallRequest req;
                        req.id = "call_" + std::to_string(std::rand());
                        req.toolname = part["functionCall"]["name"].get<std::string>();
                        req.arguments = part["functionCall"]["args"];
                        result.tool_requests->push_back(req);
                    }
                }
            }
        }
        return result;
    }
};

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
    } else if (provider_ == LLMProvider::GEMINI) {
        api_handler_ = std::make_unique<GeminiHandler>();
    } else {
        throw std::invalid_argument("Unsupported LLM provider.");
    }

    http_client_config config;
    config.set_timeout(std::chrono::seconds(timeout_seconds_));

    // ======================= 最终更正版方案 START =======================
    // 将健壮的 SSL/TLS 配置应用到所有 HTTPS 请求
    config.set_ssl_context_callback([](boost::asio::ssl::context& ctx) {
        try {
            ctx.set_options(
                boost::asio::ssl::context::default_workarounds |
                boost::asio::ssl::context::no_sslv2 |
                boost::asio::ssl::context::no_sslv3 |
                boost::asio::ssl::context::no_tlsv1 |
                boost::asio::ssl::context::no_tlsv1_1
            );
            ctx.set_verify_mode(boost::asio::ssl::verify_peer);
            ctx.load_verify_file("/etc/ssl/certs/ca-certificates.crt");
        } catch (const std::exception& e) {
            std::cerr << "[!!!] CRITICAL ERROR: Failed to configure SSL context. Exception: " << e.what() << std::endl;
            throw;
        }
    });

    // 手动、精确地分离 Base URI 和 Path，以根除尾部斜杠问题
    web::uri full_uri(utility::conversions::to_string_t(base_url));
    path_ = full_uri.path(); // 获取路径, e.g., "/api/v3/chat/completions"

    // 手动构建不带任何路径的 Base URI 字符串
    utility::string_t base_uri_str = full_uri.scheme() + utility::conversions::to_string_t("://") + full_uri.host();
    if (full_uri.port() > 0) {
        base_uri_str += utility::conversions::to_string_t(":") + utility::conversions::to_string_t(std::to_string(full_uri.port()));
    }

    // 用这个绝对干净的 Base URI 初始化客户端
    client_ = std::make_shared<http_client>(base_uri_str, config);
    // ======================== 最终更正版方案 END =======================
}

void LLMClient::initialize_shared_instance(LLMProvider provider, const std::string& base_url, const std::string& api_key) {
    std::lock_guard<std::mutex> lock(mutex);
    if (!instance) {
        if (base_url.empty() || api_key.empty()) {
            throw std::invalid_argument("LLMClient must be initialized with a non-empty base URL and API key.");
        }
        // Use make_shared with a private constructor trick
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
    // 1. 构建请求 JSON (使用handler)
    nlohmann::json request_body = api_handler_->build_request_body(default_model_, messages, available_tools);
    
    // 2. 准备 HTTP 请求
    http_request request(methods::POST);
    request.set_request_uri(path_);
    // ================== TEMPORARY DEBUGGING OVERRIDE START ==================
    if (provider_ == LLMProvider::GEMINI) {
        std::cout << "[!!!] DEBUG: OVERRIDING REQUEST BODY FOR GEMINI TEST." << std::endl;
        request_body = nlohmann::json::parse(R"({
            "contents": [{"role": "user", "parts": [{"text": "Hello"}]}]
        })");
    }
    // =================== TEMPORARY DEBUGGING OVERRIDE END ===================

    if (provider_ == LLMProvider::GEMINI) {
         request.headers().add(conversions::to_string_t("x-goog-api-key"), conversions::to_string_t(api_key_));
    } else {
        request.headers().add(conversions::to_string_t("Authorization"), conversions::to_string_t("Bearer ") + conversions::to_string_t(api_key_));
    }
    request.headers().add(conversions::to_string_t("Content-Type"), conversions::to_string_t("application/json"));

    std::string request_body_str = request_body.dump();
    request.set_body(conversions::to_string_t(request_body_str));

    Logger::getInstance()->log(std::string("--> Request (") + (provider_ == LLMProvider::GEMINI ? "Gemini" : "DeepSeek") + "):\n" + request_body.dump(4));
    //std::cout << "----------------------------------------------------" << std::endl;

    // 打印出客户端实际配置的目标 URI
    //std::cout << "\n\n--- HTTP CLIENT CONFIGURATION ---\\n" << std::endl;
    //std::cout << "Target URI: " << utility::conversions::to_utf8string(client_->base_uri().to_string()) << std::endl;
    //std::cout << "\n--- END OF CONFIGURATION ---\\n" << std::endl;

    // 使用 to_string() 获取完整的、序列化后的原始 HTTP 请求
    //utility::string_t request_str_t = request.to_string();
    //std::string request_str = utility::conversions::to_utf8string(request_str_t);

    //std::cout << "--- FULL RAW HTTP REQUEST (SERIALIZED BY LIBRARY) ---\\n" << std::endl;
    //std::cout << request_str << std::endl;
    //std::cout << "\n--- END OF RAW HTTP REQUEST ---\\n\n" << std::endl;
    // =================== NEW DEBUG CODE END ===================

    // 3. 发送请求并获取响应
    try {
        http_response response = client_->request(request).get();

        // 4. 处理响应
        if (response.status_code() == status_codes::OK) {
            auto response_body_str = conversions::to_utf8string(response.extract_string().get());
            auto response_json = nlohmann::json::parse(response_body_str);
            
            Logger::getInstance()->log(std::string("--> Request (") + (provider_ == LLMProvider::GEMINI ? "Gemini" : "DeepSeek") + "):\n" + request_body.dump(4));
            
            // 使用handler解析响应
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
            } catch (...) {
                // ignore
            }
            Logger::getInstance()->log("!!! Error: " + error_msg.str());
            throw std::runtime_error(error_msg.str());
        }
    } catch (const web::http::http_exception& e) {
        std::cerr << "\n\n!!! CRITICAL HTTP LIBRARY ERROR !!!" << std::endl;
        std::cerr << "Exception caught: " << e.what() << std::endl;
        std::cerr << "This often indicates a TLS/SSL handshake failure or a network issue." << std::endl;
        std::cerr << "Please check your system's root CA certificates.\n" << std::endl;
        throw; // 重新抛出异常
    }
}

// 配置方法
void LLMClient::set_timeout(long seconds) {
    timeout_seconds_ = seconds;
    http_client_config config;
    config.set_timeout(std::chrono::seconds(timeout_seconds_));
    client_ = std::make_shared<http_client>(client_->base_uri(), config);
}

void LLMClient::set_model(const std::string& model) {
    default_model_ = model;
}

} // namespace llm_client