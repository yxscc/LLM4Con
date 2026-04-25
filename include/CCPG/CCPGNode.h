#ifndef CCPG_NODE_H
#define CCPG_NODE_H

#include <filesystem>
#include <unordered_set>

#include "CPG/Node.h"
#include "CPG/CPG.h"
#include "CCPG/ThreadAPIUtil.h"
#include "Util/TargetPath.h"
#include "Util/PathUtils.h"
#include "Context.h"

namespace llvm {
    class Instruction;
    class CallInst;
    class Function;
}

namespace ccpg {
    class Function;
}

class CCPGEdge;
class Fucntion;
class Lock;
class Thread;

class NodeLoc {
    public:
        NodeLoc() {}
        NodeLoc(std::string fileName, int lineNumber, ccpg::Function * function) {
            std::string targetAbsolutePath = TargetPath::getInstance()->getTargetAbsolutePath();
            this->fileName = mergePaths(targetAbsolutePath, fileName);
            this->normalizedFileName = normalizePath(this->fileName);
            this->lineNumber = lineNumber;
            this->function = function;
        }
        ~NodeLoc() {}
    
        std::string mergePaths(const std::string& absolutePath, const std::string& relativePath) {
            std::filesystem::path relPath(relativePath);
            if (relPath.is_absolute()) {
                return normalizePath(relativePath);
            }
            std::vector<std::string> absParts = splitPath(absolutePath);
            std::vector<std::string> relParts = splitPath(relativePath);
    
            // 找到重叠部分
            int overlapIndex = -1;
            for (size_t i = 0; i < absParts.size(); ++i) {
                size_t len = absParts.size() - i;
                if (len <= relParts.size() && 
                    std::vector<std::string>(absParts.begin() + i, absParts.end()) ==
                    std::vector<std::string>(relParts.begin(), relParts.begin() + len)) {
                    overlapIndex = i;
                    break;
                }
            }
    
            // 拼接路径
            std::ostringstream mergedPath;
            for (size_t i = 0; i < absParts.size(); ++i) {
                if (i > 0) mergedPath << "/";
                mergedPath << absParts[i];
            }
            if (overlapIndex != -1) {
                for (size_t i = absParts.size() - overlapIndex; i < relParts.size(); ++i) {
                    mergedPath << "/" << relParts[i];
                }
            } else {
                for (size_t i = 0; i < relParts.size(); ++i) {
                    mergedPath << "/" << relParts[i];
                }
            }
            return normalizePath(mergedPath.str());
        }
    
        std::vector<std::string> splitPath(const std::string& path) {
            std::vector<std::string> parts;
            std::stringstream ss(path);
            std::string item;
            while (std::getline(ss, item, '/')) {
                if (!item.empty()) {
                    parts.push_back(item);
                }
            }
            return parts;
        }
    
        static std::vector<std::filesystem::path> extractComponents(const std::filesystem::path& path) {
            std::vector<std::filesystem::path> components;
            for (auto it = path.begin(); it != path.end(); ++it) {
                components.push_back(*it);
            }
            return components;
        }
    
    
        static bool arePathsLikelySameFile(const std::filesystem::path& path1, const std::filesystem::path& path2) {
            auto components1 = extractComponents(path1);
            auto components2 = extractComponents(path2);
    
            if (components1.empty() || components2.empty()) {
                return false;
            }
            if (components1.back() != components2.back()) {
                return false;
            }

            auto it1 = components1.rbegin();
            auto it2 = components2.rbegin();
    
            while (it1 != components1.rend() && it2 != components2.rend()) {
                if (*it1 != *it2) {
                    return false;
                }
                ++it1;
                ++it2;
            }
    
            return true;
        }
    
    
        std::string getFileName() const { return fileName; }
        std::string getNormalizedFileName() const { return normalizedFileName; }
        std::string getBaseFileName() const {
            return extractBaseFileName(normalizedFileName);
        }
        int getLineNumber() const { return lineNumber; }

        ccpg::Function* getFunction() const { return function; }

        static std::string extractBaseFileName(const std::string& path) {
            return PathUtils::extractBaseFileName(path);
        }

