#include "LLMUtil/ParallelAnalysisAgent.h"
#include "CCPG/ThreadCreationTree.h"
#include "CCPG/CCPG.h"
#include "CCPG/CCPGNode.h"

namespace llm_client {

// --- Implementation for ParallelAnalysisAgent ---

ParallelAnalysisAgent::ParallelAnalysisAgent(std::shared_ptr<LLMClient> client)
    : Conversation(client, build_system_prompt(), 20) {
    // Initialization if needed
}

std::string ParallelAnalysisAgent::build_system_prompt() {
    return R"(
You are a highly specialized static analysis expert for C/C++ programs, with a focus on thread synchronization and "happens-before" relationships.

Your task is to analyze a PAIR of threads to determine if they can execute their core logic concurrently. The key is to identify if there is a synchronization dependency that forces one thread to complete before the other begins its main execution. The most common dependency is a `pthread_join` call on the first thread that occurs before the second thread is created.

You will be given the context of two threads, including their creation sites (`fork_node_id`) and the variable names of their thread handles (`thread_handle_var`).

**Your Goal**: Determine if a happens-before relationship exists between the two threads.
- **If a dependency exists** (e.g., `parent_thread` creates `thread_1`, waits for it to finish via `join`, then creates `thread_2`), they CANNOT run in parallel.
- **If no such dependency exists**, they CAN run in parallel.

**Available Tools:**
- `get_parent_function(node_id)`: Get the function that contains the given node ID. Use this to find the context where threads are created.
- `get_control_flow_path(start_node_id, end_node_id)`: Checks if a control flow path exists from a start node to an end node within the same function. Returns the path if it exists.
- `find_synchronization_for_thread(thread_handle_var, search_scope_function_id)`: **Critical Tool.** Searches for a synchronization call (like `pthread_join`) that uses the given thread handle variable within the scope of the specified function. It returns the node information of the synchronization call if found.
- `confirm_analysis_result(can_run_in_parallel, reason, concurrent_regions)`: **Final Action.** Call this to submit your final conclusion. `concurrent_regions` should be an array of objects, each with `thread_entry_id`, `start_node_id`, and `end_node_id`.

**Your Workflow:**
1.  Use `get_parent_function` for both thread creation nodes (`fork_node_id_1`, `fork_node_id_2`) to ensure they are created in the same parent function. If not, the analysis is too complex; for now, assume they can run in parallel.
2.  **Crucial Step**: Use `find_synchronization_for_thread` on the first thread's handle (`thread_handle_var_1`) within the parent function's scope to find if a `join` call exists for it.
3.  If a `join` call is found (let's call its node `join_node_1`), you must determine its position relative to the thread creation calls.
4.  Use `get_control_flow_path` to check for the sequence: `fork_node_1` -> `join_node_1` -> `fork_node_2`.
5.  If this specific control flow path exists, it proves a "happens-before" relationship. The threads **cannot** run in parallel. Call `confirm_analysis_result` with `can_run_in_parallel: false` and specify the reason.
6.  If no such `join` call is found between the two fork sites, the threads **can** run in parallel. Call `confirm_analysis_result` with `can_run_in_parallel: true`. The `concurrent_regions` are the entire bodies of both thread entry functions.
)";
}

std::vector<Tool> ParallelAnalysisAgent::get_available_tools() const {
    return {
        {"get_thread_creation_site", "Get the creation site of a thread by its entry function ID.",
        {
            {"function_id", "number", "The ID of the thread's entry function.", true}
        }},
        {"check_happens_before", "Check if one code location is guaranteed to execute before another.", 
        {
            {"node_id_1", "number", "The ID of the first code node.", true},
            {"node_id_2", "number", "The ID of the second code node.", true}
        }},
        {"confirm_parallel_status", "Confirm the final conclusion about whether the threads can run in parallel.",
        {
            {"are_parallel", "boolean", "True if the threads can run in parallel, false otherwise.", true},
            {"justification", "string", "A brief explanation for your conclusion.", true}
        }}
    };
}

std::string ParallelAnalysisAgent::execute_tool(const std::string& tool_name, const nlohmann::json& arguments) {
    // NOTE: This is a placeholder implementation as requested.
    // You should replace the logic here with your actual static analysis calls.

    if (tool_name == "get_thread_creation_site") {
        int function_id = arguments.at("function_id").get<int>();
        // In a real implementation, you would look up the function and find its creation site.
        nlohmann::json result = {
            {"creation_site_node_id", 100 + function_id}, // Placeholder ID
            {"source_file", "main.cpp"},
            {"line_number", 50 + function_id}
        };
        return result.dump();
    } else if (tool_name == "check_happens_before") {
        int node_id_1 = arguments.at("node_id_1").get<int>();
        int node_id_2 = arguments.at("node_id_2").get<int>();
        // Placeholder: assume no happens-before relationship unless IDs are sequential.
        bool happens_before = (node_id_1 < node_id_2);
        nlohmann::json result = {
            {"happens_before", happens_before},
            {"reason", happens_before ? "Node 1 occurs before Node 2 in control flow." : "No direct ordering constraint found."}
        };
        return result.dump();
    } else if (tool_name == "confirm_parallel_status") {
        last_parallel_status_ = arguments.at("are_parallel").get<bool>();
        // The justification can be logged or stored if needed.
        return "finish";
    }

    nlohmann::json error_resp;
    error_resp["error"] = "Tool '" + tool_name + "' not found or not implemented by this agent.";
    return error_resp.dump();
}

std::string ParallelAnalysisAgent::parseResult(const std::vector<ChatMessage>& history) {
    // The result is stored in the member variable `last_parallel_status_`.
    // We return it as a string to match the `send_message` return type.
    return last_parallel_status_ ? "true" : "false";
}

bool ParallelAnalysisAgent::analyze_parallelism(int function_id_1, int function_id_2) {
    std::string user_prompt = "Determine if threads with entry function IDs " +
                              std::to_string(function_id_1) + " and " +
                              std::to_string(function_id_2) + " can execute in parallel.";
    
    // The context pointer is null here, but can be used to pass analysis-specific data.
    std::string result_str = send_message(user_prompt, nullptr);
    
    return result_str == "true";
}

} // namespace llm_client
