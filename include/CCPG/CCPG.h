#ifndef CCPG_H
#define CCPG_H

#include <unordered_set>
#include <vector>
#include <stack>
#include <filesystem>

#include "CPG/Node.h"
#include "CPG/CPG.h"
//#include "CIG/LockAnalysis.h"
#include "CCPG/CCPGEdge.h"


using namespace SVF;
using namespace ccpg;

class Thread;
class Context;

typedef std::unordered_set<CCPGEdge *> CCPGEdgeSet;
typedef std::unordered_set<CCPGNode *> CCPGNodeSet;
typedef std::unordered_set<ccpg::Function *> FunctionSet;

class CCPG {

public:
    enum SpecialCallType
    {
        Alloc,
        Free
    };

    CCPG(const CPG* cpg) : cpg(cpg) {}
    ~CCPG() {}

    const CPG* getCPG() const { return cpg; }



    CCPGNode * createCCPGNode(Node *n);
    
    CCPGEdge * createCCPGEdge(CCPGNode *src, CCPGNode *dst);

    bool containsCPGNode(Node *node) const {
        return cpgNodeToCCPGNodeMap.find(node) != cpgNodeToCCPGNodeMap.end();
    }

    void addNode(CCPGNode *node) {
        nodes.insert(node);
        IDToCCPGNode[node->getId()] = node;
        typeToNodeSet[node->getType()].insert(node);
        cpgNodeToCCPGNodeMap[node->getCPGNode()] = node;
    }

    void addEdge(CCPGEdge *edge) {
        edges.insert(edge);
    }

    CCPGNode * getCCPGNodeByCPGNode(Node *node) const {
        auto it = cpgNodeToCCPGNodeMap.find(node);
        if (it != cpgNodeToCCPGNodeMap.end()) {
            return it->second;
        }
        return nullptr;
    }

    CCPGNode *getNodeByID(int id) const {
        auto it = IDToCCPGNode.find(id);
        if (it != IDToCCPGNode.end()) {
            return it->second;
        }
        return nullptr;
    }

    CCPGNodeSet getNodes() const { return nodes; }
    std::unordered_set<CCPGEdge *> getEdges() const { return edges; }

    CCPGNodeSet getNodesByType(ThreadAPIUtil::TYPE type) const {
        auto it = typeToNodeSet.find(type);
        if (it != typeToNodeSet.end()) {
            return it->second;
        }
        return CCPGNodeSet();
    }

    CCPGNodeSet getNodesByLoc(NodeLoc loc) const {
        auto it = locToNodeSetMap.find(loc);
        if (it != locToNodeSetMap.end()) {
            return it->second;
        }
        return CCPGNodeSet();
    }

    void addNodeByLoc(NodeLoc loc, CCPGNode *node) {
        locToNodeSetMap[loc].insert(node);
    }

    void addSVFInstByLoc(NodeLoc loc, const SVFStmt * stmt) {
        if(visited.find(stmt) != visited.end()){
            return;
        }
        locToSVFStmtMap[loc].push_back(stmt);
        visited.insert(stmt);
    }

    std::vector<const SVFStmt *> getSVFStmtByLoc(NodeLoc loc) const {
        auto it = locToSVFStmtMap.find(loc);
        if (it != locToSVFStmtMap.end()) {
            return it->second;
        }
        return std::vector<const SVFStmt *>();
    }



    FunctionSet getFunctions() const { return functions; }
    void addFunction(ccpg::Function *function) {
        functions.insert(function);
        funcNodeToFunctionMap[function->getFuncNode()] = function;
        IDToFunction[function->getFuncNode()->getId()] = function;
    }
    void removeFunction(ccpg::Function *function) {
        functions.erase(function);
        funcNodeToFunctionMap.erase(function->getFuncNode());
        IDToFunction.erase(function->getFuncNode()->getId());
        for (auto it = nodes.begin(); it != nodes.end(); ) {
            CCPGNode* node = *it;
            if (node->getFunction() == function) {
                it = nodes.erase(it); // 删除并更新迭代器
            } else {
                ++it; // 继续遍历
            }
        }
    }
    ccpg::Function * createFunction(CCPGNode * funcNode);
    ccpg::Function * getFunctionById(int id) const {
        auto it = IDToFunction.find(id);
        if (it != IDToFunction.end()) {
            return it->second;
        }
        return nullptr;
    }

