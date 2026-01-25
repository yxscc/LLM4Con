// Node.h
#ifndef NODE_H
#define NODE_H

#include <string>
#include <unordered_map>
#include <unordered_set>
#include "Edge.h"
//#include "SVFIR/SVFStatements.h"
//#include "SVFIR/SVFVariables.h"

class Node {
public:
    // IMPORTANT: id must own its storage.
    // The DOT parser (graphviz) returns pointers that become invalid after agclose(),
    // so we must store a copy here.
    std::string id; // 节点的唯一标识
    std::string type; // 节点类型
    std::unordered_map<std::string, std::string> properties; // 节点属性
    std::string text; // 节点文本
    std::unordered_set<Edge*> inEdges; //入边集合
    std::unordered_set<Edge*> outEdges; // 出边集合
    std::unordered_set<Edge*> outCFGEdges;
    std::unordered_set<Edge*> inCFGEdges;
    std::unordered_set<Edge*> argumentEdges;
    std::unordered_set<Edge*> conditionEdges;

    Node(std::string id, std::string type_upper, std::unordered_map<std::string, std::string> properties){
        this->id = std::move(id);
        this->properties = properties;
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

    const char * getId() const {
        return id.c_str();
    }

    const std::string& getIdString() const {
        return id;
    }

    std::string getText(){
        return text;
    }

    void setText(std::string text){
        this->text = text;
    }

    virtual ~Node() {}

    // 获取节点类型
    std::string getType(){
        // 把type转为首字母大写的小写字符串
        return type;
    }

    std::string getCode() const {
        auto it = properties.find("CODE");
        if (it != properties.end()) {
            return it->second;
        }
        return "";
    }

    // 获取节点行
    int getLineNumber() const {
        auto it = properties.find("LINE_NUMBER");
        if (it != properties.end()) {
            if(it->second == ""){
                return -1;
            }
            return std::stoi(it->second);
        }
        return -1;
    }

    int getLineNumberEnd() const {
        auto it = properties.find("LINE_NUMBER_END");
        if (it != properties.end()) {
            return std::stoi(it->second);
        }
        return -1;
    }

    std::string getFileName() const {
        auto it = properties.find("FILENAME");
        if (it != properties.end()) {
            return it->second;
        }
        return "";
    }

    // 获取节点列
    int getColumnNumber() const {
        auto it = properties.find("COLUMN_NUMBER");
        if (it != properties.end()) {
            return std::stoi(it->second);
        }
        return -1;
    }

    // 获取name
    std::string getName() const {
        auto it = properties.find("NAME");
        if (it != properties.end()) {
            return it->second;
        }
        return "";
    }

    // 获取ArgumentIndex
    int getArgumentIndex() const {
        auto it = properties.find("ARGUMENT_INDEX");
        if (it != properties.end()) {
            return std::stoi(it->second);
        }
        return -1;
    }

    Node* getArgument(int index) const {
        for(Edge* argumentEdge : this->argumentEdges){
            if(argumentEdge->getToNode()->getArgumentIndex() == index){
                return argumentEdge->getToNode();
            }
        }
        return nullptr;
    }

    std::string getMethodFullName() const {
        auto it = properties.find("METHOD_FULL_NAME");
        if (it != properties.end()) {
            std::string funcName = it->second;
            // 查找冒号的位置
            size_t colonPos = funcName.find(':');
            if (colonPos == std::string::npos) {
                // 如果没有冒号，返回整个字符串
                return funcName;
            }
            // 提取冒号前的部分（函数名）
            return funcName.substr(0, colonPos);
        }
        return "";
    }

    std::string getProperty(const std::string& key) const {
        auto it = properties.find(key);
        if (it != properties.end()) {
            return it->second;
        }
        return "";
    }

    

};

// 预定义一些Node子类
class BlockNode : public Node {
public:
    using Node::Node;
};

class CallNode : public Node {
public:
    using Node::Node;
};


class MethodNode : public Node {
public:
    using Node::Node;
};


class ControlStructureNode : public Node {
public:
    using Node::Node;

    BlockNode* getBlock() {
        if(this->properties.find("CONTROL_STRUCTURE_TYPE") != this->properties.end() && this->properties["CONTROL_STRUCTURE_TYPE"] == "FOR"){
            for(Edge* edge : outEdges){
                if(edge->getToNode()->getType() == "Block" && edge->getToNode()->getArgumentIndex() == 4){
                    return (BlockNode*)edge->getToNode();
                }
            }
        }
        for(Edge* edge : outEdges){
            if(edge->getToNode()->getType() == "Block" && edge->getToNode()->getArgumentIndex() == -1){
                return (BlockNode*)edge->getToNode();
            }
        }
        
        return nullptr;
    }
};


#endif // NODE_H
