#include <iostream>
#include <cstdlib>

#include "CPG/Node.h"
#include "CPG/Edge.h"
#include "CPG/CPG.h"
#include "PhasarUtil/LLVMAnalyzer.h"
#include "phasar.h"

using namespace std;
using namespace psr;

void CPG::addNode(std::unique_ptr<Node> node) {
    Node* raw_node = node.get();
    const std::string& id = raw_node->getIdString();
    if(id2Node.find(id) != id2Node.end()) {
        return ;
    }
    id2Node[id] = raw_node;
    type2Nodes[raw_node->getType()].insert(raw_node);

    if (raw_node->getType() == "Method") {
        file2Methods[raw_node->getFileName()].insert(raw_node);
        if (raw_node->getName() == "main") {
            mainNodes.insert(raw_node);
        }
    }
    m_nodes.push_back(std::move(node));
}

void CPG::addEdge(std::unique_ptr<Edge> edge) {
    Edge* raw_edge = edge.get();
    m_edges.push_back(std::move(edge));
}

Node* CPG::findNode(const std::string& id) const {
    auto it = id2Node.find(id);
    if (it != id2Node.end()) {
        return it->second; // 找到了，返回裸指针
    }
    return nullptr; // 没找到，返回nullptr
}

bool CPG::isLoopBeginNode(Node* node) const {

    CPGNodeSet controlNodes = getNodesByType("ControlStructure");

    for(Node* controlNode : controlNodes) {
        if(controlNode->properties["CONTROL_STRUCTURE_TYPE"] == "FOR" || controlNode->properties["CONTROL_STRUCTURE_TYPE"] == "WHILE") {
            for(Edge* edge : controlNode->conditionEdges) {
                if(edge->getToNode() == node) {
                    return true;
                }
            }
        }
    }
    return false;
}

bool CPG::isConditionBeginNode(Node* node) const {

    CPGNodeSet controlNodes = getNodesByType("ControlStructure");

    for(Node* controlNode : controlNodes) {
        if(controlNode->properties["CONTROL_STRUCTURE_TYPE"] == "IF" || controlNode->properties["CONTROL_STRUCTURE_TYPE"] == "ELSE" || controlNode->properties["CONTROL_STRUCTURE_TYPE"] == "SWITCH") {
            for(Edge* edge : controlNode->conditionEdges) {
                if(edge->getToNode() == node) {
                    return true;
                }
            }
        }
    }
    return false;
}


Node * CPG::findMethodByLLVMFunction(const llvm::Function* llvmFunc) const {
    if (!llvmFunc) {
        return nullptr;
    }
    LLVMAnalyzer *llvmAnalyzer = LLVMAnalyzer::getInstance();
    std::string funcNameFromLLVM = llvmAnalyzer->demangle(llvmFunc->getName().str().c_str());
    int lineFromLLVM = 0;
    std::string pathFromLLVM;
    if (auto* subprogram = llvmFunc->getSubprogram()) {
        pathFromLLVM = subprogram->getFilename().str();
        lineFromLLVM = subprogram->getLine();
    }
    if (pathFromLLVM.empty() || lineFromLLVM == 0) {
        return findMethod(funcNameFromLLVM);
    }

    CPGNodeSet allMethodsInCPG = getNodesByType("Method");
    for (Node* methodNode : allMethodsInCPG) {
        if (funcNameFromLLVM.find(methodNode->getName()) == std::string::npos) {
            continue;
        }
        const std::string& pathFromCPG = methodNode->getFileName();
        if (pathFromCPG.empty()) {
            continue;
        }
        if (!PathUtils::arePathsLikelySameFile(pathFromLLVM, pathFromCPG)) {
            continue;
        }
        if (methodNode->getLineNumber() != -1 && 
        abs(methodNode->getLineNumber() - lineFromLLVM) <= 3) {
        
            if (methodNode->properties.at("CODE") != "<empty>") {
                if (methodNode->outCFGEdges.size() == 1) {
                    Edge* edge = *methodNode->outCFGEdges.begin();
                    Node* nextNode = edge->getToNode();
                    if (nextNode->getType() == "Method_return") {
                        continue;
                    }
                }
                return methodNode;
            }
        }
    }
    return nullptr;
}

