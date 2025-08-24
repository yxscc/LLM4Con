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
        {"get_function_by_name", "Get a function by its exact name.", {
            {"name", "string", "The name of the function to find.", true}
        }},
        {"get_callees", "Get all functions called from within a given function.", {
            {"function_id", "number", "The ID of the calling function.", true}
        }},
        {"get_nodes_by_location", "Get all node IDs at a specific file and line number.", {
            {"file_path", "string", "The absolute or relative path to the source file.", true},
            {"line_number", "number", "The line number in the source file.", true}
        }},
        {"get_control_flow_parents", "Get the immediate parent(s) of a node in the Control Flow Graph.", {
            {"node_id", "number", "The ID of the node whose CFG parents are to be found.", true}
        }}
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

    if (tool_name == "get_function") {
        int node_id = arguments.at("node_id").get<int>();
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

    if (tool_name == "get_function_by_name") {
        std::string name = arguments.at("name").get<std::string>();
        std::unordered_set<Node*> nodes = ccpg->getCPG()->findMethodsByName(name);
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
        return calleesJson.dump();
    }

    if (tool_name == "get_nodes_by_location") {
        std::string file_path = arguments.at("file_path").get<std::string>();
        int line_number = arguments.at("line_number").get<int>();
        
        NodeLoc loc(file_path, line_number, nullptr);
        CCPGNodeSet nodes = ccpg->getNodesByLoc(loc);

        nlohmann::json result = nlohmann::json::array();
        for (CCPGNode* node : nodes) {
            nlohmann::json node_info = {
                {"node_id", node->getId()},
                {"node_type", ThreadAPIUtil::getTypeString(node->getType())},
                {"code", node->getCPGNode()->getCode()}
            };
            result.push_back(node_info);
        }
        return result.dump();
    }

    if (tool_name == "get_control_flow_parents") {
        int node_id = arguments.at("node_id").get<int>();
        CCPGNode* node = ccpg->getNodeByID(node_id);
        if (!node) {
            return R"({"error": "Node not found for ID: )" + std::to_string(node_id) + R"("})";
        }

        nlohmann::json parents = nlohmann::json::array();
        for (CCPGEdge* edge : node->getInEdges()) {
            // A more robust implementation would check `edge->getType() == CFG`
            CCPGNode* parent = edge->getSrc();
            nlohmann::json parent_info = {
                {"node_id", parent->getId()},
                {"node_type", ThreadAPIUtil::getTypeString(parent->getType())},
                {"code", parent->getCPGNode()->getCode()}
            };
            parents.push_back(parent_info);
        }
        return parents.dump();
    }

    // If the tool name doesn't match any shared tool, return nullopt
    return std::nullopt;
}

} // namespace SharedToolKit
} // namespace llm_client
