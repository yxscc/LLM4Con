#include "LLMUtil/LLMClient.h"
#include <cpprest/filestream.h>
#include <cpprest/asyncrt_utils.h>
#include <algorithm>
#include <nlohmann/json.hpp>
#include "Util/Logger.h"

using namespace utility;

namespace llm_client {

// 构造函数
LLMClient::LLMClient(
    const std::string& base_url,
    const std::string& api_key,
    const std::string& default_model,
    size_t max_context_length)
    : api_key_(api_key),
      default_model_(default_model),
      max_context_length_(max_context_length),
      timeout_seconds_(30) {
    
    http_client_config config;
    config.set_timeout(std::chrono::seconds(timeout_seconds_));
    client_ = std::make_shared<http_client>(
        utility::conversions::to_string_t(base_url),
        config
    );
}

LLMClient::LLMResponse LLMClient::chat(const std::vector<ChatMessage>& messages, const std::vector<Tool>& available_tools) {
    // 1. 构建请求 JSON
    nlohmann::json request_body;

    // 设置模型
    request_body["model"] = default_model_;

    // 2. 转换 messages 到 JSON 数组
    nlohmann::json messages_array = nlohmann::json::array();
    for (const auto& msg : messages) {
        nlohmann::json message_json;
        message_json["role"] = msg.role_to_string();
        message_json["content"] = msg.content;

        if (msg.role_to_string() == "assistant") {
            if (msg.tool_calls.has_value()) {

                nlohmann::json tool_calls_array = nlohmann::json::array();
                for (const auto& tool_call : *msg.tool_calls) {
                    nlohmann::json tool_call_json;
                    tool_call_json["id"] = tool_call.id; // 工具调用的唯一 ID
                    tool_call_json["type"] = "function";
                    nlohmann::json function_json;
                    function_json["name"] = tool_call.toolname;
                    function_json["arguments"] = tool_call.arguments.dump(); // 如果 arguments 已经是 nlohmann::json，可以直接赋值
                    tool_call_json["function"] = function_json;
                    tool_calls_array.push_back(tool_call_json);
                }
                message_json["tool_calls"] = tool_calls_array;
                message_json["loss_weight"] = 1.0;
            }
        }
        
        if (msg.tool_call_id.has_value()) {
            message_json["tool_call_id"] = *msg.tool_call_id; // 如果有 tool_call_id，添加到消息中
        }

        messages_array.push_back(message_json);
    }
    request_body["messages"] = messages_array;

    // 3. 添加可用的工具（如果有）
    if (!available_tools.empty()) {
        nlohmann::json tools = nlohmann::json::array();
        for (const auto& tool : available_tools) {
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
                param_json["type"] = param.type; // 例如 "string", "integer", "boolean"
                param_json["description"] = param.description;
                if( param.required) {
                    required_params.push_back(param.name);
                }
                parameters_json["properties"][param.name] = param_json;
            }
            parameters_json["required"] = required_params;
            tool_json["parameters"] = parameters_json;
            function["function"] = tool_json;
            tools.push_back(function);
        }
        request_body["tools"] = tools;
    }

    // 4. 准备 HTTP 请求
    http_request request(methods::POST);
    request.headers().add(U("Authorization"), U("Bearer ") + conversions::to_string_t(api_key_));
    request.headers().add(U("Content-Type"), U("application/json"));

    // 将 nlohmann::json 转换为 string 并设置请求体
    std::string request_body_str = request_body.dump();
    request.set_body(conversions::to_string_t(request_body_str));

    // 记录请求
    Logger::getInstance()->log("--> Request:\n" + request_body.dump(4));

    // 5. 发送请求并获取响应
    http_response response = client_->request(request).get();

    // 6. 处理响应
    if (response.status_code() == status_codes::OK) {
        auto response_body_str = conversions::to_utf8string(response.extract_string().get());
        auto response_body = nlohmann::json::parse(response_body_str);

        // 记录响应
        Logger::getInstance()->log("<-- Response:\n" + response_body.dump(4));
        
        LLMResponse result;

        // 提取 assistant 回复内容
        if (response_body.contains("choices") && !response_body["choices"].empty()) {
            auto message = response_body["choices"][0]["message"];
            if(message.contains("content") && !message["content"].is_null()){
                 result.assistant_content = message["content"].get<std::string>();
            }

            // 提取 tool_calls（如果有）
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
            // ignore if body cannot be extracted
        }
        Logger::getInstance()->log("!!! Error: " + error_msg.str());
        throw std::runtime_error(error_msg.str());
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
