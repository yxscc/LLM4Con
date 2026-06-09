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

    // Modernized thread-contract ("old story") entry: surface-driven lazy/dedup
    // per-thread contracts + per-shared-object interleaving agent -> grounded
    // hypotheses (stored in confirmedHypotheses_, consumed via
    // getConfirmedHypotheses + StatefulBugDetector::detectFromHypotheses).
    // useContracts=false is the headline ablation (reason from source only).
    void runAnalysisContractMode(bool useContracts = true);

    // Legacy per-thread-contract + per-pair + Rule-template workflow.
    // Superseded by runAnalysisContractMode; kept for reference / off main path.
    std::vector<ThreadPair> runAnalysisLegacy();

    const std::vector<query::Hypothesis>& getConfirmedHypotheses() const {
        return confirmedHypotheses_;
    }

    // Exposed so downstream phases (Phase 4 LLM hypothesis-verifier filter)
    // can reuse the same client/connection without re-reading credentials.
    std::shared_ptr<LLMClient> getLLMClient() const { return llmClient; }

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
