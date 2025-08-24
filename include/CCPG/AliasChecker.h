// include/CCPG/AliasChecker.h
#ifndef ALIASCHECKER_H
#define ALIASCHECKER_H

#include "CCPGNode.h"
#include "LLMUtil/ConcurrencyContract.h"

namespace llvm {
class Value;
class Function;
class Instruction;
}

namespace LLM{
class ConcurrencyContract;
}

struct MemoryAccess {
    const llvm::Value* pointerOperand; // 指向被访问内存的指针
    bool isWrite;                    // 这次访问是读还是写
    NodeLoc location;                  // 访问发生的源代码位置
    Context context;                   // 访问发生时的执行上下文
    const llvm::Instruction* instruction;
};

using MemoryAccessMap = std::unordered_map<const llvm::Value*, std::vector<MemoryAccess>>;

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
    const llvm::Value* getLLVMThreadValue(CCPGNode * node);
    bool isLockAlias(CCPGNode *node1, CCPGNode *node2);
    const llvm::Value* getLLVMLockValue(CCPGNode * node);
    bool isAlias(const llvm::Value* V1, const llvm::Value* V2);

    const llvm::Function * getLLVMFunction(ccpg::Function * function) const;
    bool areCallsSame(const llvm::Instruction* llvmInst, Node* cpgNode) const;

    std::vector<const llvm::Value*> getPointerOperandsFromLocation(const NodeLoc& loc);
    bool isAliasOfContractVariable(const llvm::Value* V, const LLM::ConcurrencyContract::SharedVariable& contractVar, int functionId);
    std::vector<MemoryAccess> getMemoryAccessesFromLocation(const NodeLoc& loc, const Context& ctx);
    bool isThreadSafeStaticInitializationVariable(const llvm::Value* val) const;
    bool isCompilerGeneratedSafeInit(const MemoryAccess& access) const;

};

#endif // ALIASCHECKER_H