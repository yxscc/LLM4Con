#include "LLMUtil/AgentManager.h"
#include <iostream>
#include <vector>
#include "CCPG/CCPGNode.h"
#include "CPG/Node.h"
#include "LLMUtil/ConcurrencyContract.h"
#include "CCPG/ThreadCreationTree.h"

namespace llm_client {

AgentManager::AgentManager(CCPG* cpg)
    : llmClient(LLMClient::get_shared_instance(std::getenv("LLM_API_URL"), std::getenv("LLM_API_KEY"))),
      entryFinder(llmClient),
      parallelAnalyzer(llmClient),
      contractGenerator(cpg, llmClient),
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

    std::cout << "\n--- Starting LLM-based Concurrency Analysis ---" << std::endl;

    ThreadCreationTree * tree = ThreadCreationTree::getInstance();
    // 2. Find all thread creation sites

    // 3. Instantiate Agents
    // FindingThreadEntryAgent entryFinder(llmClient); // Already initialized
    // ContractGeneratorAgent contractGenerator(ccpg, llmClient); // Already initialized
    std::vector<LLM::ConcurrencyContract> allContracts;

    // 4. Iterate over each thread creation site to find entry point and generate contract
    std::unordered_set<Thread *> threads = tree->getThreads();
    for(Thread * thread : threads) {
        CCPGNode * forkNode = thread->getForkNode();
        
        // a. Use FindingThreadEntryAgent to get the entry function ID
        int entryFuncId = thread->getThreadMainFunction() ? thread->getThreadMainFunction()->getId() : -1;
        
        if (entryFuncId != -1) {
            std::cout << "Agent identified thread entry function ID: " << entryFuncId << std::endl;
            
            // b. Use ContractGeneratorAgent to generate a contract for this thread
            auto contractOpt = contractGenerator.generateContractForThread(forkNode, entryFuncId);
            
            if (contractOpt) {
                std::cout << "Successfully generated contract for thread entry ID " << entryFuncId << std::endl;
                allContracts.push_back(contractOpt.value());
            } else {
                std::cerr << "Failed to generate contract for thread entry ID " << entryFuncId << std::endl;
            }
        } else {
            std::cerr << "Could not determine thread entry for fork site " << forkNode->getId() << std::endl;
        }
    }

    // 5. Print all generated contracts
    std::cout << "\n--- All Generated Concurrency Contracts ---" << std::endl;
    if (allContracts.empty()) {
        std::cout << "No contracts were generated." << std::endl;
    } else {
        for (const auto& contract : allContracts) {
            std::cout << contract.toJson() << "\n" << std::endl;
        }
    }
    std::cout << "--- LLM-based Analysis Finished ---\n" << std::endl;
}

} // namespace llm_client