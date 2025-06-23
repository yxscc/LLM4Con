#include "Query/UseAfterFreeDetector.h"
#include "CCPG/AliasChecker.h"

using namespace SVF;

void UseAfterFreeDetector::detect() {

    ThreadCreationTree * ctc = ThreadCreationTree::getInstance();
    LSAnalysis * lsAnalysis = LSAnalysis::getInstance();
    CCPG * ccpg = lsAnalysis->getCCPG();
    AliasChecker * aliasChecker = AliasChecker::getInstance();

    std::unordered_map<std::pair<Thread*, Thread*>, std::string, pair_hash> parallelThreadPairs = ctc->getParallelThreadPairs();

    for(auto it = parallelThreadPairs.begin(); it != parallelThreadPairs.end(); it++){
        std::pair<Thread*, Thread*> pair = it->first;
        Thread * t1 = pair.first;
        Thread * t2 = pair.second;
        std::string relation = it->second;
        
        std::unordered_map<NodeLoc, Context, NodeLocHash> parallelLocs1;
        std::unordered_map<NodeLoc, Context, NodeLocHash> parallelLocs2;

        std::tie(parallelLocs1, parallelLocs2) = ctc->getParallelLocs(t1, t2);

        for(auto it = parallelLocs1.begin(); it != parallelLocs1.end(); it++){

            NodeLoc loc1 = it->first;
            Context ctx_1 = it->second;
            std::unordered_set<const CallICFGNode *> callSites1 = ccpg->getSpecialCallByLoc(loc1, CCPG::SpecialCallType::Free);
            if(callSites1.size() == 0){
                continue;
            }
    
            for(auto it_2 = parallelLocs2.begin(); it_2 != parallelLocs2.end(); it_2++){
                NodeLoc loc2 = it_2->first;
                Context ctx_2 = it_2->second;
    
                std::vector<const SVFStmt *> stmts2 = ccpg->getSVFStmtByLoc(loc2);
    
                if(stmts2.size() == 0){
                    continue;
                }
    
                auto key = (loc1 < loc2) ? std::make_pair(loc1, loc2) : std::make_pair(loc2, loc1);
    
                for(auto it_3 = callSites1.begin(); it_3 != callSites1.end(); it_3++){
                    const CallICFGNode * callNode1 = *it_3;
                    for(auto it_4 = stmts2.begin(); it_4 != stmts2.end(); it_4++){
                        const SVFStmt * stmt2 = *it_4;
                        if(aliasChecker->isUseAndFreeAlias(callNode1, stmt2)){
                            addUseAfterFree(new UseAfterFree(ctx_1, loc1, callNode1, ctx_2, loc2, stmt2));
                            break;
                        }
                    }
                }
            }
        }

        for(auto it = parallelLocs2.begin(); it != parallelLocs2.end(); it++){

            NodeLoc loc1 = it->first;
            Context ctx_1 = it->second;
            std::unordered_set<const CallICFGNode *> callSites1 = ccpg->getSpecialCallByLoc(loc1, CCPG::SpecialCallType::Free);
            if(callSites1.size() == 0){
                continue;
            }
    
            for(auto it_2 = parallelLocs1.begin(); it_2 != parallelLocs1.end(); it_2++){
                NodeLoc loc2 = it_2->first;
                Context ctx_2 = it_2->second;
    
                std::vector<const SVFStmt *> stmts2 = ccpg->getSVFStmtByLoc(loc2);
    
                if(stmts2.size() == 0){
                    continue;
                }
    
                auto key = (loc1 < loc2) ? std::make_pair(loc1, loc2) : std::make_pair(loc2, loc1);
    
                if(lsAnalysis->isProtectedBySameLock(loc1, ctx_1, loc2, ctx_2))
                    continue;
    
                for(auto it_3 = callSites1.begin(); it_3 != callSites1.end(); it_3++){
                    const CallICFGNode * callNode1 = *it_3;
                    for(auto it_4 = stmts2.begin(); it_4 != stmts2.end(); it_4++){
                        const SVFStmt * stmt2 = *it_4;
                        if(aliasChecker->isUseAndFreeAlias(callNode1, stmt2)){
                            addUseAfterFree(new UseAfterFree(ctx_1, loc1, callNode1, ctx_2, loc2, stmt2));
                            break;
                        }
                    }
                }
            }
        }

    }
}

void UseAfterFreeDetector::printUseAfterFrees(fs::path outputDir) {
    fs::path useAfterFreesOutputDir = outputDir / "useAfterFrees";
    if (!fs::exists(useAfterFreesOutputDir)) {
        fs::create_directory(useAfterFreesOutputDir);
    }

    std::ofstream file(useAfterFreesOutputDir / "useAfterFrees.txt");
    int i = 0;
    for(UseAfterFree * uaf : uafs){
        file << uaf->toString() << std::endl;
        file << "count  "<< i++ << " ----------------------------------------" << std::endl;
    }
    file.close();
}