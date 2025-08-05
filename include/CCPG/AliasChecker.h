// include/CCPG/AliasChecker.h
#ifndef ALIASCHECKER_H
#define ALIASCHECKER_H

#include "CCPGNode.h"

namespace llvm {
class Value;
class Function;
class Instruction;
}

class AliasChecker {
private:
    static AliasChecker* instance;
    AliasChecker() {}

public:
    AliasChecker(const AliasChecker&) = delete;
    AliasChecker& operator=(const AliasChecker&) = delete;

    static AliasChecker* getInstance(); // 实现会移动到cpp

    // --- 方法签名修改 ---
    bool isThreadAlias(CCPGNode *node1, CCPGNode *node2);
    bool isLockAlias(CCPGNode *node1, CCPGNode *node2);
    bool isAlias(const llvm::Value* V1, const llvm::Value* V2);

    const llvm::Function * getLLVMFunction(ccpg::Function * function) const;
    bool areCallsSame(const llvm::Instruction* llvmInst, Node* cpgNode) const;

    // isSharedAccess 和 isSharedVar 逻辑需要重新思考，暂时移除
    // bool isSharedAccess(const llvm::Value* V); 
};

#endif // ALIASCHECKER_H