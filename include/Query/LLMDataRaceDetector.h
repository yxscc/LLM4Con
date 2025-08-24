#pragma once

#include "LLMUtil/ConcurrencyContract.h"
#include "CCPG/CCPGNode.h"
#include "LLMUtil/ThreadPair.h"
#include <vector>
#include <string>
#include <set>
#include <filesystem>

namespace fs = std::filesystem;

class ThreadPair;

namespace query {

// Represents a single data race finding based on LLM Contracts
class LLMDataRace {
public:
    LLMDataRace(
        const LLM::ConcurrencyContract::SharedVariable& var1,
        const LLM::ConcurrencyContract& contract1,
        const LLM::ConcurrencyContract::SharedVariable& var2,
        const LLM::ConcurrencyContract& contract2,
        const std::string& reason,
        const NodeLoc& loc1,
        const NodeLoc& loc2
    );

    std::string toString() const;

private:
    LLM::ConcurrencyContract::SharedVariable variable1;
    const LLM::ConcurrencyContract& contract1;
    LLM::ConcurrencyContract::SharedVariable variable2;
    const LLM::ConcurrencyContract& contract2;
    std::string reason;
    NodeLoc location1;
    NodeLoc location2;
};


// Detector that finds data races based on a list of ConcurrencyContracts
class LLMDataRaceDetector {
public:
    LLMDataRaceDetector() = default;
    // The main detection method
    void detect(const std::vector<llm_client::ThreadPair>& threadPairs);
    // Prints the detected races to a file
    void printDataRaces(const fs::path& outputDir) const;

private:
    std::vector<LLMDataRace> detectedRaces;
};

} // namespace query
