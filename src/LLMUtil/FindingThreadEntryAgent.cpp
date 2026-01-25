#include "LLMUtil/FindingThreadEntryAgent.h"
#include "LLMUtil/LLMClient.h"
#include "LLMUtil/SharedToolKit.h"
#include "CCPG/CCPG.h"
#include "CCPG/ThreadAPIUtil.h"

#include <iostream>
#include <stdexcept>

namespace llm_client {

// --- Member Variable for Result ---
// We add a member to hold the result from the tool call.
long long found_entry_id_ = -1;

FindingThreadEntryAgent::FindingThreadEntryAgent(CCPG* ccpg, std::shared_ptr<LLMClient> client)
    : Conversation(std::move(client), build_system_prompt()), ccpg_(ccpg) {}

std::string FindingThreadEntryAgent::build_system_prompt() {
    return R"(
You are an expert C/C++ static analysis assistant.
Your mission is to identify the thread entry function from a given thread creation call site (e.g., `pthread_create`, `std::thread`).

CRITICAL OUTPUT FORMAT:
- You MUST use tools.
- You MUST NOT output normal chat text.
- Your ONLY acceptable final action is calling `report_entry_point_id`.

**Your Goal:**
Find the specific function that will be executed by the new thread and report its CPG Method node ID.

IMPORTANT:
- During thread-entry resolution, the callee entry function may NOT be constructed as a CCPG Function yet.
- Therefore, you MUST prefer returning a CPG Method node id, obtained via `get_cpg_method_by_name`.

**Your Workflow:**
1.  **Analyze the Call**: You will be given the source code of the thread creation call. Examine its arguments to identify which one corresponds to the thread entry function. The entry function is typically the 3rd argument for `pthread_create` and the 1st template argument or constructor argument for `std::thread`.

2.  **Handle Direct Calls**: If the argument is a direct function name literal (e.g., `worker_thread`), use `get_cpg_method_by_name` to obtain its `cpg_node_id`, then proceed to Step 4.

3.  **Handle Indirect Calls (Variables/Pointers / Wrapper Functions)**: If the argument is a variable or a complex expression (e.g., `thread_func_ptr`, `task->handler`), you MUST perform these sub-steps:
    a. **Get Context**: Use the `get_function` tool on the node ID of the thread creation call. This will give you the full source code of the calling function.
    b. **Analyze the Context for clues about the variable's value**:
        i. **Local Assignment**: Look for assignments to the variable within the function body. For example, `thread_func_ptr = worker_thread;`.
        ii. **Function Parameter**: If the variable is a parameter of the function (common wrapper pattern), you MUST:
            - use `get_function` result's `function_id` (the wrapper function id),
            - use `get_callers(function_id)` to list its callers,
            - for each caller, use `get_function_ops(caller_function_id)` to read the caller body (operations list),
            - locate the wrapper call (e.g., `create_worker(X, ...)`) and extract the argument passed to the parameter,
            - then use `get_cpg_method_by_name` on that extracted function name.
        iii. **Struct/Class Member**: If the variable is a member of a struct or class (e.g., `task->handler`), look for where this struct/class is initialized and where the member is assigned.
    c. **Find the Function**: Once you've identified the true function name, use `get_cpg_method_by_name` to get its `cpg_node_id`.

4.  **Report the Result**:
    - Prefer reporting a **CPG Method node id** (works even if the function is not in CCPG yet, e.g., function pointer entry).
    - Use `get_cpg_method_by_name` to obtain the `cpg_node_id`, then call `report_entry_point_id` with that integer ID.
    - If you are absolutely certain you already have a correct CPG Method node id from elsewhere, you may report it directly.

    **CRITICAL**: If you cannot determine the entry point after following all steps, or if the code is too complex to statically analyze, you **MUST** call `report_entry_point_id` with `function_id` set to `-1`. Do NOT apologize or explain why you failed. Just report -1. It is perfectly normal and expected to not find the entry point in some cases.
)";
}

std::string FindingThreadEntryAgent::get_tool_choice() const {
    // Force the model to emit tool calls when tools are provided (OpenAI-compatible APIs).
    // Without this, some models occasionally return normal chat text and never call
    // `report_entry_point_id`, which makes entry resolution silently fail (-1).
    return "required";
}

std::vector<Tool> FindingThreadEntryAgent::get_available_tools() const {
    // IMPORTANT: During thread-entry resolution, the target entry may not exist as a CCPG Function yet.
    // Therefore we intentionally expose only tools that are safe and sufficient:
    // - get_function (resolve wrapper function)
    // - get_callers / get_function_ops (inspect wrapper callsites in existing CCPG)
    // - get_cpg_method_by_name (resolve final entry as CPG Method node id)
    std::vector<Tool> tools;
    tools.push_back({"get_function", "Get the function containing a node ID.", {
        {"node_id", "number", "The ID of the node within the target function.", true}
    }});
    tools.push_back({"get_function_ops", "Get a function's operation nodes (CFG/CCPG nodes) with code & locations (useful when the raw function body is truncated).", {
        {"function_id", "number", "The ID of the function to inspect.", true}
    }});
    tools.push_back({"get_callers", "Get all functions that call a given function.", {
        {"function_id", "number", "The ID of the function being called.", true}
    }});
    tools.push_back({"get_cpg_method_by_name", "Get CPG Method node(s) by exact name (returns CPG node IDs; useful when the function isn't in CCPG yet, e.g., thread entry passed as function pointer).", {
        {"name", "string", "The exact method/function name to find.", true}
    }});
    tools.push_back(
        {
            "report_entry_point_id",
            "CRITICAL: Call this tool to report the final node ID of the identified thread entry function.",
            {
                Parameter("function_id", "integer", "The node ID of the thread entry function.", true)
            }
        }
    );
    return tools;
}

