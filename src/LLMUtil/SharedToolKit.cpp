#include "LLMUtil/SharedToolKit.h"
#include "CCPG/CCPG.h"
#include "CCPG/CCPGNode.h"
#include "CPG/CPG.h"
#include "phasar.h"
#include "PhasarUtil/LLVMAnalyzer.h"

using namespace psr;

namespace llm_client {
namespace SharedToolKit {

std::vector<Tool> get_shared_tools() {
    return {
        {"get_function", "Get the function containing a node ID.", {
            {"node_id", "number", "The ID of the node within the target function.", true}
        }},
        {"get_function_ops", "Get a function's operation nodes (CFG/CCPG nodes) with code & locations (useful when the raw function body is truncated).", {
            {"function_id", "number", "The ID of the function to inspect.", true}
        }},
        {"get_cpg_method_by_name", "Get CPG Method node(s) by exact name (returns CPG node IDs; useful when the function isn't in CCPG yet, e.g., thread entry passed as function pointer).", {
            {"name", "string", "The exact method/function name to find.", true}
        }},
        {"get_function_by_name", "Get a function by its exact name.", {
            {"name", "string", "The name of the function to find.", true}
        }},
        {"get_function_by_id", "Get a function by its ID.", {
            {"function_id", "number", "The ID of the function to find.", true}
        }},
        {"get_callees", "Get all functions called from within a given function.", {
            {"function_id", "number", "The ID of the calling function.", true}
        }},
        {"get_callers", "Get all functions that call a given function.", {
            {"function_id", "number", "The ID of the function being called.", true}
        }},
    };
}

std::optional<std::string> handle_shared_tool(
    const std::string& tool_name, 
    const nlohmann::json& arguments, 
    CCPG* ccpg) 
{
    if (!ccpg) {
        return R"({"error": "CCPG context is not available."})";
    }

    if (tool_name == "get_function_ops") {
        int function_id = arguments.at("function_id").get<int>();
        ccpg::Function* function = ccpg->getFunctionById(function_id);
        if (!function || !function->getFuncNode() || !function->getFuncNode()->getCPGNode()) {
            return R"({"error": "Function not found for ID: )" + std::to_string(function_id) + R"("})";
        }

        nlohmann::json ops = nlohmann::json::array();
        for (CCPGNode* node : function->getNodes()) {
            if (!node || !node->getCPGNode()) continue;
            const std::string& code = node->getCPGNode()->getCode();
            if (code.empty() || code == "<empty>") continue;
            ops.push_back({
                {"node_id", node->getId()},
                {"code", code},
                {"location", node->getNodeLoc().toString()}
            });
        }

        nlohmann::json result = {
            {"function_id", function->getId()},
            {"function_name", function->getFuncNode()->getCPGNode()->getName()},
            {"operations", ops}
        };
        return result.dump();
    }

    if (tool_name == "get_cpg_method_by_name") {
        std::string name = arguments.at("name").get<std::string>();
        const CPG* cpg = ccpg->getCPG();
        if (!cpg) {
            return R"({"error": "CPG context is not available."})";
        }

        std::unordered_set<Node*> nodes = cpg->findMethodsByName(name);

        nlohmann::json methods = nlohmann::json::array();
        for (Node* node : nodes) {
            if (!node) continue;
            long long id_num = -1;
            try {
                id_num = std::stoll(std::string(node->getId()));
            } catch (...) {
                continue;
            }
            std::string method_name = node->getName();
            if (method_name.empty()) {
                method_name = node->getMethodFullName();
            }
            nlohmann::json info = {
                {"cpg_node_id", id_num},
                {"method_name", method_name},
                {"method_code", node->getCode()},
                {"file", node->getProperty("FILENAME")},
                {"line", node->getLineNumber()}
            };
            methods.push_back(info);
        }
        return methods.dump();
    }

    if (tool_name == "get_function") {
        int node_id = -1;
        if (arguments.contains("node_id")) {
            node_id = arguments.at("node_id").get<int>();
        } else if (arguments.contains("function_id")) {
            node_id = arguments.at("function_id").get<int>();
        } else {
            return R"({"error": "Missing required argument 'node_id' or 'function_id'."})";
        }
        
        CCPGNode* node = ccpg->getNodeByID(node_id);
        ccpg::Function* function = node ? node->getFunction() : nullptr;
        if (function) {
            nlohmann::json result = {
                {"function_id", function->getId()},
                {"function_name", function->getFuncNode()->getCPGNode()->getName()},
                {"function_body", function->getFuncNode()->getCPGNode()->getCode()}
            };
            return result.dump();
        } else {
            return R"({"error": "Function not found for node ID: )" + std::to_string(node_id) + R"("})";
        }
    }

    if (tool_name == "get_function_by_id") {
        int function_id = arguments.at("function_id").get<int>();
        ccpg::Function* function = ccpg->getFunctionById(function_id);
        if (function) {
            nlohmann::json result = {
                {"function_id", function->getId()},
                {"function_name", function->getFuncNode()->getCPGNode()->getName()},
                {"function_body", function->getFuncNode()->getCPGNode()->getCode()}
            };
            return result.dump();
        } else {
            return R"({"error": "Function not found for ID: )" + std::to_string(function_id) + R"("})";
        }
    }

    if (tool_name == "get_function_by_name") {
        std::string name = arguments.at("name").get<std::string>();
        std::unordered_set<Node*> nodes = ccpg->getCPG()->findMethodsByName(name);

        // Fallback: Fuzzy search if no exact match found
        if (nodes.empty()) {
            CPGNodeSet all_methods = ccpg->getCPG()->getNodesByType("Method");
            for (Node* methodNode : all_methods) {
                std::string methodName = methodNode->getName();
                // Check for substring match
                if (methodName.find(name) != std::string::npos && methodNode->getProperty("CODE") != "<empty>") {
                     if(methodNode->outCFGEdges.size() == 1){
                        std::unordered_set<Edge*> outEdges = methodNode->outCFGEdges;
                        Edge* edge = *outEdges.begin();
                        Node* nextNode = edge->getToNode();
                        if(nextNode->getType() == "Method_return"){
                            continue;
                        }
                    }
                    nodes.insert(methodNode);
                }
            }
        }

        nlohmann::json functions = nlohmann::json::array();
        for (Node* node : nodes) {
            CCPGNode* function_node = ccpg->getCCPGNodeByCPGNode(node);
            ccpg::Function* function = function_node ? function_node->getFunction() : nullptr;
            if (function) {
                nlohmann::json function_info = {
                    {"function_id", function->getId()},
                    {"function_name", function->getFuncNode()->getCPGNode()->getName()},
                    {"function_body", function->getFuncNode()->getCPGNode()->getCode()}
                };
                functions.push_back(function_info);
            }
        }
        return functions.dump();
    }

    if (tool_name == "get_callees") {
        int function_id = arguments.at("function_id").get<int>();
        ccpg::Function* func = ccpg->getFunctionById(function_id);
        if (!func) {
            return R"({"error": "Function not found for ID: )" + std::to_string(function_id) + R"("})";
        }

        nlohmann::json calleesJson = nlohmann::json::array();
        for (CCPGNode* node : func->getNodes()) {
            if (node->isCallSite()) {
                 CCPGEdge* callEdge = ccpg->hasCallEdge(node);
                if (!callEdge) continue;
                CCPGNode* calleeNode = callEdge->getDst();
                if (calleeNode) {
                     ccpg::Function* callee_func = calleeNode->getFunction();
                     if(callee_func){
                        nlohmann::json calleeInfo = {
                            {"callee_function_id", callee_func->getId()},
                            {"callee_function_name", callee_func->getFuncNode()->getCPGNode()->getName()},
                            {"callsite_code", node->getCPGNode()->getCode()},
                            {"callsite_node_id", node->getId()}
                        };
                        calleesJson.push_back(calleeInfo);
                     }
                }
            }
        }
        nlohmann::json result;
        result["callees"] = calleesJson;
        return result.dump();
    }

    if (tool_name == "get_callers") {
        int function_id = arguments.at("function_id").get<int>();
        ccpg::Function* func = ccpg->getFunctionById(function_id);
        if (!func) {
            return R"({"error": "Function not found for ID: )" + std::to_string(function_id) + R"("})";
        }

        nlohmann::json callersJson = nlohmann::json::array();
        ccpg::FunctionSet callers = func->getCallers();
        for (ccpg::Function* caller_func : callers) {
            if (caller_func) {
                nlohmann::json callerInfo = {
                    {"caller_function_id", caller_func->getId()},
                    {"caller_function_name", caller_func->getFuncNode()->getCPGNode()->getName()}
                };
                callersJson.push_back(callerInfo);
            }
        }
        nlohmann::json result;
        result["callers"] = callersJson;
        return result.dump();
    }

    // If the tool name doesn't match any shared tool, return nullopt
    return std::nullopt;
}

} // namespace SharedToolKit
} // namespace llm_client
