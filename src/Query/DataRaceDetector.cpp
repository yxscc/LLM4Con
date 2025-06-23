#include "Query/DataRaceDetector.h"
#include "CCPG/AliasChecker.h"

using namespace SVF;

std::set<std::pair<NodeLoc, NodeLoc>> dataRaces;

void DataRaceDetector::detect() {

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

        for(auto it = parallelLocs1.begin(); it != parallelLocs1.end(); it++){

            NodeLoc loc1 = it->first;
            Context ctx_1 = it->second;
            std::vector<const SVFStmt *> stmts1 = ccpg->getSVFStmtByLoc(loc1);

            if(stmts1.size() == 0){
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
    
                // 检查是否已存在
                if (dataRaces.count(key)) {
                    continue;
                }
    
                if(lsAnalysis->isProtectedBySameLock(loc1, ctx_1, loc2, ctx_2))
                    continue;
    
                bool isDataRace = false;
    
                for(auto it_3 = stmts1.begin(); it_3 != stmts1.end(); it_3++){
                    for(auto it_4 = stmts2.begin(); it_4 != stmts2.end(); it_4++){
                        if(aliasChecker->isStmtAlias(*it_3, *it_4)){
                            if(!aliasChecker->areSameField(*it_3, ccpg->getNodesByLoc(loc1), *it_4, ccpg->getNodesByLoc(loc2))){
                                continue;
                            }
                            const SVFStmt * stmt1 = *it_3;
                            const SVFStmt * stmt2 = *it_4;
                            addDataRace(new DataRace(ctx_1, loc1, *it_3, ctx_2, loc2, *it_4));
                            dataRaces.insert(key);
                            isDataRace = true;
                            break;
                        }
                    }
                    if(isDataRace){
                        break;
                    }
                }
            }
        }
    }
}

void DataRaceDetector::printDataRaces(fs::path outputDir) {

    fs::path dataRacesOutputDir = outputDir / "dataraces";
    if (!fs::exists(dataRacesOutputDir)) {
        fs::create_directory(dataRacesOutputDir);
    }

    std::ofstream file(dataRacesOutputDir / "dataraces.txt");
    int i = 1;
    for(DataRace * datarace : dataraces){
        file << datarace->toString() << std::endl;
        file << "count  "<< i++ << " ----------------------------------------" << std::endl;
    }
    file.close();
}