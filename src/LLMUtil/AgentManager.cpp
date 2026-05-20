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
#include "LLMUtil/DetectorAgent.h"
#include "Query/VulnerabilitySurfaceGenerator.h"

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

std::vector<llm_client::ThreadPair> AgentManager::runAnalysisAgentMode() {
    if (!llmClient || !ccpg) {
        std::cerr << "LLM Client or CCPG not initialized. Aborting analysis.";
        return {};
    }

    std::cout << "\n--- Starting Agent-Mode Concurrency Analysis (Open Hypothesis) ---" << std::endl;

    ThreadCreationTree* tct = ThreadCreationTree::getInstance();

    // Phase 1: Generate vulnerability surface (pure static, no LLM)
    std::cout << "\n[Phase 1: Generating Vulnerability Surface (static)]" << std::endl;
    query::VulnerabilitySurfaceGenerator surfaceGen(ccpg, tct);
    auto surface = surfaceGen.generate();

    fs::path surface_path = TargetPath::getInstance()->getOutputDir() / "vulnerability_surface.json";
    std::ofstream surface_file(surface_path);
    if (surface_file.is_open()) {
        surface_file << surface.toJson().dump(2);
        surface_file.close();
        std::cout << "Vulnerability surface saved to: " << surface_path << std::endl;
    }

    size_t high_risk_count = 0;
    for (const auto& obj : surface.shared_objects) {
        if (obj.risk_score > 0) high_risk_count++;
    }

    std::cout << "\n  Threads: " << surface.total_thread_count
              << ", Conflicting pairs: " << surface.conflicting_pair_count
              << ", Shared objects: " << surface.shared_objects.size()
              << ", High-risk objects: " << high_risk_count
              << std::endl;

    if (surface.shared_objects.empty()) {
        std::cout << "No shared objects found. Skipping LLM analysis." << std::endl;
        return {};
    }

    // v23 P9a smoke aid: when LACE_EARLY_EXIT_AFTER_SURFACE is set, exit
    // right after Phase 1 so we can collect [P9a] stats and surface JSON
    // across many CVEs without spending any LLM tokens.
    if (const char* early = std::getenv("LACE_EARLY_EXIT_AFTER_SURFACE")) {
        if (early[0] != '\0' && early[0] != '0') {
            std::cout << "[LACE_EARLY_EXIT_AFTER_SURFACE] exiting after Phase 1"
                      << std::endl;
            return {};
        }
    }

    // Phase 2: DetectorAgent with open-hypothesis verification
    std::cout << "\n[Phase 2: DetectorAgent (open hypothesis, single LLM session)]" << std::endl;
    DetectorAgent detector(llmClient, ccpg);
    DetectorAgent::DetectionResult detectionResult;
    try {
        detectionResult = detector.runDetection(surface);
    } catch (const std::exception& e) {
        std::cerr << "\n[Phase 2 ERROR] LLM API call failed: " << e.what() << std::endl;
        std::cerr << "Vulnerability surface was saved successfully. Re-run when LLM API is available." << std::endl;
        return {};
    }

    confirmedHypotheses_ = std::move(detectionResult.confirmed);

    std::cout << "\n[Phase 2 Complete] " << confirmedHypotheses_.size()
              << " hypotheses confirmed by constraint verification." << std::endl;

    // Log confirmed hypotheses
    fs::path hyp_log_path = TargetPath::getInstance()->getOutputDir() / "confirmed_hypotheses.log";
    std::ofstream hyp_file(hyp_log_path);
    if (hyp_file.is_open()) {
        hyp_file << "========= Open-Hypothesis Detection Results =========\n\n";
        for (const auto& h : confirmedHypotheses_) {
            nlohmann::json hj = h.toJson();
            if (hj.contains("nodes") && hj["nodes"].is_object()) {
                nlohmann::json nodes_with_code;
                for (auto& [role, node_id] : hj["nodes"].items()) {
                    nlohmann::json node_info;
                    node_info["id"] = node_id;
                    CCPGNode* ccpg_node = ccpg->getNodeByID(node_id.get<int>());
                    if (ccpg_node && ccpg_node->getCPGNode()) {
                        node_info["code"] = ccpg_node->getCPGNode()->getCode();
                    } else {
                        node_info["code"] = "[Code not found]";
                    }
                    nodes_with_code[role] = node_info;
                }
                hj["nodes"] = nodes_with_code;
            }
            hyp_file << hj.dump(4) << "\n\n";
        }
        hyp_file.close();
        std::cout << "Hypotheses logged to: " << hyp_log_path << std::endl;
    }

    std::cout << "\n--- Agent-Mode Analysis Finished ---\n" << std::endl;

    // Return empty vector; caller should use getConfirmedHypotheses() instead
    return {};
}

