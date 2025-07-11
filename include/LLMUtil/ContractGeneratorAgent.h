#ifndef CONTRACTGENERATORAGENT_H
#define CONTRACTGENERATORAGENT_H

#include "LLMUtil/ConcurrencyContract.h"
#include "LLMUtil/Conversation.h"
#include "CCPG/CCPG.h"
#include <string>
#include <optional>
#include <set>

class ContractGeneratorAgent : public llm_client::Conversation {
public:
    ContractGeneratorAgent(CCPG* ccpg, std::shared_ptr<llm_client::LLMClient> client);

    // Generates a single contract for a given thread entry function
    std::optional<LLM::ConcurrencyContract> generateContractForThread(LLM::NodeID threadEntryPointNodeID);

private:
    // Helper to get the code context for the LLM prompt
    std::string getFunctionAndCalleesCode(LLM::NodeID functionNodeID, int depth = 1);
    void findCalleesRecursive(const CCPGNode* currentNode, std::string& code, std::set<const CCPGNode*>& visited, int current_depth, int max_depth);

    // Overrides from Conversation
    std::string parseResult(const std::vector<llm_client::ChatMessage>& history) override;

    CCPG* ccpg;
};

#endif // CONTRACTGENERATORAGENT_H