        static bool fileNamesMatch(const std::string& path1, const std::string& path2) {
            return PathUtils::arePathsLikelySameFile(path1, path2);
        }
    
        bool operator==(const NodeLoc& other) const {
            if (lineNumber != other.lineNumber) return false;
            if (arePathsLikelySameFile(normalizedFileName, other.normalizedFileName))
                return true;
            return getBaseFileName() == other.getBaseFileName();
        }

        bool operator<(const NodeLoc& other) const {
            std::string base1 = getBaseFileName();
            std::string base2 = other.getBaseFileName();
            if (base1 != base2) return base1 < base2;
            if (lineNumber != other.lineNumber) return lineNumber < other.lineNumber;
            return normalizedFileName < other.normalizedFileName;
        }
    
        std::string toString() const {
            return fileName + ":" + std::to_string(lineNumber);
        }
    
    
    
    private:
        std::string normalizePath(const std::string& path) const {
            std::filesystem::path p(path);
            return p.lexically_normal().string();
        }

        std::string fileName;
        std::string normalizedFileName;
        int lineNumber;
        ccpg::Function * function;
    };
    
    struct NodeLocHash {
        std::size_t operator()(const NodeLoc& nl) const {
            std::size_t h1 = std::hash<std::string>()(nl.getBaseFileName());
            std::size_t h2 = std::hash<int>()(nl.getLineNumber());
            return h1 ^ (h2 << 1); 
        }
    };
    

typedef std::unordered_set<CCPGEdge *> CCPGEdgeSet;

class CCPGNode {

private:
    int id;
    ThreadAPIUtil::TYPE type;
    Node * cpgNode;
    CCPGEdgeSet inEdges;
    CCPGEdgeSet outEdges;
    NodeLoc nodeLoc;
    ccpg::Function * function;
    int controlFlowOrder = 0;
    bool callSite;
    const llvm::CallInst* llvmCallInst = nullptr; // 专门存储CallInst
    std::vector<Lock *> intraProceduralLocks;
    std::unordered_set<Thread *> relevantThreads;

public:
    
    CCPGNode() {}
    CCPGNode(Node* node, ThreadAPIUtil::TYPE type) 
    : cpgNode(node), 
    type(type),
    function(nullptr),
    callSite(false),
    controlFlowOrder(0) {
    // 其他初始化逻辑
    }
    ~CCPGNode() {}

    void setId(int id) { this->id = id; }
    int getId() const { return id; }

    void setCallSite(bool isCallSite) { this->callSite = isCallSite; }
    bool isCallSite() const { return callSite; }

    void setLLVMCallInst(const llvm::CallInst* CI) { llvmCallInst = CI; }
    const llvm::CallInst* getLLVMCallInst() const { return llvmCallInst; }

    void setType(ThreadAPIUtil::TYPE type) { this->type = type; }
    ThreadAPIUtil::TYPE getType() const { return type; }

    void setControlFlowOrder(int order) { this->controlFlowOrder = order; }
    int getControlFlowOrder() const { return controlFlowOrder; }

    CCPGEdgeSet getInEdges() const { return inEdges; }
    void addInEdge(CCPGEdge *edge) { inEdges.insert(edge); }
    CCPGEdgeSet getOutEdges() const { return outEdges; }
    void addOutEdge(CCPGEdge *edge) { outEdges.insert(edge); }

    Node * getCPGNode() const { return cpgNode; }

    void setNodeLoc(const NodeLoc& nodeLoc) { this->nodeLoc = nodeLoc; }
    const NodeLoc& getNodeLoc() const { return nodeLoc; }

    ccpg::Function * getFunction() const { return function; }
    void setFunction(ccpg::Function * function) { this->function = function; }

    std::unordered_set<Thread *> getRelevantThreads() const { return relevantThreads; }
    void addRelevantThread(Thread * thread) { relevantThreads.insert(thread); }

    void addIntraProceduralLock(Lock * lock) { intraProceduralLocks.push_back(lock); }
    std::vector<Lock *> getIntraProceduralLocks() const { return intraProceduralLocks; }

