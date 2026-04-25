#ifndef AGENT_MANAGER_H
#define AGENT_MANAGER_H

#include <memory>
#include "LLMUtil/LLMClient.h"
#include "LLMUtil/FindingThreadEntryAgent.h"
#include "LLMUtil/ContractGeneratorAgent.h"
#include "LLMUtil/ParallelAnalysisAgent.h"
#include "Query/HypothesisVerifier.h"
#include "CCPG/CCPG.h"

namespace llm_client {

#include "LLMUtil/ConcurrencyContract.h"
#include <vector>

class AgentManager {
public:
    AgentManager(CCPG* cpg);

    std::vector<ThreadPair> runAnalysis();

    // New: open-hypothesis agent mode
    std::vector<ThreadPair> runAnalysisAgentMode();

    // Legacy per-thread-contract + per-pair workflow
    std::vector<ThreadPair> runAnalysisLegacy();

    const std::vector<query::Hypothesis>& getConfirmedHypotheses() const {
        return confirmedHypotheses_;
    }

private:
    std::shared_ptr<LLMClient> llmClient;
    FindingThreadEntryAgent entryFinder;
    ContractGeneratorAgent contractGenerator;
    ParallelAnalysisAgent parallelAnalyzer;
    CCPG* ccpg;
    std::vector<query::Hypothesis> confirmedHypotheses_;
};

} // namespace llm_client

#endif // AGENT_MANAGER_H
