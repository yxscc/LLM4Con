#include <set>

#include "CCPG/LSAnalysis.h"
#include "CCPG/AliasChecker.h"

using namespace ccpg;

LSAnalysis* LSAnalysis::instance = nullptr;

void LSAnalysis::build() {
    
    for(ccpg::Function * function : ccpg->getFunctions()){

        std::vector<Lock *> tempLockSet;
        std::set<CCPGNode *> visited;
        std::set<CCPGNode *> worklist;
        worklist.insert(function->getFuncNode());

        // 函数入口节点的锁集为空
        nodeLockSets[function->getFuncNode()] = {};

        while (!worklist.empty()) {
            CCPGNode* node = *worklist.begin();
            worklist.erase(node);
            visited.insert(node);

            // 获取当前节点的锁集
            std::vector<Lock*>& currentLockSet = nodeLockSets[node];

            // 处理锁操作
            if (node->getType() == ThreadAPIUtil::TYPE::ACQUIRE) {
                // 创建新锁对象
                Lock* lock = new Lock(getLocks().size() + 1);
                addLock(lock);
                lock->addRelatedNode(node);

                // 将锁加入当前锁集
                currentLockSet.push_back(lock);
            } else if (node->getType() == ThreadAPIUtil::TYPE::RELEASE) {
                // 释放锁
                AliasChecker* aliasChecker = AliasChecker::getInstance();
                for (auto it = currentLockSet.rbegin(); it != currentLockSet.rend(); ++it) {
                    if (!(*it)->hasRelease() && aliasChecker->isLockAlias((*it)->getAcquire(), node)) {
                        (*it)->addRelatedNode(node);
                        currentLockSet.erase(std::next(it).base());
                        break;
                    }
                }
            }
            if (node->isCallSite()){
                CCPGEdge * callEdge = ccpg->hasCallEdge(node);
                if(callEdge != nullptr){
                    ccpg::Function * callee = ccpg->getFunctionByCCPGNode(callEdge->getDst());
                    if(callee->isAcquirePotential()){
                        Lock* lock = new Lock(getLocks().size() + 1);
                        addLock(lock);
                        lock->addRelatedNode(*(callee->getNodesByType(ThreadAPIUtil::TYPE::ACQUIRE).begin()));
                        // 将锁加入当前锁集
                        currentLockSet.push_back(lock);
                    }
                    else if(callee->isReleasePotential()){
                        AliasChecker* aliasChecker = AliasChecker::getInstance();
                        CCPGNode * releaseNode = *(callee->getNodesByType(ThreadAPIUtil::TYPE::RELEASE).begin());
                        for (auto it = currentLockSet.rbegin(); it != currentLockSet.rend(); ++it) {
                            if (!(*it)->hasRelease() && aliasChecker->isLockAlias((*it)->getAcquire(), releaseNode)) {
                                (*it)->addRelatedNode(releaseNode);
                                currentLockSet.erase(std::next(it).base());
                                break;
                            }
                        }
                    }
                }
            }

            // 遍历出边，将锁集传递给后继节点
            for (CCPGEdge* edge : node->getOutEdges()) {
                if (edge->getType() == CCPGEdge::EdgeType::ORDER) {
                    CCPGNode* dst = edge->getDst();

                    // 如果目标节点未被访问过，初始化其锁集
                    if (visited.find(dst) == visited.end()) {
                        worklist.insert(dst);

                        // 继承当前节点的锁集
                        nodeLockSets[dst] = currentLockSet;
                    } else {
                        // 如果目标节点已被访问过，合并锁集
                        auto& dstLockSet = nodeLockSets[dst];
                        dstLockSet.insert(dstLockSet.end(), currentLockSet.begin(), currentLockSet.end());

                        // 去重
                        std::sort(dstLockSet.begin(), dstLockSet.end());
                        dstLockSet.erase(std::unique(dstLockSet.begin(), dstLockSet.end()), dstLockSet.end());
                    }
                }
            }
        }
    }
}

bool LSAnalysis::isProtectedBySameLock(CCPGNode * node1, CCPGNode * node2) {
    std::vector<Lock *> locks1 = nodeLockSets[node1];
    std::vector<Lock *> locks2 = nodeLockSets[node2];

    AliasChecker* aliasChecker = AliasChecker::getInstance();

    for (Lock* lock1 : locks1) {
        for (Lock* lock2 : locks2) {
            if (aliasChecker->isLockAlias(lock1->getAcquire(), lock2->getAcquire())) {
                return true;
            }
        }
    }

    return false;
}