    FunctionSet getEntryFunctions() const { return entryFunctions; }
    void addEntryFunction(ccpg::Function *function) { entryFunctions.insert(function); }

    void build();
    //std::unordered_set<Node*> findChildren(Node* node, std::unordered_set<Node*> visited_node = std::unordered_set<Node*>());
    std::unordered_set<Node*> findChildren(Node* node);

    ccpg::Function * getFunctionByCCPGNode(CCPGNode *node);

    void mapSVFInstructions();
    void addStructFieldStmt(const SVFStmt *stmt);

    CCPGNodeSet getEntries();

    CCPGNode * findCalleeByCaller(CCPGNode *caller);

    bool existsEdge(CCPGNode *src, CCPGNode *dst) const;

    CCPGNodeSet getNodesByType(ThreadAPIUtil::TYPE type) {
        auto it = typeToNodeSet.find(type);
        if (it != typeToNodeSet.end()) {
            return it->second;
        }
        return CCPGNodeSet();
    }

    CCPGEdge * hasCallEdge(CCPGNode *node);
    bool hasHBEdge(CCPGNode *node);

    ccpg::Function * getFunctionByFuncNode(CCPGNode *funcNode){
        auto it = funcNodeToFunctionMap.find(funcNode);
        if (it != funcNodeToFunctionMap.end()) {
            return it->second;
        }
        return nullptr;
    }

    ccpg::Function * createFunctionByCaller(CCPGNode *caller);

    void inferTemporality();

    bool isLoopBeginNode(CCPGNode* ccpgNode) const { return cpg->isLoopBeginNode(ccpgNode->getCPGNode()); }

    void constructContext();

    void labelForkPotential();
    void labelJoinPotential();
    void labelAPI();

    void replaceNode(CCPGNode *oldNode, CCPGNode *newNode);
    void replaceCallSiteBySpecificAPI(CCPGNode * api);
    void deleteNode(CCPGNode *node);
    
    CCPGNode * getCallSiteInFunction(const ccpg::Function * caller, const ccpg::Function * callee);

    void addSpecialCall(NodeLoc loc, SpecialCallType type, const CallICFGNode* callNode) {
        locToSpecialCallMap[loc][type].insert(callNode);
    }
    std::unordered_set<const CallICFGNode *> getSpecialCallByLoc(NodeLoc loc, SpecialCallType type) const {
        auto it = locToSpecialCallMap.find(loc);
        if (it != locToSpecialCallMap.end()) {
            auto it2 = it->second.find(type);
            if (it2 != it->second.end()) {
                return it2->second;
            }
        }
        return std::unordered_set<const CallICFGNode *>();
    }

    void dump(fs::path outputDir);

    private:
    const CPG* cpg;

    CCPGNodeSet nodes;
    CCPGEdgeSet edges;
    FunctionSet functions;
    TypeToNodeSetMap typeToNodeSet;
    std::unordered_map<int, CCPGNode *> IDToCCPGNode;
    std::unordered_map<int, ccpg::Function *> IDToFunction;
    std::unordered_map<Node *, CCPGNode *> cpgNodeToCCPGNodeMap;
    std::unordered_map<CCPGNode *, ccpg::Function *> funcNodeToFunctionMap;
    std::unordered_map<NodeLoc, CCPGNodeSet, NodeLocHash> locToNodeSetMap;
    std::unordered_map<NodeLoc, std::vector<const SVFStmt *>, NodeLocHash> locToSVFStmtMap;
    std::unordered_map<
    NodeLoc, 
    std::unordered_map< SpecialCallType, std::unordered_set<const CallICFGNode *>>, 
    NodeLocHash> locToSpecialCallMap;
    FunctionSet entryFunctions;
    std::unordered_set<const SVFStmt *> visited;
    std::unordered_map<Node*, std::unordered_set<Node*>> findChildrenCache;
};

#endif