std::vector<llm_client::ThreadPair> AgentManager::runAnalysis() {
    return runAnalysisLegacy();
}

std::vector<llm_client::ThreadPair> AgentManager::runAnalysisLegacy() {
    if (!llmClient || !ccpg) {
        std::cerr << "LLM Client or CCPG not initialized. Aborting analysis.";
        return {};
    }

    std::cout << "\n--- Starting Legacy LLM-based Concurrency Analysis ---" << std::endl;

    ThreadCreationTree* tct = ThreadCreationTree::getInstance();
    std::unordered_set<Thread*> threads = tct->getThreads();
    
    const auto candidateSharedObjects = tct->collectCandidateSharedObjects();

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

    std::cout << "\n[Phase 2: Analyzing Thread Pairs (LLM-based filtering)]" << std::endl;
    std::vector<ThreadPair> analysisResults;
    std::vector<Thread*> thread_vec(threads.begin(), threads.end());

    int total_pairs = thread_vec.size() * (thread_vec.size() - 1) / 2;
    int current_pair = 0;
    int skipped_by_llm = 0;
    
    int skipped_by_static = 0;
    for (size_t i = 0; i < thread_vec.size(); ++i) {
        for (size_t j = i + 1; j < thread_vec.size(); ++j) {
            Thread* thread1 = thread_vec[i];
            Thread* thread2 = thread_vec[j];

            if (!tct->mayHappenInParallel(thread1, thread2)) {
                skipped_by_static++;
                continue;
            }

            auto it1 = contractMap.find(thread1);
            auto it2 = contractMap.find(thread2);

            if (it1 != contractMap.end() && it2 != contractMap.end()) {
                current_pair++;
                std::cout << "Analyzing pair [" << current_pair << "/" << total_pairs << "]: "
                          << "Thread " << thread1->getId() << " and Thread " << thread2->getId() << std::endl;
                
                ThreadPair pair(thread1, it1->second, thread2, it2->second);
                parallelAnalyzer.analyze_parallelism(pair);
                
                std::cout << "  - Designed for Parallelism: " << (pair.analysis.designed_for_parallelism ? "Yes" : "No") << std::endl;
                std::cout << "    Reasoning: " << pair.analysis.design_reasoning << std::endl;
                std::cout << "  - Actually Concurrent: " << (pair.analysis.actually_concurrent ? "Yes" : "No") << std::endl;
                std::cout << "    Reasoning: " << pair.analysis.concurrency_reasoning << std::endl;

                if (!pair.analysis.actually_concurrent) {
                    skipped_by_llm++;
                    std::cout << "  -> Skipped by LLM (no concurrency risk identified)" << std::endl;
                    continue;
                }

                analysisResults.push_back(std::move(pair));
            }
        }
    }
    
    std::cout << "  -> Total pairs: " << total_pairs 
              << ", Skipped by static analysis: " << skipped_by_static
              << ", Skipped by LLM: " << skipped_by_llm 
              << ", Proceeding with: " << analysisResults.size() << std::endl;

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
                    nlohmann::json rule_json = rule_ptr->to_json();
                    if (rule_json.contains("nodes") && rule_json["nodes"].is_object()) {
                        nlohmann::json nodes_with_code;
                        for (auto& [role, node_id] : rule_json["nodes"].items()) {
                            nlohmann::json node_info;
                            node_info["id"] = node_id;
                            CCPGNode* ccpg_node = ccpg->getNodeByID(node_id.get<int>());
                            if (ccpg_node && ccpg_node->getCPGNode()) {
                                node_info["code"] = ccpg_node->getCPGNode()->getCode();
                            } else {
                                node_info["code"] = "[Code not found for this node ID]";
                            }
                            nodes_with_code[role] = node_info;
                        }
                        rule_json["nodes"] = nodes_with_code;
                    }
                    rules_file << rule_json.dump(4) << "\n\n"; 
                }
            }
        }
        rules_file.close();
        std::cout << "Temporal rules have been logged to: " << rules_log_path << std::endl;
    }

    std::cout << "\n--- Legacy LLM-based Analysis Finished ---\n" << std::endl;
    return analysisResults;
}

} // namespace llm_client
