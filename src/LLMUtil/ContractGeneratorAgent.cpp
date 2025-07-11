#include "LLMUtil/ContractGeneratorAgent.h"
#include "CPG/Node.h"
#include "CCPG/CCPGNode.h"
#include <iostream>

static std::string build_generator_system_prompt() {
    return "You are an expert software architect specializing in concurrency. Analyze the provided C++ code snippet which represents a thread's execution path. Your task is to deduce the thread's concurrency contract and respond with a single, complete JSON object that strictly follows the specified format. Do not include any explanations or markdown formatting in your response.";
}

ContractGeneratorAgent::ContractGeneratorAgent(CCPG* ccpg, std::shared_ptr<llm_client::LLMClient> client)
    : llm_client::Conversation(client, build_generator_system_prompt(), 15), ccpg(ccpg) {}

std::optional<LLM::ConcurrencyContract> ContractGeneratorAgent::generateContractForThread(LLM::NodeID threadEntryPointNodeID) {
    const CCPGNode* entryNode = ccpg->getNodeByID(threadEntryPointNodeID);
    if (!entryNode || !entryNode->getCPGNode()) {
        std::cerr << "ContractGeneratorAgent: Could not find valid node for ID " << threadEntryPointNodeID << std::endl;
        return std::nullopt;
    }

    std::cout << "ContractGeneratorAgent: Analyzing thread entry function: " << entryNode->getCPGNode()->getName() << std::endl;

    std::string contextCode = getFunctionAndCalleesCode(threadEntryPointNodeID);

    std::string userPrompt = "Based on the following code, generate a concurrency contract. Pay close attention to variable names, lock usage, and the overall purpose of the thread.\n\nCode:\n```cpp\n" + contextCode + "\n```\n\nJSON Response Format:\n" +
    R"({
        "threadEntryPointFunctionID": )" + std::to_string(threadEntryPointNodeID) + R"(,
        "threadName": "ExampleThreadName",
        "role": "WORKER",
        "coreSharedState": [
            {"cpgNodeID": 0, "variableName": "shared_var_name", "typeName": "VarType"}
        ],
        "syncDiscipline": {
            "dataLockBindings": [
                {
                    "lock": {"cpgNodeID": 0, "lockName": "mutex_name", "isCustom": false},
                    "protectedResources": [{"cpgNodeID": 0, "variableName": "shared_var_name", "typeName": "VarType"}]
                }
            ],
            "lockOrders": [],
            "customProtocols": []
        },
        "stateModel": null,
        "happensBeforeConstraints": [],
        "confidenceScore": 0.9,
        "reasoning": "A brief explanation of the deduction.",
        "criticFeedback": [],
        "version": 1
    })";

    std::string llmResponse = send_message(userPrompt, nullptr);
    std::cout << "ContractGeneratorAgent: Received response from LLM." << std::endl;

    try {
        return LLM::ConcurrencyContract::fromJson(llmResponse);
    } catch (const std::exception& e) {
        std::cerr << "ContractGeneratorAgent: Failed to parse LLM response for " << entryNode->getCPGNode()->getName() << ". Error: " << e.what() << std::endl;
        std::cerr << "LLM Response was: " << llmResponse << std::endl;
        return std::nullopt;
    }
}

std::string ContractGeneratorAgent::getFunctionAndCalleesCode(LLM::NodeID functionNodeID, int depth) {
    const CCPGNode* startNode = ccpg->getNodeByID(functionNodeID);
    if (!startNode) {
        return "// Function not found in CPG.\n";
    }
    
    std::string allCode;
    std::set<const CCPGNode*> visited;
    findCalleesRecursive(startNode, allCode, visited, 0, depth);
    return allCode;
}

void ContractGeneratorAgent::findCalleesRecursive(const CCPGNode* currentNode, std::string& code, std::set<const CCPGNode*>& visited, int current_depth, int max_depth) {
    if (!currentNode || !currentNode->getCPGNode() || visited.count(currentNode) || current_depth > max_depth) {
        return;
    }

    visited.insert(currentNode);
    code += "// Function: " + currentNode->getCPGNode()->getName() + " (Line: " + std::to_string(currentNode->getCPGNode()->getLineNumber()) + ")\n";
    code += currentNode->getCPGNode()->getCode() + "\n\n";

    for (const auto* edge : currentNode->getOutEdges()) {
        if (edge->getType() == CCPGEdge::EdgeType::CALL) {
            const CCPGNode* callee = static_cast<const CCPGNode*>(edge->getDst());
            findCalleesRecursive(callee, code, visited, current_depth + 1, max_depth);
        }
    }
}

std::string ContractGeneratorAgent::parseResult(const std::vector<llm_client::ChatMessage>& history) {
    // The raw JSON response is the result.
    if (!history.empty() && history.back().role == llm_client::MessageRole::ASSISTANT) {
        return history.back().content;
    }
    return "";
}