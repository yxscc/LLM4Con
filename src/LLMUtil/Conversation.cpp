#include <fstream>
#include "CCPG/ThreadCreationTree.h"

namespace llm_client{

Conversation::Conversation(
        std::shared_ptr<LLMClient> client,
        const std::string& system_prompt,
        size_t max_history
    ) : client_(client),
        base_system_prompt_(system_prompt),
        max_history_messages_(max_history),
        tool_execution_context_(nullptr) {
        // 其他初始化逻辑（如果需要）
    }

std::string Conversation::send_message(const std::string& user_message, void* context_for_tools) {
    this->tool_execution_context_ = context_for_tools;

    std::string effective_sys_prompt = build_effective_system_prompt();
    if (history_.empty() || history_[0].role != MessageRole::SYSTEM) {
        history_.insert(history_.begin(), {MessageRole::SYSTEM, effective_sys_prompt});
    } else {
        history_[0].content = effective_sys_prompt; // Update if tools/prompt changed
    }

    history_.push_back({MessageRole::USER, user_message, std::nullopt, std::nullopt});
    prune_history();

    while (true) {
        std::vector<Tool> current_tools = get_available_tools(); // Get tools for current context/agent
        LLMClient::LLMResponse llm_response = client_->chat(history_, current_tools);

        ChatMessage assistant_message = {MessageRole::ASSISTANT, llm_response.assistant_content};
        if (llm_response.tool_requests && !llm_response.tool_requests->empty()) {
            assistant_message.tool_calls = llm_response.tool_requests;
        }
        history_.push_back(assistant_message);
        prune_history();

        if (!llm_response.tool_requests || llm_response.tool_requests->empty()) {
            // No tool calls requested, this is the final assistant response for this turn
            this->tool_execution_context_ = nullptr; // Clear context after turn
            return llm_response.assistant_content;
        }

        // Process tool calls
        // Note: Some LLMs might allow parallel tool calls.
        // Here, we process them and add results, then let LLM decide next step.
        for (const auto& tool_req : *llm_response.tool_requests) {
            std::string tool_result_content = execute_tool(tool_req.toolname, tool_req.arguments);
            history_.push_back({MessageRole::TOOL, tool_result_content, std::nullopt, tool_req.id});
            prune_history();
            if(tool_result_content == "finish"){
                // Special case to end the conversation
                this->tool_execution_context_ = nullptr; // Clear context after turn
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

int FindingThreadEntryAgent::find_thread_entry(CCPGNode * node) {
    
    if (!node) {
        return -1; // Invalid node
    }

    std::string user_prompt = "Analyze the following node to find the thread entry point: \nSource code: " + node->getCPGNode()->getCode() + 
                            "\nNodeID: " + std::to_string(node->getId()) +
                            "\nNode Type: " + ThreadAPIUtil::getTypeString(node->getType());

    std::string result = send_message(user_prompt, nullptr);


    return 0; // Indicating success

}

std::string FindingThreadEntryAgent::execute_tool(const std::string& tool_name, const nlohmann::json& arguments) {
    CCPG * ccpg = ThreadCreationTree::getInstance()->getCCPG();
    const CPG * cpg = ccpg->getCPG();

    if (tool_name == "get_function") {
        nlohmann::json result = nlohmann::json::object();
        int node_id = arguments.at("node_id").get<int>();
        CCPGNode* function_node = ccpg->getNodeByID(node_id);
        ccpg::Function * function = function_node ? function_node->getFunction() : nullptr;
        if (function) {
            result["function_id"] = function->getId();
            result["function_body"] = function->getFuncNode()->getCPGNode()->getCode();
            return result.dump();
        } else {
            return R"({"error": "Function not found for node ID: )" + std::to_string(node_id) + R"("})";
        }
    } else if (tool_name == "get_callers") {
        int function_id = arguments.at("function_id").get<int>();
        ccpg::Function * function = ccpg->getFunctionById(function_id);
        CCPGNodeSet callsites = function->getCallSites();
        nlohmann::json result = nlohmann::json::array();
        for (const auto& callsite : callsites) {
            nlohmann::json callsite_info = {
                {"callsite_node_id", callsite->getId()},
                {"callsite_code", callsite->getCPGNode()->getCode()},
            };
            result.push_back(callsite_info);
        }
        return result.dump();
    } else if (tool_name == "confirm_thread_entry") {
        int entry_function_id = arguments.at("entry_function_id").get<int>();
        last_entry_point_ = std::to_string(entry_function_id);
        return "finish";
    } else if (tool_name == "get_function_by_name") {
        std::string name = arguments.at("name").get<std::string>();
        std::unordered_set<Node *> nodes = cpg->findMethodsByName(name);
        nlohmann::json functions = nlohmann::json::array();
        for(Node * node : nodes) {
            CCPGNode * function_node = ccpg->getCCPGNodeByCPGNode(node);
            ccpg::Function * function = function_node ? function_node->getFunction() : nullptr;
            if (function_node) {
                nlohmann::json function_info = {
                    {"function_id", function->getId()},
                    {"function_body", function->getFuncNode()->getCPGNode()->getCode()},
                };
                functions.push_back(function_info);
            }
        }
        return functions.dump();
    }

    // Default case for unrecognized tools
    nlohmann::json error_resp;
    error_resp["error"] = "Tool '" + tool_name + "' not found or not implemented by this agent.";
    error_resp["arguments_received"] = arguments;
    return error_resp.dump();
}

std::string FindingThreadEntryAgent::parseResult(const std::vector<ChatMessage>& history) {
    if (!history.empty()) {
        for(auto it = history.rbegin(); it != history.rend(); ++it) {
            ChatMessage msg = *it;
            if (msg.role == MessageRole::ASSISTANT && msg.tool_calls.has_value() && !msg.tool_calls->empty()) {
                // Return the first tool call's content (adjust based on your actual ToolCall structure)
                ToolCallRequest tool_call = msg.tool_calls->front();
                if(tool_call.toolname == "confirm_thread_entry"){
                    return tool_call.arguments["entry_function_id"];
                }
            }
        }
    }
    return "No valid assistant response found.";   
}

}
