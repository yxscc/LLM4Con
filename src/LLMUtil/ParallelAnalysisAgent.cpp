// src/LLMUtil/ParallelAnalysisAgent.cpp

#include "LLMUtil/ParallelAnalysisAgent.h"
#include "CCPG/ThreadCreationTree.h"
#include "CCPG/CCPG.h"
#include "CCPG/CCPGNode.h"
#include "LLMUtil/SharedToolKit.h"
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

// Helper context for the conversation, to hold the state of the rule being built
struct RuleBuildingContext {
    ThreadPair* pair;
    std::optional<StatefulRule> current_rule;
};

ParallelAnalysisAgent::ParallelAnalysisAgent(std::shared_ptr<LLMClient> client)
    : Conversation(client, build_system_prompt(), 35) {} // Increased history for multi-step tools

std::string ParallelAnalysisAgent::build_system_prompt() {
    return R"(
You are a world-class expert in concurrent software architecture and a specialist in finding logic-based concurrency vulnerabilities. Your task is to analyze a pair of threads by first describing their interactions and then identifying violations of implicit stateful protocols like Time-of-Check-to-Time-of-Use (TOCTOU).

**Analysis Strategy (VERY IMPORTANT):**
1.  **Ground Your Analysis:** Your analysis MUST be grounded in the provided source code. Start by examining the thread entry functions provided in the initial prompt.
2.  **Explore the Call Graph:** Use the `get_callees` tool on the entry functions to discover the actual functions involved in each thread's execution. Do NOT guess function names.
3.  **Analyze Function Bodies:** Read the source code of the functions you discover using `get_function` to understand their behavior.
4.  **Propose Rules Based on Evidence:** Only propose a rule (`start_rule`) and nominate functions (`nominate_function_for_role`) based on functions you have verified to exist through your exploration.

**Core Analysis Principles (VERY IMPORTANT):**
1.  **Distinguish State vs. Synchronization**: Your analysis must differentiate between operations that manage synchronization (e.g., locking/unlocking a mutex) and operations that handle state (e.g., reading a value to make a decision, then using that value). A bug often lies in the unprotected window between a state **check** and its **use**, not in the synchronization primitives themselves.
2.  **Focus on Data Flow and Semantic Purpose**: Don't just look at function names. Analyze the code to understand the purpose of the functions. Trace how data (especially shared data) flows from one function to another to identify meaningful "check-then-use" or other stateful patterns.
3.  **Ground Analysis in Code**: Your analysis MUST be grounded in the provided source code. Use the exploration tools (`get_callees`, etc.) to discover the actual functions involved in each thread's execution before proposing a rule.

**Your Workflow:**

**Step 1: Analyze Parallelism & Interactions.**
- First, call `confirm_parallelism` to state your assessment of whether the threads are intended to be parallel and if they can execute concurrently.
- Describe the high-level interaction between the two threads based on the contracts and your initial code exploration.

**Step 2: Propose and Validate Stateful Temporal Ordering Rules (CRITICAL).**
- After confirming parallelism, you MUST follow your Analysis Strategy to explore the code.
- **2.1. Propose a Potential Rule:** Based on your exploration, if you identify a potential vulnerability, call `start_rule`. For a TOCTOU, identify the shared object being checked and used.
- **2.2. Nominate and Verify Functions for Roles:** For the active rule, nominate functions that you have discovered and verified. For EACH role, call `nominate_function_for_role`.
    - `state_check_function`: The function in one thread that reads the state of the shared object.
    - `state_modify_function`: The function in the OTHER thread that invalidates the state after the check.
    - `state_use_function`: The function in the FIRST thread that uses the shared object, assuming the state is still valid.
    - `resolving_function` (Optional): A function that re-validates the state or otherwise resolves the race condition before the 'use'.
- **2.3. Finalize the Rule:** Once all necessary functions are nominated, call `finalize_rule`.
- **2.4. Repeat or Finish:** You can start a new rule if you find other vulnerabilities. If not, call `finish_analysis`.

**CRITICAL INSTRUCTION**: The function names you provide MUST be an EXACT match to the C function names found in the source code. If a function nomination fails, use your exploration tools to find the correct name before trying again.
)";
}