std::string FindingThreadEntryAgent::execute_tool(const std::string& tool_name, const nlohmann::json& arguments) {
    // Track whether the model actually attempted the required exploration steps.
    if (tool_name == "get_function") {
        attempted_get_function_ = true;
    } else if (tool_name == "get_callers" || tool_name == "get_function_ops" || tool_name == "get_function_by_id") {
        attempted_call_graph_ = true;
    } else if (tool_name == "get_function_by_name") {
        attempted_name_lookup_ = true;
    } else if (tool_name == "get_cpg_method_by_name") {
        attempted_cpg_lookup_ = true;
    }

    // Special handling: remember the last successful CPG method candidate id.
    if (tool_name == "get_cpg_method_by_name") {
        auto shared_result = SharedToolKit::handle_shared_tool(tool_name, arguments, ccpg_);
        if (shared_result) {
            try {
                auto parsed = nlohmann::json::parse(*shared_result, nullptr, false);
                if (parsed.is_array() && !parsed.empty() && parsed[0].contains("cpg_node_id") && parsed[0]["cpg_node_id"].is_number()) {
                    last_candidate_cpg_method_id_ = parsed[0]["cpg_node_id"].get<long long>();
                }
            } catch (...) {
                // ignore
            }
            return *shared_result;
        }
        return R"({"error":"Internal: get_cpg_method_by_name failed."})";
    }

    if (tool_name == "report_entry_point_id") {
        if (arguments.contains("function_id") && arguments["function_id"].is_number()) {
            // CPG node IDs can exceed 32-bit. Use 64-bit to avoid overflow.
            found_entry_id_ = arguments["function_id"].get<long long>();

            // If we already found a concrete CPG method candidate, do not allow reporting -1.
            if (found_entry_id_ == -1 && last_candidate_cpg_method_id_ > 0) {
                return "{\"error\":\"A valid CPG method candidate was found. Do NOT report -1. Report the candidate cpg_node_id instead.\","
                       "\"candidate_cpg_node_id\":" + std::to_string(last_candidate_cpg_method_id_) + "}";
            }

            // Guardrail: do not allow premature failure (-1) without any real attempt.
            // This prevents the model from immediately returning -1 and skipping the
            // wrapper/caller analysis that is required for cases like create_worker(func,...).
            if (found_entry_id_ == -1) {
                const bool attempted_any_resolution =
                    attempted_get_function_ &&
                    (attempted_call_graph_ || attempted_name_lookup_ || attempted_cpg_lookup_);
                if (!attempted_any_resolution) {
                    found_entry_id_ = -1; // keep as -1, but do NOT finish the loop
                    return "{\"error\":\"Do not report -1 yet. You must attempt resolution using tools first (get_function + get_callers/get_function_ops or get_function_by_name or get_cpg_method_by_name), then retry report_entry_point_id.\","
                           "\"required_attempts\":[\"get_function\",\"one_of(get_callers|get_function_ops|get_function_by_name|get_cpg_method_by_name)\"]}";
                }
            }
            return "finish"; 
        }
        return "Error: 'function_id' is a required integer argument.";
    }

    auto shared_result = SharedToolKit::handle_shared_tool(tool_name, arguments, ccpg_);
    if (shared_result) {
        return *shared_result;
    }

    return "Error: Unknown tool name '" + tool_name + "'.";
}

// This function is now simplified. The result is captured by the tool call.
std::string FindingThreadEntryAgent::parseResult(const std::vector<ChatMessage>& history) {
    // The result is now stored in found_entry_id_, not parsed from the final message.
    // We just need a placeholder return.
    return std::to_string(found_entry_id_);
}

long long FindingThreadEntryAgent::find_thread_entry(CCPGNode* node) {
    if (!node) {
        return -1; // Invalid node
    }
    
    // Reset the result for this run.
    found_entry_id_ = -1;
    attempted_get_function_ = false;
    attempted_call_graph_ = false;
    attempted_name_lookup_ = false;
    attempted_cpg_lookup_ = false;
    last_candidate_cpg_method_id_ = -1;

    std::string user_prompt = "Analyze the following thread creation call:\n"
                            "Source code: " + node->getCPGNode()->getCode() + "\n"
                            "Node ID: " + std::to_string(node->getId());

    // send_message will now handle the tool-calling loop.
    // When the LLM calls "report_entry_point_id", execute_tool will set found_entry_id_
    // and return "finish", which should terminate the loop.
    send_message(user_prompt);

    // After the conversation, the result is in our member variable.
    return found_entry_id_;
}

} // namespace llm_client
