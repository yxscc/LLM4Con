#include <iostream>
#include <cstdlib> // For system()
#include <filesystem>
#include "CPG/Node.h"
#include "CPG/Edge.h"
#include "CPG/CPG.h"

using namespace std;

void CPG::addNode(Node* node) {
    if(id2Node.find(node->id) != id2Node.end()) {
        return ;
    }
    nodes.insert(node);
    id2Node[node->id] = node;
    type2Nodes[node->getType()].insert(node);

    if(node->getType() == "Method") {
        file2Methods[node->getFileName()].insert(node);
        if(node->getName() == "main") {
            mainNodes.insert(node);
        }
    }
}

void CPG::addEdge(Edge* edge) {
    if(edges.find(edge) != edges.end()) {
        return ;
    }
    edges.insert(edge);
}

Node* CPG::findNode(const char * id) const {
    return id2Node.at(id);
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


