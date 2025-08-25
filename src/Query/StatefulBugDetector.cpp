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
       << "Violation observed in threads " << threads.thread1->getId() << " and " << threads.thread2->getId() << ".\n"
       << "Forbidden Sequence Trace:\n";
    
    for (const auto& step : path) {
        ss << "  -> Function '" << step.first << "' called at " << step.second.toString() << "\n";
    }
    
    ss << "==========================================================";
    return ss.str();
}

/**
 * @brief 核心的路径搜索函数，使用BFS在CCPG上查找违规路径。
 * @param start_node 违规序列的起始节点。
 * @param target_func_name 违规序列的结束函数名。
 * @param resolver_func_name “解决”函数名，路径中不能包含此函数。
 * @param shared_object_instance 正在追踪的共享对象实例的LLVM Value。
 * @param ccpg CCPG的指针。
 * @param aliasChecker 别名分析器的指针。
 * @param violation_path [out] 如果找到路径，这里将存储违规的调用序列。
 * @return 如果找到违规路径，返回true。
 */
bool findViolatingPath(
    CCPGNode* start_node,
    const std::string& target_func_name,
    const std::string& resolver_func_name,
    const llvm::Value* shared_object_instance,
    std::vector<std::pair<std::string, NodeLoc>>& violation_path) 
{
    CCPG* ccpg = ThreadCreationTree::getInstance()->getCCPG();
    AliasChecker* aliasChecker = AliasChecker::getInstance();
    std::string object_type_name_unused;
    // BFS队列，存储<当前节点, 到达该节点的路径>
    std::queue<std::pair<CCPGNode*, std::vector<CCPGNode*>>> worklist;
    // 存储已访问过的节点，防止循环
    std::set<CCPGNode*> visited;

    worklist.push({start_node, {start_node}});
    visited.insert(start_node);

    while (!worklist.empty()) {
        auto current_pair = worklist.front();
        worklist.pop();
        CCPGNode* current_node = current_pair.first;
        std::vector<CCPGNode*> current_path = current_pair.second;

        // 遍历当前节点的所有出边 (包括控制流和Happens-Before边)
        for (CCPGEdge* edge : current_node->getOutEdges()) {
            CCPGNode* next_node = edge->getDst();

            if (visited.count(next_node)) {
                continue;
            }

            // 检查next_node是否是我们要找的“违规”函数调用
            if (next_node->isCallSite() && next_node->getCPGNode()->getName() == target_func_name) {
                // 确认这个调用操作的是同一个共享对象实例
                // (注意：这里需要一个辅助函数来从CCPGNode获取LLVM Value)
                const llvm::Value* target_val = aliasChecker->getLLVMThreadValue(next_node); // 示例，需要替换为获取函数参数的通用方法
                if (target_val && aliasChecker->isAlias(shared_object_instance, target_val)) {
                    // 找到了！构建并返回违规路径
                    violation_path.clear();
                    for(CCPGNode* path_node : current_path) {
                        violation_path.push_back({path_node->getCPGNode()->getName(), path_node->getNodeLoc()});
                    }
                    violation_path.push_back({next_node->getCPGNode()->getName(), next_node->getNodeLoc()});
                    return true;
                }
            }

            // 检查next_node是否是“解决”函数，如果是，则剪枝
            if (!resolver_func_name.empty() && next_node->isCallSite() && next_node->getCPGNode()->getName() == resolver_func_name) {
                // 这条路径是安全的，不再继续探索
                continue; 
            }

            // 如果都不是，则将该节点加入队列继续探索
            visited.insert(next_node);
            std::vector<CCPGNode*> new_path = current_path;
            new_path.push_back(next_node);
            worklist.push({next_node, new_path});
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
            std::cout << "  - Applying rule '" << rule["rule_id"].get<std::string>() << "'..." << std::endl;
            
            std::string object_type_name = rule["shared_object_type"].get<std::string>();
            std::string start_func = rule["forbidden_sequence"][0]["function"].get<std::string>();
            std::string target_func = rule["forbidden_sequence"][1]["function"].get<std::string>();
            std::string resolver_func = rule.value("resolving_function", "");

            for (const llvm::Value* instance : candidateSharedObjects) {
                // --- 修正点: 使用安全的方式获取类型信息 ---
                llvm::Type* value_type = nullptr;
                // 全局变量的类型信息存储在 getValueType() 中
                if (const auto* gv = llvm::dyn_cast<const llvm::GlobalVariable>(instance)) {
                    value_type = gv->getValueType();
                } 
                // 对于其他指针，我们不能安全地获取其指向的类型，暂时跳过
                // （这对于PoC是足够的，因为关键对象是全局的）
                else {
                    continue; 
                }

                if (value_type && value_type->isStructTy()) {
                    llvm::StructType* st = llvm::dyn_cast<llvm::StructType>(value_type);
                    if (!st || st->isLiteral() || !st->getStructName().contains(object_type_name)) {
                        continue; // 类型不匹配，跳过此对象
                    }
                } else { continue; }

                for (CCPGNode* start_node : ccpg->getNodes()) {
                    if (start_node->isCallSite() && start_node->getCPGNode()->getName() == start_func) {
                        const llvm::Value* start_val = aliasChecker->getLLVMValueForArgument(start_node, object_type_name);
                        if (!start_val || !aliasChecker->isAlias(instance, start_val)) {
                            continue;
                        }

                        std::vector<std::pair<std::string, NodeLoc>> violation_path;
                        // --- 修正点: 修正 findViolatingPath 的调用参数 ---
                        if (findViolatingPath(start_node, target_func, resolver_func, instance, violation_path)) {
                            std::cout << "    [+] VIOLATION FOUND for rule: " << rule["rule_id"].get<std::string>() << std::endl;
                            this->detectedBugs.emplace_back(rule, violation_path, pair);
                            goto next_instance;
                        }
                    }
                }
            }
            next_instance:;
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