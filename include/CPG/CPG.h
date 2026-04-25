// CPG.h
#ifndef CPG_H
#define CPG_H

#include "Node.h"
#include "Edge.h"
#include "Util/PathUtils.h"
#include <vector>
#include <memory>
#include <filesystem>
#include <iostream>
#include <cstring>

namespace fs = std::filesystem;

namespace llvm {
    class Function;
    class Instruction;
} // namespace llvm

typedef std::unordered_set<Node*> CPGNodeSet;

class CPG {
private:
    std::vector<std::unique_ptr<Node>> m_nodes;
    std::vector<std::unique_ptr<Edge>> m_edges;
    std::unordered_map<std::string, Node*> id2Node; // 根据ID查找节点
    CPGNodeSet mainNodes; // 主函数节点
    
    std::unordered_map<std::string, CPGNodeSet> type2Nodes; // 根据类型查找节点
    std::unordered_map<std::string, CPGNodeSet> file2Methods; // 根据名称查找节点

public:
    CPG() {}
    ~CPG() = default;

    void addNode(std::unique_ptr<Node> node);
    void addEdge(std::unique_ptr<Edge> edge);
    Node* findNode(const std::string& id) const;

    // CPGNodeSet getNodes() const { return nodes; }
    // std::unordered_set<Edge*> getEdges() const { return edges; }

    CPGNodeSet getMainNodes() const { return mainNodes; }
    bool isMainNode(Node* node) const { return mainNodes.find(node) != mainNodes.end(); }

    bool isLoopBeginNode(Node* node) const;

    bool isConditionBeginNode(Node* node) const;

    CPGNodeSet getNodesByType(std::string type) const {
        if(type2Nodes.find(type) == type2Nodes.end()) {
            return CPGNodeSet();
        }
        return type2Nodes.at(type);
    }

    CPGNodeSet getMethodsByFileName(std::string fileName) const {
        std::string result = fileName;
    
        if (result.compare(0, 2, "..") == 0) {
            result = result.substr(2);
        } else if (result.compare(0, 1, ".") == 0) {
            result = result.substr(1);
        }
    
        auto it = file2Methods.find(result);
        if (it != file2Methods.end()) {
            return it->second;
        }

        for (const auto& [key, methods] : file2Methods) {
            if(key.empty()) continue;
            if (PathUtils::arePathsLikelySameFile(key, result)) {
                return methods;
            }
        }
    
        return CPGNodeSet();
    }

    // Generate the Linux syscall-entry naming variants for a given "base"
    // name. The kernel's SYSCALL_DEFINE macro produces several alias
    // functions in the IR (architecture-specific prefixes, internal helpers)
    // that all refer to the same C-level body. The CPG, however, typically
    // only contains the unprefixed base. This helper lets findMethod fall
    // back through the common variants so the IR entry name can still be
    // mapped to the CPG method node.
    static std::vector<std::string> syscallNameVariants(const std::string& name) {
        std::vector<std::string> out;
        out.push_back(name);
        // Well-known prefixes produced by SYSCALL_DEFINE* macro expansion.
        static const char* kPrefixes[] = {
            "__x64_sys_", "__ia32_sys_", "__arm64_sys_", "__arm_sys_",
            "__riscv_sys_", "__powerpc_sys_", "__se_sys_", "__do_sys_",
            "SyS_", "sys_"
        };
        std::string base = name;
        for (const char* p : kPrefixes) {
            std::size_t plen = std::strlen(p);
            if (base.size() > plen && base.compare(0, plen, p) == 0) {
                base = base.substr(plen);
                break;
            }
        }
        if (base != name) out.push_back(base);
        // Also try each prefix applied to the stripped base, so that when
        // the CPG has one form and the IR has another we can find either.
        for (const char* p : kPrefixes) {
            std::string candidate = std::string(p) + base;
            if (candidate != name) out.push_back(candidate);
        }
        return out;
    }

