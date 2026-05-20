#ifndef LOCKANALYSIS_H
#define LOCKANALYSIS_H

#include <unordered_set>
#include <unordered_map>
#include <vector>

#include "ThreadCreationTree.h"

class Lock{
public:
    Lock(int id): id(id) {}
    ~Lock() {}

    int getId() { return id; }

    void addRelatedNode(CCPGNode * node) {
        ThreadAPIUtil::TYPE type = node->getType();
        typeToNode[type] = node;
    }

    CCPGNode * getRelatedNode(ThreadAPIUtil::TYPE type) {
        return typeToNode[type];
    }

    CCPGNode * getAcquire() { return typeToNode[ThreadAPIUtil::TYPE::ACQUIRE]; }
    CCPGNode * getRelease() { return typeToNode[ThreadAPIUtil::TYPE::RELEASE]; }

    bool hasRelease() { return typeToNode.find(ThreadAPIUtil::TYPE::RELEASE) != typeToNode.end(); }


private:
    int id;
    std::unordered_map<ThreadAPIUtil::TYPE, CCPGNode *> typeToNode;
};

class LSAnalysis {
private:
    std::unordered_set<Lock *> locks;
    CCPG * ccpg;
    std::unordered_map<CCPGNode*, std::vector<Lock*>> nodeLockSets;

    LSAnalysis() {}
    static LSAnalysis * instance;

public:
    static LSAnalysis * getInstance() {
        if (instance == nullptr) {
            instance = new LSAnalysis();
        }
        return instance;
    }

    ~LSAnalysis() {}

    void setCCPG(CCPG * ccpg) { this->ccpg = ccpg; }
    CCPG * getCCPG() { return ccpg; }

    void addLock(Lock * lock) { locks.insert(lock); }
    std::unordered_set<Lock *> getLocks() { return locks; }

    void build();

    std::vector<Lock*> getLockSet(NodeLoc loc, Context ctx);
    // Overload that uses a SPECIFIC CCPGNode for the base lockset rather
    // than picking `*(getNodesByLoc(loc).begin())` (which is non-deterministic
    // when several CCPG nodes share a NodeLoc — a common situation for
    // macro-expanded sites and synthesised list-helper accesses). This
    // is the form used by the v19 verifier.
    std::vector<Lock*> getLockSet(CCPGNode * node, Context ctx);
    bool isProtectedBySameLock(CCPGNode * node1, CCPGNode * node2);
    bool isProtectedBySameLock(NodeLoc loc1, Context ctx1, NodeLoc loc2, Context ctx2);
    bool isDeadLock(NodeLoc loc1, Context ctx1, NodeLoc loc2, Context ctx2);
    bool hasLockOrderConflict(Lock * lock1, Lock * lock2, std::vector<Lock *> & lockSet);
};

#endif