    void removeOutEdge(CCPGEdge * edge) {
        outEdges.erase(edge);
    }

    void removeInEdge(CCPGEdge * edge) {
        inEdges.erase(edge);
    }

};



typedef std::unordered_set<CCPGNode *> CCPGNodeSet;

namespace ccpg {

typedef std::unordered_map<ThreadAPIUtil::TYPE, CCPGNodeSet> TypeToNodeSetMap;
typedef std::unordered_set<Context *> ContextSet;
typedef std::unordered_set<Function *> FunctionSet;

class Function {
private:
    CCPGNodeSet nodes;
    CCPGEdgeSet edges;
    TypeToNodeSetMap typeToNodeSet;
    CCPGNode * funcNode;
    ContextSet contextSet;
    bool forkPotential = false;
    bool acquirePotential = false;
    bool releasePotential = false;
    bool joinPotential = false;
    std::unordered_map<NodeLoc, CCPGNodeSet, NodeLocHash> locToNodeSetMap;
    const llvm::Function * llvmFunction = nullptr;

public:
    Function(CCPGNode * funcNode) {
        this->funcNode = funcNode;
    }
    ~Function() {}

    void addNode(CCPGNode *node) {
        nodes.insert(node);
        typeToNodeSet[node->getType()].insert(node);
    }
    CCPGNodeSet getNodes() const { return nodes; }
    
    void addEdge(CCPGEdge *edge) { edges.insert(edge); }
    CCPGEdgeSet getEdges() const { return edges; }

    void setLLVMFunction(const llvm::Function * func) { this->llvmFunction = func; }
    const llvm::Function * getLLVMFunction() const { return llvmFunction; }

    CCPGNodeSet getNodesByType(ThreadAPIUtil::TYPE type) const {
        auto it = typeToNodeSet.find(type);
        if (it != typeToNodeSet.end()) {
            return it->second;
        }
        return CCPGNodeSet();
    }

    void addNodeByLoc(NodeLoc loc, CCPGNode *node) {
        locToNodeSetMap[loc].insert(node);
    }

    CCPGNode * getFuncNode() const { return funcNode; }

    bool isForkPotential() const { return forkPotential; }
    void setForkPotential(bool isForkPotential) { this->forkPotential = isForkPotential; }

    bool isAcquirePotential() const { return acquirePotential; }
    void setAcquirePotential(bool isAcquirePotential) { this->acquirePotential = isAcquirePotential; }

    bool isReleasePotential() const { return releasePotential; }
    void setReleasePotential(bool isReleasePotential) { this->releasePotential = isReleasePotential; }

    bool isJoinPotential() const { return joinPotential; }
    void setJoinPotential(bool isJoinPotential) { this->joinPotential = isJoinPotential; }

    ContextSet getContextSet() const { return contextSet; }
    void addContext(Context * context) { contextSet.insert(context); }

    std::unordered_map<NodeLoc, CCPGNodeSet, NodeLocHash> getLocToNodeSetMap() const { return locToNodeSetMap; }

    std::unordered_set<NodeLoc, NodeLocHash> findLocsInScope(NodeLoc l_1, NodeLoc l_2) const{
        std::unordered_set<NodeLoc, NodeLocHash> locs;
        for(auto it = locToNodeSetMap.begin(); it != locToNodeSetMap.end(); it++){
            NodeLoc loc = it->first;
            if(loc.getLineNumber() >= l_1.getLineNumber() && loc.getLineNumber() <= l_2.getLineNumber()){
                locs.insert(loc);
            }
        }
        return locs;
    }
    std::unordered_set<NodeLoc, NodeLocHash> findAllLocs() const{
        std::unordered_set<NodeLoc, NodeLocHash> locs;
        for(auto it = locToNodeSetMap.begin(); it != locToNodeSetMap.end(); it++){
            locs.insert(it->first);
        }
        return locs;
    }

    FunctionSet getCallers() const ;

    CCPGNodeSet getCallSites() const ;

    int getCallOrder(const ccpg::Function * childFunc) const ;

    int getId() const {
        return funcNode->getId();
    }
};
};

#endif // CCPG_NODE_H