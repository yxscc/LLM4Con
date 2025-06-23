#include <vector>
#include <string>

class CCPGNode;

class Context {
public:
    Context() {}
    ~Context() {}

    const std::vector<CCPGNode*>& getCallStack() const { return callStack; }
    void push(CCPGNode* node) { callStack.push_back(node); }
    void pop() { callStack.pop_back(); }
    int size() const { return callStack.size(); }

    Context(const Context& other) {
        callStack = other.callStack;  // 确保执行深拷贝
    }

    CCPGNode* top() const { return callStack.back(); }

    Context* extend(CCPGNode* node) {
        Context* newContext = new Context();
        newContext->callStack = callStack;
        newContext->push(node);
        return newContext;
    }

    Context* popNode() {
        Context* newContext = new Context();
        newContext->callStack = callStack;
        newContext->pop();
        return newContext;
    }

    bool contains(CCPGNode* node) {
        for(CCPGNode* n : callStack) {
            if(n == node) {
                return true;
            }
        }
        return false;
    }

    //重写相等运算符
    bool operator==(const Context& other) const {
        if(callStack.size() != other.callStack.size()) {
            return false;
        }
        for(int i = 0; i < callStack.size(); i++) {
            if(callStack[i] != other.callStack[i]) {
                return false;
            }
        }
        return true;
    }

    bool equal(Context* other) {
        std::vector<CCPGNode*> callStack2 = other->getCallStack();
        if(callStack.size() != callStack2.size()) {
            return false;
        }
        for(int i = 0; i < callStack.size(); i++) {
            if(callStack[i] != callStack2[i]) {
                return false;
            }
        }
        return true;
    }

    std::string toString() const;

    /*CCPGNode* getForkNodeByContext(CCPG * ccpg){
        for(auto it = callStack.rbegin(); it != callStack.rend(); it++) {
            if((*it)->getType() == ThreadAPIUtil::TYPE::FORK) {
                return *it;
            }
        }
        std::unordered_set<CCPGNode*> nodes = ccpg->getMain();
        return *(nodes.begin());
    }*/


private:
    std::vector<CCPGNode*> callStack;
};