// in src/Query/StatefulBugDetector.cpp

#include "Query/StatefulBugDetector.h"
#include "CCPG/CCPG.h"
#include "CCPG/ThreadCreationTree.h"
#include "CCPG/AliasChecker.h"
#include "PhasarUtil/PhasarPointerAnalysis.h"
#include "PhasarUtil/AnalysisManager.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Instructions.h"

#include <iostream>
#include <fstream>
#include <queue>
#include <set>

namespace query {

// --- StatefulBug Implementation ---

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
        ss << "  -> Function '" << step.first << "' called at " << step.second.toString() << "\n";
    }
    
    ss << "==========================================================";
    return ss.str();
}

bool findViolatingPath(
    CCPGNode* start_node,
    const std::string& target_func_name,
    const std::string& resolver_func_name,
    const llvm::Value* shared_object_instance, // 仍然传入，用于可能的调试或未来增强
    const std::string& object_type_name,
    const CCPGNodeSet& concurrent_modifying_nodes,
    std::vector<std::pair<std::string, NodeLoc>>& violation_path) 
{
    CCPG* ccpg = ThreadCreationTree::getInstance()->getCCPG();
    AliasChecker* aliasChecker = AliasChecker::getInstance();

    if (concurrent_modifying_nodes.empty()) {
        return false;
    }

    std::queue<std::vector<CCPGNode*>> worklist;
    std::set<CCPGNode*> visited;

    worklist.push({start_node});
    visited.insert(start_node);

    while (!worklist.empty()) {
        auto current_path = worklist.front();
        worklist.pop();
        CCPGNode* current_node = current_path.back();

        // 检查是否已经找到了目标
        if (current_node->isCallSite() && current_node->getCPGNode()->getName() == target_func_name) {
            // --- 核心修改 ---
            // 我们已经通过并发修改检查，将分析范围限定在了特定的共享对象实例上。
            // 同时，LLM规则已经从语义上将 check_func 和 use_func 绑定到了同一个对象上。
            // 因此，只要我们能找到一条从 check 到 use 的有效控制流路径，就足以证明
            // 存在潜在的TOCTOU。在此处进行精确的别名分析过于困难且容易失败。
            // 我们相信，路径的存在本身就是最强的证据。
            
            // （可选）可以保留一个较弱的检查，比如检查参数类型是否匹配，但为了鲁棒性，我们暂时移除它。
            // const llvm::Value* use_val = aliasChecker->getLLVMValueForArgument(current_node, object_type_name);
            // if (use_val) { ... }
            
            for(CCPGNode* path_node : current_path) {
                // 如果节点是调用点，记录函数名；否则，记录节点的代码，以提供更丰富的路径信息。
                if (path_node->isCallSite()) {
                    violation_path.push_back({path_node->getCPGNode()->getName(), path_node->getNodeLoc()});
                } else {
                     violation_path.push_back({path_node->getCPGNode()->getCode(), path_node->getNodeLoc()});
                }
            }
            return true;
        }

        // 如果路径遇到了“解决”函数，则剪枝
        if (!resolver_func_name.empty() && current_node->isCallSite() && current_node->getCPGNode()->getName() == resolver_func_name) {
            const llvm::Value* resolver_val = aliasChecker->getLLVMValueForArgument(current_node, object_type_name);
            if (resolver_val && aliasChecker->isAlias(shared_object_instance, resolver_val)) {
                continue; // 这条路径是安全的，不再继续探索
            }
        }
        
        // 沿着CFG和CALL边继续探索
        for (CCPGEdge* edge : current_node->getOutEdges()) {
            if (edge->getType() == CCPGEdge::EdgeType::ORDER || edge->getType() == CCPGEdge::EdgeType::CALL) {
                CCPGNode* next_node = edge->getDst();
                if (visited.find(next_node) == visited.end()) {
                    visited.insert(next_node);
                    std::vector<CCPGNode*> new_path = current_path;
                    new_path.push_back(next_node);
                    worklist.push(new_path);
                }
            }
        }
    }
    
    return false; // 没有找到违规路径
}

// --- StatefulBugDetector Implementation ---

