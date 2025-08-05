// CPG.h
#ifndef CPG_H
#define CPG_H

#include "Node.h"
#include "Edge.h"
#include <vector>
#include <memory>
#include <filesystem>

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
    
        // 遍历 file2Methods，查找后缀匹配的 key
        for (const auto& [key, methods] : file2Methods) {
            if(key == "") {
                continue;
            }
            if (key.size() >= result.size() &&
                key.find(result) != std::string::npos) {
                return methods; // 返回第一个匹配到的
            }
            if (result.size() >= key.size() &&
                result.find(key) != std::string::npos) {
                return methods; // 返回第一个匹配到的
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
            return findMethod(node->getName());
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
