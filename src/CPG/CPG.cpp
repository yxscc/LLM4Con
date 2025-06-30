#include <iostream>
#include <cstdlib> // For system()
#include <filesystem>
#include "CPG/Node.h"
#include "CPG/Edge.h"
#include "CPG/CPG.h"

using namespace std;

void CPG::addNode(std::unique_ptr<Node> node) {
    Node* raw_node = node.get();
    if(id2Node.find(raw_node->id) != id2Node.end()) {
        return ;
    }
    id2Node[raw_node->id] = raw_node;
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


