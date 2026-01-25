// CPG.h
#ifndef CPG_H
#define CPG_H

#include "Node.h"
#include "Edge.h"
#include <vector>
#include <memory>
#include <filesystem>
#include <iostream>

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
    
        // 规范化路径，删除开头的 ".." 或 "."
        if (result.compare(0, 2, "..") == 0) {
            result = result.substr(2);
        } else if (result.compare(0, 1, ".") == 0) {
            result = result.substr(1);
        }
    
        // 如果直接匹配到，直接返回
        auto it = file2Methods.find(result);
        if (it != file2Methods.end()) {
            return it->second;
        }

        // 提取查询路径的basename (最后一个/之后的部分)
        std::string resultBasename = result;
        size_t lastSlash = result.find_last_of("/");
        if (lastSlash != std::string::npos) {
            resultBasename = result.substr(lastSlash + 1);
        }
    
        // 遍历 file2Methods，只匹配文件名完全相等的情况
        for (const auto& [key, methods] : file2Methods) {
            if(key == "") {
                continue;
            }
            // 提取key的basename
            std::string keyBasename = key;
            size_t keyLastSlash = key.find_last_of("/");
            if (keyLastSlash != std::string::npos) {
                keyBasename = key.substr(keyLastSlash + 1);
            }
            // 精确匹配basename
            if (keyBasename == resultBasename) {
                return methods;
            }
        }
    
        return CPGNodeSet(); // 没有匹配项，返回空集合
    }

    Node* findMethod(std::string name) const {

        CPGNodeSet methodNodes = type2Nodes.at("Method");

        for(Node* methodNode : methodNodes){
            if(methodNode->getName() == name && methodNode->properties["CODE"] != "<empty>"){
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
        return nullptr;
    }

    std::unordered_set<Node*> findMethodsByName(std::string name) const {
        std::unordered_set<Node*> methods;
        CPGNodeSet methodNodes = type2Nodes.at("Method");

        for(Node* methodNode : methodNodes){
            if(methodNode->getName() == name && methodNode->properties["CODE"] != "<empty>"){
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
