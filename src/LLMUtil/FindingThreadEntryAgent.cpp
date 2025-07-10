#include "LLMUtil/FindingThreadEntryAgent.h"
#include "CCPG/ThreadCreationTree.h"
#include "CCPG/CCPG.h"
#include "CCPG/CCPGNode.h"

namespace llm_client {

FindingThreadEntryAgent::FindingThreadEntryAgent(std::shared_ptr<LLMClient> client)
    : Conversation(client, build_system_prompt(), 15)
{
    // Additional initialization if needed
}

std::string FindingThreadEntryAgent::build_system_prompt() {
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

std::vector<Tool> FindingThreadEntryAgent::get_available_tools() const {
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

int FindingThreadEntryAgent::find_thread_entry(CCPGNode * node) {
    if (!node) {
        return -1; // Invalid node
    }

    std::string user_prompt = "Analyze the following node to find the thread entry point: \nSource code: " + node->getCPGNode()->getCode() + 
                            "\nNodeID: " + std::to_string(node->getId()) +
                            "\nNode Type: " + ThreadAPIUtil::getTypeString(node->getType());

    std::string result = send_message(user_prompt, nullptr);

    // Assuming the result is the function ID, you might want to parse and return it.
    // For now, returning 0 for success as in the original code.
    return 0;
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
                ToolCallRequest tool_call = msg.tool_calls->front();
                if(tool_call.toolname == "confirm_thread_entry"){
                    // Assuming arguments is a JSON object, and we need to get the value of "entry_function_id"
                    if (tool_call.arguments.is_object() && tool_call.arguments.contains("entry_function_id")) {
                        return tool_call.arguments["entry_function_id"].dump();
                    }
                }
            }
        }
    }
    return "No valid assistant response found.";   
}

} // namespace llm_client
