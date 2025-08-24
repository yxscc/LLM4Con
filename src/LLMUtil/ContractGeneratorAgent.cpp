#include "LLMUtil/ContractGeneratorAgent.h"
#include "LLMUtil/SharedToolKit.h"
#include "CCPG/ThreadCreationTree.h"
#include "CPG/Node.h"
#include <iostream>
#include <sstream>

namespace llm_client {

ContractGeneratorAgent::ContractGeneratorAgent(CCPG* ccpg, std::shared_ptr<LLMClient> client)
    : Conversation(client, build_system_prompt(), 25), ccpg_(ccpg) {}

std::string ContractGeneratorAgent::build_system_prompt() {
    return R"(
You are an elite static analysis expert for C/C++ multithreaded programs. 
Your mission is to analyze a thread's entry function to build a precise "Concurrency Contract".
You will be provided with the source code of a thread's entry function and its creation site.

Your task is to build the contract by calling a series of tools in a specific order. You can also use shared query tools like `get_callees` to explore the code.

**Your Workflow:**
1.  **Analyze Role & Summary**: Start by analyzing the thread's purpose. Respond by calling `confirm_role_and_summary`.
2.  **Identify Shared Variables**: After the user prompts you to continue, identify all shared variables. For EACH variable, call `report_shared_variable`. When finished, call `finish_reporting_shared_variables`.
3.  **Identify Sync Primitives**: After the next prompt, identify all synchronization primitives (mutexes, semaphores, etc.). For EACH primitive, call `report_sync_primitive`. When finished, call `finish_reporting_sync_primitives`.
4.  **Finalize**: Once all information is reported, call `finalize_contract` to complete the process.
)";
}

std::vector<Tool> ContractGeneratorAgent::get_available_tools() const {
    auto tools = SharedToolKit::get_shared_tools();
    
    // Add agent-specific tools
    tools.push_back({"confirm_role_and_summary", "Confirms the role and summary of the thread.", {
        {"role", "string", "A single, concise category (e.g., 'Worker', 'Logger', 'I/O Handler').", true},
        {"summary", "string", "A one-sentence description of the thread's function.", true}
    }});
    tools.push_back({"report_shared_variable", "Reports a single shared variable accessed by the thread.", {
        {"variable_name", "string", "The name of the shared variable.", true},
        {"variable_type", "string", "The C/C++ type of the variable.", true},
        {"access_type", "string", "Must be one of 'Read', 'Write', or 'ReadWrite'.", true},
        {"protecting_primitives", "array", "An array of identifiers for the sync primitives guarding this variable.", true}
    }});
    tools.push_back({"finish_reporting_shared_variables", "Call this after all shared variables have been reported.", {}});
    tools.push_back({"report_sync_primitive", "Reports a single synchronization primitive used in the thread.", {
        {"identifier", "string", "The variable name of the primitive (e.g., 'g_mutex').", true},
        {"type", "string", "Must be one of 'MUTEX', 'SEMAPHORE', 'COND_VAR', or 'CUSTOM_ATOMIC'.", true},
        {"purpose", "string", "A brief description of what this primitive protects or signals.", true}
    }});
    tools.push_back({"finish_reporting_sync_primitives", "Call this after all synchronization primitives have been reported.", {}});
    tools.push_back({"finalize_contract", "Final action to submit the complete contract.", {}});

    return tools;
}

std::optional<LLM::ConcurrencyContract> ContractGeneratorAgent::generateContractForThread(Thread* thread) {
    if (!thread || !thread->getThreadMainFunction()) {
        return std::nullopt;
    }

    auto contract = std::make_unique<LLM::ConcurrencyContract>(
        thread->getId(), 
        thread->getThreadMainFunction()->getId()
    );

    std::string fork_stmt = thread->getForkNode()->getCPGNode()->getCode();
    std::string entry_func_code = thread->getThreadMainFunction()->getFuncNode()->getCPGNode()->getCode();

    std::string user_prompt = 
        "Analyze the following thread to construct its Concurrency Contract.\n"
        "Fork statement: " + fork_stmt + "\n"
        "Thread entry function body:\n```cpp\n" + entry_func_code + "\n```";

    send_message(user_prompt, contract.get());

    return std::move(*contract);
}

std::string ContractGeneratorAgent::execute_tool(const std::string& tool_name, const nlohmann::json& arguments) {
    // Try shared tools first
    auto shared_result = SharedToolKit::handle_shared_tool(tool_name, arguments, ccpg_);
    if (shared_result) {
        return *shared_result;
    }

    // Handle agent-specific tools
    auto* contract = static_cast<LLM::ConcurrencyContract*>(this->get_context_for_tools());
    if (!contract) {
        return R"({"error": "Internal context error: ConcurrencyContract not found."})";
    }

    if (tool_name == "confirm_role_and_summary") {
        contract->setRole(arguments.at("role").get<std::string>());
        contract->setSummary(arguments.at("summary").get<std::string>());
        return R"({"status": "Role and summary recorded. Now, please identify all shared variables."})";
    }
    
    if (tool_name == "report_shared_variable") {
        LLM::ConcurrencyContract::SharedVariable var;
        var.variableName = arguments.at("variable_name").get<std::string>();
        var.variableType = arguments.at("variable_type").get<std::string>();
        var.accessType = arguments.at("access_type").get<std::string>();
        var.protectingPrimitives = arguments.at("protecting_primitives").get<std::vector<std::string>>();
        contract->addSharedVariable(var);
        return R"({"status": "Shared variable recorded. Report more or call 'finish_reporting_shared_variables'."})";
    }

    if (tool_name == "finish_reporting_shared_variables") {
        return R"({"status": "Shared variable reporting complete. Now, please identify all synchronization primitives."})";
    }

    if (tool_name == "report_sync_primitive") {
        LLM::ConcurrencyContract::SynchronizationPrimitive prim;
        prim.identifier = arguments.at("identifier").get<std::string>();
        // Note: This requires robust mapping from string to enum, which is omitted for brevity
        prim.type = LLM::ConcurrencyContract::PrimitiveType::MUTEX; // Placeholder
        prim.purpose = arguments.at("purpose").get<std::string>();
        contract->addSynchronizationPrimitive(prim);
        return R"({"status": "Synchronization primitive recorded. Report more or call 'finish_reporting_sync_primitives'."})";
    }

    if (tool_name == "finish_reporting_sync_primitives") {
        return R"({"status": "Synchronization primitive reporting complete. Please call 'finalize_contract' to finish."})";
    }

    if (tool_name == "finalize_contract") {
        return "finish"; // Signal completion
    }

    nlohmann::json error_resp;
    error_resp["error"] = "Tool '" + tool_name + "' not found or not implemented by this agent.";
    return error_resp.dump();
}

std::string ContractGeneratorAgent::parseResult(const std::vector<llm_client::ChatMessage>& history) {
    // The contract is built progressively in the context object.
    // A simple confirmation is sufficient.
    return "Contract generated.";
}

} // namespace llm_client
