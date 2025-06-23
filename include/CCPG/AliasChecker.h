#ifndef ALIASCHECKER_H
#define ALIASCHECKER_H

#include "CCPGNode.h"

using namespace ccpg;
using namespace SVF;

class AliasChecker {
private:
    static AliasChecker * instance;
    AliasChecker() {}

public:
    // 删除复制构造函数和赋值运算符
    AliasChecker(const AliasChecker&) = delete;
    AliasChecker& operator=(const AliasChecker&) = delete;

    // 获取唯一实例的静态方法
    static AliasChecker* getInstance() {
        if (instance == nullptr) {
            instance = new AliasChecker();
        }
        return instance;
    }

    const SVF::SVFFunction * getSVFFunction(ccpg::Function * function);
    Node * findMethodBySVFFunction(const SVFFunction * svfFunction) const;

    bool isThreadAlias(CCPGNode * node1, CCPGNode * node2);
    bool isLockAlias(CCPGNode * node1, CCPGNode * node2);

    bool isSharedAccess(const LoadStmt * l);
    bool isSharedAccess(const StoreStmt * s);
    bool isSharedVar(const SVFVar * var);

    bool isStmtAlias(const SVFStmt * stmt1, const SVFStmt * stmt2);
    bool isUseAndFreeAlias(const CallICFGNode* node, const SVFStmt * stmt2);
    bool isFreeAndFreeAlias(const CallICFGNode* node1, const CallICFGNode* node2);
    bool areSameField(const SVFStmt * stmt1, const CCPGNodeSet & nodes1, const SVFStmt * stmt2, const CCPGNodeSet & nodes2);
    bool areCallsSame(const CallICFGNode* svfNode, Node* joernNode);


};

#endif // ALIASCHECKER_H