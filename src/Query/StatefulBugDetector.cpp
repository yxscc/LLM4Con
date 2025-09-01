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


/**
 * @brief (重构) 检查在单个线程内，从start_node到end_node是否存在控制流路径.
 * @return True 如果存在路径, false 否则.
 */
bool is_reachable_intra_thread(CCPGNode* start_node, CCPGNode* end_node, Thread* thread) {
    if (!start_node || !end_node || !thread) return false;
    if (start_node == end_node) return true;

    // 确保两个节点都在同一个线程内
    if (thread->getNodes().find(start_node) == thread->getNodes().end() ||
        thread->getNodes().find(end_node) == thread->getNodes().end()) {
        return false;
    }

    std::queue<CCPGNode*> worklist;
    std::set<CCPGNode*> visited;

    worklist.push(start_node);
    visited.insert(start_node);

    while (!worklist.empty()) {
        CCPGNode* current = worklist.front();
        worklist.pop();

        for (CCPGEdge* edge : current->getOutEdges()) {
            // 只沿着CFG边 (ORDER) 探索
            if (edge->getType() == CCPGEdge::EdgeType::ORDER) {
                CCPGNode* next = edge->getDst();
                if (next == end_node) {
                    return true;
                }
                // 只探索在同一个线程内且未访问过的节点
                if (thread->getNodes().count(next) && visited.find(next) == visited.end()) {
                    visited.insert(next);
                    worklist.push(next);
                }
            }
        }
    }
    return false;
}


// --- REWRITTEN DETECT FUNCTION ---
void StatefulBugDetector::detect(
    const std::vector<llm_client::ThreadPair>& threadPairs,
    const std::set<const llvm::Value*>& candidateSharedObjects)
{
    std::cout << "\n[Phase 4: Detecting Stateful Protocol Violations (Rule-Based)]" << std::endl;

    CCPG* ccpg = ThreadCreationTree::getInstance()->getCCPG();
    ThreadCreationTree* tct = ThreadCreationTree::getInstance();

    for (const auto& pair : threadPairs) {
        if (!pair.analysis.actually_concurrent) {
            continue;
        }

        for (const auto& rule_ptr : pair.analysis.temporal_rules) {

            if(const auto* toctou_rule = dynamic_cast<const llm_client::TOCTOURule*>(rule_ptr.get())){
                std::cout << "  -> Verifying TOCTOU rule..." << std::endl;

                // 1. 获取LLM提名的节点
                int check_node_id = toctou_rule->get_node_for_role("state_check_operation");
                int modify_node_id = toctou_rule->get_node_for_role("state_modify_operation");
                int use_node_id = toctou_rule->get_node_for_role("resource_use_operation");

                if (check_node_id == -1 || modify_node_id == -1 || use_node_id == -1) {
                    std::cerr << "     [!] Rule is incomplete, skipping." << std::endl;
                    continue;
                }

                CCPGNode* check_node = ccpg->getNodeByID(check_node_id);
                CCPGNode* modify_node = ccpg->getNodeByID(modify_node_id);
                CCPGNode* use_node = ccpg->getNodeByID(use_node_id);
                
                if (!check_node || !modify_node || !use_node) {
                    std::cerr << "     [!] Could not find all nodes for rule, skipping." << std::endl;
                    continue;
                }

                // 2. 确定哪个线程是检查者，哪个是修改者
                Thread *checker_thread = nullptr, *modifier_thread = nullptr;
                if (pair.thread1->getNodes().count(check_node) && pair.thread2->getNodes().count(modify_node)) {
                    checker_thread = pair.thread1;
                    modifier_thread = pair.thread2;
                } else if (pair.thread2->getNodes().count(check_node) && pair.thread1->getNodes().count(modify_node)) {
                    checker_thread = pair.thread2;
                    modifier_thread = pair.thread1;
                } else {
                    std::cerr << "     [!] Nominated check/modify nodes are not in the expected thread pair, skipping." << std::endl;
                    continue;
                }

                // 3. 验证并发性
                bool can_be_concurrent = tct->mayThreadsRunConcurrently(checker_thread, modifier_thread);
                if (!can_be_concurrent) {
                    std::cout << "     [-] Concurrency check failed. Threads are not concurrent." << std::endl;
                    continue;
                }
                std::cout << "     [+] Concurrency check passed." << std::endl;

                // 4. 验证可达性 (Check -> Use)
                bool is_reachable = is_reachable_intra_thread(check_node, use_node, checker_thread);
                if (!is_reachable) {
                    std::cout << "     [-] Reachability check failed. No path from CHECK to USE." << std::endl;
                    continue;
                }
                std::cout << "     [+] Reachability check passed." << std::endl;

                // **如果所有验证都通过，则报告一个缺陷**
                std::cout << "    [!!!] POTENTIAL TOCTOU VIOLATION FOUND for rule." << std::endl;

                std::vector<std::pair<std::string, CCPGNode*>> full_violation_path;
                full_violation_path.push_back({"[CHECK] Operation", check_node});
                full_violation_path.push_back({"[MODIFY - Concurrent] Operation", modify_node});
                full_violation_path.push_back({"[USE] Operation", use_node});

                this->detectedBugs.emplace_back(toctou_rule->to_json(), full_violation_path, pair);
            }

            else if(const auto* datarace_rule = dynamic_cast<const llm_client::DataRaceRule*>(rule_ptr.get())){
                // 1. 获取LLM提名的节点
                int read_node_id = datarace_rule->get_node_for_role("read_operation");
                int write_node_id = datarace_rule->get_node_for_role("write_operation");

                CCPGNode* read_node = ccpg->getNodeByID(read_node_id);
                CCPGNode* write_node = ccpg->getNodeByID(write_node_id);

                if (!read_node || !write_node) {
                    std::cerr << "     [!] Could not find all nodes for rule, skipping." << std::endl;
                    continue;
                }

                // 2. 确定哪个线程是读取者，哪个是写入者
                Thread *reader_thread = nullptr, *writer_thread = nullptr;
                if (pair.thread1->getNodes().count(read_node) && pair.thread2->getNodes().count(write_node)) {
                    reader_thread = pair.thread1;
                    writer_thread = pair.thread2;
                } else if (pair.thread2->getNodes().count(read_node) && pair.thread1->getNodes().count(write_node)) {
                    reader_thread = pair.thread2;
                    writer_thread = pair.thread1;
                } else {
                    std::cerr << "     [!] Nominated read/write nodes are not in the expected thread pair, skipping." << std::endl;
                    continue;
                }

                // 3. 验证并发性
                bool can_be_concurrent = tct->mayThreadsRunConcurrently(reader_thread, writer_thread);
                if (!can_be_concurrent) {
                    std::cout << "     [-] Concurrency check failed. Threads are not concurrent." << std::endl;
                    continue;
                }
                std::cout << "     [+] Concurrency check passed." << std::endl;

                // **如果所有验证都通过，则报告一个缺陷**
                std::cout << "    [!!!] POTENTIAL DATA RACE VIOLATION FOUND for rule." << std::endl;

                std::vector<std::pair<std::string, CCPGNode*>> full_violation_path;
                full_violation_path.push_back({"[READ] Operation", read_node});
                full_violation_path.push_back({"[WRITE - Concurrent] Operation", write_node});

                this->detectedBugs.emplace_back(datarace_rule->to_json(), full_violation_path, pair);
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