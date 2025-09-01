// src/CCPG/AliasChecker.cpp
#include "phasar.h"
#include "CCPG/AliasChecker.h"
#include "PhasarUtil/AnalysisManager.h"
#include "PhasarUtil/LLVMAnalyzer.h"
#include "CCPG/ThreadCreationTree.h"
#include "llvm/Demangle/Demangle.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/DerivedTypes.h"

using namespace psr;

// 初始化静态实例
AliasChecker* AliasChecker::instance = nullptr;

AliasChecker* AliasChecker::getInstance() {
    if (instance == nullptr) {
        instance = new AliasChecker();
    }
    return instance;
}

// --- 重构后的方法实现 ---

bool AliasChecker::isAlias(const llvm::Value* V1, const llvm::Value* V2) {
    if (!V1 || !V2) return false;
    PointerAnalysisInterface* analyzer = AnalysisManager::getInstance()->getPointerAnalyzer();
    if (!analyzer) return false; // 如果分析器未初始化
    return analyzer->areAliases(V1, V2);
}

bool AliasChecker::isThreadAlias(CCPGNode * node1, CCPGNode * node2) {
    const llvm::Value* v1 = getLLVMThreadValue(node1);
    const llvm::Value* v2 = getLLVMThreadValue(node2);

    return isAlias(v1, v2);
}

const llvm::Value* AliasChecker::getLLVMThreadValue(CCPGNode* node) {
    if (!node || !node->isCallSite()) {
        return nullptr;
    }

    const llvm::CallInst* callInst = node->getLLVMCallInst();
    if (!callInst) {
        return nullptr;
    }

    if (node->getType() == ThreadAPIUtil::TYPE::FORK) {
        return callInst->getArgOperand(0);
    } 
    else if (node->getType() == ThreadAPIUtil::TYPE::JOIN) {
        const llvm::Value* joinArg = callInst->getArgOperand(0);

        if (const llvm::LoadInst* loadInst = llvm::dyn_cast<llvm::LoadInst>(joinArg)) {
            return loadInst->getPointerOperand();
        }
        return joinArg;
    }

    return nullptr;
}

