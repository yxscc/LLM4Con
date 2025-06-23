// HB.h      happens-before graph
#ifndef HB_H
#define HB_H

#include <unordered_set>
#include <unordered_map>
#include <stack>
#include <filesystem>

#include "ThreadCreationTree.h"

class HB {
private:
    CCPG* ccpg; //ccpg指针
    std::unordered_map<CCPGNode*, std::unordered_set<CCPGNode*>> exitMap = std::unordered_map<CCPGNode*, std::unordered_set<CCPGNode*>>();
    std::unordered_set<CCPGNode*> mainNodes;
    bool hasHBEdge = false; 
    static HB* instance;

public:
    HB() {}
    ~HB() {}

    void setCCPG(CCPG* ccpg) { this->ccpg = ccpg; }

    CCPG* getCCPG() const { return ccpg; }

    static HB* getInstance() {
        if(instance == nullptr){
            instance = new HB();
        }
        return instance;
    }

    CCPGNodeSet getNodes() const { return ccpg->getNodes(); }
    std::unordered_set<CCPGEdge*> getEdges() const { return ccpg->getEdges(); }
    std::unordered_set<CCPGNode*> getMainNodes() const { return mainNodes; }

    void buildExitMap(std::unordered_set<Thread*> threads);

    void buildHB(std::unordered_set<Thread*> threads);

    // 连接并行函数出口和对应的JOIN节点，当前方法暂时不需要
    void addHBEdge();

    // 复制循环内的线程操作节点
    void copyThreadNode();

    // 给节点添加当前可能并发的方法信息
    void addConcurrentInfo();
    void conInfoCallBack(CCPGNode* node, Context* context, std::unordered_set<CCPGNode*> concurrentNodes);

    bool isForkNode(CCPGNode* node);
    bool isJoinNode(CCPGNode* node);
    bool isExitNode(CCPGNode* node);
    bool isLoopBackNode(CCPGNode* node);

    // 根据forkNode获取子线程的方法节点
    CCPGNode* getStartNode(CCPGNode* forkNode);

    // 判断节点执行有无先后关系，参数无顺序要求，即只要有先后关系都返回true。反之返回false
    void nodeHappensBefore(CCPGNode* node1, CCPGNode* node2);

    bool concurrentMatch(Context* context, std::unordered_set<CCPGNode*> forkSet);
    bool contextHappensBefore(CCPGNode* node1, Context* context_1, CCPGNode* node2, Context* context_2);
    bool concurrentNodesEqual(std::unordered_set<CCPGNode*> nodes_1, std::unordered_set<CCPGNode*> nodes_2);
};

#endif