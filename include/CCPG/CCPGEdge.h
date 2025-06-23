#include "CCPG/CCPGNode.h"

class CCPGEdge {
public:
    enum EdgeType 
        {
            ORDER,
            HB,
            CALL,
        };
    
    CCPGEdge(CCPGNode *src, CCPGNode *dst) : src(src), dst(dst) {}
    ~CCPGEdge() {}

    CCPGNode *getSrc() const { return src; }
    CCPGNode *getDst() const { return dst; }

    EdgeType getType() const { return type; }
    void setType(EdgeType type) { this->type = type; }

    std::string getTypeString() {
        switch(type) {
            case EdgeType::ORDER:
                return "ORDER";
            case EdgeType::HB:
                return "HB";
            case EdgeType::CALL:
                return "CALL";
            default:
                return "UNKNOWN";
        }
    }
private:
    CCPGNode* src;
    CCPGNode* dst;
    EdgeType type = EdgeType::ORDER;
};