bool AliasChecker::isLockAlias(CCPGNode * node1, CCPGNode * node2) {
    assert(node1->getType() == ThreadAPIUtil::TYPE::ACQUIRE || node1->getType() == ThreadAPIUtil::TYPE::RELEASE);
    assert(node2->getType() == ThreadAPIUtil::TYPE::ACQUIRE || node2->getType() == ThreadAPIUtil::TYPE::RELEASE);
    const llvm::CallInst* callInst1 = node1->getLLVMCallInst();
    const llvm::CallInst* callInst2 = node2->getLLVMCallInst();

    if(!callInst1 || !callInst2){
        return false;
    }

    const llvm::Value* lock1 = callInst1->getArgOperand(0);
    const llvm::Value* lock2 = callInst2->getArgOperand(0);

    if(!lock1 || !lock2){
        return false;
    }

    return isAlias(lock1, lock2);
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

bool AliasChecker::isThreadSafeStaticInitializationVariable(const llvm::Value* val) const {
    if (val && val->hasName()) {
        llvm::StringRef name = val->getName();
        if (name.startswith("_ZGV")) {
            return true;
        }
    }
    return false;
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

std::vector<const llvm::Value*> AliasChecker::getPointerOperandsFromLocation(const NodeLoc& loc) {
    std::vector<const llvm::Value*> operands;
    auto* pa = static_cast<PhasarPointerAnalysis*>(AnalysisManager::getInstance()->getPointerAnalyzer());
    if (!pa) return operands;

    // 获取所有 Load 指令的操作数
    auto loads = pa->getLoadInstsByLoc(loc);
    for (const auto* load_inst : loads) {
        operands.push_back(load_inst->getPointerOperand());
    }

    // 获取所有 Store 指令的操作数
    auto stores = pa->getStoreInstsByLoc(loc);
    for (const auto* store_inst : stores) {
        operands.push_back(store_inst->getPointerOperand());
    }

    return operands;
}

bool AliasChecker::isAliasOfContractVariable(const llvm::Value* V, const LLM::ConcurrencyContract::SharedVariable& contractVar, int functionId) {
    if (!V) return false;

    auto* analyzer = AnalysisManager::getInstance()->getPointerAnalyzer();
    if (!analyzer) return false;

    // 1. 根据 functionId 获取函数名
    CCPG* ccpg = ThreadCreationTree::getInstance()->getCCPG();
    if (!ccpg) return false;
    ccpg::Function* func = ccpg->getFunctionById(functionId);
    if (!func) return false; // 如果找不到函数，则无法继续
    std::string funcName = func->getFuncNode()->getCPGNode()->getName();

    // 2. 根据函数名和变量名获取合约变量的 llvm::Value
    const llvm::Value* contractValue = analyzer->getValueByName(funcName, contractVar.variableName);

    // 2.1 如果在函数内找不到，尝试将其作为全局变量查找
    if (!contractValue) {
        contractValue = analyzer->getValueByName("", contractVar.variableName);
        if (!contractValue) {
            // 如果全局也找不到，说明这个变量可能在IR中被优化掉了，或者名字不匹配
            return false;
        }
    }

    // 3. 比较别名
    return isAlias(V, contractValue);
}

std::vector<MemoryAccess> AliasChecker::getMemoryAccessesFromLocation(const NodeLoc& loc, const Context& ctx) {
    std::vector<MemoryAccess> accesses;
    auto* pa = static_cast<PhasarPointerAnalysis*>(AnalysisManager::getInstance()->getPointerAnalyzer());
    if (!pa) return accesses;

    // 1. 从 NodeLoc 获取期望的函数上下文
    ccpg::Function* expected_cpg_func = loc.getFunction();
    if (!expected_cpg_func) {
        // 如果 NodeLoc 没有关联的函数，则无法进行匹配
        return accesses;
    }
    const llvm::Function* expected_llvm_func = expected_cpg_func->getLLVMFunction();
    if (!expected_llvm_func) {
        std::cerr << "[Warning] Could not find LLVM function for CPG function: " 
                  << expected_cpg_func->getFuncNode()->getCPGNode()->getName() << std::endl;
        // 如果无法映射到LLVM函数，为避免漏报，可以选择不进行过滤，但这里我们选择严格匹配
        return accesses;
    }

    // 2. 获取所有在该物理位置的 Load 指令，然后进行过滤
    auto loads = pa->getLoadInstsByLoc(loc);
    for (const auto* load_inst : loads) {
        // **关键检查**: 只有当指令的父函数与期望的函数相同时，才将其视为有效的内存访问
        if (load_inst->getFunction() == expected_llvm_func) {
            accesses.push_back({load_inst->getPointerOperand(), false, loc, ctx, load_inst});
        }
    }

    // 3. 获取所有在该物理位置的 Store 指令，然后进行过滤
    auto stores = pa->getStoreInstsByLoc(loc);
    for (const auto* store_inst : stores) {
        // **关键检查**: 只有当指令的父函数与期望的函数相同时，才将其视为有效的内存访问
        if (store_inst->getFunction() == expected_llvm_func) {
            accesses.push_back({store_inst->getPointerOperand(), true, loc, ctx, store_inst});
        }
    }

    // (可选) 增强调试输出，以观察过滤效果
    /*if (!loads.empty() || !stores.empty()) {
        std::cout << "[DEBUG PRINT] GetAccess at " << loc.toString() 
                  << ": Found " << loads.size() << " loads and " 
                  << stores.size() << " stores. After function context filtering, "
                  << accesses.size() << " accesses remain." << std::endl;
    }*/

    return accesses;
}

bool AliasChecker::isCompilerGeneratedSafeInit(const MemoryAccess& access) const {
    // 规则 1: 此过滤器只适用于写操作
    if (!access.isWrite || !access.instruction) {
        return false;
    }

    const auto* storeInst = llvm::dyn_cast<llvm::StoreInst>(access.instruction);
    if (!storeInst) {
        return false;
    }

    // 规则 2: 被写入的指针必须是一个静态局部变量 (_ZZ...)
    const auto* ptrOp = storeInst->getPointerOperand();
    if (!ptrOp->hasName() || !ptrOp->getName().startswith("_ZZ")) {
        return false;
    }
    
    // 规则 3: 将复杂的控制流检查委托给Phasar分析器
    auto* pa = static_cast<PhasarPointerAnalysis*>(AnalysisManager::getInstance()->getPointerAnalyzer());
    if (!pa) {
        return false;
    }

    return pa->isGuardedByStaticInitializer(storeInst);
}

/**
 * @brief 根据调用点和一个期望的类型名，获取函数参数对应的llvm::Value。
 *
 * 这个函数会检查调用点的所有参数，如果某个参数的指针类型指向的结构体
 * 名称与期望的 object_type_name 匹配，则返回该参数的Value。
 * 如果 object_type_name 为空，则返回第一个指针类型的参数。
 *
 * @param call_site 代表函数调用的CCPGNode。
 * @param object_type_name 期望的共享对象的C++结构体名称。
 * @return 匹配到的参数的llvm::Value*，如果没有找到则返回nullptr。
 */
const llvm::Value* AliasChecker::getLLVMValueForArgument(CCPGNode* call_site, const std::string& object_type_name) {
    if (!call_site || !call_site->isCallSite() || !call_site->getLLVMCallInst()) {
        return nullptr;
    }

    const llvm::CallInst* callInst = call_site->getLLVMCallInst();

    for (unsigned i = 0; i < callInst->arg_size(); ++i) {
        const llvm::Value* arg = callInst->getArgOperand(i);

        // 确保参数是一个指针类型
        if (arg && arg->getType()->isPointerTy()) {
            // 如果不需要特定的类型名，则返回第一个找到的指针参数
            if (object_type_name.empty()) {
                return arg;
            }

            // 追溯指针的源头，剥去所有类型转换（cast）
            const llvm::Value* baseValue = arg->stripPointerCasts();
            llvm::Type* allocatedType = nullptr;

            // 检查源头是否是一个内存分配指令
            if (const auto* alloca = llvm::dyn_cast<llvm::AllocaInst>(baseValue)) {
                allocatedType = alloca->getAllocatedType();
            } 
            // 检查源头是否是一个全局变量
            else if (const auto* global = llvm::dyn_cast<llvm::GlobalVariable>(baseValue)) {
                allocatedType = global->getValueType();
            }
            // 检查源头是否是另一个函数的参数
            else if (const auto* funcArg = llvm::dyn_cast<llvm::Argument>(baseValue)) {
                 if (funcArg->getType()->isPointerTy()){
                     allocatedType = funcArg->getParamByValType();
                 }
            }


            // 如果我们成功找到了源头的类型
            if (allocatedType) {
                // 安全地将其转换为结构体类型并检查名称
                if (llvm::StructType* st = llvm::dyn_cast<llvm::StructType>(allocatedType)) {
                    if (st && !st->isLiteral() && st->hasName() && st->getStructName().contains(object_type_name)) {
                        // 匹配成功！返回原始的、未被剥离的参数
                        return arg;
                    }
                }
            }
        }
    }

    return nullptr;
}