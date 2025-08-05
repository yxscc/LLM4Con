/*#include "Query/NullReferenceDetector.h"
#include "CCPG/AliasChecker.h"

using namespace SVF;

std::set<std::pair<NodeLoc, NodeLoc>> NullReferences;

bool NullReferenceDetector::isNullReference(const SVFStmt * stmt1, const SVFStmt * stmt2) {
    SVFManager * svfManager = SVFManager::getInstance();
    SVFIR * pag = svfManager->getSVFIR();
    AliasChecker * aliasChecker = AliasChecker::getInstance();
    if(SVFUtil::isa<LoadStmt>(stmt1) && SVFUtil::isa<LoadStmt>(stmt2)){
        return false;
    }
    if(SVFUtil::isa<StoreStmt>(stmt1) && SVFUtil::isa<StoreStmt>(stmt2)){
        return false;
    }
    if(const StoreStmt* s = SVFUtil::dyn_cast<StoreStmt>(stmt1)){
        if(!pag->isNullPtr(s->getRHSVarID())){
            return false;
        }
        return aliasChecker->isStmtAlias(stmt1, stmt2);
    }
    if(const StoreStmt* s = SVFUtil::dyn_cast<StoreStmt>(stmt2)){
        if(!pag->isNullPtr(s->getRHSVarID())){
            return false;
        }
        return aliasChecker->isStmtAlias(stmt1, stmt2);
    }
    return false;
}

void NullReferenceDetector::detect() {

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
                if (NullReferences.count(key)) {
                    continue;
                }
    
                for(auto it_3 = stmts1.begin(); it_3 != stmts1.end(); it_3++){
                    for(auto it_4 = stmts2.begin(); it_4 != stmts2.end(); it_4++){
                        if(isNullReference(*it_3, *it_4)){
                            const SVFStmt * stmt1 = *it_3;
                            const SVFStmt * stmt2 = *it_4;
                            addNullReference(new NullReference(ctx_1, loc1, *it_3, ctx_2, loc2, *it_4));
                            NullReferences.insert(key);
                            break;
                        }
                    }
                }
            }
        }
    }
}

void NullReferenceDetector::printNullReferences(fs::path outputDir) {

    fs::path NullReferencesOutputDir = outputDir / "nullReferences";
    if (!fs::exists(NullReferencesOutputDir)) {
        fs::create_directory(NullReferencesOutputDir);
    }

    std::ofstream file(NullReferencesOutputDir / "nullReferences.txt");
    int i = 0;
    for(NullReference * npr : nprs){
        file << npr->toString() << std::endl;
        file << "count  "<< i++ << " ----------------------------------------" << std::endl;
    }
    file.close();
}*/