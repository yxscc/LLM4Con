#include "LLMUtil/ContractGeneratorAgent.h"
#include "CCPG/ThreadCreationTree.h"
#include "CPG/Node.h"
#include "CCPG/CCPGNode.h"
#include <iostream>



namespace llm_client {
std::string ContractGeneratorAgent::build_system_prompt() {
    return R"(
You are an elite static analysis expert for C/C++ multithreaded programs. Your mission is to analyze the source code of a thread's entry function and generate a precise "Concurrency Contract".

A Concurrency Contract is a structured summary of a thread's behavior regarding shared resources. It must contain three key components:
1.  **Thread Role**: A high-level, semantic description of the thread's purpose (e.g., "Worker thread processing tasks from a shared queue", "Logging thread writing to a global log file").
2.  **Core Shared State**: A list of all shared variables (global or heap-allocated) that the thread reads from or writes to. For each variable, you must specify the access type (Read, Write, or ReadWrite) and any locks used to protect that access.
3.  **Synchronization Discipline**: A summary of the overall locking strategy used by the thread (e.g., "Coarse-grained locking with a single global mutex", "Atomic operations for counters", "Lock-free").

You will be given the source code of the thread's entry function. To perform your analysis, you must use the following tools to explore the code and its context:

**Available Tools:**
- `get_callees_info(function_id)`: Gets information about functions called directly by the given function. Use this to understand the full behavior of the thread by exploring its call chain. Returns a list of callee objects, each with `id`, `name`, and `code`.
- `get_lock_usage_in_function(function_id)`: Identifies all lock/unlock operations within a function and the mutex variables they use. This is essential for determining the "Synchronization Discipline" and the `protecting_locks` for each shared variable.
- `confirm_contract(thread_role, synchronization_discipline, shared_states)`: **Final action.** Once you have gathered all necessary information and are confident in your analysis, you must call this function to submit the complete contract. The `shared_states` parameter must be a JSON array of objects, where each object has keys: `variable_name`, `access_type`, and `protecting_locks` (an array of strings).

**Your Workflow:**
1.  **Initial Analysis**: Start with the provided entry function code. Form an initial hypothesis about its role.
2.  **Explore Deeper**: If the entry function calls other complex functions, use `get_callees_info()` to get their code. Recursively use  `get_lock_usage_in_function()` on these callees to build a complete picture of data access and locking.
3.  **Synthesize and Conclude**: Consolidate all the information gathered. Determine the final `thread_role`, `synchronization_discipline`, and construct the detailed `shared_states` array.
4.  **Submit**: Call `confirm_contract()` with the complete, structured data. Do not add any extra commentary in the final call.
)";
}
//- `get_shared_variables_in_function(function_id)`: **Crucial tool.** Leverages backend points-to analysis to get a list of confirmed shared variables accessed within a function. Use this as the primary source for identifying the "Core Shared State".

ContractGeneratorAgent::ContractGeneratorAgent(CCPG* ccpg, std::shared_ptr<llm_client::LLMClient> client)
    : llm_client::Conversation(client, build_system_prompt(), 15), ccpg(ccpg) {}

std::optional<LLM::ConcurrencyContract> ContractGeneratorAgent::generateContractForThread(CCPGNode * forkNode, int threadEntryPointNodeID) {
    
}

// 在 ContractGeneratorAgent.cpp 中

std::vector<Tool> ContractGeneratorAgent::get_available_tools() const {
    return {
        // Tool 1: 获取被调用函数的信息
        {"get_callees_info", 
         "Gets information about functions called directly by the given function. Use this to explore the thread's call chain.",
         {
             {"function_id", "number", "The ID of the function to analyze for its callees.", true}
         }},

        // Tool 2: 获取函数内访问的共享变量 (关键工具)
        /*{"get_shared_variables_in_function",
         "Leverages backend points-to analysis to get a list of confirmed shared variables accessed within a function.",
         {
             {"function_id", "number", "The ID of the function to find shared variable accesses in.", true}
         }},*/

        // Tool 3: 获取函数内的锁使用情况
        {"get_lock_usage_in_function",
         "Identifies all lock/unlock operations within a function and the mutex variables they use.",
         {
             {"function_id", "number", "The ID of the function to analyze for lock usage.", true}
         }},

        // Tool 4: 最终确认并提交契约 (结束工具)
        {"confirm_contract",
         "Submits the complete Concurrency Contract. This is the final action.",
         {
             {"thread_role", "string", "A high-level, semantic description of the thread's purpose.", true},
             {"synchronization_discipline", "string", "A summary of the thread's overall locking strategy.", true},
             {"shared_states", "array", 
              "An array of objects detailing each shared variable access. Each object must contain 'variable_name' (string), 'access_type' (string: 'Read', 'Write', or 'ReadWrite'), and 'protecting_locks' (array of strings).", true}
         }}
    };
}

