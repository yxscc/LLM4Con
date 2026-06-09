#include "LLMUtil/LLMClient.h"
#include <algorithm>
#include <nlohmann/json.hpp>
#include "Util/Logger.h"
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <array>
#include <iostream>
#include <utility>
#include <thread> 
#include <chrono> 
#include <unistd.h> 
#include <sys/wait.h> 
#include <filesystem> 
#include <vector>  
#include <cerrno>   
#include <cstring>  

namespace llm_client {

std::shared_ptr<LLMClient> LLMClient::instance = nullptr;
std::mutex LLMClient::mutex;

void LLMClient::reset_instance() {
    std::lock_guard<std::mutex> lock(mutex);
    if (instance) {
        instance.reset(); // 销毁当前实例并将 shared_ptr 置为 nullptr
    }
}

// --- GPT5's Robust Helper Functions ---

/**
 * @brief Executes a shell command and captures its combined stdout and stderr, along with the exit code.
 * @param cmd The command to execute.
 * @return A pair containing the command's output and its exit code.
 */
static std::pair<std::string, int> exec_with_status(const std::string& cmd) {
    std::array<char, 4096> buffer;
    std::string result;
    // Note: "2>&1" redirects stderr to stdout, so we capture everything.
    std::string cmd_with_stderr = cmd + " 2>&1";
    FILE* pipe = popen(cmd_with_stderr.c_str(), "r");
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (true) {
        size_t n = fread(buffer.data(), 1, buffer.size(), pipe);
        if (n > 0) {
            result.append(buffer.data(), n);
        }
        if (n < buffer.size()) {
            if (feof(pipe)) break;
            if (ferror(pipe)) {
                 // You might want to log an error here, but we'll get the exit code anyway.
                 break;
            }
        }
    }
    int status = pclose(pipe);
    int exit_code = -1;
    if (WIFEXITED(status)) {
        exit_code = WEXITSTATUS(status);
    }
    return {result, exit_code};
}

/**
 * @brief Creates a temporary file with a unique name and writes the payload to it.
 * @param payload The string content to write to the file.
 * @return The unique path to the created temporary file.
 */
static std::string write_temp_json_unique(const std::string& payload) {
    // 1. Get the system's temporary directory path portably.
    std::filesystem::path temp_dir = std::filesystem::temp_directory_path();
    // **FIX:** The template for mkstemp MUST end in "XXXXXX". The ".json" suffix was incorrect.
    std::string temp_template_str = (temp_dir / "llm_req_XXXXXX").string();

    // 2. mkstemp requires a mutable C-string, so we use a vector.
    std::vector<char> tmpl(temp_template_str.begin(), temp_template_str.end());
    tmpl.push_back('\0'); // Null-terminate

    int fd = mkstemp(tmpl.data());
    if (fd == -1) {
        // Provide a more informative error message.
        throw std::runtime_error("mkstemp failed in directory '" + temp_dir.string() + "'. Error: " + std::strerror(errno));
    }

    // The vector `tmpl` now contains the actual unique filename.
    std::string unique_filename(tmpl.data());

    FILE* f = fdopen(fd, "w");
    if (!f) {
        close(fd);
        std::remove(unique_filename.c_str());
        throw std::runtime_error("fdopen failed for temp file: " + unique_filename);
    }

    size_t n = fwrite(payload.data(), 1, payload.size(), f);
    fclose(f); // This also closes fd

    if (n != payload.size()) {
        std::remove(unique_filename.c_str());
        throw std::runtime_error("failed to write request body completely to " + unique_filename);
    }

    return unique_filename;
}

/**
 * @brief Escapes single quotes in a string for safe use in a shell command.
 * @param s The string to escape.
 * @return The escaped string.
 */
static std::string sh_escape_single_quotes(const std::string& s) {
    std::string out;
    out.reserve(s.size() + 8);
    for (char c : s) {
        if (c == '\'') {
            out += "'\\''";
        } else {
            out += c;
        }
    }
    return out;
}

/**
 * @brief Sanitizes a string to ensure valid UTF-8 encoding.
 * Replaces invalid UTF-8 sequences with replacement character.
 */
static std::string sanitize_utf8_string(const std::string& input) {
    std::string output;
    output.reserve(input.size());
    size_t i = 0;
    while (i < input.size()) {
        unsigned char c = static_cast<unsigned char>(input[i]);
        if (c < 0x80) {
            // ASCII: keep printable chars and common whitespace
            if (c >= 0x20 || c == '\n' || c == '\t' || c == '\r') {
                output += static_cast<char>(c);
            } else {
                output += ' '; // Replace control chars with space
            }
            i++;
        } else if ((c & 0xE0) == 0xC0) {
            // 2-byte UTF-8
            if (i + 1 < input.size() && (static_cast<unsigned char>(input[i + 1]) & 0xC0) == 0x80) {
                output += input[i];
                output += input[i + 1];
                i += 2;
            } else {
                output += '?';
                i++;
            }
        } else if ((c & 0xF0) == 0xE0) {
            // 3-byte UTF-8
            if (i + 2 < input.size() &&
                (static_cast<unsigned char>(input[i + 1]) & 0xC0) == 0x80 &&
                (static_cast<unsigned char>(input[i + 2]) & 0xC0) == 0x80) {
                output += input[i];
                output += input[i + 1];
                output += input[i + 2];
                i += 3;
            } else {
                output += '?';
                i++;
            }
        } else if ((c & 0xF8) == 0xF0) {
            // 4-byte UTF-8
            if (i + 3 < input.size() &&
                (static_cast<unsigned char>(input[i + 1]) & 0xC0) == 0x80 &&
                (static_cast<unsigned char>(input[i + 2]) & 0xC0) == 0x80 &&
                (static_cast<unsigned char>(input[i + 3]) & 0xC0) == 0x80) {
                output += input[i];
                output += input[i + 1];
                output += input[i + 2];
                output += input[i + 3];
                i += 4;
            } else {
                output += '?';
                i++;
            }
        } else {
            // Invalid UTF-8 start byte
            output += '?';
            i++;
        }
    }
    return output;
}

/**
 * @brief Recursively sanitizes all strings in a JSON object for valid UTF-8.
 */
static void sanitize_json_utf8(nlohmann::json& j) {
    if (j.is_string()) {
        j = sanitize_utf8_string(j.get<std::string>());
    } else if (j.is_array()) {
        for (auto& elem : j) {
            sanitize_json_utf8(elem);
        }
    } else if (j.is_object()) {
        for (auto& [key, val] : j.items()) {
            sanitize_json_utf8(val);
        }
    }
}

// Repair an OpenAI-style message history so that:
//   (a) every ASSISTANT.tool_call has a matching following TOOL response, and
//       conversely every TOOL has a matching preceding ASSISTANT.tool_call;
//   (b) every tool_call.id is globally unique across the entire conversation.
//
// (a) is needed because Conversation::prune_history() can drop ASSISTANT or TOOL
// messages mid-stream when history exceeds max_history.
// (b) is needed because some LLMs (notably Claude) sometimes generate the same
// tool_use_id for repeated calls of the same tool with the same args, and
// upstream gateways (OpenAI -> Anthropic translators) reject duplicates with
// "unexpected `tool_use_id` found in `tool_result` blocks: tooluse_xxx".
//
// The function rewrites every tool_call.id to a fresh "lacecall_<N>" id and
// rewrites the matching TOOL.tool_call_id (FIFO match against the most recent
// ASSISTANT's tool_calls). Orphan TOOLs are dropped; orphan ASSISTANT.tool_calls
// are filled with placeholder TOOL responses so the wire format is self-consistent.
static std::vector<ChatMessage> sanitize_messages_for_tool_calls(
    const std::vector<ChatMessage>& messages) {
    std::vector<ChatMessage> result;
    result.reserve(messages.size() + 4);

    // FIFO of (original_id, rewritten_id) pairs the most recent ASSISTANT
    // requested and have not yet seen a matching TOOL response.
    std::vector<std::pair<std::string, std::string>> pending_renames;
    long long call_counter = 0;
    long long renames_count = 0;
    long long orphans_dropped = 0;
    long long placeholders_injected = 0;

    auto flush_pending = [&](const char* reason) {
        if (pending_renames.empty()) return;
        placeholders_injected += pending_renames.size();
        for (const auto& p : pending_renames) {
            ChatMessage placeholder;
            placeholder.role = MessageRole::TOOL;
            placeholder.content = "[Tool result missing due to history pruning; treat as no-op.]";
            placeholder.tool_call_id = p.second;
            result.push_back(std::move(placeholder));
        }
        pending_renames.clear();
        (void)reason;
    };

    for (const auto& msg_in : messages) {
        ChatMessage msg = msg_in;  // mutable copy for id rewrites
        switch (msg.role) {
            case MessageRole::TOOL: {
                if (!msg.tool_call_id.has_value()) {
                    orphans_dropped++;
                    break;
                }
                const std::string& orig_id = *msg.tool_call_id;
                auto it = std::find_if(pending_renames.begin(), pending_renames.end(),
                    [&](const std::pair<std::string, std::string>& p) {
                        return p.first == orig_id;
                    });
                if (it == pending_renames.end()) {
                    orphans_dropped++;
                    break;
                }
                msg.tool_call_id = it->second;  // rewrite to fresh id
                pending_renames.erase(it);
                result.push_back(msg);
                break;
            }
            case MessageRole::ASSISTANT: {
                flush_pending("new assistant message");
                if (msg.tool_calls.has_value()) {
                    for (auto& tc : *msg.tool_calls) {
                        std::string fresh = "lacecall_" + std::to_string(call_counter++);
                        if (tc.id != fresh) renames_count++;
                        pending_renames.push_back({tc.id, fresh});
                        tc.id = fresh;
                    }
                }
                result.push_back(msg);
                break;
            }
            case MessageRole::USER:
            case MessageRole::SYSTEM:
            default: {
                flush_pending(msg.role_to_string().c_str());
                result.push_back(msg);
                break;
            }
        }
    }
    flush_pending("end of history");

    if (renames_count || orphans_dropped || placeholders_injected) {
        Logger::getInstance()->log(
            "[sanitize] tool_calls rewritten=" + std::to_string(renames_count) +
            ", orphans_dropped=" + std::to_string(orphans_dropped) +
            ", placeholders_injected=" + std::to_string(placeholders_injected));
    }
    return result;
}

// --- OpenAIHandler (handles OpenAI-compatible APIs) ---
class OpenAIHandler : public APIHandler {
public:
    nlohmann::json build_request_body(
        const std::string& model,
        const std::vector<ChatMessage>& messages_raw,
        const std::vector<Tool>& tools,
        const std::string& tool_choice
    ) override {
        // Repair tool_use/tool_result pairing before serialization so upstream
        // gateways (incl. OpenAI->Anthropic translators) cannot reject us for
        // orphan tool_use_id / tool_result mismatches.
        const std::vector<ChatMessage> messages = sanitize_messages_for_tool_calls(messages_raw);

        nlohmann::json request_body;
        request_body["model"] = model;
        request_body["temperature"] = 1.0;

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

                    if (param.raw_schema.has_value()) {
                        param_json = *param.raw_schema;
                    } else {
                        param_json["type"] = param.type;
                        param_json["description"] = param.description;

                        if (param.type == "array" && param.items.has_value() && param.items->get()) {
                            nlohmann::json items_json;
                            const auto& item_schema = **param.items;
                            items_json["type"] = item_schema.type;
                            if (!item_schema.description.empty()) {
                                items_json["description"] = item_schema.description;
                            }
                            param_json["items"] = items_json;
                        }
                    }
                    
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
            // Be explicit: some OpenAI-compatible gateways default tool_choice to "none"
            // even when tools are present. Setting it ensures the model may emit tool_calls.
            request_body["tool_choice"] = tool_choice;
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

// --- GeminiHandler (defines JSON body structure) ---
class GeminiHandler : public APIHandler {
public:
    nlohmann::json build_request_body(
        const std::string& model,
        const std::vector<ChatMessage>& messages,
        const std::vector<Tool>& tools,
        const std::string& /*tool_choice*/
    ) override {
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
        
        if (!tools.empty()) {
            nlohmann::json function_declarations = nlohmann::json::array();
            for (const auto& tool : tools) {
                function_declarations.push_back(tool.to_json());
            }
            request_body["tools"] = nlohmann::json::array({
                {{"functionDeclarations", function_declarations}}
            });
        }
        
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
      timeout_seconds_(120), // Increased default timeout
      base_url_(base_url) 
{
    if (provider_ == LLMProvider::OPENAI) {
        api_handler_ = std::make_unique<OpenAIHandler>();
    } else if (provider_ == LLMProvider::GEMINI) {
        api_handler_ = std::make_unique<GeminiHandler>();
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

LLMClient::LLMResponse LLMClient::chat(
    const std::vector<ChatMessage>& messages,
    const std::vector<Tool>& available_tools,
    const std::string& tool_choice
) {
    nlohmann::json request_body = api_handler_->build_request_body(default_model_, messages, available_tools, tool_choice);
    // Sanitize all strings in JSON to ensure valid UTF-8 before serialization
    sanitize_json_utf8(request_body);
    std::string request_body_str = request_body.dump();
    std::string provider_name = (provider_ == LLMProvider::GEMINI) ? "Gemini" : "OpenAI";
    Logger::getInstance()->log("--> Request (" + provider_name + "):\n" + request_body.dump(4));

    // 1. Create a unique temporary file to avoid race conditions
    std::string tmp_filename = write_temp_json_unique(request_body_str);

    // 2. Construct a robust curl command
    // IMPORTANT:
    // Do NOT use curl's built-in --retry here. When a gateway returns JSON error bodies,
    // curl may emit multiple bodies concatenated together across internal retries, which
    // breaks JSON parsing. We implement retry/backoff ourselves below.
    std::string timeout_opt = " --connect-timeout 10 --max-time " + std::to_string(timeout_seconds_);
    std::string cmd;
    if (provider_ == LLMProvider::GEMINI) {
        cmd = "curl -sS -k" + timeout_opt +
              " -X POST -H \"Content-Type: application/json\"" +
              " -H \"x-goog-api-key: " + sh_escape_single_quotes(api_key_) + "\"" +
              " '" + sh_escape_single_quotes(base_url_) + "'" +
              " -d @" + tmp_filename +
              " 2>&1";
    } else { // OPENAI and compatible APIs
        cmd = "curl -sS -k" + timeout_opt +
              " -X POST -H \"Content-Type: application/json\"" +
              " -H \"Authorization: Bearer " + sh_escape_single_quotes(api_key_) + "\"" +
              " '" + sh_escape_single_quotes(base_url_) + "'" +
              " -d @" + tmp_filename +
              " 2>&1";
    }

    // 3. Execute with retry logic
    const int max_retries = 10;
    int attempt = 0;
    std::string response_str;
    int exit_code = -1;
    nlohmann::json response_json;

    while (attempt < max_retries) {
        auto [out, code] = exec_with_status(cmd);
        response_str = std::move(out);
        exit_code = code;
        
        bool ok_transport = (exit_code == 0) && !response_str.empty();
        if (!ok_transport) {
            Logger::getInstance()->log("curl attempt " + std::to_string(attempt + 1) +
                                       " failed, exit=" + std::to_string(exit_code) +
                                       ", output:\n" + response_str);
        } else {
            // Parse JSON response. If parsing fails, treat it as a transient error and retry.
            response_json = nlohmann::json::parse(response_str, nullptr, false);
            if (response_json.is_discarded()) {
                Logger::getInstance()->log(std::string("<-- Raw response (") + provider_name + "):\n" + response_str);
                ok_transport = false;
            } else if (response_json.contains("error")) {
                // Gateway/API returned an error JSON. Log and retry with backoff.
                Logger::getInstance()->log("<-- Response (" + provider_name + "):\n" + response_json.dump(4));
                ok_transport = false;
            }
        }

        if (ok_transport) {
            break; // Success: parsed JSON and no top-level error
        }

        ++attempt;
        if (attempt < max_retries) {
            // Exponential backoff: 200ms, 800ms, 1800ms, ... up to ~10s
            int sleep_ms = 200 * attempt * attempt;
            if (sleep_ms > 10000) sleep_ms = 10000;
            std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms));
        }
    }

    // Clean up the temporary file
    std::remove(tmp_filename.c_str());

    if (attempt >= max_retries) {
        // Best-effort error reporting: if we ended with a parsed JSON error, keep it.
        if (!response_json.is_discarded() && !response_json.is_null() && response_json.contains("error")) {
            throw std::runtime_error("API request failed after retries (error JSON): " + response_json.dump());
        }
        if (exit_code != 0) {
            throw std::runtime_error("API request failed after retries (curl exit=" + std::to_string(exit_code) + "): " + response_str);
        }
        if (response_str.empty()) {
            throw std::runtime_error("API request failed after retries: empty response from curl.");
        }
        throw std::runtime_error("API request failed after retries: could not parse a valid JSON response.");
    }

    // 4. At this point response_json is parsed and has no top-level "error".
    Logger::getInstance()->log("<-- Response (" + provider_name + "):\n" + response_json.dump(4));
    
    // Extract and accumulate token usage statistics. Contract-generation Phase A
    // can issue several independent chats concurrently, so this shared counter
    // needs a small critical section.
    {
        std::lock_guard<std::mutex> lock(stats_mutex_);
        token_stats_.total_requests++;
        if (response_json.contains("usage")) {
            const auto& usage = response_json["usage"];
            if (usage.contains("prompt_tokens")) {
                token_stats_.total_prompt_tokens += usage["prompt_tokens"].get<size_t>();
            }
            if (usage.contains("completion_tokens")) {
                token_stats_.total_completion_tokens += usage["completion_tokens"].get<size_t>();
            }
        }
    }
    
    return api_handler_->parse_response(response_json);
}


void LLMClient::set_timeout(long seconds) {
    timeout_seconds_ = seconds;
}

void LLMClient::set_model(const std::string& model) {
    default_model_ = model;
}

} // namespace llm_client

