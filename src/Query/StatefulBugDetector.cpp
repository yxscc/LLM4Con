// in src/Query/StatefulBugDetector.cpp

#include "Query/StatefulBugDetector.h"
#include "CCPG/CCPG.h"
#include "CCPG/ThreadCreationTree.h"
#include "CCPG/AliasChecker.h"
#include "PhasarUtil/AnalysisManager.h"
#include "llvm/IR/Value.h"
#include <iostream>
#include <fstream>
#include <queue>
#include <set>

namespace query {

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
                                 const std::set<const llvm::Value*>& candidateSharedObjects)
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
                std::cout << "    [!!!] POTENTIAL " << pattern << " VIOLATION FOUND for rule." << std::endl;
                detectedBugs.push_back(*bug);
            }
        }
    }
}


void StatefulBugDetector::printResults(const fs::path& outputDir) const {
    if (detectedBugs.empty()) {
        std::cout << "No stateful protocol violations detected." << std::endl;
        return;
    }

    fs::path bugsOutputDir = outputDir / "stateful_bugs";
    if (!fs::exists(bugsOutputDir)) {
        fs::create_directory(bugsOutputDir);
    }

    std::ofstream file(bugsOutputDir / "bugs.txt");
    int i = 1;
    for (const auto& bug : detectedBugs) {
        file << bug.toString() << "\n\n";
        file << "count  " << i++ << " ----------------------------------------" << std::endl;
    }
    file.close();
    std::cout << "Stateful bug detection complete. " << detectedBugs.size() 
              << " potential bugs found. Results are in: " << (bugsOutputDir / "bugs.txt") << std::endl;
}

} // namespace query