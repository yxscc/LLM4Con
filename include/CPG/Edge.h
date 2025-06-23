// Edge.h
#ifndef EDGE_H
#define EDGE_H

#include <string>
#include <unordered_map>
#include "Node.h"

class Node;

class Edge {
private:
    std::string type; // 边类型
    const char * fromId; // 起始节点ID
    Node* fromNode; // 起始节点
    const char * toId; // 终止节点ID
    Node* toNode; // 终止节点

public:
    Edge(const char * inV, const char * outV, std::string type_upper) {
        this->fromId = outV;
        this->toId = inV;
        this->type = type_upper;
        for(int i = 0; i < type.size(); i++){
            if(i == 0){
                type[i] = toupper(type[i]);
            }
            else{
                type[i] = tolower(type[i]);
            }
        }
    }

    // 可以根据需要添加虚析构函数
    virtual ~Edge() {}

    // 获取边类型
    std::string getType() {
        return type;
    }

    const char * getFromId() {
        return fromId;
    }

    const char * getToId() {
        return toId;
    }

    Node* getFromNode() {
        return fromNode;
    }
    void setFromNode(Node* node) {
        fromNode = node;
    }

    Node* getToNode() {
        return toNode;
    }
    void setToNode(Node* node) {
        toNode = node;
    }

};



#endif // EDGE_H
