/*#include "Query/DoubleFreeDetector.h"
#include "CCPG/AliasChecker.h"

using namespace SVF;

void DoubleFreeDetector::detect() {

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
    
                std::unordered_set<const CallICFGNode *> callSites2 = ccpg->getSpecialCallByLoc(loc2, CCPG::SpecialCallType::Free);
    
                if(callSites2.size() == 0){
                    continue;
                }
    
                auto key = (loc1 < loc2) ? std::make_pair(loc1, loc2) : std::make_pair(loc2, loc1);

    
                for(auto it_3 = callSites1.begin(); it_3 != callSites1.end(); it_3++){
                    const CallICFGNode * callNode1 = *it_3;
                    for(auto it_4 = callSites2.begin(); it_4 != callSites2.end(); it_4++){
                        const CallICFGNode * callNode2 = *it_4;
                        if(aliasChecker->isFreeAndFreeAlias(callNode1, callNode2)){
                            addDoubleFree(new DoubleFree(ctx_1, loc1, callNode1, ctx_2, loc2, callNode2));
                            break;
                        }
                    }
                }
            }
        }
    }
}

void DoubleFreeDetector::printDoubleFrees(fs::path outputDir) {
    fs::path doubleFreesOutputDir = outputDir / "doubleFrees";
    if (!fs::exists(doubleFreesOutputDir)) {
        fs::create_directory(doubleFreesOutputDir);
    }

    std::ofstream file(doubleFreesOutputDir / "DoubleFrees.txt");
    int i = 1;
    for(DoubleFree * df : dfs){
        file << df->toString() << std::endl;
        file << "count  "<< i++ << " ----------------------------------------" << std::endl;
    }
    file.close();
}*/