#include "Query/DeadLockDetector.h"
#include "CCPG/AliasChecker.h"

using namespace SVF;

void DeadLockDetector::detect(){
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

        // 获取两个线程的并行位置
        std::tie(parallelLocs1, parallelLocs2) = ctc->getParallelLocs(t1, t2);

        CCPGNodeSet acquires1 = t1->getNodesByType(ThreadAPIUtil::TYPE::ACQUIRE);
        CCPGNodeSet acquires2 = t2->getNodesByType(ThreadAPIUtil::TYPE::ACQUIRE);

        for(auto it = acquires1.begin(); it != acquires1.end(); it++){
            CCPGNode * acquire1 = *it;
            for(auto it_2 = acquires2.begin(); it_2 != acquires2.end(); it_2++){
                CCPGNode * acquire2 = *it_2;
                
                NodeLoc loc1 = acquire1->getNodeLoc();
                NodeLoc loc2 = acquire2->getNodeLoc();

                if(parallelLocs1.find(loc1) != parallelLocs1.end() && parallelLocs2.find(loc2) != parallelLocs2.end()){
                    if(lsAnalysis->isDeadLock(loc1, parallelLocs1[loc1], loc2, parallelLocs2[loc2])){
                        //addDeadLock(new DeadLock(parallelLocs1[loc1], loc1, acquire1->getLocks(), parallelLocs2[loc2], loc2, acquire2->getLocks()));
                    }
                }


            }
        }
}