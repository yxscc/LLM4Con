#include <fstream>
#include "CCPG/ThreadCreationTree.h"
#include "LLMUtil/Conversation.h"
#include "Util/Logger.h"
#include <algorithm>
#include <cstdlib>
#include <iostream>

namespace llm_client{

namespace {
bool mirrorTraceToStderr() {
    const char* v = std::getenv("LACE_TRACE_STDERR");
    return v && std::string(v) != "0";
}

std::string compactTrace(const std::string& text, std::size_t limit = 360) {
    std::string out = text.substr(0, std::min(limit, text.size()));
    std::replace(out.begin(), out.end(), '\n', ' ');
    if (text.size() > limit) out += " ...";
    return out;
}
} // namespace

Conversation::Conversation(
        std::shared_ptr<LLMClient> client,
        const std::string& system_prompt,
        size_t max_history
    ) : client_(client),
        base_system_prompt_(system_prompt),
        max_history_messages_(max_history){
            fs::path log_path = TargetPath::getInstance()->getOutputDir() / "llm_simplified_trace.log";
            simplified_log_file_.open(log_path, std::ios_base::app);
        }

std::string Conversation::send_message(const std::string& user_message, void* context_for_tools) {
    this->context_for_tools_ = context_for_tools; // Store context for tool execution

    std::string effective_sys_prompt = build_effective_system_prompt();
    if (history_.empty() || history_[0].role != MessageRole::SYSTEM) {
        history_.insert(history_.begin(), {MessageRole::SYSTEM, effective_sys_prompt});
    } else {
        history_[0].content = effective_sys_prompt; // Update if tools/prompt changed
    }

    {
        ChatMessage user_msg{MessageRole::USER, user_message, std::nullopt, std::nullopt};
        if (pin_next_user_) {
            user_msg.pinned = true;   // keep task setup / contracts under token budget
            pin_next_user_ = false;
        }
        history_.push_back(std::move(user_msg));
    }
    if (simplified_log_file_.is_open()) {
        simplified_log_file_ << "User: " << user_message << std::endl;
    }
    if (mirrorTraceToStderr()) {
        std::cerr << "[trace] User: " << compactTrace(user_message) << std::endl;
    }
    prune_history();

    int turn = 0;
    while (true) {
        // Hard per-call turn cap (opt-in). Bounds pathological sessions where
        // the model keeps calling uncapped reporting tools without ever
        // finishing. Already-committed tool side effects (e.g. proposed
        // hypotheses) are preserved; we just stop asking the model for more.
        if (max_turns_ > 0 && turn >= max_turns_) {
            if (simplified_log_file_.is_open()) {
                simplified_log_file_ << "[turn-cap] reached max_turns_=" << max_turns_
                                     << ", terminating session" << std::endl;
            }
            if (mirrorTraceToStderr()) {
                std::cerr << "[trace] turn-cap reached (" << max_turns_
                          << "), terminating session" << std::endl;
            }
            return parseResult(history_);
        }
        ++turn;
        std::string dynamic_sys_prompt = build_effective_system_prompt();
        if (!history_.empty() && history_[0].role == MessageRole::SYSTEM) {
            history_[0].content = dynamic_sys_prompt;
        }
        std::vector<Tool> current_tools = get_available_tools(); // Get tools for current context/agent
        LLMClient::LLMResponse llm_response = client_->chat(history_, current_tools, get_tool_choice());

        ChatMessage assistant_message = {MessageRole::ASSISTANT, llm_response.assistant_content};
        if (llm_response.tool_requests && !llm_response.tool_requests->empty()) {
            assistant_message.tool_calls = llm_response.tool_requests;
            for (const auto& tool_req : *llm_response.tool_requests) {
                if (simplified_log_file_.is_open()) {
                    simplified_log_file_ << "LLM Tool Call: " << tool_req.toolname << " with args " << tool_req.arguments.dump() << std::endl;
                }
                if (mirrorTraceToStderr()) {
                    std::cerr << "[trace] LLM Tool Call: " << tool_req.toolname
                              << " args=" << compactTrace(tool_req.arguments.dump(), 240)
                              << std::endl;
                }
            }
        }
        history_.push_back(assistant_message);
        prune_history();

        if (!llm_response.tool_requests || llm_response.tool_requests->empty()) {
            return llm_response.assistant_content;
        }

        for (const auto& tool_req : *llm_response.tool_requests) {
            std::string tool_result_content = execute_tool(tool_req.toolname, tool_req.arguments);
            if (simplified_log_file_.is_open()) {
                simplified_log_file_ << "Tool Result: " << tool_result_content << std::endl;
            }
            if (mirrorTraceToStderr()) {
                std::cerr << "[trace] Tool Result: " << tool_req.toolname
                          << " bytes=" << tool_result_content.size()
                          << " " << compactTrace(tool_result_content, 240)
                          << std::endl;
            }
            ChatMessage tool_response_msg;
            tool_response_msg.role = MessageRole::TOOL;
            tool_response_msg.content = tool_result_content;
            tool_response_msg.tool_call_id = tool_req.id;
            tool_response_msg.tool_name = tool_req.toolname; // <-- 新增的赋值

            history_.push_back(tool_response_msg);
            prune_history();
            if(tool_result_content == "finish"){
                return parseResult(history_);
            }
        }
        // Loop back to send tool results to LLM
    }
}

void Conversation::set_system_prompt(const std::string& prompt) {
    base_system_prompt_ = prompt;
    // Force update or insertion of system prompt
    std::string effective_sys_prompt = build_effective_system_prompt();
    if (!history_.empty() && history_[0].role == MessageRole::SYSTEM) {
        history_[0].content = effective_sys_prompt;
    } else {
            // If history is not empty but has no system prompt, or if it's empty
        history_.insert(history_.begin(), {MessageRole::SYSTEM, effective_sys_prompt});
    }
    prune_history(); // Ensure it's still within limits
}

void Conversation::reset() {
    history_.clear();
}

}
