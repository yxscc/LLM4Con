// src/LLMUtil/AgentManager.cpp

#include "LLMUtil/AgentManager.h"
#include <iostream>
#include <vector>
#include <map>
#include "CCPG/CCPGNode.h"
#include "CPG/Node.h"
#include "LLMUtil/ConcurrencyContract.h"
#include "CCPG/ThreadCreationTree.h"
#include "LLMUtil/ThreadPair.h"

namespace llm_client {

AgentManager::AgentManager(CCPG* cpg)
    : llmClient(LLMClient::get_instance()),
      entryFinder(cpg, llmClient),
      parallelAnalyzer(llmClient),
      contractGenerator(cpg, llmClient),
      ccpg(cpg) {
    if (!llmClient) {
        std::cerr << "Failed to initialize LLM Client. Please check URL and API key." << std::endl;
    }
}

std::vector<llm_client::ThreadPair> AgentManager::runAnalysis() {
    if (!llmClient || !ccpg) {
        std::cerr << "LLM Client or CCPG not initialized. Aborting analysis.";
        return {};
    }

    std::cout << "\n--- Starting LLM-based Concurrency Analysis ---" << std::endl;

    ThreadCreationTree* tct = ThreadCreationTree::getInstance();
    std::unordered_set<Thread*> threads = tct->getThreads();
    
    // Phase 1, Step A: Identify candidate shared objects once at the beginning.
    const auto candidateSharedObjects = tct->collectCandidateSharedObjects();

    // Phase 1, Step B: Generate Concurrency Contracts for all threads
    std::cout << "\n[Phase 1: Generating Concurrency Contracts]" << std::endl;
    std::map<Thread*, LLM::ConcurrencyContract> contractMap;
    for(Thread* thread : threads) {
        int entryFuncId = thread->getThreadMainFunction() ? thread->getThreadMainFunction()->getId() : -1;
        if (entryFuncId != -1) {
            std::cout << "Analyzing thread " << thread->getId() << " (Entry Function ID: " << entryFuncId << ")" << std::endl;
            auto contractOpt = contractGenerator.generateContractForThread(thread);
            if (contractOpt) {
                std::cout << "  -> Successfully generated contract." << std::endl;
                contractMap.emplace(thread, std::move(contractOpt.value()));
            } else {
                std::cerr << "  -> Failed to generate contract." << std::endl;
            }
        } else {
            std::cerr << "Could not determine thread entry for fork site " << thread->getForkNode()->getId() << std::endl;
        }
    }

    // Step 2: Analyze parallelism and build access maps for each pair of threads
    std::cout << "\n[Phase 2: Analyzing Parallelism and Building Access Maps]" << std::endl;
    std::vector<ThreadPair> analysisResults;
    std::vector<Thread*> thread_vec(threads.begin(), threads.end());

    std::cout << "  -> Pre-calculating raw memory access maps for all threads..." << std::endl;
    std::map<Thread*, MemoryAccessMap> raw_map_cache;
    for (Thread* thread : thread_vec) {
        raw_map_cache[thread] = tct->buildRawMemoryAccessMap(thread);
    }
    std::cout << "  -> Caching complete." << std::endl;

    for (size_t i = 0; i < thread_vec.size(); ++i) {
        for (size_t j = i + 1; j < thread_vec.size(); ++j) {
            Thread* thread1 = thread_vec[i];
            Thread* thread2 = thread_vec[j];

            auto it1 = contractMap.find(thread1);
            auto it2 = contractMap.find(thread2);

            if (it1 != contractMap.end() && it2 != contractMap.end()) {
                std::cout << "Analyzing pair: Thread " << thread1->getId() << " and Thread " << thread2->getId() << std::endl;
                ThreadPair pair(thread1, it1->second, thread2, it2->second);
                parallelAnalyzer.analyze_parallelism(pair);
                
                std::cout << "  - Designed for Parallelism: " << (pair.analysis.designed_for_parallelism ? "Yes" : "No") << std::endl;
                std::cout << "    Reasoning: " << pair.analysis.design_reasoning << std::endl;
                std::cout << "  - Actually Concurrent: " << (pair.analysis.actually_concurrent ? "Yes" : "No") << std::endl;
                std::cout << "    Reasoning: " << pair.analysis.concurrency_reasoning << std::endl;

                // If threads can run concurrently, build and cache their MemoryAccessMaps
                if (pair.analysis.actually_concurrent) {
                    //std::cout << "  -> Building memory access maps for concurrent pair..." << std::endl;
                    // Pass the pre-computed candidateSharedObjects to the builder function
                    //pair.analysis.accessMap1 = tct->buildMemoryAccessMapForThread(pair.thread1, pair.thread2, candidateSharedObjects);
                    //pair.analysis.accessMap2 = tct->buildMemoryAccessMapForThread(pair.thread2, pair.thread1, candidateSharedObjects);
                    // 1. 从缓存中获取预先计算好的 raw maps
                    const auto& rawMap1 = raw_map_cache.at(pair.thread1);
                    const auto& rawMap2 = raw_map_cache.at(pair.thread2);

                    AliasChecker* aliasChecker = AliasChecker::getInstance();

                    // 2. 基于别名分析，找到两个 maps 的交集
                    for (const auto& [mem_loc1, accesses1] : rawMap1) {
                        for (const auto& [mem_loc2, accesses2] : rawMap2) {
                            if (aliasChecker->isAlias(mem_loc1, mem_loc2)) {
                                // 这是一个真正的共享对象，因为它被两个线程都访问了
                                // 将对应的访问添加到最终的 maps 中
                                pair.analysis.accessMap1[mem_loc1] = accesses1;
                                pair.analysis.accessMap2[mem_loc2] = accesses2;
                                // 既然我们为 mem_loc1 找到了匹配项，就可以停止在 rawMap2 中继续搜索
                                // 并移动到 rawMap1 的下一个内存位置
                                goto next_location_in_map1;
                            }
                        }
                    next_location_in_map1:;
                    }
                }

                analysisResults.push_back(std::move(pair));
            }
        }
    }

    fs::path rules_log_path = TargetPath::getInstance()->getOutputDir() / "temporal_rules.log";
    std::ofstream rules_file(rules_log_path);
    if (rules_file.is_open()) {
        rules_file << "========= Generated Temporal Ordering Rules =========\n\n";
        int pair_count = 1;
        for (const auto& pair : analysisResults) {
            rules_file << "--- Pair " << pair_count++ << ": Thread " << pair.thread1->getId() 
                    << " vs Thread " << pair.thread2->getId() << " ---\n";
            if (pair.analysis.temporal_rules.empty()) {
                rules_file << "  No rules generated for this pair.\n\n";
            } else {
                for (const auto& rule_ptr : pair.analysis.temporal_rules) {
                    // Get the initial JSON object for the rule
                    nlohmann::json rule_json = rule_ptr->to_json();

                    // Check if the "nodes" key exists and is an object
                    if (rule_json.contains("nodes") && rule_json["nodes"].is_object()) {
                        nlohmann::json nodes_with_code;
                        // Iterate over each node role (e.g., "state_check_operation")
                        for (auto& [role, node_id] : rule_json["nodes"].items()) {
                            nlohmann::json node_info;
                            node_info["id"] = node_id;

                            // Look up the CCPGNode by its ID
                            CCPGNode* ccpg_node = ccpg->getNodeByID(node_id.get<int>());
                            if (ccpg_node && ccpg_node->getCPGNode()) {
                                // Add the code to our new node_info object
                                node_info["code"] = ccpg_node->getCPGNode()->getCode();
                            } else {
                                node_info["code"] = "[Code not found for this node ID]";
                            }
                            nodes_with_code[role] = node_info;
                        }
                        // Replace the old "nodes" object with our new, more detailed one
                        rule_json["nodes"] = nodes_with_code;
                    }
                    
                    // Write the modified JSON to the file
                    rules_file << rule_json.dump(4) << "\n\n"; 
                }
            }
        }
        rules_file.close();
        std::cout << "Temporal rules have been logged to: " << rules_log_path << std::endl;
    }

    std::cout << "\n--- LLM-based Analysis Finished ---\n" << std::endl;
    return analysisResults;
}

} // namespace llm_client