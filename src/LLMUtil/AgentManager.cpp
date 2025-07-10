#include "LLMUtil/AgentManager.h"
#include <iostream>
#include <vector>
#include "CCPG/CCPGNode.h"
#include "CPG/Node.h"

namespace llm_client {

AgentManager::AgentManager(CCPG* cpg)
    : llmClient(LLMClient::get_shared_instance(std::getenv("LLM_API_URL"), std::getenv("LLM_API_KEY"))),
      entryFinder(llmClient),
      parallelAnalyzer(llmClient),
      ccpg(cpg) {
    if (!llmClient) {
        std::cerr << "Failed to initialize LLM Client. Please check URL and API key." << std::endl;
    }
}

void AgentManager::runAnalysis() {
    if (!llmClient || !ccpg) {
        std::cerr << "LLM Client or CCPG not initialized. Aborting analysis." << std::endl;
        return;
    }

    std::cout << "\n--- Step 1: Finding Thread Entry Points ---" << std::endl;
    
    std::vector<int> threadEntryFuncIds;
    CCPGNodeSet forkNodes = ccpg->getNodesByType(ThreadAPIUtil::TYPE::FORK);

    for (CCPGNode* node : forkNodes) {
        Node* cpgNode = node->getCPGNode();
        // Check if the node is a call and its name is pthread_create
        if (cpgNode && cpgNode->getType() == "Call" && cpgNode->getMethodFullName() == "pthread_create") {
            // The 3rd argument to pthread_create is the start routine. Joern's argument index is 1-based.
            Node* argNode = cpgNode->getArgument(3); 
            if (argNode) {
                // The argument node's code/name holds the name of the thread entry function.
                const std::string& entryFuncName = argNode->getName();
                
                bool found = false;
                // Now, find the function definition node (METHOD type) in the entire CCPG
                for (CCPGNode* funcDefNode : ccpg->getNodes()) {
                    Node* funcDefCpgNode = funcDefNode->getCPGNode();
                    if (funcDefCpgNode && funcDefCpgNode->getType() == "Method" && funcDefCpgNode->getName() == entryFuncName) {
                        threadEntryFuncIds.push_back(funcDefNode->getId());
                        std::cout << "Found thread entry point: '" << entryFuncName << "' (Function ID: " << funcDefNode->getId() << ")" << std::endl;
                        found = true;
                        break; 
                    }
                }
                if (!found) {
                    std::cerr << "Warning: Could not find function definition for thread entry '" << entryFuncName << "'" << std::endl;
                }
            }
        }
    }

    if (threadEntryFuncIds.size() < 2) {
        std::cout << "Found fewer than two thread entry points. No parallel analysis needed." << std::endl;
        return;
    }

    // For simplicity, we'll just analyze the first two found entry points.
    int func_id_1 = threadEntryFuncIds[0];
    int func_id_2 = threadEntryFuncIds[1];

    std::cout << "\n--- Step 2: Analyzing Parallel Execution for functions " << func_id_1 << " and " << func_id_2 << " ---" << std::endl;
    
    bool can_run_in_parallel = parallelAnalyzer.analyze_parallelism(func_id_1, func_id_2);

    std::cout << "\n--- Analysis Result ---" << std::endl;
    if (can_run_in_parallel) {
        std::cout << "Conclusion: The two threads CAN potentially run in parallel." << std::endl;
    } else {
        std::cout << "Conclusion: The two threads CANNOT run in parallel due to synchronization constraints." << std::endl;
    }
}

} // namespace llm_client
