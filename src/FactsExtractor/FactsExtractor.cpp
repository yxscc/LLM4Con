// src/FactsExtractor/FactsExtractor.cpp

#include "FactsExtractor/FactsExtractor.h"
#include "CCPG/CCPG.h"
#include "CCPG/CCPGNode.h"
#include "CCPG/ThreadCreationTree.h"
#include "CCPG/ThreadAPIUtil.h"
#include "PhasarUtil/PhasarPointerAnalysis.h"
#include "PhasarUtil/AnalysisManager.h"
#include "PhasarUtil/LLVMAnalyzer.h"

#include <llvm/IR/Value.h>
#include <llvm/IR/GlobalVariable.h>

#include <fstream>
#include <sstream>
#include <regex>
#include <queue>
#include <set>
#include <climits>
#include <iostream>

namespace facts {

FactsExtractor::FactsExtractor(CCPG* ccpg, PhasarPointerAnalysis* pta)
    : ccpg_(ccpg), pta_(pta) {}

nlohmann::json FactsExtractor::extractFacts(const std::string& cve_id, const std::string& code_file) {
    nlohmann::json result;
    result["id"] = cve_id;
    result["code_file"] = code_file;
    
    if (ground_truth_.bug_type.empty() == false) {
        result["ground_truth"] = {
            {"bug_type", ground_truth_.bug_type},
            {"bug_lines", ground_truth_.bug_lines},
            {"description", ground_truth_.description}
        };
    }
    
    nlohmann::json verifiable_facts;
    
    std::cout << "  [DEBUG] Extracting shared_variables..." << std::endl;
    verifiable_facts["shared_variables"] = extractSharedVariables();
    std::cout << "  [DEBUG] Extracting memory_accesses..." << std::endl;
    verifiable_facts["memory_accesses"] = extractMemoryAccesses();
    std::cout << "  [DEBUG] Extracting alias_pairs..." << std::endl;
    verifiable_facts["alias_pairs"] = extractAliasPairs();
    std::cout << "  [DEBUG] Extracting lock_operations..." << std::endl;
    verifiable_facts["lock_operations"] = extractLockOperations();
    std::cout << "  [DEBUG] Extracting protected_regions..." << std::endl;
    verifiable_facts["protected_regions"] = extractProtectedRegions();
    std::cout << "  [DEBUG] Extracting thread_contexts..." << std::endl;
    verifiable_facts["thread_contexts"] = extractThreadContexts();
    std::cout << "  [DEBUG] Extracting parallel_pairs..." << std::endl;
    verifiable_facts["parallel_pairs"] = extractParallelPairs();
    std::cout << "  [DEBUG] Extracting happens_before..." << std::endl;
    verifiable_facts["happens_before"] = extractHappensBefore();
    std::cout << "  [DEBUG] Extracting unprotected_accesses..." << std::endl;
    verifiable_facts["unprotected_accesses"] = extractUnprotectedAccesses();
    std::cout << "  [DEBUG] All facts extracted." << std::endl;
    
    result["verifiable_facts"] = verifiable_facts;
    return result;
}

std::string FactsExtractor::getThreadNameForNode(CCPGNode* node) {
    if (!node) return "unknown";
    
    ThreadCreationTree* tct = ThreadCreationTree::getInstance();
    for (Thread* thread : tct->getThreads()) {
        if (thread->getNodes().count(node) > 0) {
            if (thread->getThreadMainFunction() && 
                thread->getThreadMainFunction()->getFuncNode() &&
                thread->getThreadMainFunction()->getFuncNode()->getCPGNode()) {
                return thread->getThreadMainFunction()->getFuncNode()->getCPGNode()->getName();
            }
            return "thread_" + std::to_string(thread->getId());
        }
    }
    return "main";
}

std::string FactsExtractor::getLockNameFromNode(CCPGNode* node) {
    if (!node || !node->getCPGNode()) return "unknown_lock";
    
    std::string code = node->getCPGNode()->getCode();
    
    std::regex lock_regex(R"(pthread_mutex_(?:lock|unlock)\s*\(\s*&?\s*(\w+))");
    std::smatch match;
    if (std::regex_search(code, match, lock_regex)) {
        return match[1].str();
    }
    
    std::regex generic_lock_regex(R"((?:mutex_lock|spin_lock|lock)\s*\(\s*&?\s*(\w+))");
    if (std::regex_search(code, match, generic_lock_regex)) {
        return match[1].str();
    }
    
    Node* arg = node->getCPGNode()->getArgument(1);
    if (arg) {
        std::string argCode = arg->getCode();
        argCode.erase(std::remove(argCode.begin(), argCode.end(), '&'), argCode.end());
        argCode.erase(std::remove(argCode.begin(), argCode.end(), ' '), argCode.end());
        return argCode;
    }
    
    return "lock";
}

std::string FactsExtractor::getVariableNameFromNode(CCPGNode* node) {
    if (!node || !node->getCPGNode()) return "";
    
    std::string code = node->getCPGNode()->getCode();
    
    // Pattern 1: obj->field = value (extract "obj->field")
    std::regex arrow_assign_regex(R"((\w+)->(\w+)\s*=)");
    std::smatch match;
    if (std::regex_search(code, match, arrow_assign_regex)) {
        return match[1].str() + "->" + match[2].str();
    }
    
    // Pattern 2: obj.field = value (extract "obj.field")
    std::regex dot_assign_regex(R"((\w+)\.(\w+)\s*=)");
    if (std::regex_search(code, match, dot_assign_regex)) {
        return match[1].str() + "." + match[2].str();
    }
    
    // Pattern 3: *ptr = value (extract "*ptr")
    std::regex deref_assign_regex(R"(\*(\w+)\s*=)");
    if (std::regex_search(code, match, deref_assign_regex)) {
        return "*" + match[1].str();
    }
    
    // Pattern 4: simple var = value (extract "var")
    std::regex simple_assign_regex(R"((\w+)\s*=)");
    if (std::regex_search(code, match, simple_assign_regex)) {
        return match[1].str();
    }
    
    // Pattern 5: free(ptr) or kfree(ptr) (extract "ptr")
    std::regex free_regex(R"((?:free|kfree)\s*\(\s*(\w+)\s*\))");
    if (std::regex_search(code, match, free_regex)) {
        return match[1].str();
    }
    
    // Pattern 6: malloc assignment: ptr = malloc(...) (extract "ptr")
    std::regex malloc_regex(R"((\w+)\s*=\s*.*(?:malloc|calloc|kmalloc))");
    if (std::regex_search(code, match, malloc_regex)) {
        return match[1].str();
    }
    
    // Pattern 7: obj->field read (extract "obj->field")
    std::regex arrow_read_regex(R"((\w+)->(\w+))");
    if (std::regex_search(code, match, arrow_read_regex)) {
        return match[1].str() + "->" + match[2].str();
    }
    
    // Pattern 8: obj.field read (extract "obj.field")  
    std::regex dot_read_regex(R"((\w+)\.(\w+))");
    if (std::regex_search(code, match, dot_read_regex)) {
        return match[1].str() + "." + match[2].str();
    }
    
    // Fallback: first identifier
    std::regex first_id_regex(R"((\w+))");
    if (std::regex_search(code, match, first_id_regex)) {
        return match[1].str();
    }
    
    return code.substr(0, std::min(code.size(), size_t(30)));
}

nlohmann::json FactsExtractor::extractSharedVariables() {
    nlohmann::json shared_vars = nlohmann::json::array();
    
    try {
        std::set<const llvm::Value*> candidates = ThreadCreationTree::getInstance()->collectCandidateSharedObjects();
        
        std::map<std::string, std::set<std::string>> var_to_threads;
        
        ThreadCreationTree* tct = ThreadCreationTree::getInstance();
        for (Thread* thread : tct->getThreads()) {
            if (!thread) continue;
            
            std::string thread_name = "thread_" + std::to_string(thread->getId());
            if (thread->getThreadMainFunction() && 
                thread->getThreadMainFunction()->getFuncNode() &&
                thread->getThreadMainFunction()->getFuncNode()->getCPGNode()) {
                thread_name = thread->getThreadMainFunction()->getFuncNode()->getCPGNode()->getName();
            }
            
            for (CCPGNode* node : thread->getNodes()) {
                if (!node || !node->getCPGNode()) continue;
                std::string code = node->getCPGNode()->getCode();
                if (code.empty()) continue;
                
                for (const llvm::Value* val : candidates) {
                    if (!val || !val->hasName()) continue;
                    std::string var_name = LLVMAnalyzer::getInstance()->demangle_valueName(val->getName().str().c_str());
                    if (var_name.empty()) continue;
                    
                    if (code.find(var_name) != std::string::npos) {
                        var_to_threads[var_name].insert(thread_name);
                    }
                }
            }
        }
    
        for (const auto& [var_name, threads] : var_to_threads) {
            if (threads.size() >= 2) {
                nlohmann::json sv;
                sv["var_name"] = var_name;
                sv["threads"] = nlohmann::json::array();
                for (const auto& t : threads) {
                    sv["threads"].push_back(t);
                }
                sv["evidence"] = "accessed by " + std::to_string(threads.size()) + " threads";
                shared_vars.push_back(sv);
                
                shared_vars_.push_back({var_name, std::vector<std::string>(threads.begin(), threads.end()), sv["evidence"]});
            }
        }
        
        // Second pass: detect variables shared via memory operations across threads
        // Look for patterns: alloc in thread A, free in thread B (e.g., port)
        std::map<std::string, std::map<std::string, std::set<std::string>>> var_ops_by_thread;
        // var_ops_by_thread[var_name][thread_name] = {ALLOC, FREE, READ, WRITE, CALL}
        
        for (Thread* thread : tct->getThreads()) {
            if (!thread) continue;
            std::string thread_name = "thread_" + std::to_string(thread->getId());
            if (thread->getThreadMainFunction() &&
                thread->getThreadMainFunction()->getFuncNode() &&
                thread->getThreadMainFunction()->getFuncNode()->getCPGNode()) {
                thread_name = thread->getThreadMainFunction()->getFuncNode()->getCPGNode()->getName();
            }
            
            for (CCPGNode* node : thread->getNodes()) {
                if (!node || !node->getCPGNode()) continue;
                std::string code = node->getCPGNode()->getCode();
                if (code.empty()) continue;
                
                // Extract pointer variable names from operations
                std::regex ptr_pattern(R"(\b(\w+)\s*=\s*\w+\s*\()");  // ptr = func(...)
                std::regex free_pattern(R"((?:free|kfree)\s*\(\s*(\w+)\s*\))");
                std::regex use_pattern(R"((\w+)->)");  // ptr->field
                std::regex call_pattern(R"(\w+\s*\([^)]*\b(\w+)\b[^)]*\))");  // func(..., ptr, ...)
                
                std::smatch match;
                
                // Check for allocation: ptr = malloc/create_func(...)
                if (code.find("malloc") != std::string::npos || 
                    code.find("calloc") != std::string::npos ||
                    code.find("create") != std::string::npos) {
                    if (std::regex_search(code, match, ptr_pattern)) {
                        var_ops_by_thread[match[1].str()][thread_name].insert("ALLOC");
                    }
                }
                
                // Check for free
                if (std::regex_search(code, match, free_pattern)) {
                    var_ops_by_thread[match[1].str()][thread_name].insert("FREE");
                }
                
                // Check for pointer dereference (use)
                std::string::const_iterator searchStart(code.cbegin());
                while (std::regex_search(searchStart, code.cend(), match, use_pattern)) {
                    std::string var = match[1].str();
                    if (var != "client" && var != "info" && var != "args") {  // Skip common non-shared
                        var_ops_by_thread[var][thread_name].insert("USE");
                    }
                    searchStart = match.suffix().first;
                }
                
                // Check for function calls with pointer arguments
                if (code.find("(") != std::string::npos && code.find("port") != std::string::npos) {
                    std::regex port_arg(R"(\(\s*(\w*port\w*)\s*[,)])");
                    if (std::regex_search(code, match, port_arg)) {
                        var_ops_by_thread[match[1].str()][thread_name].insert("CALL");
                    }
                }
            }
        }
        
        // Find variables accessed by multiple threads
        for (const auto& [var_name, thread_ops] : var_ops_by_thread) {
            if (thread_ops.size() >= 2 && var_to_threads.find(var_name) == var_to_threads.end()) {
                // This variable is used by 2+ threads but wasn't detected before
                nlohmann::json sv;
                sv["var_name"] = var_name;
                sv["threads"] = nlohmann::json::array();
                std::vector<std::string> thread_list;
                std::string evidence_ops;
                
                for (const auto& [tname, ops] : thread_ops) {
                    sv["threads"].push_back(tname);
                    thread_list.push_back(tname);
                    for (const auto& op : ops) {
                        evidence_ops += tname + ":" + op + " ";
                    }
                }
                
                sv["evidence"] = "cross-thread operations: " + evidence_ops;
                shared_vars.push_back(sv);
                shared_vars_.push_back({var_name, thread_list, sv["evidence"].get<std::string>()});
            }
        }
        
    } catch (const std::exception& e) {
        std::cerr << "[FactsExtractor] Error in extractSharedVariables: " << e.what() << std::endl;
    }
    
    return shared_vars;
}

nlohmann::json FactsExtractor::extractMemoryAccesses() {
    nlohmann::json accesses = nlohmann::json::array();
    
    try {
        ThreadCreationTree* tct = ThreadCreationTree::getInstance();
        if (!tct) return accesses;
        
        for (Thread* thread : tct->getThreads()) {
            if (!thread) continue;
            
            std::string thread_name = "unknown";
            if (thread->getThreadMainFunction() && 
                thread->getThreadMainFunction()->getFuncNode() &&
                thread->getThreadMainFunction()->getFuncNode()->getCPGNode()) {
                thread_name = thread->getThreadMainFunction()->getFuncNode()->getCPGNode()->getName();
            }
            
            for (CCPGNode* node : thread->getNodes()) {
                if (!node || !node->getCPGNode()) continue;
            
            int line = node->getNodeLoc().getLineNumber();
            std::string code = node->getCPGNode()->getCode();
            std::string var = getVariableNameFromNode(node);
            
            ThreadAPIUtil::TYPE type = node->getType();
            
            if (type == ThreadAPIUtil::TYPE::ASSIGNMENT) {
                bool is_write = (code.find('=') != std::string::npos && 
                                code.find("==") == std::string::npos);
                
                nlohmann::json access;
                access["line"] = line;
                access["var"] = var;
                access["type"] = is_write ? "WRITE" : "READ";
                access["thread"] = thread_name;
                access["code"] = code;
                accesses.push_back(access);
                
                memory_accesses_.push_back({line, var, is_write ? "WRITE" : "READ", thread_name, code});
            }
            
            if (code.find("free(") != std::string::npos || 
                code.find("kfree(") != std::string::npos ||
                code.find("delete ") != std::string::npos) {
                nlohmann::json access;
                access["line"] = line;
                access["var"] = var;
                access["type"] = "FREE";
                access["thread"] = thread_name;
                access["code"] = code;
                accesses.push_back(access);
                
                memory_accesses_.push_back({line, var, "FREE", thread_name, code});
            }
            
            if (code.find("malloc(") != std::string::npos || 
                code.find("calloc(") != std::string::npos ||
                code.find("kmalloc(") != std::string::npos ||
                code.find("new ") != std::string::npos) {
                nlohmann::json access;
                access["line"] = line;
                access["var"] = var;
                access["type"] = "ALLOC";
                access["thread"] = thread_name;
                access["code"] = code;
                accesses.push_back(access);
                
                memory_accesses_.push_back({line, var, "ALLOC", thread_name, code});
            }
        }
    }
    } catch (const std::exception& e) {
        std::cerr << "[FactsExtractor] Error in extractMemoryAccesses: " << e.what() << std::endl;
    }
    
    return accesses;
}

nlohmann::json FactsExtractor::extractAliasPairs() {
    nlohmann::json aliases = nlohmann::json::array();
    
    if (!pta_) return aliases;
    
    std::set<const llvm::Value*> candidates = ThreadCreationTree::getInstance()->collectCandidateSharedObjects();
    std::vector<const llvm::Value*> candidates_vec(candidates.begin(), candidates.end());
    
    for (size_t i = 0; i < candidates_vec.size() && i < 50; ++i) {
        for (size_t j = i + 1; j < candidates_vec.size() && j < 50; ++j) {
            const llvm::Value* v1 = candidates_vec[i];
            const llvm::Value* v2 = candidates_vec[j];
            
            if (!v1 || !v2 || !v1->hasName() || !v2->hasName()) continue;
            
            bool is_alias = pta_->areAliases(v1, v2);
            
            if (is_alias) {
                std::string name1 = LLVMAnalyzer::getInstance()->demangle_valueName(v1->getName().str().c_str());
                std::string name2 = LLVMAnalyzer::getInstance()->demangle_valueName(v2->getName().str().c_str());
                
                nlohmann::json pair;
                pair["var_a"] = name1;
                pair["var_b"] = name2;
                pair["is_alias"] = true;
                aliases.push_back(pair);
                
                alias_pairs_.push_back({name1, name2, true});
            }
        }
    }
    
    return aliases;
}

nlohmann::json FactsExtractor::extractLockOperations() {
    nlohmann::json lock_ops = nlohmann::json::array();
    
    CCPGNodeSet acquire_nodes = ccpg_->getNodesByType(ThreadAPIUtil::TYPE::ACQUIRE);
    for (CCPGNode* node : acquire_nodes) {
        if (!node) continue;
        
        nlohmann::json op;
        op["line"] = node->getNodeLoc().getLineNumber();
        op["lock"] = getLockNameFromNode(node);
        op["op"] = "acquire";
        op["thread"] = getThreadNameForNode(node);
        op["code"] = node->getCPGNode() ? node->getCPGNode()->getCode() : "";
        lock_ops.push_back(op);
        
        lock_ops_.push_back({
            node->getNodeLoc().getLineNumber(),
            getLockNameFromNode(node),
            "acquire",
            getThreadNameForNode(node),
            node->getCPGNode() ? node->getCPGNode()->getCode() : ""
        });
    }
    
    CCPGNodeSet release_nodes = ccpg_->getNodesByType(ThreadAPIUtil::TYPE::RELEASE);
    for (CCPGNode* node : release_nodes) {
        if (!node) continue;
        
        nlohmann::json op;
        op["line"] = node->getNodeLoc().getLineNumber();
        op["lock"] = getLockNameFromNode(node);
        op["op"] = "release";
        op["thread"] = getThreadNameForNode(node);
        op["code"] = node->getCPGNode() ? node->getCPGNode()->getCode() : "";
        lock_ops.push_back(op);
        
        lock_ops_.push_back({
            node->getNodeLoc().getLineNumber(),
            getLockNameFromNode(node),
            "release",
            getThreadNameForNode(node),
            node->getCPGNode() ? node->getCPGNode()->getCode() : ""
        });
    }
    
    return lock_ops;
}

nlohmann::json FactsExtractor::extractProtectedRegions() {
    nlohmann::json regions = nlohmann::json::array();
    
    ThreadCreationTree* tct = ThreadCreationTree::getInstance();
    
    for (Thread* thread : tct->getThreads()) {
        std::string thread_name = getThreadNameForNode(nullptr);
        if (thread->getThreadMainFunction() && 
            thread->getThreadMainFunction()->getFuncNode() &&
            thread->getThreadMainFunction()->getFuncNode()->getCPGNode()) {
            thread_name = thread->getThreadMainFunction()->getFuncNode()->getCPGNode()->getName();
        }
        
        CCPGNodeSet acquire_nodes = thread->getNodesByType(ThreadAPIUtil::TYPE::ACQUIRE);
        
        for (CCPGNode* acquire_node : acquire_nodes) {
            if (!acquire_node) continue;
            
            std::string lock_name = getLockNameFromNode(acquire_node);
            int start_line = acquire_node->getNodeLoc().getLineNumber();
            int end_line = start_line;
            
            std::queue<CCPGNode*> worklist;
            std::set<CCPGNode*> visited;
            worklist.push(acquire_node);
            visited.insert(acquire_node);
            
            while (!worklist.empty()) {
                CCPGNode* current = worklist.front();
                worklist.pop();
                
                for (CCPGEdge* edge : current->getOutEdges()) {
                    if (edge->getType() != CCPGEdge::EdgeType::ORDER) continue;
                    
                    CCPGNode* next = edge->getDst();
                    if (!next || visited.count(next)) continue;
                    
                    if (next->getType() == ThreadAPIUtil::TYPE::RELEASE) {
                        std::string release_lock = getLockNameFromNode(next);
                        if (release_lock == lock_name) {
                            end_line = std::max(end_line, next->getNodeLoc().getLineNumber());
                            continue;
                        }
                    }
                    
                    visited.insert(next);
                    end_line = std::max(end_line, next->getNodeLoc().getLineNumber());
                    worklist.push(next);
                }
            }
            
            if (end_line > start_line) {
                nlohmann::json region;
                region["start"] = start_line;
                region["end"] = end_line;
                region["lock"] = lock_name;
                region["thread"] = thread_name;
                regions.push_back(region);
                
                protected_regions_.push_back({start_line, end_line, lock_name, thread_name});
            }
        }
    }
    
    return regions;
}

nlohmann::json FactsExtractor::extractThreadContexts() {
    nlohmann::json contexts = nlohmann::json::array();
    
    ThreadCreationTree* tct = ThreadCreationTree::getInstance();
    
    for (Thread* thread : tct->getThreads()) {
        std::string func_name = "unknown";
        int start_line = INT_MAX;
        int end_line = 0;
        
        if (thread->getThreadMainFunction() && 
            thread->getThreadMainFunction()->getFuncNode() &&
            thread->getThreadMainFunction()->getFuncNode()->getCPGNode()) {
            func_name = thread->getThreadMainFunction()->getFuncNode()->getCPGNode()->getName();
            start_line = thread->getThreadMainFunction()->getFuncNode()->getNodeLoc().getLineNumber();
        }
        
        for (CCPGNode* node : thread->getNodes()) {
            if (!node) continue;
            int line = node->getNodeLoc().getLineNumber();
            start_line = std::min(start_line, line);
            end_line = std::max(end_line, line);
        }
        
        nlohmann::json ctx;
        ctx["function"] = func_name;
        ctx["start_line"] = start_line;
        ctx["end_line"] = end_line;
        ctx["thread_id"] = thread->getId();
        contexts.push_back(ctx);
        
        thread_contexts_.push_back({func_name, start_line, end_line, thread->getId()});
    }
    
    return contexts;
}

nlohmann::json FactsExtractor::extractParallelPairs() {
    nlohmann::json pairs = nlohmann::json::array();
    
    ThreadCreationTree* tct = ThreadCreationTree::getInstance();
    std::vector<Thread*> threads_vec(tct->getThreads().begin(), tct->getThreads().end());
    
    for (size_t i = 0; i < threads_vec.size(); ++i) {
        for (size_t j = i + 1; j < threads_vec.size(); ++j) {
            Thread* t1 = threads_vec[i];
            Thread* t2 = threads_vec[j];
            
            // Skip if not sibling threads (mayHappenInParallel requires same parent)
            if (t1->getParent() != t2->getParent()) {
                continue;
            }
            
            bool can_parallel = tct->mayHappenInParallel(t1, t2);
            
            if (can_parallel) {
                std::string t1_name = "thread_" + std::to_string(t1->getId());
                std::string t2_name = "thread_" + std::to_string(t2->getId());
                
                if (t1->getThreadMainFunction() && t1->getThreadMainFunction()->getFuncNode()) {
                    t1_name = t1->getThreadMainFunction()->getFuncNode()->getCPGNode()->getName();
                }
                if (t2->getThreadMainFunction() && t2->getThreadMainFunction()->getFuncNode()) {
                    t2_name = t2->getThreadMainFunction()->getFuncNode()->getCPGNode()->getName();
                }
                
                int line_a = t1->getThreadMainFunction() ? 
                    t1->getThreadMainFunction()->getFuncNode()->getNodeLoc().getLineNumber() : 0;
                int line_b = t2->getThreadMainFunction() ? 
                    t2->getThreadMainFunction()->getFuncNode()->getNodeLoc().getLineNumber() : 0;
                
                nlohmann::json pair;
                pair["line_a"] = line_a;
                pair["line_b"] = line_b;
                pair["thread_a"] = t1_name;
                pair["thread_b"] = t2_name;
                pair["can_parallel"] = true;
                pairs.push_back(pair);
                
                parallel_pairs_.push_back({line_a, line_b, t1_name, t2_name, true});
            }
        }
    }
    
    return pairs;
}

nlohmann::json FactsExtractor::extractHappensBefore() {
    nlohmann::json hb_relations = nlohmann::json::array();
    
    try {
        ThreadCreationTree* tct = ThreadCreationTree::getInstance();
        if (!tct) return hb_relations;
        
        for (Thread* thread : tct->getThreads()) {
            if (!thread) continue;
            
            // Skip main thread (has no fork node)
            if (thread->isMainThread()) continue;
            
            CCPGNode* fork_node = thread->getForkNode();
            CCPGNode* join_node = thread->getJoinNode();
            
            // Additional null checks
            if (!fork_node) continue;
            
            // Check fork -> entry relationship
            if (fork_node && thread->getThreadMainFunction() && 
                thread->getThreadMainFunction()->getFuncNode()) {
                int fork_line = fork_node->getNodeLoc().getLineNumber();
                int entry_line = thread->getThreadMainFunction()->getFuncNode()->getNodeLoc().getLineNumber();
                
                nlohmann::json hb;
                hb["line_a"] = fork_line;
                hb["line_b"] = entry_line;
                hb["relation"] = "before";
                hb["type"] = "fork";
                hb_relations.push_back(hb);
                
                hb_relations_.push_back({fork_line, entry_line, "before"});
            }
            
            // Check exit -> join relationship
            if (join_node && thread->getThreadMainFunction()) {
                int join_line = join_node->getNodeLoc().getLineNumber();
                
                int exit_line = 0;
                for (CCPGNode* node : thread->getNodes()) {
                    if (node) {
                        exit_line = std::max(exit_line, node->getNodeLoc().getLineNumber());
                    }
                }
                
                if (exit_line > 0) {
                    nlohmann::json hb;
                    hb["line_a"] = exit_line;
                    hb["line_b"] = join_line;
                    hb["relation"] = "before";
                    hb["type"] = "join";
                    hb_relations.push_back(hb);
                    
                    hb_relations_.push_back({exit_line, join_line, "before"});
                }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "[FactsExtractor] Error in extractHappensBefore: " << e.what() << std::endl;
    }
    
    return hb_relations;
}

bool FactsExtractor::isNodeInProtectedRegion(CCPGNode* node, const std::string& lock) {
    if (!node) return false;
    
    int line = node->getNodeLoc().getLineNumber();
    
    for (const auto& region : protected_regions_) {
        if (region.lock == lock && line >= region.start && line <= region.end) {
            return true;
        }
    }
    return false;
}

nlohmann::json FactsExtractor::extractUnprotectedAccesses() {
    nlohmann::json unprotected = nlohmann::json::array();
    
    for (const auto& access : memory_accesses_) {
        if (access.type == "READ" || access.type == "WRITE") {
            bool is_protected = false;
            std::vector<std::string> locks_held;
            
            for (const auto& region : protected_regions_) {
                if (access.line >= region.start && access.line <= region.end &&
                    access.thread == region.thread) {
                    is_protected = true;
                    locks_held.push_back(region.lock);
                }
            }
            
            for (const auto& sv : shared_vars_) {
                if (access.var == sv.var_name && !is_protected) {
                    nlohmann::json entry;
                    entry["line"] = access.line;
                    entry["var"] = access.var;
                    entry["type"] = access.type;
                    entry["thread"] = access.thread;
                    entry["locks_held"] = locks_held;
                    unprotected.push_back(entry);
                    break;
                }
            }
        }
    }
    
    return unprotected;
}

GroundTruth parseGroundTruthFromReadme(const std::string& readme_path) {
    GroundTruth gt;
    
    std::ifstream file(readme_path);
    if (!file.is_open()) {
        return gt;
    }
    
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    
    std::regex type_regex(R"(Type[:\s]+(\w+[-\w]*))");
    std::smatch match;
    if (std::regex_search(content, match, type_regex)) {
        gt.bug_type = match[1].str();
    }
    
    if (content.find("Use-After-Free") != std::string::npos || 
        content.find("use-after-free") != std::string::npos) {
        gt.bug_type = "UseAfterFree";
    } else if (content.find("Double-Free") != std::string::npos || 
               content.find("double-free") != std::string::npos) {
        gt.bug_type = "DoubleFree";
    } else if (content.find("Race Condition") != std::string::npos || 
               content.find("Data Race") != std::string::npos ||
               content.find("data race") != std::string::npos) {
        gt.bug_type = "DataRace";
    } else if (content.find("Deadlock") != std::string::npos || 
               content.find("deadlock") != std::string::npos) {
        gt.bug_type = "Deadlock";
    } else if (content.find("NULL") != std::string::npos && 
               content.find("dereference") != std::string::npos) {
        gt.bug_type = "NullPointerDereference";
    }
    
    std::regex cve_regex(R"(CVE-\d{4}-\d+)");
    if (std::regex_search(content, match, cve_regex)) {
        gt.description = "Vulnerability: " + match[0].str();
    }
    
    return gt;
}

} // namespace facts
