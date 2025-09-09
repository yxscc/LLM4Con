// src/LLMUtil/ParallelAnalysisAgent.cpp

#include "LLMUtil/ParallelAnalysisAgent.h"
#include "CCPG/ThreadCreationTree.h"
#include "CCPG/CCPG.h"
#include "CCPG/CCPGNode.h"
#include "LLMUtil/SharedToolKit.h"
#include <sstream>
#include <nlohmann/json.hpp>
#include <queue>
#include <set>

namespace llm_client {

// 辅助函数：将并发规约序列化为字符串，用于提示
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

struct RuleBuildingContext {
    ThreadPair* pair;
    std::unique_ptr<Rule> current_rule; // 使用智能指针
    std::string current_rule_summary;
};

// 构造函数
ParallelAnalysisAgent::ParallelAnalysisAgent(std::shared_ptr<LLMClient> client)
    : Conversation(client, "", 45), ccpg_(ThreadCreationTree::getInstance()->getCCPG()) {
    
    // 初始化所有支持的规则
    m_supported_rules["TOCTOU"] = std::make_unique<TOCTOURule>();
    m_supported_rules["DataRace"] = std::make_unique<DataRaceRule>();
    m_supported_rules["USE_AFTER_FREE"] = std::make_unique<UseAfterFreeRule>();
    m_supported_rules["DOUBLE_FREE"] = std::make_unique<DoubleFreeRule>();
    m_supported_rules["NULL_POINTER_DEREFERENCE"] = std::make_unique<NullPointerDereferenceRule>();
    m_supported_rules["DEADLOCK"] = std::make_unique<DeadlockRule>();
    // m_supported_rules["DOUBLE_FETCH"] = std::make_unique<DoubleFetchRule>();
    
    // 在构造函数中调用 set_system_prompt，而不是依赖基类构造函数
    set_system_prompt(build_system_prompt());
}

// 构建系统提示，引导LLM遵循我们设计的精细化工作流
std::string ParallelAnalysisAgent::build_system_prompt() {
    std::stringstream ss;
    ss << R"(You are a world-class expert in concurrent software architecture. Your mission is to analyze the interaction between two threads and define the unstated, implicit temporal rules that shared variables must obey to prevent logical bugs.

**Your analysis is a structured, multi-step process:**

**Step 1: High-Level Concurrency Assessment.**
- First, review the thread contexts and concurrency contracts.
- You MUST make a high-level judgment on concurrency by calling the `confirm_parallelism` tool.

**Step 2: Detailed Vulnerability Rule Generation (ONLY if threads are concurrent).**
If concurrent, you will define rules based on the following supported vulnerability patterns:

)";
    
    // **动态插入所有规则的描述**
    for (const auto& pair : m_supported_rules) {
        ss << "--- Pattern: " << pair.first << " ---\n";
        ss << pair.second->get_description() << "\n\n";
    }

    ss << R"(
**Your strategy for defining a rule is as follows:**
1.  **Start a Rule:** Call `start_rule` with the `pattern_type` you want to investigate.
2.  **Gather Evidence:** Use tools like `get_successors_chunked` to find nodes for the required roles.
3.  **Nominate Nodes:** Call `nominate_node_for_role` for each required role.
4.  **Confirm the Rule:** Call `propose_rule_for_confirmation`, review the system-generated summary, and then call `confirm_rule`.

**Step 3: Finish.**
- Once all rules are defined, call `finish_analysis`.
)";
    return ss.str();
}