bool LSAnalysis::isProtectedBySameLock(NodeLoc loc1, Context ctx1, NodeLoc loc2, Context ctx2) {
    CCPG * ccpg = LSAnalysis::getInstance()->getCCPG();
    AliasChecker* aliasChecker = AliasChecker::getInstance();
    
    std::vector<Lock*> ctxlockSet1, ctxlockSet2;

    ctxlockSet1.insert(ctxlockSet1.begin(), nodeLockSets[*(ccpg->getNodesByLoc(loc1).begin())].begin(), nodeLockSets[*(ccpg->getNodesByLoc(loc1).begin())].end());
    ctxlockSet2.insert(ctxlockSet2.begin(), nodeLockSets[*(ccpg->getNodesByLoc(loc2).begin())].begin(), nodeLockSets[*(ccpg->getNodesByLoc(loc2).begin())].end());

    const std::vector<CCPGNode*>& callStack1 = ctx1.getCallStack();
    const std::vector<CCPGNode*>& callStack2 = ctx2.getCallStack();
    for(auto it = callStack1.rbegin(); it != callStack1.rend(); it++){
        CCPGNode * node = *it;
        std::vector<Lock*> locks = nodeLockSets[node];
        ctxlockSet1.insert(ctxlockSet1.begin(), locks.begin(), locks.end());
    }
    for(auto it = callStack2.rbegin(); it != callStack2.rend(); it++){
        CCPGNode * node = *it;
        std::vector<Lock*> locks = nodeLockSets[node];
        ctxlockSet2.insert(ctxlockSet2.begin(), locks.begin(), locks.end());
    }

    for (Lock* lock1 : ctxlockSet1) {
        for (Lock* lock2 : ctxlockSet2) {
            if (aliasChecker->isLockAlias(lock1->getAcquire(), lock2->getAcquire())) {
                return true;
            }
        }
    }

    return false;
}

bool LSAnalysis::isDeadLock(NodeLoc loc1, Context ctx1, NodeLoc loc2, Context ctx2) {
    CCPG * ccpg = LSAnalysis::getInstance()->getCCPG();
    AliasChecker* aliasChecker = AliasChecker::getInstance();
    
    std::vector<Lock*> ctxlockSet1, ctxlockSet2;

    ctxlockSet1.insert(ctxlockSet1.begin(), nodeLockSets[*(ccpg->getNodesByLoc(loc1).begin())].begin(), nodeLockSets[*(ccpg->getNodesByLoc(loc1).begin())].end());
    ctxlockSet2.insert(ctxlockSet2.begin(), nodeLockSets[*(ccpg->getNodesByLoc(loc2).begin())].begin(), nodeLockSets[*(ccpg->getNodesByLoc(loc2).begin())].end());

    const std::vector<CCPGNode*>& callStack1 = ctx1.getCallStack();
    const std::vector<CCPGNode*>& callStack2 = ctx2.getCallStack();
    for(auto it = callStack1.rbegin(); it != callStack1.rend(); it++){
        CCPGNode * node = *it;
        std::vector<Lock*> locks = nodeLockSets[node];
        ctxlockSet1.insert(ctxlockSet1.begin(), locks.begin(), locks.end());
    }
    for(auto it = callStack2.rbegin(); it != callStack2.rend(); it++){
        CCPGNode * node = *it;
        std::vector<Lock*> locks = nodeLockSets[node];
        ctxlockSet2.insert(ctxlockSet2.begin(), locks.begin(), locks.end());
    }

        // 检测是否存在死锁
    // 第一阶段：检查CTX1锁顺序在CTX2中是否反转
    for (size_t i = 0; i < ctxlockSet1.size(); ++i) {
        for (size_t j = i+1; j < ctxlockSet1.size(); ++j) {
            Lock* earlier = ctxlockSet1[i];
            Lock* later = ctxlockSet1[j];
            
            // 跳过同一锁的别名（如通过pthread_mutex_init创建的多个指针指向同一锁）
            if (aliasChecker->isLockAlias(earlier->getAcquire(), later->getAcquire())) continue;
            
            // 检测是否在CTX2中存在相反顺序
            if (hasLockOrderConflict(later, earlier, ctxlockSet2)) {
                return true;
            }
        }
    }

    // 第二阶段：检查CTX2锁顺序在CTX1中是否反转
    for (size_t i = 0; i < ctxlockSet2.size(); ++i) {
        for (size_t j = i+1; j < ctxlockSet2.size(); ++j) {
            Lock* earlier = ctxlockSet2[i];
            Lock* later = ctxlockSet2[j];
            
            if (aliasChecker->isLockAlias(earlier->getAcquire(), later->getAcquire())) continue;
            
            if (hasLockOrderConflict(later, earlier, ctxlockSet1)) {
                return true;
            }
        }
    }

    return false;
}

// 辅助函数实现
bool LSAnalysis::hasLockOrderConflict(Lock* expectedFirst, Lock* expectedSecond,
                                      std::vector<Lock*>& lockset) {
    AliasChecker* aliasChecker = AliasChecker::getInstance();
    bool foundFirst = false;
    
    // 遍历锁集合，检查是否存在 expectedSecond -> expectedFirst 的逆序
    for (Lock* lock : lockset) {
        if (aliasChecker->isLockAlias(lock->getAcquire(), expectedSecond->getAcquire())) {
            foundFirst = true; // 先发现expectedSecond的别名
        } else if (foundFirst && aliasChecker->isLockAlias(lock->getAcquire(), expectedFirst->getAcquire())) {
            // 在发现expectedSecond之后发现expectedFirst，形成逆序
            return true;
        }
    }
    return false;
}