std::vector<Tool> ParallelAnalysisAgent::get_available_tools() const {
    auto tools = SharedToolKit::get_shared_tools();

    tools.push_back(
        {"check_happens_before", "Checks if a happens-before relationship exists between two threads.", {}}
    );
    tools.push_back(
        {"confirm_parallelism", "Confirms the initial parallelism analysis.", {
            {"designed_for_parallelism", "boolean", "True if the threads are designed to run in parallel.", true},
            {"design_reasoning", "string", "A brief explanation for the design intent conclusion.", true},
            {"actually_concurrent", "boolean", "True if the code allows concurrent execution.", true},
            {"concurrency_reasoning", "string", "A brief explanation for the execution concurrency conclusion.", true}
        }}
    );
    tools.push_back(
        {"start_rule", "Starts the definition of a new stateful vulnerability rule by declaring its pattern and the shared object it applies to.", {
            {"rule_id", "string", "A unique name for this rule instance, e.g., 'SWAP_CACHE_TOCTOU'.", true},
            {"pattern_type", "string", "The general pattern type. Must be one of: 'TOCTOU', 'DESTRUCTIVE_REINIT', 'DOUBLE_ACTION'.", true},
            {"shared_object_type", "string", "The C-style type of the shared object, e.g., 'struct folio'.", true},
            {"description", "string", "A brief, natural language description of the suspected vulnerability.", true}
        }}
    );
    tools.push_back(
        {"nominate_function_for_role", "Nominates a specific, real function from the source code to fill a role within the currently active vulnerability pattern rule.", {
            {"role", "string", "The role this function plays in the pattern (e.g., 'state_check_function', 'state_modify_function', 'state_use_function', 'action_function', 'resolving_function').", true},
            {"function_name", "string", "The EXACT name of the C function to nominate. This function must exist in the code.", true}
        }}
    );
    tools.push_back(
        {"finalize_rule", "Finalizes and submits the currently active rule after all necessary function roles have been nominated.", {}}
    );
    tools.push_back(
        {"finish_analysis", "Call this tool after all rules have been proposed to finish the analysis.", {}}
    );
    
    return tools;
}