// 定义LLM可用的工具集
std::vector<Tool> ParallelAnalysisAgent::get_available_tools() const {
    auto tools = SharedToolKit::get_shared_tools();

    // Stage 1 Tool
    tools.push_back(
        {"confirm_parallelism", "Confirms the initial parallelism analysis.", {
            {"designed_for_parallelism", "boolean", "True if the threads are designed to run in parallel.", true},
            {"design_reasoning", "string", "A brief explanation for the design intent conclusion.", true},
            {"actually_concurrent", "boolean", "True if the code allows concurrent execution.", true},
            {"concurrency_reasoning", "string", "A brief explanation for the execution concurrency conclusion.", true}
        }}
    );

    // Stage 2 Tools
    tools.push_back(
        {"get_function_entry_node", "Gets the first operational node ID of a function.", {
            {"function_id", "number", "The ID of the function.", true}
        }}
    );
    tools.push_back(
        {"get_successors_chunked", "Gets a batch of successor nodes in the control flow graph via BFS.", {
            {"node_id", "number", "The ID of the node to start the traversal from.", true},
            {"chunk_size", "number", "The maximum number of nodes to return (default: 10).", false}
        }}
    );
    tools.push_back(
        {"start_rule", "Starts the definition of a new stateful rule.", {
            {"rule_id", "string", "A unique name for this rule instance, e.g., 'TASK_STATUS_TOCTOU'.", true},
            {"pattern_type", "string", "The pattern type to define. Currently supported: 'TOCTOU', 'DataRace', 'USE_AFTER_FREE', 'DOUBLE_FREE', 'NULL_POINTER_DEREFERENCE', 'DEADLOCK'.", true},
            {"shared_object_type", "string", "The C-style type of the shared object.", true},
            {"rule_summary", "string", "A clear, natural language summary of the rule's intent.", true}
        }}
    );
    tools.push_back(
        {"nominate_node_for_role", "Nominates a specific node ID to fill a role within the currently active rule.", {
            {"role", "string", "The role this node plays. Must be one of the roles required by the current rule pattern.", true},
            {"node_id", "number", "The EXACT ID of the node.", true}
        }}
    );
    tools.push_back(
        {"propose_rule_for_confirmation", "Proposes the current rule for finalization and returns a structured summary for semantic review.", {}}
    );
    tools.push_back(
        {"confirm_rule", "Confirms or rejects the proposed rule based on semantic review.", {
            {"is_semantically_correct", "boolean", "True if the structured summary correctly matches the rule's intent.", true}
        }}
    );
    tools.push_back(
        {"finish_analysis", "Call this tool after all analysis is complete.", {}}
    );

    return tools;
}