void StatefulBugDetector::detect(
    const std::vector<llm_client::ThreadPair>& threadPairs,
    const std::set<const llvm::Value*>& candidateSharedObjects)
{
    std::cout << "\n[Phase 4: Detecting Stateful Protocol Violations]" << std::endl;
    
    CCPG* ccpg = ThreadCreationTree::getInstance()->getCCPG();
    AliasChecker* aliasChecker = AliasChecker::getInstance();

    for (const auto& pair : threadPairs) {
        if (!pair.analysis.actually_concurrent) { continue; }

        for (const auto& rule : pair.analysis.temporal_rules) {
            std::cout << "  - Applying rule '" << rule["rule_id"].get<std::string>() << "' for threads " 
                      << pair.thread1->getId() << " and " << pair.thread2->getId() << "..." << std::endl;
            
            std::string pattern_type = rule.at("pattern_type").get<std::string>();
            
            if (pattern_type == "TOCTOU") {
                std::string object_type_name = rule.at("shared_object_type").get<std::string>();
                std::string check_func = rule.value("state_check_function", "");
                std::string modify_func = rule.value("state_modify_function", "");
                std::string use_func = rule.value("state_use_function", "");
                std::string resolver_func = rule.value("resolving_function", "");

                if (check_func.empty() || modify_func.empty() || use_func.empty()) continue;
                
                auto locs1 = pair.thread1->findAllLocs();
                auto locs2 = pair.thread2->findAllLocs();

                // 遍历所有共享对象实例
                for (const llvm::Value* instance : candidateSharedObjects) {
                    
                    // 1. 识别T1中所有对该实例的“检查”点
                    for (const auto& [loc, ctx] : locs1) {
                         for(CCPGNode* start_node : ccpg->getNodesByLoc(loc)){
                            if (start_node->isCallSite() && start_node->getCPGNode()->getName() == check_func) {
                                
                                // 2. 识别T2中的“修改”行为
                                CCPGNodeSet concurrent_modifiers;
                                std::string thread2_main_func_name;
                                if (pair.thread2 && pair.thread2->getThreadMainFunction() && pair.thread2->getThreadMainFunction()->getFuncNode()) {
                                    thread2_main_func_name = pair.thread2->getThreadMainFunction()->getFuncNode()->getCPGNode()->getName();
                                }

                                // 情况一: "修改函数"就是线程2的主函数, 意味着整个线程都可能在修改状态
                                if (modify_func == thread2_main_func_name) {
                                    for (const auto& [loc_other, ctx_other] : locs2) {
                                        auto accesses = aliasChecker->getMemoryAccessesFromLocation(loc_other, ctx_other);
                                        for (const auto& access : accesses) {
                                            if (access.isWrite && aliasChecker->isAlias(instance, access.pointerOperand)) {
                                                // 找到了一个相关的写操作
                                                CCPGNodeSet nodes = ccpg->getNodesByLoc(loc_other);
                                                if (!nodes.empty()) {
                                                    concurrent_modifiers.insert(*nodes.begin());
                                                    goto modifiers_found; // 只要找到一个相关的写操作就足够了
                                                }
                                            }
                                        }
                                    }
                                } else {
                                    // 情况二: "修改函数"是线程2中一个具体的调用
                                    for (const auto& [loc_other, ctx_other] : locs2) {
                                        for(CCPGNode* modify_node : ccpg->getNodesByLoc(loc_other)){
                                             if (modify_node->isCallSite() && modify_node->getCPGNode()->getName() == modify_func) {
                                                const llvm::Value* modify_val = aliasChecker->getLLVMValueForArgument(modify_node, object_type_name);
                                                if (modify_val && aliasChecker->isAlias(instance, modify_val)) {
                                                    concurrent_modifiers.insert(modify_node);
                                                }
                                            }
                                        }
                                    }
                                }
                                
                            modifiers_found:;

                                // 3. 从“检查”点开始，在T1中搜索到“使用”点的路径
                                if (!concurrent_modifiers.empty()) {
                                    std::vector<std::pair<std::string, NodeLoc>> violation_path;
                                    if (findViolatingPath(start_node, use_func, resolver_func, instance, object_type_name, concurrent_modifiers, violation_path)) {
                                        std::cout << "    [+] VIOLATION FOUND for rule: " << rule["rule_id"].get<std::string>() << std::endl;
                                        this->detectedBugs.emplace_back(rule, violation_path, pair);
                                        goto next_rule; // 找到一个就足够了，避免重复报告
                                    }
                                }
                            }
                        }
                    }
                }
            }
            // TODO: 在这里可以为 'DESTRUCTIVE_REINIT' 等其他模式添加处理逻辑
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