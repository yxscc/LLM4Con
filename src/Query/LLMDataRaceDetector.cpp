#include "Query/LLMDataRaceDetector.h"
#include "Query/ConcurrencyAnalysisHelper.h"
#include "CCPG/AliasChecker.h"
#include "CCPG/CCPG.h"
#include "CCPG/ThreadCreationTree.h"
#include "CCPG/LSAnalysis.h"
#include "PhasarUtil/AnalysisManager.h"
#include "PhasarUtil/PhasarPointerAnalysis.h"
#include "llvm/IR/Instructions.h"
#include <iostream>
#include <regex>
#include <unordered_set>
#include <cxxabi.h>
#include <filesystem>
#include <queue>
#include <algorithm>
#include <fstream> 
#include "LLMUtil/ThreadPair.h"

namespace query {

// --- LLMDataRace Implementation ---

LLMDataRace::LLMDataRace(
    const LLM::ConcurrencyContract::SharedVariable& var1,
    const LLM::ConcurrencyContract& c1,
    const LLM::ConcurrencyContract::SharedVariable& var2,
    const LLM::ConcurrencyContract& c2,
    const std::string& reason,
    const NodeLoc& loc1,
    const NodeLoc& loc2
) : variable1(var1), contract1(c1), variable2(var2), contract2(c2), reason(reason), location1(loc1), location2(loc2) {}

std::string LLMDataRace::toString() const {
    CCPG* ccpg = ThreadCreationTree::getInstance()->getCCPG();
    std::string code1 = "[Code not found]";
    if (ccpg) {
        CCPGNodeSet nodes1 = ccpg->getNodesByLoc(location1);
        if (!nodes1.empty()) {
            code1 = (*nodes1.begin())->getCPGNode()->getCode();
        }
    }
    std::string code2 = "[Code not found]";
    if (ccpg) {
        CCPGNodeSet nodes2 = ccpg->getNodesByLoc(location2);
        if (!nodes2.empty()) {
            code2 = (*nodes2.begin())->getCPGNode()->getCode();
        }
    }

    std::stringstream ss;
    ss << "========== LLM-Based Data Race Detected ==========\n"
       << "Reason: " << reason << "\n\n"
       << "Access 1 (Thread " << contract1.threadId << "):\n"
       << "  - Variable: " << variable1.variableName << " (Type: " << variable1.variableType << ")\n"
       << "  - Access: " << variable1.accessType << "\n"
       << "  - Location: " << location1.toString() << "\n"
       << "  - Source Code: " << code1 << "\n"
       << "  - Protected by: [";
    for (size_t i = 0; i < variable1.protectingPrimitives.size(); ++i) {
        ss << variable1.protectingPrimitives[i] << (i == variable1.protectingPrimitives.size() - 1 ? "" : ", ");
    }
    ss << "]\n\n"
       << "Access 2 (Thread " << contract2.threadId << "):\n"
       << "  - Variable: " << variable2.variableName << " (Type: " << variable2.variableType << ")\n"
       << "  - Access: " << variable2.accessType << "\n"
       << "  - Location: " << location2.toString() << "\n" // <-- ADDED
       << "  - Source Code: " << code2 << "\n" // <-- ADDED
       << "  - Protected by: [";
    for (size_t i = 0; i < variable2.protectingPrimitives.size(); ++i) {
        ss << variable2.protectingPrimitives[i] << (i == variable2.protectingPrimitives.size() - 1 ? "" : ", ");
    }
    ss << "]\n"
       << "==================================================";
    return ss.str();
}


void LLMDataRaceDetector::detect(const std::vector<llm_client::ThreadPair>& threadPairs) {
    LSAnalysis* lsAnalysis = LSAnalysis::getInstance();
    AliasChecker* aliasChecker = AliasChecker::getInstance();
    CCPG* ccpg = ThreadCreationTree::getInstance()->getCCPG();
    
    std::set<std::pair<NodeLoc, NodeLoc>> reported_loc_pairs;
    for (const auto& pair : threadPairs) {

        std::cout << "\n[DEBUG PRINT] Analyzing concurrent pair: Thread " 
                  << pair.thread1->getId() << " and Thread " << pair.thread2->getId() << std::endl;

        // 使用通用的辅助器来遍历所有并发访问对
        forEachConcurrentAccessPair(pair, 
            [&](const MemoryAccess& acc1, const LLM::ConcurrencyContract& contract1,
                const MemoryAccess& acc2, const LLM::ConcurrencyContract& contract2) 
            {

                if (!acc1.isWrite && !acc2.isWrite) {
                    return;
                }

                if (aliasChecker->isCompilerGeneratedSafeInit(acc1) || aliasChecker->isCompilerGeneratedSafeInit(acc2)) {
                    return;
                }

                auto key = (acc1.location < acc2.location) 
                         ? std::make_pair(acc1.location, acc2.location) 
                         : std::make_pair(acc2.location, acc1.location);

                if (reported_loc_pairs.count(key)) {
                    return;
                }

                if (!lsAnalysis->isProtectedBySameLock(acc1.location, acc1.context, acc2.location, acc2.context)) {

                    reported_loc_pairs.insert(key);

                    CCPGNodeSet nodes1 = ccpg->getNodesByLoc(acc1.location);
                    CCPGNodeSet nodes2 = ccpg->getNodesByLoc(acc2.location);
                    std::string code1 = nodes1.empty() ? "[Code not found]" : (*nodes1.begin())->getCPGNode()->getCode();
                    std::string code2 = nodes2.empty() ? "[Code not found]" : (*nodes2.begin())->getCPGNode()->getCode();

                    std::cout << "\n[DEBUG PRINT] >>> NEW UNIQUE DATA RACE FOUND <<<" << std::endl;
                    std::cout << "  - Thread 1 Access: " << (acc1.isWrite ? "WRITE" : "READ") 
                              << " at " << acc1.location.toString() << std::endl;
                    std::cout << "    Source Code: " << code1 << std::endl;
                    std::cout << "  - Thread 2 Access: " << (acc2.isWrite ? "WRITE" : "READ") 
                              << " at " << acc2.location.toString() << std::endl;
                    std::cout << "    Source Code: " << code2 << std::endl;
                    
                    std::string v1_str, v2_str;
                    llvm::raw_string_ostream os1(v1_str), os2(v2_str);
                    acc1.pointerOperand->print(os1);
                    acc2.pointerOperand->print(os2);
                    std::cout << "  - Pointer Operand 1: " << os1.str() << std::endl;
                    std::cout << "  - Pointer Operand 2: " << os2.str() << std::endl;
                    std::cout << "============================================" << std::endl;
                    
                    std::string reason = "Unprotected concurrent access to a shared memory location.";
                    
                    const LLM::ConcurrencyContract::SharedVariable* relevant_var1 = nullptr;
                    const LLM::ConcurrencyContract::SharedVariable* relevant_var2 = nullptr;

                    for(const auto& cv : contract1.sharedVariables) {
                       if (aliasChecker->isAliasOfContractVariable(acc1.pointerOperand, cv, contract1.entryPointFunctionId)) {
                           relevant_var1 = &cv;
                           break;
                       }
                    }
                    for(const auto& cv : contract2.sharedVariables) {
                       if (aliasChecker->isAliasOfContractVariable(acc2.pointerOperand, cv, contract2.entryPointFunctionId)) {
                           relevant_var2 = &cv;
                           break;
                       }
                    }

                    LLM::ConcurrencyContract::SharedVariable temp_var1 = relevant_var1 ? *relevant_var1 : LLM::ConcurrencyContract::SharedVariable{"[statically found]", "[unknown]", acc1.isWrite ? "Write" : "Read"};
                    LLM::ConcurrencyContract::SharedVariable temp_var2 = relevant_var2 ? *relevant_var2 : LLM::ConcurrencyContract::SharedVariable{"[statically found]", "[unknown]", acc2.isWrite ? "Write" : "Read"};

                    if(relevant_var1 && relevant_var2) {
                        reason = "Unprotected concurrent access to shared variable '" + relevant_var1->variableName + "' (verified by both static analysis and LLM contract).";
                    }
                    
                    this->detectedRaces.emplace_back(temp_var1, contract1, temp_var2, contract2, reason, acc1.location, acc2.location);
                }
            }
        );
    }
}

void LLMDataRaceDetector::printDataRaces(const fs::path& outputDir) const {
    if (detectedRaces.empty()) {
        std::cout << "No LLM-guided data races detected." << std::endl;
        return;
    }

    fs::path dataRacesOutputDir = outputDir / "dataraces_llm";
    if (!fs::exists(dataRacesOutputDir)) {
        fs::create_directory(dataRacesOutputDir);
    }

    std::ofstream file(dataRacesOutputDir / "dataraces.txt");
    int i = 1;
    for (const auto& race : detectedRaces) {
        file << race.toString() << "\n\n";
        file << "count  " << i++ << " ----------------------------------------" << std::endl;
    }
    file.close();
    std::cout << "LLM-guided data race detection complete. " << detectedRaces.size() 
              << " potential races found. Results are in: " << (dataRacesOutputDir / "dataraces.txt") << std::endl;
}

} // namespace query