// 工具的具体实现
std::string ParallelAnalysisAgent::execute_tool(const std::string& tool_name, const nlohmann::json& arguments) {
    auto shared_result = SharedToolKit::handle_shared_tool(tool_name, arguments, ccpg_);
    if (shared_result) { return *shared_result; }

    auto* context = static_cast<RuleBuildingContext*>(this->get_context_for_tools());
    if (!context || !context->pair) { return R"({"error": "Internal context error."})"; }

    // Stage 1 Tool
    if (tool_name == "confirm_parallelism") {
        context->pair->analysis.designed_for_parallelism = arguments.at("designed_for_parallelism").get<bool>();
        context->pair->analysis.design_reasoning = arguments.at("design_reasoning").get<std::string>();
        context->pair->analysis.actually_concurrent = arguments.at("actually_concurrent").get<bool>();
        context->pair->analysis.concurrency_reasoning = arguments.at("concurrency_reasoning").get<std::string>();
        
        if (context->pair->analysis.actually_concurrent) {
            return R"({"status": "Parallelism analysis confirmed. The threads CAN run concurrently. Please proceed to Stage 2 to define rules."})";
        } else {
            return R"({"status": "Parallelism analysis confirmed. The threads CANNOT run concurrently. No further analysis is needed. Call 'finish_analysis'."})";
        }
    }

    // Stage 2 Tools
    if (tool_name == "get_function_entry_node") {
        int function_id = arguments.at("function_id").get<int>();
        ccpg::Function* func = ccpg_->getFunctionById(function_id);
        if (!func || !func->getFuncNode()) {
            return R"({"error": "Function not found."})";
        }
        CCPGNode* func_cpg_node = func->getFuncNode();
        if (!func_cpg_node->getOutEdges().empty()) {
            CCPGNode* entry_op_node = (*func_cpg_node->getOutEdges().begin())->getDst();
            return nlohmann::json{
                {"entry_node_id", entry_op_node->getId()},
                {"code", entry_op_node->getCPGNode()->getCode()}
            }.dump();
        }
        return R"({"error": "Function has no operations."})";
    }

    if (tool_name == "get_successors_chunked") {
        int start_node_id = arguments.at("node_id").get<int>();
        int chunk_size = arguments.value("chunk_size", 10);
        CCPGNode* start_node = ccpg_->getNodeByID(start_node_id);
        if (!start_node) {
             return R"({"error": "Start node not found."})";
        }

        nlohmann::json successors = nlohmann::json::array();
        std::queue<CCPGNode*> worklist;
        std::set<CCPGNode*> visited;
        worklist.push(start_node);
        visited.insert(start_node);

        while (!worklist.empty() && successors.size() < static_cast<size_t>(chunk_size)) {
            CCPGNode* current = worklist.front();
            worklist.pop();
            for (CCPGEdge* edge : current->getOutEdges()) {
                if (edge->getType() == CCPGEdge::EdgeType::ORDER) {
                    CCPGNode* successor = edge->getDst();
                    if (visited.find(successor) == visited.end()) {
                        visited.insert(successor);
                        worklist.push(successor);
                        if (successors.size() < static_cast<size_t>(chunk_size)) {
                             successors.push_back({
                                {"node_id", successor->getId()},
                                {"code", successor->getCPGNode()->getCode()},
                                {"location", successor->getNodeLoc().toString()}
                            });
                        }
                    }
                }
            }
        }
        return successors.dump();
    }

    if (tool_name == "start_rule") {
        if (context->current_rule) {
            return R"({"error": "A rule is already being built. You must finalize or discard it first."})";
        }
        std::string pattern_type = arguments.at("pattern_type").get<std::string>();
        
        std::string rule_id = arguments.at("rule_id").get<std::string>();
        std::string object_type = arguments.at("shared_object_type").get<std::string>();
        context->current_rule_summary = arguments.at("rule_summary").get<std::string>();

        // **从支持的规则列表中查找并克隆规则实例**
        auto it = m_supported_rules.find(pattern_type);
        if (it != m_supported_rules.end()) {
            // 这里我们创建一个新的实例，而不是移动，因为m_supported_rules是模板
            if (pattern_type == "TOCTOU") {
                 context->current_rule = std::make_unique<TOCTOURule>();
            } else if (pattern_type == "DataRace") {
                context->current_rule = std::make_unique<DataRaceRule>();
            } else if (pattern_type == "USE_AFTER_FREE") {
                context->current_rule = std::make_unique<UseAfterFreeRule>();
            } else if (pattern_type == "DOUBLE_FREE") {
                context->current_rule = std::make_unique<DoubleFreeRule>();
            } else if (pattern_type == "NULL_POINTER_DEREFERENCE") {
                context->current_rule = std::make_unique<NullPointerDereferenceRule>();
            } else if (pattern_type == "DEADLOCK") {
                context->current_rule = std::make_unique<DeadlockRule>();
            }
        } else {
            return "{\"error\": \"Unsupported pattern_type: '" + pattern_type + "'.\"}";
        }       

        context->current_rule->set_metadata(rule_id, object_type, context->current_rule_summary);
        nlohmann::json response;
        response["status"] = "Rule initiated for pattern '" + pattern_type + "'.";
        response["required_roles"] = context->current_rule->get_required_roles();
        return response.dump();
    }

    if (tool_name == "nominate_node_for_role") {
        if (!context->current_rule) {
            return R"({"error": "No active rule. You must call 'start_rule' first."})";
        }
        std::string role = arguments.at("role").get<std::string>();
        int node_id = arguments.at("node_id").get<int>();

        // **根据当前规则验证角色名称**
        const auto& valid_roles = context->current_rule->get_required_roles();
        if (std::find(valid_roles.begin(), valid_roles.end(), role) == valid_roles.end()) {
            std::string valid_roles_str;
            for(const auto& r : valid_roles) { valid_roles_str += "'" + r + "', "; }
            return R"({"error": "Invalid role ')" + role + R"('. For a ')" + context->current_rule->get_pattern_type() + R"(' rule, role must be one of: )" + valid_roles_str + R"("})";
        }

        if (!ccpg_->getNodeByID(node_id)) {
            return R"({"error": "Node with ID )" + std::to_string(node_id) + R"( not found."})";
        }
        
        context->current_rule->set_node_for_role(role, node_id);
        
        return R"({"status": "Node )" + std::to_string(node_id) + R"( nominated for role ')" + role + R"('. Nominate other required nodes or call 'propose_rule_for_confirmation'."})";
    }

    if (tool_name == "propose_rule_for_confirmation") {
        if (!context->current_rule) {
            return R"({"error": "No active rule to propose."})";
        }
        if (!context->current_rule->is_ready()) {
            return R"({"error": "Rule is not ready. Not all required roles have been filled."})";
        }
        
        std::string summary = context->current_rule->generate_confirmation_summary(ccpg_, context->current_rule_summary);
        return nlohmann::json{{"confirmation_request", summary}}.dump();
    }

    if (tool_name == "confirm_rule") {
        if (!context->current_rule) {
            return R"({"error": "No active rule to confirm."})";
        }
        bool is_correct = arguments.at("is_semantically_correct").get<bool>();
        if (is_correct) {
            // **将 unique_ptr 移动到 ThreadPairAnalysis 中**
            context->pair->analysis.temporal_rules.push_back(std::move(context->current_rule));
            context->current_rule_summary.clear();
            return R"({"status": "Rule confirmed and recorded. You can start a new rule or finish analysis."})";
        } else {
            context->current_rule.reset(); // 丢弃规则
            context->current_rule_summary.clear();
            return R"({"status": "Rule discarded based on your review. You can start a new rule or finish analysis."})";
        }
    }

    if (tool_name == "finish_analysis") {
        return "finish";
    }

    return R"({"error": "Tool not implemented."})";
}

