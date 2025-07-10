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
        You are a sophisticated program analysis expert specializing in concurrency. 
        Your task is to determine if two threads, identified by their entry functions, can potentially execute in parallel (a "May-Happen-in-Parallel" or MHP analysis).

        You will be given the function IDs of the two thread entry points. To make your determination, you must investigate the program's structure to see if there are any synchronization constraints that would prevent them from running at the same time.

        Use the following tools to conduct your analysis:
        - `get_thread_creation_site(function_id)`: Finds where a thread with a given entry function is created. This helps you locate the `pthread_create` or similar calls.
        - `check_happens_before(node_id_1, node_id_2)`: Checks if the execution of code at `node_id_1` is guaranteed to happen before the execution of code at `node_id_2`. This is crucial for checking if a `pthread_join` on one thread occurs before the creation of another.
        - `confirm_parallel_status(are_parallel, justification)`: Once you have a definitive answer, call this function. Set `are_parallel` to `true` or `false`, and provide a clear `justification` for your conclusion.

        Your analysis process should be:
        1. For each thread entry function, find its creation site using `get_thread_creation_site`.
        2. Analyze the control flow between these creation sites. For example, if thread B is created after thread A is joined, they cannot run in parallel.
        3. Use `check_happens_before` to verify any potential ordering between thread creation and join calls.
        4. Based on your findings, make a final decision and report it with `confirm_parallel_status`.
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