std::string ContractGeneratorAgent::execute_tool(const std::string& tool_name, const nlohmann::json& arguments) {
    // 获取CCPG单例，用于访问静态分析数据
    CCPG* ccpg = ThreadCreationTree::getInstance()->getCCPG();

    if (tool_name == "get_callees_info") {
        int function_id = arguments.at("function_id").get<int>();
        ccpg::Function* func = ccpg->getFunctionById(function_id);

        if (!func) {
            return R"({"error": "Function not found for ID: )" + std::to_string(function_id) + R"("})";
        }

        nlohmann::json calleesJson = nlohmann::json::array();
        for(CCPGNode * node : func->getNodes()) {
            if (node->isCallSite()) {
                CCPGEdge * callEdge = ccpg->hasCallEdge(node);
                if (!callEdge) continue;
                CCPGNode * calleeNode = callEdge->getDst();
                if (calleeNode) {
                    nlohmann::json calleeInfo = {
                        {"id", calleeNode->getId()},
                        {"name", calleeNode->getCPGNode()->getName()},
                        {"code", calleeNode->getCPGNode()->getCode()}
                    };
                    calleesJson.push_back(calleeInfo);
                }
            }
        }

        return calleesJson.dump();

    } /*else if (tool_name == "get_shared_variables_in_function") {
        int function_id = arguments.at("function_id").get<int>();
        ccpg::Function* func = ccpg->getFunctionById(function_id);

        if (!func) {
            return R"({"error": "Function not found for ID: )" + std::to_string(function_id) + R"("})";
        }

        nlohmann::json sharedVarsJson = nlohmann::json::array();
        // [ASSUMPTION] 假设你的Function对象在SVF分析后，存储了访问过的共享变量信息。
        // getAccessedSharedVariables() 应返回一个结构体列表，每个结构体包含变量名和访问类型。
        const auto& sharedVars = func->getAccessedSharedVariables(); 
        for (const auto& varInfo : sharedVars) {
            nlohmann::json varJson = {
                {"variable_name", varInfo.getName()},
                {"access_type", varInfo.getAccessTypeString()} // e.g., "Read", "Write"
            };
            sharedVarsJson.push_back(varJson);
        }
        return sharedVarsJson.dump();

    }*/ else if (tool_name == "get_lock_usage_in_function") {
        int function_id = arguments.at("function_id").get<int>();
        ccpg::Function* func = ccpg->getFunctionById(function_id);

        if (!func) {
            return R"({"error": "Function not found for ID: )" + std::to_string(function_id) + R"("})";
        }

        nlohmann::json locksJson = nlohmann::json::array();
        // [ASSUMPTION] 假设你的Function对象存储了其内部的锁操作信息。
        // getLockOperations() 应返回一个结构体列表，每个结构体包含锁变量名和操作类型。
        const auto& acquires = func->getNodesByType(ThreadAPIUtil::TYPE::ACQUIRE);
        const auto& releases = func->getNodesByType(ThreadAPIUtil::TYPE::RELEASE);
        for (const auto& lockOp : acquires) {
            nlohmann::json lockJson = {
                {"code", lockOp->getCPGNode()->getCode()},
                {"operation", "lock"} // 假设获取锁操作
            };
            locksJson.push_back(lockJson);
        }
        for (const auto& lockOp : releases) {
            nlohmann::json lockJson = {
                {"code", lockOp->getCPGNode()->getCode()},
                {"operation", "unlock"} // 假设释放锁操作
            };
            locksJson.push_back(lockJson);
        }
        return locksJson.dump();

    } else if (tool_name == "confirm_contract") {
        // 当LLM调用此工具时，它意味着分析完成。
        // 我们将完整的契约内容（即LLM传递的所有参数）保存下来。
        last_generated_contract_ = arguments.dump();
        
        // 返回一个特殊的 "finish" 字符串来终止对话。
        // 这需要你的 Conversation 基类能够识别并处理这个信号。
        return "finish"; 
    }

    // 如果工具名称未匹配，返回错误信息
    nlohmann::json error_resp;
    error_resp["error"] = "Tool '" + tool_name + "' not found or not implemented by ContractGeneratorAgent.";
    error_resp["arguments_received"] = arguments;
    return error_resp.dump();
}


std::string ContractGeneratorAgent::parseResult(const std::vector<llm_client::ChatMessage>& history) {
    // The raw JSON response is the result.
    if (!history.empty() && history.back().role == llm_client::MessageRole::ASSISTANT) {
        return history.back().content;
    }
    return "";
}
}