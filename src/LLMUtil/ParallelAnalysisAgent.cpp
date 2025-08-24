#include "LLMUtil/ParallelAnalysisAgent.h"
#include "CCPG/ThreadCreationTree.h"
#include "CCPG/CCPG.h"
#include "CCPG/CCPGNode.h"
#include <sstream>
#include <nlohmann/json.hpp>

namespace llm_client {

// Helper to serialize a contract to a string for the prompt
std::string contract_to_string(const LLM::ConcurrencyContract& contract) {
    std::stringstream ss;
    ss << "  - Thread ID: " << contract.threadId << "\n";
    ss << "  - Role: " << contract.role << "\n";
    ss << "  - Summary: " << contract.summary << "\n";
    ss << "  - Shared Variables: ";
    if (contract.sharedVariables.empty()) {
        ss << "None identified.\n";
    } else {
        ss << "\n";
        for (const auto& var : contract.sharedVariables) {
            ss << "    - Name: " << var.variableName << ", Access: " << var.accessType << "\n";
        }
    }
    return ss.str();
}

ParallelAnalysisAgent::ParallelAnalysisAgent(std::shared_ptr<LLMClient> client)
    : Conversation(client, build_system_prompt(), 20) {}

std::string ParallelAnalysisAgent::build_system_prompt() {
    return R"(
You are a world-class expert in concurrent software architecture and formal verification. Your task is to perform a multi-part analysis on a pair of threads to uncover subtle concurrency protocol violations.

**Your Analysis Workflow:**

**Part 1: Analyze Design Intent & Execution Concurrency**
- Based on the semantic information in the two "Concurrency Contracts", determine if the threads are *designed* to be parallel.
- Use the `check_happens_before` tool to determine if the code *actually allows* the threads to run concurrently.

**Part 2: Infer Stateful Temporal Ordering Rules (CRITICAL TASK)**
- This is your most important task. Carefully analyze the interaction between the two threads, focusing on any shared data structures identified in their contracts.
- Your goal is to infer the implicit "protocol" or "state machine" that governs the safe use of these shared objects.
- If you identify such a protocol, you MUST formalize it as one or more **Stateful Temporal Ordering Rules**. A rule defines a sequence of function calls on a shared object that is forbidden unless a specific "resolving" function is called in between.

**Part 3: Final Action**
- After completing all analysis, you MUST call the `confirm_parallel_analysis_with_rules` tool to submit your complete findings.
- This single tool call must include your analysis of parallelism AND any temporal rules you have inferred. If no specific rules are found, submit an empty list for the `temporal_rules` parameter.

**Example of a Stateful Temporal Ordering Rule (for a work queue scenario):**
If you observe that a `work_struct` is first added to a queue via `queue_work()` and should not be re-initialized with `INIT_WORK()` before being processed by `process_one_work()`, you would generate the following rule:

```json
{
  "rule_id": "WORK_QUEUE_STATE_PROTOCOL",
  "description": "A 'work_struct' that has been queued (pending) must not be re-initialized before it is processed.",
  "shared_object_type": "struct mock_work_struct",
  "forbidden_sequence": [
    { "function": "mock_queue_work", "effect": "sets state to PENDING" },
    { "function": "mock_INIT_WORK", "effect": "destructively resets state" }
  ],
  "resolving_function": "process_one_work"
}

**Final Action**
After completing both parts of the analysis, you MUST call the `confirm_parallel_analysis` tool to submit your complete findings. Provide a clear reason for each part of your analysis.
)";
}

std::vector<Tool> ParallelAnalysisAgent::get_available_tools() const {
    return {
        {"check_happens_before", "Checks if a happens-before relationship exists between two threads (e.g., join(t1) before fork(t2)).",
        {
            // No parameters needed, the agent will provide context internally.
        }},
        {"confirm_parallel_analysis", "Confirms the final, two-part analysis of the thread pair.",
        {
            {"designed_for_parallelism", "boolean", "True if the threads are designed to run in parallel based on their contracts.", true},
            {"design_reasoning", "string", "A brief explanation for the design intent conclusion.", true},
            {"actually_concurrent", "boolean", "True if the code allows the threads to execute at the same time.", true},
            {"concurrency_reasoning", "string", "A brief explanation for the execution concurrency conclusion (based on the happens-before check).", true},
            {"temporal_rules", "array", "An array of JSON objects, where each object represents a Stateful Temporal Ordering Rule. Each object must have fields like 'rule_id', 'description', 'shared_object_type', 'forbidden_sequence', and 'resolving_function'. Pass an empty array if no rules are found.", true}
        }}
    };
}

std::string ParallelAnalysisAgent::execute_tool(const std::string& tool_name, const nlohmann::json& arguments) {
    auto* pair = static_cast<ThreadPair*>(this->get_context_for_tools());
    if (!pair) {
        return R"({"error": "Internal context error: ThreadPair not found."})";
    }

    if (tool_name == "check_happens_before") {
        bool can_run_concurrently = ThreadCreationTree::getInstance()->mayThreadsRunConcurrently(pair->thread1, pair->thread2);
        
        nlohmann::json result = {
            {"happens_before_found", !can_run_concurrently}
        };
        return result.dump();
    }
    
    if (tool_name == "confirm_parallel_analysis") {
        pair->analysis.designed_for_parallelism = arguments.at("designed_for_parallelism").get<bool>();
        pair->analysis.design_reasoning = arguments.at("design_reasoning").get<std::string>();
        pair->analysis.actually_concurrent = arguments.at("actually_concurrent").get<bool>();
        pair->analysis.concurrency_reasoning = arguments.at("concurrency_reasoning").get<std::string>();

        if (arguments.contains("temporal_rules") && arguments.at("temporal_rules").is_array()) {
            for (const auto& rule_json : arguments.at("temporal_rules")) {
                pair->analysis.temporal_rules.push_back(rule_json);
            }
        }
        return "finish"; // Signal completion
    }

    nlohmann::json error_resp;
    error_resp["error"] = "Tool '" + tool_name + "' not found or not implemented by this agent.";
    return error_resp.dump();
}

std::string ParallelAnalysisAgent::parseResult(const std::vector<ChatMessage>& history) {
    return "Analysis complete.";
}

void ParallelAnalysisAgent::analyze_parallelism(ThreadPair& pair) {
    std::string contract1_str = contract_to_string(pair.contract1);
    std::string contract2_str = contract_to_string(pair.contract2);

    std::string user_prompt = 
        "Please perform the multi-part analysis on the following thread pair.\n\n"
        "**Contract for Thread 1:**\n" + contract1_str + "\n"
        "**Contract for Thread 2:**\n" + contract2_str + "\n"
        "First, analyze design and execution parallelism. Second, and most importantly, infer any Stateful Temporal Ordering Rules. Finally, report your combined findings with `confirm_parallel_analysis_with_rules`.";

    send_message(user_prompt, &pair);
}

} // namespace llm_client
