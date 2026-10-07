#include <set>
#include <queue>
#include <vector>
#include <iterator>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>

#include "CCPG/LSAnalysis.h"
#include "CCPG/AliasChecker.h"

using namespace ccpg;

LSAnalysis* LSAnalysis::instance = nullptr;

// Intra-procedural "which locks are definitely held here" analysis.
//
// This is a classic must-analysis over the ORDER (intra-procedural CFG)
// edges, solved to fixpoint:
//
//     IN[n]  = intersection of OUT[p] over n's ORDER predecessors
//     OUT[n] = IN[n] + gen(n) - kill(n)
//
// and `nodeLockSets[n]` stores OUT[n], i.e. the locks held once n has
// executed. Intersection (rather than union) is what makes "protected"
// sound: a node reachable both with and by-passing the lock must not be
// reported as protected.
//
// The previous implementation walked each function once with a
// `std::set<CCPGNode*>` worklist — so nodes came out in POINTER order,
// not CFG order — and marked every popped node `visited`, never
// re-enqueueing it. A lockset therefore stopped dead at the first
// already-visited successor, which in practice was almost immediately.
// Across the 72-case kernel set that left 0 of 17392 surface accesses
// carrying any lock at all.
void LSAnalysis::build() {
    AliasChecker* aliasChecker = AliasChecker::getInstance();

    auto orderSuccessors = [](CCPGNode* n) {
        std::vector<CCPGNode*> out;
        for (CCPGEdge* edge : n->getOutEdges()) {
            if (edge->getType() == CCPGEdge::EdgeType::ORDER && edge->getDst())
                out.push_back(edge->getDst());
        }
        return out;
    };
    auto orderPredecessors = [](CCPGNode* n) {
        std::vector<CCPGNode*> in;
        for (CCPGEdge* edge : n->getInEdges()) {
            if (edge->getType() == CCPGEdge::EdgeType::ORDER && edge->getSrc())
                in.push_back(edge->getSrc());
        }
        return in;
    };

    for (ccpg::Function * function : ccpg->getFunctions()) {
        if (function == nullptr) continue;
        CCPGNode* entry = function->getFuncNode();
        if (entry == nullptr) continue;

        // ---- Reachable node set, in BFS order from the entry ---------------
        // Nodes the entry cannot reach keep an empty lockset, matching the
        // old default-constructed behaviour rather than being initialised to
        // "everything is held".
        std::vector<CCPGNode*> rpo;
        std::unordered_set<CCPGNode*> reachable;
        {
            std::queue<CCPGNode*> q;
            q.push(entry);
            reachable.insert(entry);
            while (!q.empty()) {
                CCPGNode* n = q.front();
                q.pop();
                rpo.push_back(n);
                for (CCPGNode* s : orderSuccessors(n)) {
                    if (reachable.insert(s).second) q.push(s);
                }
            }
        }

        // ---- gen/kill, with each lock object allocated exactly once --------
        // A fixpoint revisits nodes, so allocating a Lock inside the loop (as
        // the old code did) would mint a fresh lock identity per visit and
        // break every downstream `isLockAlias` comparison.
        std::unordered_map<CCPGNode*, Lock*> genLock;
        std::unordered_map<CCPGNode*, CCPGNode*> killVia;
        for (CCPGNode* node : rpo) {
            CCPGNode* acquireSite = nullptr;
            CCPGNode* releaseSite = nullptr;

            if (node->getType() == ThreadAPIUtil::TYPE::ACQUIRE) {
                acquireSite = node;
            } else if (node->getType() == ThreadAPIUtil::TYPE::RELEASE) {
                releaseSite = node;
            } else if (node->isCallSite()) {
                CCPGEdge* callEdge = ccpg->hasCallEdge(node);
                ccpg::Function* callee =
                    callEdge ? ccpg->getFunctionByCCPGNode(callEdge->getDst())
                             : nullptr;
                if (callee != nullptr) {
                    // A wrapper whose whole body is one acquire (or one
                    // release) stands in for the lock operation itself.
                    if (callee->isAcquirePotential()) {
                        auto acqs = callee->getNodesByType(ThreadAPIUtil::TYPE::ACQUIRE);
                        if (!acqs.empty()) acquireSite = *acqs.begin();
                    } else if (callee->isReleasePotential()) {
                        auto rels = callee->getNodesByType(ThreadAPIUtil::TYPE::RELEASE);
                        if (!rels.empty()) releaseSite = *rels.begin();
                    }
                }
            }

            if (acquireSite != nullptr) {
                Lock* lock = new Lock(static_cast<int>(getLocks().size()) + 1);
                addLock(lock);
                lock->addRelatedNode(acquireSite);
                genLock[node] = lock;
            } else if (releaseSite != nullptr) {
                killVia[node] = releaseSite;
            }
        }

        // Nothing to propagate in a function with no lock operation at all.
        if (genLock.empty() && killVia.empty()) {
            for (CCPGNode* node : rpo) nodeLockSets[node];
            continue;
        }

        std::vector<Lock*> universe;
        universe.reserve(genLock.size());
        for (const auto& kv : genLock) universe.push_back(kv.second);
        std::sort(universe.begin(), universe.end());

        auto applyTransfer = [&](CCPGNode* node,
                                 const std::vector<Lock*>& in) {
            std::vector<Lock*> out = in;
            auto g = genLock.find(node);
            if (g != genLock.end()) {
                if (std::find(out.begin(), out.end(), g->second) == out.end())
                    out.push_back(g->second);
                std::sort(out.begin(), out.end());
                return out;
            }
            auto k = killVia.find(node);
            if (k != killVia.end()) {
                CCPGNode* releaseNode = k->second;
                out.erase(std::remove_if(out.begin(), out.end(),
                                         [&](Lock* l) {
                                             return l != nullptr &&
                                                    aliasChecker->isLockAlias(
                                                        l->getAcquire(),
                                                        releaseNode);
                                         }),
                          out.end());
                // Record the pairing so `Lock::getRelease()` still resolves.
                for (Lock* l : universe) {
                    if (l != nullptr && !l->hasRelease() &&
                        aliasChecker->isLockAlias(l->getAcquire(), releaseNode)) {
                        l->addRelatedNode(releaseNode);
                    }
                }
            }
            return out;
        };

        // ---- Fixpoint ------------------------------------------------------
        // IN starts at TOP (every lock) for non-entry nodes so that a loop
        // back-edge cannot spuriously clear the set on the first pass.
        std::unordered_map<CCPGNode*, std::vector<Lock*>> IN, OUT;
        IN.reserve(rpo.size());
        OUT.reserve(rpo.size());
        for (CCPGNode* node : rpo) {
            IN[node] = (node == entry) ? std::vector<Lock*>() : universe;
            OUT[node] = applyTransfer(node, IN[node]);
        }

        bool changed = true;
        size_t rounds = 0;
        const size_t maxRounds = rpo.size() + 16;
        while (changed && rounds < maxRounds) {
            changed = false;
            ++rounds;
            for (CCPGNode* node : rpo) {
                std::vector<Lock*> in;
                if (node == entry) {
                    // A root frame holds nothing; caller-held locks are added
                    // later by the Context-walking `getLockSet` overloads.
                    in.clear();
                } else {
                    bool first = true;
                    for (CCPGNode* p : orderPredecessors(node)) {
                        if (reachable.find(p) == reachable.end()) continue;
                        const std::vector<Lock*>& po = OUT[p];
                        if (first) {
                            in = po;
                            first = false;
                        } else {
                            std::vector<Lock*> tmp;
                            std::set_intersection(in.begin(), in.end(),
                                                  po.begin(), po.end(),
                                                  std::back_inserter(tmp));
                            in.swap(tmp);
                        }
                        if (in.empty()) break;
                    }
                    if (first) in.clear();  // no reachable predecessor
                }

                if (in != IN[node]) {
                    IN[node] = in;
                    std::vector<Lock*> out = applyTransfer(node, in);
                    if (out != OUT[node]) {
                        OUT[node] = std::move(out);
                    }
                    changed = true;
                }
            }
        }

        for (CCPGNode* node : rpo) nodeLockSets[node] = OUT[node];
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

std::vector<Lock*> LSAnalysis::getLockSet(NodeLoc loc, Context ctx) {
    CCPG * ccpg = LSAnalysis::getInstance()->getCCPG();
    std::vector<Lock*> ctxlockSet;

    // v19 P3: union the locksets of EVERY CCPG node at this location
    // rather than picking `getNodesByLoc(loc).begin()` (which is the
    // non-deterministic first iterator of an unordered_set). When a
    // macro-expanded line or a synthesised helper site produces several
    // CCPG nodes that share a NodeLoc, the previous code would silently
    // drop any caller-held lock that happened to live in the OTHER
    // node's CFG-built lockset — that was a major source of v18
    // `lockset_wrong` false positives (caller mutex_lock(&X) followed
    // by helper(): the helper's NodeLoc node sees no lock, the
    // caller's NodeLoc node does, and `.begin()` could pick either).
    auto siblings = ccpg->getNodesByLoc(loc);
    for (CCPGNode* sib : siblings) {
        if (!sib) continue;
        const auto& sl = nodeLockSets[sib];
        ctxlockSet.insert(ctxlockSet.begin(), sl.begin(), sl.end());
    }

    const std::vector<CCPGNode*>& callStack = ctx.getCallStack();
    for(auto it = callStack.rbegin(); it != callStack.rend(); it++){
        CCPGNode * node = *it;
        std::vector<Lock*> locks = nodeLockSets[node];
        ctxlockSet.insert(ctxlockSet.begin(), locks.begin(), locks.end());
    }

    // Dedup so downstream loop is O(unique-locks).
    std::sort(ctxlockSet.begin(), ctxlockSet.end());
    ctxlockSet.erase(std::unique(ctxlockSet.begin(), ctxlockSet.end()),
                     ctxlockSet.end());
    return ctxlockSet;
}

std::vector<Lock*> LSAnalysis::getLockSet(CCPGNode * node, Context ctx) {
    std::vector<Lock*> ctxlockSet;
    if (!node) return ctxlockSet;

    // Base lockset: the specific CFG-built lockset at THIS node. Avoids
    // the unordered-set `.begin()` ambiguity that the NodeLoc overload
    // suffers from.
    {
        const auto& sl = nodeLockSets[node];
        ctxlockSet.insert(ctxlockSet.begin(), sl.begin(), sl.end());
    }

    // Walk the call stack like the NodeLoc overload, accumulating
    // caller-held locks at every frame.
    const std::vector<CCPGNode*>& callStack = ctx.getCallStack();
    for (auto it = callStack.rbegin(); it != callStack.rend(); ++it) {
        CCPGNode * cn = *it;
        if (!cn) continue;
        const auto& locks = nodeLockSets[cn];
        ctxlockSet.insert(ctxlockSet.begin(), locks.begin(), locks.end());
    }

    std::sort(ctxlockSet.begin(), ctxlockSet.end());
    ctxlockSet.erase(std::unique(ctxlockSet.begin(), ctxlockSet.end()),
                     ctxlockSet.end());
    return ctxlockSet;
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