    Node* findMethod(std::string name) const {

        auto it = type2Nodes.find("Method");
        if (it == type2Nodes.end()) return nullptr;
        const CPGNodeSet& methodNodes = it->second;

        // Try each candidate name in order of descending specificity.
        for (const std::string& candidate : syscallNameVariants(name)) {
            for(Node* methodNode : methodNodes){
                if(methodNode->getName() == candidate && methodNode->properties["CODE"] != "<empty>"){
                    if(methodNode->outCFGEdges.size() == 1){
                        std::unordered_set<Edge*> outEdges = methodNode->outCFGEdges;
                        Edge* edge = *outEdges.begin();
                        Node* nextNode = edge->getToNode();
                        if(nextNode->getType() == "Method_return"){
                            continue;
                        }
                    }
                    return methodNode;
                }
            }
        }
        return nullptr;
    }

    std::unordered_set<Node*> findMethodsByName(std::string name) const {
        std::unordered_set<Node*> methods;
        auto it = type2Nodes.find("Method");
        if (it == type2Nodes.end()) return methods;
        const CPGNodeSet& methodNodes = it->second;

        // Try all syscall-style name variants so IR-side prefixed names
        // (e.g. __x64_sys_foo) also match CPG nodes named sys_foo or foo.
        std::vector<std::string> candidates = syscallNameVariants(name);
        std::unordered_set<std::string> tried(candidates.begin(), candidates.end());

        for(Node* methodNode : methodNodes){
            const std::string& mname = methodNode->getName();
            if(tried.count(mname) && methodNode->properties["CODE"] != "<empty>"){
                if(methodNode->outCFGEdges.size() == 1){
                    std::unordered_set<Edge*> outEdges = methodNode->outCFGEdges;
                    Edge* edge = *outEdges.begin();
                    Node* nextNode = edge->getToNode();
                    if(nextNode->getType() == "Method_return"){
                        continue;
                    }
                }
                methods.insert(methodNode);
            }
        }
        return methods;
    }

    Node* findMethod(Node* node) const {
        bool hasCallEdge = false;
        Node* method = nullptr;
        std::string callerName = node->getName();
        std::string callFullName = node->getProperty("METHOD_FULL_NAME");
        std::string callerFile = node->getFileName();
        if (callerFile.empty()) {
            for (Edge* edge : node->inEdges) {
                if (edge->getType() == "Contains") {
                    Node* parent = edge->getFromNode();
                    if (parent && parent->getType() == "Method") {
                        callerFile = parent->getFileName();
                        break;
                    }
                }
            }
        }
        if (!callFullName.empty() && callFullName != "<empty>") {
            CPGNodeSet methodNodes = type2Nodes.at("Method");
            Node* bestFullMatch = nullptr;
            for (Node* methodNode : methodNodes) {
                std::string fullName = methodNode->getProperty("FULL_NAME");
                if (!fullName.empty() && fullName == callFullName) {
                    if (methodNode->properties["CODE"] != "<empty>") {
                        if (!callerFile.empty() &&
                            methodNode->getFileName() == callerFile) {
                            return methodNode;
                        }
                        if (!bestFullMatch) {
                            bestFullMatch = methodNode;
                        }
                    }
                }
            }
            if (bestFullMatch) {
                return bestFullMatch;
            }
        }
        for(auto edge : node->outEdges){
            std::string calleeName = edge->getToNode()->getName();
            std::string type = edge->getType();
            if(edge->getType() == "Call"
            && (calleeName.find(callerName) != std::string::npos || callerName.find(calleeName) != std::string::npos)
            && edge->getToNode()->properties["CODE"] != "<empty>"){
                if(method == nullptr){
                    method = edge->getToNode();
                    hasCallEdge = true;
                }
                else{
                    Node* temp = edge->getToNode();
                    if(temp->properties["CODE"].length() > method->properties["CODE"].length()){
                        method = temp;
                    }
                }
            }

        }
        if(hasCallEdge){
            return method;
        }
        else{
            std::string name;
            Node* fallback = findMethod(node->getName());
            return fallback;
        }
    }

    Node * findMethodByLLVMFunction(const llvm::Function* llvmFunc) const;
    
    bool hasContainsEdge(Node * callee, Node * node) const{
        for(auto edge : node->inEdges){
            if(edge->getType() == "Contains" && edge->getFromNode() == callee){
                return true;
            }
        }
        return false;
    }

};

#endif // CPG_H
