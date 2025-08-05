// src/PhasarUtil/PhasarPointerAnalysis.cpp

#include "PhasarUtil/PhasarPointerAnalysis.h"

#include "phasar.h"

using namespace psr;

PhasarPointerAnalysis::PhasarPointerAnalysis(const std::string &bitcodeFilePath) {
    DB = std::make_unique<psr::LLVMProjectIRDB>(bitcodeFilePath);
    TH = std::make_unique<psr::DIBasedTypeHierarchy>(*DB);
    
    ICFG = std::make_unique<psr::LLVMBasedICFG>(DB.get(), psr::CallGraphAnalysisType::OTF, std::vector<std::string>{});
    
    PTA = std::make_unique<psr::LLVMAliasSet>(DB.get());

    if (DB) {
        for (const llvm::Function *F : DB->getAllFunctions()) {
            if (F->isDeclaration()) continue;
            for (const auto &BB : *F) {
                for (const auto &I : BB) {
                    if (const auto &Loc = I.getDebugLoc()) {
                        std::string File = Loc->getFilename().str();
                        unsigned Line = Loc.getLine();
                        
                        if (!File.empty() && Line > 0) {
                            NodeLoc NLoc(File, Line, nullptr);

                            if (const auto *CI = llvm::dyn_cast<llvm::CallInst>(&I)) {
                                LocToCallInstsMap[NLoc].push_back(CI);
                            } else if (const auto *LI = llvm::dyn_cast<llvm::LoadInst>(&I)) {
                                LocToLoadInstsMap[NLoc].push_back(LI);
                            } else if (const auto *SI = llvm::dyn_cast<llvm::StoreInst>(&I)) {
                                LocToStoreInstsMap[NLoc].push_back(SI);
                            }
                        }
                    }
                }
            }
        }
    }
}

std::vector<const llvm::CallInst *> PhasarPointerAnalysis::getCallInstsByLoc(const NodeLoc &Loc) const {
    NodeLoc Key(Loc.getFileName(), Loc.getLineNumber(), nullptr);
    auto It = LocToCallInstsMap.find(Key);
    return (It != LocToCallInstsMap.end()) ? It->second : std::vector<const llvm::CallInst *>();
}

std::vector<const llvm::LoadInst *> PhasarPointerAnalysis::getLoadInstsByLoc(const NodeLoc &Loc) const {
    NodeLoc Key(Loc.getFileName(), Loc.getLineNumber(), nullptr);
    auto It = LocToLoadInstsMap.find(Key);
    return (It != LocToLoadInstsMap.end()) ? It->second : std::vector<const llvm::LoadInst *>();
}

std::vector<const llvm::StoreInst *> PhasarPointerAnalysis::getStoreInstsByLoc(const NodeLoc &Loc) const {
    NodeLoc Key(Loc.getFileName(), Loc.getLineNumber(), nullptr);
    auto It = LocToStoreInstsMap.find(Key);
    return (It != LocToStoreInstsMap.end()) ? It->second : std::vector<const llvm::StoreInst *>();
}

// Destructor needs to be defined in the .cpp file where the types are complete
PhasarPointerAnalysis::~PhasarPointerAnalysis() = default;

bool PhasarPointerAnalysis::areAliases(const llvm::Value *V1, const llvm::Value *V2) {
    if (!V1 || !V2) {
        return false;
    }
    // Use the alias() method from LLVMAliasSet, which returns an AliasResult enum
    return PTA->alias(V1, V2) != psr::AliasResult::NoAlias;
}

std::set<const llvm::Value *> PhasarPointerAnalysis::getPointsToSet(const llvm::Value *Ptr) {
    // IMPORTANT: The LLVMAliasSet analysis does not provide points-to sets, only alias results.
    // This function is kept to satisfy the interface, but it will not return useful data with this backend.
    // For true points-to sets, a different Phasar analysis would be needed.
    return {};
}

const llvm::Value* PhasarPointerAnalysis::getValueByName(const std::string &funcName, const std::string &varName) {
    const llvm::Function* F = DB->getFunction(funcName);
    if (!F) {
        return nullptr;
    }
    for (const auto &BB : *F) {
        for (const auto &I : BB) {
            if (I.hasName() && I.getName() == varName) {
                return &I;
            }
        }
    }
    for (const auto &Arg : F->args()) {
        if (Arg.hasName() && Arg.getName() == varName) {
            return &Arg;
        }
    }
    return nullptr;
}

std::vector<EntryPointInfo> PhasarPointerAnalysis::getPotentialEntryPoints() {
    std::vector<EntryPointInfo> potentialEntries;
    if (!ICFG) {
        // 如果ICFG本身是空的，直接返回
        return potentialEntries;
    }

    // 1. 直接从ICFG获取所有函数。您是对的，这个API在ICFG上。
    auto Functions = ICFG->getAllFunctions();

    // 2. 遍历ICFG中的所有函数
    for (const llvm::Function *F : Functions) {
        
        // 3. 应用更精确的入口点过滤条件
        // 条件a: 函数必须有定义，而不是只有声明
        if (F->isDeclaration()) {
            continue;
        }
        
        // 条件b: 函数必须是外部可见的
        if (!F->hasExternalLinkage()) {
            continue;
        }

        // 条件c (关键): 函数在程序内部没有调用者
        // 我们通过ICFG的getCallersOf接口来查询调用图
        if (!ICFG->getCallersOf(F).empty()) {
            continue;
        }

        // 4. 提取信息 (这部分逻辑不变)
        EntryPointInfo info;
        info.functionName = F->getName().str();

        if (auto *SP = F->getSubprogram()) {
            info.fileName = SP->getFilename().str();
            info.lineNumber = SP->getLine();
        } else {
            info.fileName = "N/A";
            info.lineNumber = 0;
        }

        potentialEntries.push_back(info);
    }

    return potentialEntries;
}

std::vector<const llvm::Function *> PhasarPointerAnalysis::getAllLLVMFunctions() const {
    std::vector<const llvm::Function *> AllFunctions;
    if (DB) {
        // LLVMProjectIRDB 通过基类继承了 getAllFunctions() 方法
        for (const llvm::Function *F : DB->getAllFunctions()) {
            AllFunctions.push_back(F);
        }
    }
    return AllFunctions;
}

std::vector<const llvm::Function*> PhasarPointerAnalysis::getCalleesOfCallAt(const llvm::Instruction* callInst) const {
    if (!ICFG || !callInst) {
        return {};
    }
    // 内部实现细节：调用ICFG的具体方法
    auto calleeRange = ICFG->getCalleesOfCallAt(callInst);
    // 将Phasar返回的范围（range）转换为一个标准的vector
    return {calleeRange.begin(), calleeRange.end()};
}