std::string ParallelAnalysisAgent::execute_tool(const std::string& tool_name, const nlohmann::json& arguments) {
    auto shared_result = SharedToolKit::handle_shared_tool(tool_name, arguments, ThreadCreationTree::getInstance()->getCCPG());
    if (shared_result) { return *shared_result; }

    auto* context = static_cast<RuleBuildingContext*>(this->get_context_for_tools());
    if (!context || !context->pair) { return R"({"error": "Internal context error: ThreadPair not found."})"; }

    if (tool_name == "check_happens_before") {
        bool can_run_concurrently = ThreadCreationTree::getInstance()->mayThreadsRunConcurrently(context->pair->thread1, context->pair->thread2);
        return nlohmann::json{{"happens_before_found", !can_run_concurrently}}.dump();
    }
    
    if (tool_name == "confirm_parallelism") {
        context->pair->analysis.designed_for_parallelism = arguments.at("designed_for_parallelism").get<bool>();
        context->pair->analysis.design_reasoning = arguments.at("design_reasoning").get<std::string>();
        context->pair->analysis.actually_concurrent = arguments.at("actually_concurrent").get<bool>();
        context->pair->analysis.concurrency_reasoning = arguments.at("concurrency_reasoning").get<std::string>();
        
        return R"({"status": "Parallelism analysis confirmed. Now, please follow your analysis strategy to explore the code and propose rules."})";
    }

    if (tool_name == "start_rule") {
        if (context->current_rule.has_value()) {
            return R"({"error": "A rule is already being built. You must call 'finalize_rule' or 'cancel_rule' before starting a new one."})";
        }
        StatefulRule rule;
        rule["rule_id"] = arguments.at("rule_id").get<std::string>();
        rule["description"] = arguments.at("description").get<std::string>();
        rule["shared_object_type"] = arguments.at("shared_object_type").get<std::string>();
        rule["pattern_type"] = arguments.at("pattern_type").get<std::string>();
        context->current_rule = rule;
        return R"({"status": "Rule initiated. Now, nominate functions for their roles using 'nominate_function_for_role'."})";
    }

    if (tool_name == "nominate_function_for_role") {
        if (!context->current_rule.has_value()) {
            return R"({"error": "No active rule. You must call 'start_rule' first."})";
        }
        std::string role = arguments.at("role").get<std::string>();
        std::string func_name = arguments.at("function_name").get<std::string>();

        // --- Verification Step ---
        if (ThreadCreationTree::getInstance()->getCPG()->findMethodsByName(func_name).empty()) {
            return R"({"error": "Function ')" + func_name + R"(' not found in the codebase. Please use your tools to find an exact, existing function name."})";
        }

        (*context->current_rule)[role] = func_name;
        return R"({"status": "Function ')" + func_name + R"(' successfully nominated for role ')" + role + R"('. Nominate another function or call 'finalize_rule'."})";
    }
    
    if (tool_name == "finalize_rule") {
        if (!context->current_rule.has_value()) {
            return R"({"error": "No active rule to finalize."})";
        }
        context->pair->analysis.temporal_rules.push_back(*context->current_rule);
        context->current_rule.reset(); // Clear the current rule
        return R"({"status": "Rule has been finalized and recorded. You can now start a new rule with 'start_rule' or finish with 'finish_analysis'."})";
    }

    if (tool_name == "finish_analysis") {
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
    
    // Create the context for this analysis run
    RuleBuildingContext context{&pair, std::nullopt};
    
    // Retrieve source code for entry functions to provide more context to the LLM
    std::string entry_func1_code = "[Source code not available]";
    if (pair.thread1 && pair.thread1->getThreadMainFunction() && pair.thread1->getThreadMainFunction()->getFuncNode()) {
        entry_func1_code = pair.thread1->getThreadMainFunction()->getFuncNode()->getCPGNode()->getCode();
    }
    
    std::string entry_func2_code = "[Source code not available]";
    if (pair.thread2 && pair.thread2->getThreadMainFunction() && pair.thread2->getThreadMainFunction()->getFuncNode()) {
        entry_func2_code = pair.thread2->getThreadMainFunction()->getFuncNode()->getCPGNode()->getCode();
    }
    
    std::stringstream prompt_ss;
    prompt_ss << "Please perform the multi-part analysis on the following thread pair.\n\n"
              << "**Contract for Thread 1 (ID: " << pair.thread1->getId() << "):**\n" << contract1_str << "\n"
              << "**Contract for Thread 2 (ID: " << pair.thread2->getId() << "):**\n" << contract2_str << "\n"
              << "--- Analysis Context ---\n"
              << "Thread " << pair.thread1->getId() << " entry function (ID=" << pair.contract1.entryPointFunctionId
              << ", Name='" << pair.thread1->getThreadMainFunction()->getFuncNode()->getCPGNode()->getName() << "'):\n"
              << "```cpp\n" << entry_func1_code << "\n```\n\n"
              << "Thread " << pair.thread2->getId() << " entry function (ID=" << pair.contract2.entryPointFunctionId
              << ", Name='" << pair.thread2->getThreadMainFunction()->getFuncNode()->getCPGNode()->getName() << "'):\n"
              << "```cpp\n" << entry_func2_code << "\n```\n\n"
              << "First, call `confirm_parallelism`. Then, follow your analysis strategy to explore the code (especially the provided functions) and propose rules if any vulnerabilities are found.";

    send_message(prompt_ss.str(), &context);
}

} // namespace llm_client