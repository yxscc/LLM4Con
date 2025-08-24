#ifndef AGENT_MANAGER_H
#define AGENT_MANAGER_H

#include <memory>
#include "LLMUtil/LLMClient.h"
#include "LLMUtil/FindingThreadEntryAgent.h"
#include "LLMUtil/ContractGeneratorAgent.h"
#include "LLMUtil/ParallelAnalysisAgent.h"
#include "CCPG/CCPG.h"

namespace llm_client {

#include "LLMUtil/ConcurrencyContract.h"
#include <vector>

class AgentManager {
public:
    AgentManager(CCPG* cpg);
    std::vector<ThreadPair> runAnalysis();

private:
    std::shared_ptr<LLMClient> llmClient;
    FindingThreadEntryAgent entryFinder;
    ContractGeneratorAgent contractGenerator;
    ParallelAnalysisAgent parallelAnalyzer;
    CCPG* ccpg;
};

} // namespace llm_client

#endif // AGENT_MANAGER_H