#include "LLMUtil/FindingThreadEntryAgent.h"
#include "LLMUtil/SharedToolKit.h"
#include "CCPG/CCPG.h"
#include "CCPG/CCPGNode.h"
#include "CCPG/ThreadAPIUtil.h"
#include <iostream>
#include <stdexcept>

namespace llm_client {

FindingThreadEntryAgent::FindingThreadEntryAgent(CCPG* ccpg, std::shared_ptr<LLMClient> client)
    : Conversation(client, build_system_prompt(), 15), ccpg_(ccpg)
{
}

std::string FindingThreadEntryAgent::build_system_prompt() {
    return R"(
You are a multithreaded program analysis expert responsible for identifying the entry function of a thread.
Your objective is to determine the entry function of the thread created by a given fork statement (e.g., `pthread_create`).
The entry function is the first function executed by the new thread.

You have access to a set of tools to query the source code's structure.

**Your Workflow:**
1.  You will be given the source code of the fork statement.
2.  Use the available tools (`get_function`, `get_function_by_name`, `get_callees`) to trace back the origin of the function pointer passed to the fork statement.
3.  Once you are confident you have identified the correct entry function, you MUST call the `confirm_thread_entry` tool to finalize the analysis.

**Example Analysis:**
- If the entry is a direct function name like `worker_thread`, use `get_function_by_name` to find its ID.
- If the entry is a variable, you may need to analyze the calling function to find where that variable was assigned.
)";
}

std::vector<Tool> FindingThreadEntryAgent::get_available_tools() const {
    // Start with the shared tools
    auto tools = SharedToolKit::get_shared_tools();

    // Add agent-specific tools
    tools.push_back(
        {"confirm_thread_entry", "Confirm the thread entry function by providing its ID.",
        {
            {"entry_function_id", "number", "The ID of the identified thread entry function.", true}
        }}
    );
    
    return tools;
}

int FindingThreadEntryAgent::find_thread_entry(CCPGNode* node) {
    if (!node) {
        return -1; // Invalid node
    }

    std::string user_prompt = "Analyze the following node to find the thread entry point: \nSource code: " + node->getCPGNode()->getCode() +
                            "\nNodeID: " + std::to_string(node->getId()) +
                            "\nNode Type: " + ThreadAPIUtil::getTypeString(node->getType());

    // Pass the CCPG context to the conversation for the tools to use.
    std::string result_str = send_message(user_prompt, ccpg_);

    try {
        // The result from parseResult will be the ID as a string.
        return std::stoi(result_str);
    } catch (const std::invalid_argument& ia) {
        std::cerr << "Error: Could not parse thread entry ID from LLM response: " << result_str << std::endl;
        return -1;
    } catch (const std::out_of_range& oor) {
        std::cerr << "Error: Thread entry ID from LLM response is out of range: " << result_str << std::endl;
        return -1;
    }
}

std::string FindingThreadEntryAgent::execute_tool(const std::string& tool_name, const nlohmann::json& arguments) {
    // First, try to handle it as a shared tool
    auto shared_result = SharedToolKit::handle_shared_tool(tool_name, arguments, ccpg_);
    if (shared_result) {
        return *shared_result;
    }

    // If not a shared tool, handle agent-specific tools
    if (tool_name == "confirm_thread_entry") {
        // The result is stored in the context of the parseResult function.
        // Here we just signal that the conversation is over.
        return "finish";
    }

    nlohmann::json error_resp;
    error_resp["error"] = "Tool '" + tool_name + "' not found or not implemented by this agent.";
    error_resp["arguments_received"] = arguments;
    return error_resp.dump();
}

std::string FindingThreadEntryAgent::parseResult(const std::vector<ChatMessage>& history) {
    if (!history.empty()) {
        // Search backwards for the last assistant message that made the final tool call
        for (auto it = history.rbegin(); it != history.rend(); ++it) {
            const auto& msg = *it;
            if (msg.role == MessageRole::ASSISTANT && msg.tool_calls.has_value()) {
                for (const auto& tool_call : *msg.tool_calls) {
                    if (tool_call.toolname == "confirm_thread_entry") {
                        if (tool_call.arguments.is_object() && tool_call.arguments.contains("entry_function_id")) {
                            return tool_call.arguments["entry_function_id"].dump();
                        }
                    }
                }
            }
        }
    }
    // Return an invalid ID if no confirmation was found
    return "-1";
}

} // namespace llm_client
