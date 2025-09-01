#pragma once

#include "LLMUtil/ConcurrencyContract.h"
#include "CCPG/AliasChecker.h"
#include "LLMUtil/Rule.h"
#include <string>
#include <vector>

class Thread; // Forward declaration

namespace llm_client {

using StatefulRule = nlohmann::json;

// Describes the final analysis of a pair of threads
struct ThreadPairAnalysis {
    bool designed_for_parallelism = false;
    std::string design_reasoning;

    bool actually_concurrent = false;
    std::string concurrency_reasoning;

    MemoryAccessMap accessMap1;
    MemoryAccessMap accessMap2;

    std::vector<std::unique_ptr<Rule>> temporal_rules; 
};

// Describes a pair of threads to be analyzed
struct ThreadPair {
    Thread* thread1;
    Thread* thread2;
    const LLM::ConcurrencyContract& contract1;
    const LLM::ConcurrencyContract& contract2;

    // Analysis Results
    ThreadPairAnalysis analysis;

    ThreadPair(Thread* t1, const LLM::ConcurrencyContract& c1, Thread* t2, const LLM::ConcurrencyContract& c2)
        : thread1(t1), contract1(c1), thread2(t2), contract2(c2) {}
};

} // namespace llm_client
