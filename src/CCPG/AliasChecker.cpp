// src/CCPG/AliasChecker.cpp
#include "phasar.h"
#include "CCPG/AliasChecker.h"
#include "PhasarUtil/AnalysisManager.h"
#include "PhasarUtil/LLVMAnalyzer.h"

using namespace psr;

// 初始化静态实例
AliasChecker* AliasChecker::instance = nullptr;

AliasChecker* AliasChecker::getInstance() {
    if (instance == nullptr) {
        instance = new AliasChecker();
    }
    return instance;
}


// --- 辅助函数：从CCPGNode获取用于别名分析的 llvm::Value* ---
const llvm::Value* getAliasRelevantValue(CCPGNode* node) {
    if (!node || !node->getCPGNode()) return nullptr;

    Node* cpgNode = node->getCPGNode();
    
    // 对于线程fork/join，关键是第一个参数
    if (node->getType() == ThreadAPIUtil::TYPE::FORK || node->getType() == ThreadAPIUtil::TYPE::JOIN) {
        Node* arg = cpgNode->getArgument(1); // 假设线程句柄是第一个参数
        if(arg) {
            // 这里需要一个机制将CPG节点映射回LLVM IR的Value
            // 这是一个复杂的任务，暂时我们假设可以获取到
            // TODO: 实现从 CPG Node 到 llvm::Value* 的映射
        }
    }
    
    // 对于锁，关键也是第一个参数
    if (node->getType() == ThreadAPIUtil::TYPE::ACQUIRE || node->getType() == ThreadAPIUtil::TYPE::RELEASE) {
        Node* arg = cpgNode->getArgument(1); // 假设锁变量是第一个参数
         if(arg) {
            // TODO: 实现从 CPG Node 到 llvm::Value* 的映射
        }
    }
    
    return nullptr; // 暂时返回空
}


// --- 重构后的方法实现 ---

bool AliasChecker::isAlias(const llvm::Value* V1, const llvm::Value* V2) {
    if (!V1 || !V2) return false;
    PointerAnalysisInterface* analyzer = AnalysisManager::getInstance()->getPointerAnalyzer();
    if (!analyzer) return false; // 如果分析器未初始化
    return analyzer->areAliases(V1, V2);
}

bool AliasChecker::isThreadAlias(CCPGNode * node1, CCPGNode * node2) {
    // TODO: 实现 getAliasRelevantValue
    const llvm::Value* v1 = getAliasRelevantValue(node1);
    const llvm::Value* v2 = getAliasRelevantValue(node2);
    
    return isAlias(v1, v2);
}

bool AliasChecker::isLockAlias(CCPGNode * node1, CCPGNode * node2) {
    const llvm::Value* v1 = getAliasRelevantValue(node1);
    const llvm::Value* v2 = getAliasRelevantValue(node2);

    return isAlias(v1, v2);
}

const llvm::Function* AliasChecker::getLLVMFunction(ccpg::Function * function) const {
    if(function->getLLVMFunction() != nullptr){
        return function->getLLVMFunction();
    }

    PointerAnalysisInterface* analyzer = AnalysisManager::getInstance()->getPointerAnalyzer();
    if (!analyzer) {
        return nullptr;
    }
    std::vector<const llvm::Function *> llvmFunctions = analyzer->getAllLLVMFunctions();

    Node* cpgNode = function->getFuncNode()->getCPGNode();
    std::string cpgFuncName = cpgNode->getName();
    std::string cpgFileName = cpgNode->getFileName();
    int cpgLineNum = cpgNode->getLineNumber();

    for (const llvm::Function* llvmFunc : llvmFunctions) {
        if (!llvmFunc || llvmFunc->isDeclaration()) {
            continue; // 跳过没有函数体的声明
        }

        std::string llvmFuncNameDemangled = LLVMAnalyzer::getInstance()->demangle(llvmFunc->getName().str().c_str());
        if (cpgFuncName == llvmFuncNameDemangled) {

            // b. 如果名字相同，再比较源文件位置作为最终确认
            if (auto *SP = llvmFunc->getSubprogram()) {
                unsigned llvmLineNum = SP->getLine();

                if (cpgLineNum == llvmLineNum) {
                    function->setLLVMFunction(llvmFunc);
                    return llvmFunc;
                }
            } else {
                 function->setLLVMFunction(llvmFunc);
                 return llvmFunc;
            }
        }
    }
    return nullptr;
}

bool AliasChecker::areCallsSame(const llvm::Instruction* llvmInst, Node* cpgNode) const {
    // 首先，确保LLVM指令是一个调用指令
    const llvm::CallInst* callInst = llvm::dyn_cast<llvm::CallInst>(llvmInst);
    if (!callInst) {
        return false;
    }

    // 获取被调用的函数
    const llvm::Function* calledFunc = callInst->getCalledFunction();
    if (!calledFunc) {
        // 间接调用（函数指针），目前难以精确匹配，可以暂时返回true或false
        // 或者比较调用的参数数量等启发式信息
        return false; 
    }

    // 获取函数名并进行demangle
    std::string llvmFuncName = LLVMAnalyzer::getInstance()->demangle(calledFunc->getName().str().c_str());
    std::string cpgFuncName = cpgNode->getName();

    // 比较函数名。可以根据需要使用更宽松的比较（如 find）
    return llvmFuncName == cpgFuncName;
}