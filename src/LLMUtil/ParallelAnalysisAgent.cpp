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

ParallelAnalysisAgent::ParallelAnalysisAgent(std::shared_ptr<LLMClient> client)
    : Conversation(client, build_system_prompt(), 25) {}

std::string ParallelAnalysisAgent::build_system_prompt() {
    return R"(
You are a world-class expert in concurrent software architecture and formal verification. Your task is to analyze a pair of threads in a step-by-step manner to uncover subtle concurrency protocol violations.

**Your Analysis Workflow:**

**Step 1: Analyze Parallelism.**
- First, analyze the provided contracts and use the `check_happens_before` tool.
- Then, you MUST call the `confirm_parallelism` tool to submit your findings.

**Step 2: Propose Rules (if any).**
- After you confirm parallelism, you will be prompted to propose rules.
- Carefully analyze the thread interactions to infer any implicit "Stateful Temporal Ordering Rules".
- **CRITICAL INSTRUCTION**: The function names you provide in the rule (e.g., `forbidden_function_1`) MUST be an EXACT match to the C function names found in the source code provided in the context. Do NOT invent, generalize, or paraphrase function names.

**Step 3: Finish Analysis.**
- After you have proposed all the rules you can find, you MUST call the `finish_analysis` tool to complete the process.
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
        {"propose_temporal_rule", "Proposes a single Stateful Temporal Ordering Rule.", {
            {"rule_id", "string", "A unique, descriptive string identifier (e.g., 'WORK_QUEUE_STATE_PROTOCOL').", true},
            {"description", "string", "A natural language explanation of the rule.", true},
            {"shared_object_type", "string", "The C-style type of the shared object (e.g., 'struct mock_work_struct').", true},
            {"forbidden_function_1", "string", "The EXACT name of the first C function in the forbidden sequence, as it appears in the source code.", true},
            {"forbidden_function_2", "string", "The EXACT name of the second C function in the forbidden sequence, as it appears in the source code.", true},
            {"resolving_function", "string", "The EXACT name of the C function that resolves the state. Can be an empty string if none.", true}
        }}
    );
    tools.push_back(
        {"finish_analysis", "Call this tool after all rules have been proposed to finish the analysis.", {}}
    );
    
    return tools;
}

std::string ParallelAnalysisAgent::execute_tool(const std::string& tool_name, const nlohmann::json& arguments) {
    auto shared_result = SharedToolKit::handle_shared_tool(tool_name, arguments, ThreadCreationTree::getInstance()->getCCPG());
    if (shared_result) { return *shared_result; }

    auto* pair = static_cast<ThreadPair*>(this->get_context_for_tools());
    if (!pair) { return R"({"error": "Internal context error: ThreadPair not found."})"; }

    if (tool_name == "check_happens_before") {
        bool can_run_concurrently = ThreadCreationTree::getInstance()->mayThreadsRunConcurrently(pair->thread1, pair->thread2);
        return nlohmann::json{{"happens_before_found", !can_run_concurrently}}.dump();
    }
    
    if (tool_name == "confirm_parallelism") {
        pair->analysis.designed_for_parallelism = arguments.at("designed_for_parallelism").get<bool>();
        pair->analysis.design_reasoning = arguments.at("design_reasoning").get<std::string>();
        pair->analysis.actually_concurrent = arguments.at("actually_concurrent").get<bool>();
        pair->analysis.concurrency_reasoning = arguments.at("concurrency_reasoning").get<std::string>();
        
        // 返回一个提示，引导LLM进入下一步
        return R"({"status": "Parallelism analysis confirmed. Now, please propose any Stateful Temporal Ordering Rules by calling 'propose_temporal_rule', or call 'finish_analysis' if there are no rules."})";
    }

    if (tool_name == "propose_temporal_rule") {
        // 将简单的参数重新组装成我们需要的JSON格式
        nlohmann::json rule;
        rule["rule_id"] = arguments.at("rule_id").get<std::string>();
        rule["description"] = arguments.at("description").get<std::string>();
        rule["shared_object_type"] = arguments.at("shared_object_type").get<std::string>();
        rule["forbidden_sequence"] = nlohmann::json::array({
            {{"function", arguments.at("forbidden_function_1").get<std::string>()}},
            {{"function", arguments.at("forbidden_function_2").get<std::string>()}}
        });
        rule["resolving_function"] = arguments.at("resolving_function").get<std::string>();
        
        pair->analysis.temporal_rules.push_back(rule);
        
        // 返回提示，让LLM继续或结束
        return R"({"status": "Rule has been recorded. Propose another rule, or call 'finish_analysis' to complete."})";
    }

    if (tool_name == "finish_analysis") {
        return "finish"; // 发出结束信号
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
        "**Contract for Thread 1 (ID: " + std::to_string(pair.thread1->getId()) + "):**\n" + contract1_str + "\n"
        "**Contract for Thread 2 (ID: " + std::to_string(pair.thread2->getId()) + "):**\n" + contract2_str + "\n"
        "First, call `confirm_parallelism`. Then, you will be prompted to propose rules.";
    
    send_message(user_prompt, &pair);
}

} // namespace llm_client
