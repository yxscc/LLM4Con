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

// --- StatefulBug Implementation (no change needed) ---

StatefulBug::StatefulBug(
    const llm_client::StatefulRule& violated_rule,
    const std::vector<std::pair<std::string, NodeLoc>>& violation_path,
    const llm_client::ThreadPair& thread_pair
) : rule(violated_rule), path(violation_path), threads(thread_pair) {}

std::string StatefulBug::toString() const {
    std::stringstream ss;
    ss << "========== Stateful Protocol Violation Detected ==========\n"
       << "Rule Violated: " << rule["rule_id"].get<std::string>() << "\n"
       << "Description: " << rule["description"].get<std::string>() << "\n"
       << "Shared Object Type: " << rule["shared_object_type"].get<std::string>() << "\n\n"
       << "Violation observed between Thread " << threads.thread1->getId() << " and Thread " << threads.thread2->getId() << ".\n\n"
       << "--- Forbidden Sequence Trace ---\n";
    
    for (const auto& step : path) {
        ss << "  -> " << step.first << " at " << step.second.toString() << "\n";
    }
    
    ss << "==========================================================";
    return ss.str();
}


// --- NEW HEURISTIC SEARCH HELPER ---
/**
 * @brief Heuristically searches forward from a start_node for a "use" of a shared object.
 * A "use" is defined as any function call that takes the shared object instance (or an alias)
 * as an argument.
 * @return True if a potential "use" is found, false otherwise.
 */
bool find_heuristic_use_path(
    CCPGNode* start_node,
    const llvm::Value* shared_object_instance,
    const std::string& object_type_name,
    int search_limit,
    std::vector<std::pair<std::string, NodeLoc>>& use_path)
{
    AliasChecker* aliasChecker = AliasChecker::getInstance();
    std::queue<CCPGNode*> worklist;
    std::set<CCPGNode*> visited;

    worklist.push(start_node);
    visited.insert(start_node);
    int nodes_searched = 0;

    while (!worklist.empty() && nodes_searched < search_limit) {
        CCPGNode* current_node = worklist.front();
        worklist.pop();
        nodes_searched++;

        if (current_node != start_node && current_node->isCallSite()) {
            const llvm::Value* arg_val = aliasChecker->getLLVMValueForArgument(current_node, object_type_name);
            if (arg_val && aliasChecker->isAlias(shared_object_instance, arg_val)) {
                use_path.push_back({current_node->getCPGNode()->getName(), current_node->getNodeLoc()});
                return true; // Found a potential "use"
            }
        }

        // Traverse forward along the CFG within the same function.
        for (CCPGEdge* edge : current_node->getOutEdges()) {
            if (edge->getType() == CCPGEdge::EdgeType::ORDER) { // ORDER edge represents CFG
                CCPGNode* next_node = edge->getDst();
                if (next_node->getFunction() == start_node->getFunction() && visited.find(next_node) == visited.end()) {
                    visited.insert(next_node);
                    worklist.push(next_node);
                }
            }
        }
    }
    return false; // No subsequent "use" found within the search limit.
}


// --- REWRITTEN DETECT FUNCTION ---
void StatefulBugDetector::detect(
    const std::vector<llm_client::ThreadPair>& threadPairs,
    const std::set<const llvm::Value*>& /*candidateSharedObjects*/) // candidateSharedObjects no longer needed here
{
    std::cout << "\n[Phase 4: Detecting Stateful Protocol Violations (Heuristic Approach)]" << std::endl;

    CCPG* ccpg = ThreadCreationTree::getInstance()->getCCPG();
    AliasChecker* aliasChecker = AliasChecker::getInstance();

    for (const auto& pair : threadPairs) {
        if (!pair.analysis.actually_concurrent) {
            continue;
        }

        for (const auto& rule : pair.analysis.temporal_rules) {
            if (rule.value("pattern_type", "") != "TOCTOU") {
                continue;
            }

            std::string object_type_name = rule.value("shared_object_type", "");
            std::string check_func_name = rule.value("state_check_function", "");
            std::string modify_func_name = rule.value("state_modify_function", "");

            if (check_func_name.empty() || modify_func_name.empty()) {
                continue;
            }

            // STAGE 1: Find all potential check and modify sites
            CCPGNodeSet check_sites;
            for (CCPGNode* node : pair.thread1->getNodes()) {
                if (node->isCallSite() && node->getCPGNode()->getName() == check_func_name) {
                    check_sites.insert(node);
                }
            }

            CCPGNodeSet modify_sites;
            for (CCPGNode* node : pair.thread2->getNodes()) {
                if (node->isCallSite() && node->getCPGNode()->getName() == modify_func_name) {
                    modify_sites.insert(node);
                }
            }
            
            // STAGE 2: Find a concurrent, aliased pair of (check, modify)
            for (CCPGNode* check_node : check_sites) {
                for (CCPGNode* modify_node : modify_sites) {
                    
                    const llvm::Value* check_val = aliasChecker->getLLVMValueForArgument(check_node, object_type_name);
                    const llvm::Value* modify_val = aliasChecker->getLLVMValueForArgument(modify_node, object_type_name);

                    if (check_val && modify_val && aliasChecker->isAlias(check_val, modify_val)) {
                        
                        // STAGE 3: Heuristically search for a "Use" after the "Check"
                        std::vector<std::pair<std::string, NodeLoc>> use_path;
                        if (find_heuristic_use_path(check_node, check_val, object_type_name, 100, use_path)) {
                            std::cout << "    [+] POTENTIAL TOCTOU VIOLATION FOUND for rule: " << rule["rule_id"].get<std::string>() << std::endl;
                            
                            std::vector<std::pair<std::string, NodeLoc>> full_violation_path;
                            full_violation_path.push_back({"[CHECK] Function '" + check_func_name + "'", check_node->getNodeLoc()});
                            full_violation_path.push_back({"[MODIFY - Concurrent] Function '" + modify_func_name + "'", modify_node->getNodeLoc()});
                            full_violation_path.push_back({"[USE] Heuristically found use via function '" + use_path[0].first + "'", use_path[0].second});

                            this->detectedBugs.emplace_back(rule, full_violation_path, pair);
                            goto next_rule; // Found one instance for this rule, move to the next rule.
                        }
                    }
                }
            }
        next_rule:;
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