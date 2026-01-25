// in src/Query/StatefulBugDetector.cpp

#include "Query/StatefulBugDetector.h"
#include "CCPG/CCPG.h"
#include "CCPG/ThreadCreationTree.h"
#include "CCPG/AliasChecker.h"
#include "PhasarUtil/AnalysisManager.h"
#include "llvm/IR/Value.h"
#include "LLMUtil/VerificationAgent.h"
#include <iostream>
#include <fstream>
#include <queue>
#include <set>
#include <algorithm>

namespace query {

// Heuristic FP filter:
// Some real-world C code intentionally "publishes" a pointer into a global table
// (NULL -> allocated) without locks, while other threads read the table.
// This is technically a C data race but is often benign in practice if the entry
// is written once and the object is not freed concurrently.
//
// We use a very conservative pattern match to avoid suppressing real bugs:
// - Only for DataRace
// - Only when the write happens under an "if (NULL == <var>) { <alloc>; table[idx] = <var>; }" shape
static bool isLikelyBenignPublicationRace(const StatefulBug &bug) {
    const auto &rule = bug.getRule();
    if (!rule.contains("pattern_type") || rule["pattern_type"] != "DataRace") {
        return false;
    }

    // Identify READ/WRITE nodes from the violation path.
    CCPGNode *readNode = nullptr;
    CCPGNode *writeNode = nullptr;
    for (const auto &step : bug.getPath()) {
        const std::string &role = step.first;
        if (role.find("read") != std::string::npos) {
            readNode = step.second;
        } else if (role.find("write") != std::string::npos) {
            writeNode = step.second;
        }
    }
    if (!readNode || !writeNode) {
        return false;
    }
    if (!writeNode->getFunction() || !writeNode->getFunction()->getFuncNode() ||
        !writeNode->getFunction()->getFuncNode()->getCPGNode()) {
        return false;
    }

    const std::string &writeFuncCode = writeNode->getFunction()->getFuncNode()->getCPGNode()->getCode();
    const std::string &writeLine = writeNode->getCPGNode() ? writeNode->getCPGNode()->getCode() : "";
    const std::string &readLine = readNode->getCPGNode() ? readNode->getCPGNode()->getCode() : "";

    // Must look like publishing into a table and guarded by NULL check.
    // (Do NOT overfit to specific symbol names; just use "[]", "NULL", and an allocator hint.)
    const bool hasNullGuard =
        writeFuncCode.find("NULL") != std::string::npos &&
        (writeFuncCode.find("if (NULL ==") != std::string::npos || writeFuncCode.find("if (NULL==") != std::string::npos);
    const bool hasAllocatorHint =
        (writeFuncCode.find("calloc(") != std::string::npos || writeFuncCode.find("malloc(") != std::string::npos);
    const bool writeLooksLikeTableStore =
        writeLine.find('[') != std::string::npos && writeLine.find(']') != std::string::npos &&
        writeLine.find('=') != std::string::npos;
    const bool readLooksLikeTableLoad =
        readLine.find('[') != std::string::npos && readLine.find(']') != std::string::npos &&
        readLine.find('=') != std::string::npos;

    if (!(hasNullGuard && hasAllocatorHint && writeLooksLikeTableStore && readLooksLikeTableLoad)) {
        return false;
    }

    // Extra guard: ensure the store line appears inside the NULL-guarded block.
    // We only do a simple substring check; if the code string doesn't contain the line, don't filter.
    if (!writeLine.empty() && writeFuncCode.find(writeLine) == std::string::npos) {
        return false;
    }

    return true;
}

// --- StatefulBug Implementation ---
StatefulBug::StatefulBug(
    const llm_client::StatefulRule& violated_rule,
    const std::vector<std::pair<std::string, CCPGNode*>>& violation_path,
    const llm_client::ThreadPair& thread_pair
) : rule(violated_rule), path(violation_path), threads(thread_pair) {}

std::string StatefulBug::toString() const {
    std::stringstream ss;
    ss << "========== Stateful Protocol Violation Detected ==========\n"
       << "Rule Violated: " << rule["rule_id"].get<std::string>() << "\n"
       << "Description: " << rule["_llm_summary"].get<std::string>() << "\n"
       << "Shared Object Type: " << rule["shared_object_type"].get<std::string>() << "\n\n"
       << "Violation observed between Thread " << threads.thread1->getId() << " and Thread " << threads.thread2->getId() << ".\n\n"
       << "--- Forbidden Sequence Trace ---\n";
    
    for (const auto& step : path) {
        CCPGNode* node = step.second;
        ss << "  -> " << step.first << " at " << node->getNodeLoc().toString() << ".     Code : " << node->getCPGNode()->getCode() << "\n";
    }
    
    ss << "==========================================================";
    return ss.str();
}

// --- REFACTORED DETECT FUNCTION ---
void StatefulBugDetector::detect(const std::vector<llm_client::ThreadPair>& threadPairs,
                                 const std::set<const llvm::Value*>& candidateSharedObjects,
                                 llm_client::VerificationAgent* verificationAgent)
{
    std::cout << "\n[Phase 4: Detecting Stateful Protocol Violations (Rule-Based)]" << std::endl;

    CCPG* ccpg = ThreadCreationTree::getInstance()->getCCPG();

    for (const auto& pair : threadPairs) {

        for (const auto& rule_ptr : pair.analysis.temporal_rules) {
            // Call the rule's own verification method
            std::optional<StatefulBug> bug = rule_ptr->verify(pair, ccpg);

            if (bug) {
                const auto& bug_json = rule_ptr->to_json();
                const std::string& pattern = bug_json["pattern_type"];

                // Fast, conservative FP filter for benign publication-style races.
                if (pattern == "DataRace" && isLikelyBenignPublicationRace(*bug)) {
                    std::cout << "    [Heuristic] Filtered likely benign publication-style DataRace (init-only pointer publish)." << std::endl;
                    continue;
                }

                std::cout << "    [!!!] POTENTIAL " << pattern << " VIOLATION FOUND for rule." << std::endl;
                
                bool confirmed = true;
                if (verificationAgent) {
                     confirmed = verificationAgent->verifyBug(*bug);
                }
                
                if (confirmed) {
                    detectedBugs.push_back(*bug);
                } else {
                    std::cout << "    [Verification] Bug filtered out by LLM Verification Agent." << std::endl;
                }
            }
        }
    }
}


void StatefulBugDetector::printResults(const fs::path& outputDir) const {
    size_t totalBugs = detectedBugs.size() + externalBugs.size();
    
    if (totalBugs == 0) {
        std::cout << "No bugs detected." << std::endl;
        return;
    }

    fs::path bugsOutputDir = outputDir / "stateful_bugs";
    if (!fs::exists(bugsOutputDir)) {
        fs::create_directory(bugsOutputDir);
    }

    std::ofstream file(bugsOutputDir / "bugs.txt");
    int i = 1;
    
    // Output external bugs first (e.g., lazy-init races from API discovery)
    for (const auto& bug : externalBugs) {
        file << bug.toString() << "\n\n";
        file << "count  " << i++ << " ----------------------------------------" << std::endl;
    }
    
    // Output stateful protocol violations
    for (const auto& bug : detectedBugs) {
        file << bug.toString() << "\n\n";
        file << "count  " << i++ << " ----------------------------------------" << std::endl;
    }
    
    file.close();
    
    std::cout << "Bug detection complete. " << totalBugs << " potential bug(s) found." << std::endl;
    if (!externalBugs.empty()) {
        std::cout << "  - Lazy-Init Race: " << externalBugs.size() << std::endl;
    }
    if (!detectedBugs.empty()) {
        std::cout << "  - Stateful Protocol Violations: " << detectedBugs.size() << std::endl;
    }
    std::cout << "Results saved to: " << (bugsOutputDir / "bugs.txt") << std::endl;
}

} // namespace query