// 驱动整个分析流程的入口函数
void ParallelAnalysisAgent::analyze_parallelism(ThreadPair& pair) {
    reset();
    std::string contract1_str = contract_to_string(pair.contract1);
    std::string contract2_str = contract_to_string(pair.contract2);
    RuleBuildingContext context{&pair, nullptr, ""};
    
    std::string entry_func1_code = "[Source code not available]";
    if (pair.thread1 && pair.thread1->getThreadMainFunction() && pair.thread1->getThreadMainFunction()->getFuncNode()) {
        entry_func1_code = pair.thread1->getThreadMainFunction()->getFuncNode()->getCPGNode()->getCode();
    }
    
    std::string entry_func2_code = "[Source code not available]";
    if (pair.thread2 && pair.thread2->getThreadMainFunction() && pair.thread2->getThreadMainFunction()->getFuncNode()) {
        entry_func2_code = pair.thread2->getThreadMainFunction()->getFuncNode()->getCPGNode()->getCode();
    }
    
    std::stringstream prompt_ss;
    prompt_ss << "Please perform the multi-stage analysis on the following thread pair.\n\n"
              << "**Contract for Thread 1 (ID: " << pair.thread1->getId() << "):**\n" << contract1_str << "\n"
              << "**Contract for Thread 2 (ID: " << pair.thread2->getId() << "):**\n" << contract2_str << "\n"
              << "--- Analysis Context ---\n"
              << "Thread " << pair.thread1->getId() << " entry function (ID=" << pair.contract1.entryPointFunctionId
              << ", Name='" << pair.thread1->getThreadMainFunction()->getFuncNode()->getCPGNode()->getName() << "'):\n"
              << "```cpp\n" << entry_func1_code << "\n```\n\n"
              << "Thread " << pair.thread2->getId() << " entry function (ID=" << pair.contract2.entryPointFunctionId
              << ", Name='" << pair.thread2->getThreadMainFunction()->getFuncNode()->getCPGNode()->getName() << "'):\n"
              << "```cpp\n" << entry_func2_code << "\n```\n\n"
              << "Please begin with Stage 1: High-Level Concurrency Assessment.";

    send_message(prompt_ss.str(), &context);
}

std::string ParallelAnalysisAgent::parseResult(const std::vector<ChatMessage>& history) {
    // 因为所有状态都通过工具调用来处理，这个函数可以返回一个简单的完成消息。
    return "Analysis complete.";
}

} // namespace llm_client