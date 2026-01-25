// src/PhasarUtil/PhasarPointerAnalysis.cpp

#include "PhasarUtil/PhasarPointerAnalysis.h"

#include "phasar.h"
#include <algorithm>

using namespace psr;

// Priority levels for kernel entry points (lower = higher priority)
static int getKernelEntryPriority(const std::string& name) {
    // Priority 1: System calls (highest priority)
    if (name.find("sys_") == 0 || name.find("SyS_") == 0 || 
        name.find("SYSC_") == 0 || name.find("__x64_sys_") == 0 ||
        name.find("__ia32_sys_") == 0 || name.find("compat_sys_") == 0) {
        return 1;
    }
    // Priority 2: Module init/exit (but not generic helpers)
    if (name == "init_module" || name == "cleanup_module") {
        return 2;
    }
    // Priority 3: Subsystem init functions (e.g., blk_dev_init, cfq_init)
    if ((name.find("_init") == name.length() - 5 || name.find("_exit") == name.length() - 5) &&
        name.find("list_") == std::string::npos && name.find("__init") == std::string::npos &&
        name.find("kref_") == std::string::npos && name.find("hlist_") == std::string::npos) {
        return 3;
    }
    // Priority 4: Work/thread handlers
    if ((name.find("_work") != std::string::npos || name.find("_handler") != std::string::npos ||
         name.find("_callback") != std::string::npos || name.find("_thread") != std::string::npos) &&
        name.find("__init") == std::string::npos && name.find("list_") == std::string::npos) {
        return 4;
    }
    // Not a kernel entry
    return 0;
}

// Helper function to check if a function name matches Linux kernel entry patterns
static bool isKernelEntryFunction(const std::string& name) {
    return getKernelEntryPriority(name) > 0;
}

PhasarPointerAnalysis::PhasarPointerAnalysis(const std::string &bitcodeFilePath,
                                             const std::vector<std::string>& userEntryPoints) {
    DB = std::make_unique<psr::LLVMProjectIRDB>(bitcodeFilePath);

    // Try to find entry points: user-specified > 'main' > auto-detected kernel entries
    std::vector<std::string> entryPoints;
    
    // Priority 1: User-specified entry points (from config file)
    if (!userEntryPoints.empty()) {
        std::cout << "Using " << userEntryPoints.size() << " user-specified entry point(s):" << std::endl;
        for (const auto& ep : userEntryPoints) {
            // Verify the function exists in the bitcode
            if (DB && DB->getFunctionDefinition(ep)) {
                entryPoints.push_back(ep);
                std::cout << "  - " << ep << " (found)" << std::endl;
            } else {
                std::cerr << "  - " << ep << " (NOT FOUND in bitcode, skipping)" << std::endl;
            }
        }
    }
    // Priority 2: Check for 'main'
    else if (DB && DB->getFunctionDefinition("main")) {
        entryPoints.push_back("main");
        std::cout << "Found 'main' function as entry point." << std::endl;
    } else {
        // No 'main' found - look for Linux kernel entry points
        std::cout << "No 'main' function found. Searching for kernel entry points..." << std::endl;
        
        if (DB) {
            // Collect entries with their priorities
            std::vector<std::pair<int, std::string>> prioritizedEntries;
            for (const llvm::Function *F : DB->getAllFunctions()) {
                if (F->isDeclaration()) continue;
                std::string funcName = F->getName().str();
                int priority = getKernelEntryPriority(funcName);
                if (priority > 0) {
                    prioritizedEntries.push_back({priority, funcName});
                }
            }
            
            // Sort by priority (lower number = higher priority)
            std::sort(prioritizedEntries.begin(), prioritizedEntries.end(),
                      [](const auto& a, const auto& b) { return a.first < b.first; });
            
            for (const auto& pe : prioritizedEntries) {
                entryPoints.push_back(pe.second);
            }
        }
        
        if (entryPoints.empty()) {
            std::cerr << "Error: LLVMProjectIRDB failed to load the bitcode file or could not find any entry points in: " 
                      << bitcodeFilePath << std::endl;
            TH = std::make_unique<psr::DIBasedTypeHierarchy>(*DB);
            ICFG = std::make_unique<psr::LLVMBasedICFG>(DB.get(), psr::CallGraphAnalysisType::CHA, std::vector<std::string>{});
            PTA = std::make_unique<psr::LLVMAliasSet>(DB.get());
            return;
        }
        
        std::cout << "Found " << entryPoints.size() << " kernel entry point(s), top 10:" << std::endl;
        for (size_t i = 0; i < std::min(entryPoints.size(), size_t(10)); i++) {
            std::cout << "  - " << entryPoints[i] << std::endl;
        }
        if (entryPoints.size() > 10) {
            std::cout << "  ... and " << (entryPoints.size() - 10) << " more" << std::endl;
        }
    }

    std::cout << "[Phasar] Building DIBasedTypeHierarchy..." << std::endl;
    std::cout.flush();
    TH = std::make_unique<psr::DIBasedTypeHierarchy>(*DB);
    std::cout << "[Phasar] DIBasedTypeHierarchy done." << std::endl;
    
    std::cout << "[Phasar] Building LLVMBasedICFG (CHA)..." << std::endl;
    std::cout.flush();
    ICFG = std::make_unique<psr::LLVMBasedICFG>(DB.get(), psr::CallGraphAnalysisType::CHA, entryPoints);
    std::cout << "[Phasar] LLVMBasedICFG done." << std::endl;
    
    std::cout << "[Phasar] Building LLVMAliasSet..." << std::endl;
    std::cout.flush();
    PTA = std::make_unique<psr::LLVMAliasSet>(DB.get());
    std::cout << "[Phasar] LLVMAliasSet done." << std::endl;
    
    // Store discovered entry points for later use
    discoveredEntryPoints_ = entryPoints;

    std::cout << "[Phasar] Building location maps for all functions..." << std::endl;
    std::cout.flush();
    size_t funcCount = 0;
    size_t totalFuncs = 0;
    for (const llvm::Function *F : DB->getAllFunctions()) {
        if (!F->isDeclaration()) totalFuncs++;
    }
    
    for (const llvm::Function *F : DB->getAllFunctions()) {
        if (F->isDeclaration()) continue;
        funcCount++;
        if (funcCount % 1000 == 0 || funcCount == totalFuncs) {
            std::cout << "[Phasar] Processed " << funcCount << "/" << totalFuncs << " functions..." << std::endl;
            std::cout.flush();
        }
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
    std::cout << "[Phasar] Location maps built. Total functions: " << funcCount << std::endl;
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
    return potentialEntries;
}

EntryPointInfo PhasarPointerAnalysis::getMainFunction() const {
    EntryPointInfo mainInfo;
    
    // First try 'main'
    if (const llvm::Function *MainFunc = DB->getFunctionDefinition("main")) {
        mainInfo.functionName = MainFunc->getName().str();

        if (auto *SP = MainFunc->getSubprogram()) {
            mainInfo.fileName = SP->getFilename().str();
            mainInfo.lineNumber = SP->getLine();
        } else {
            mainInfo.fileName = "N/A";
            mainInfo.lineNumber = 0;
        }

        std::cout << "DEBUG: Found entry point -> " << mainInfo.toString() << std::endl;
        return mainInfo;
    }
    
    // No 'main' - try to return the first discovered kernel entry point
    if (!discoveredEntryPoints_.empty()) {
        const std::string& firstEntry = discoveredEntryPoints_[0];
        if (const llvm::Function *EntryFunc = DB->getFunctionDefinition(firstEntry)) {
            mainInfo.functionName = EntryFunc->getName().str();

            if (auto *SP = EntryFunc->getSubprogram()) {
                mainInfo.fileName = SP->getFilename().str();
                mainInfo.lineNumber = SP->getLine();
            } else {
                mainInfo.fileName = "N/A";
                mainInfo.lineNumber = 0;
            }

            std::cout << "DEBUG: Using kernel entry point -> " << mainInfo.toString() << std::endl;
            return mainInfo;
        }
    }
    
    std::cerr << "Warning: Could not find a 'main' function definition in the provided bitcode." << std::endl;
    return {};
}

std::vector<EntryPointInfo> PhasarPointerAnalysis::getAllEntryPointInfos() const {
    std::vector<EntryPointInfo> allEntries;
    
    for (const std::string& entryName : discoveredEntryPoints_) {
        if (const llvm::Function *EntryFunc = DB->getFunctionDefinition(entryName)) {
            EntryPointInfo info;
            info.functionName = EntryFunc->getName().str();
            
            if (auto *SP = EntryFunc->getSubprogram()) {
                info.fileName = SP->getFilename().str();
                info.lineNumber = SP->getLine();
            } else {
                info.fileName = "N/A";
                info.lineNumber = 0;
            }
            
            allEntries.push_back(info);
        }
    }
    
    std::cout << "getAllEntryPointInfos: Returning " << allEntries.size() << " entry points for kernel module analysis" << std::endl;
    return allEntries;
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

std::vector<const llvm::GlobalVariable*> PhasarPointerAnalysis::getAllGlobalVariables() const {
    std::vector<const llvm::GlobalVariable*> globalVars;
    if (DB) {
        for (const auto& Global : DB->getModule()->globals()) {
            globalVars.push_back(&Global);
        }
    }
    return globalVars;
}

bool PhasarPointerAnalysis::isGuardedByStaticInitializer(const llvm::StoreInst* storeInst) const {
    if (!storeInst) {
        return false;
    }

    // 1. 获取Store指令所在的BasicBlock
    const llvm::BasicBlock* currentBlock = storeInst->getParent();
    if (!currentBlock) {
        return false;
    }

    // 2. 获取Store指令的目标指针（即被写入的静态变量）
    const llvm::Value* storedPtr = storeInst->getPointerOperand();
    if (!storedPtr->hasName() || !storedPtr->getName().startswith("_ZZ")) {
        return false; // 我们只关心对静态局部变量的写操作
    }

    // 3. 构建对应的守卫变量名
    std::string varName = storedPtr->getName().str();
    std::string guardName = "_ZGV" + varName.substr(1); // 将 _ZZ 替换为 _ZGV

    // 4. 回溯查找控制流前驱，寻找守卫检查
    // 为了避免在复杂的CFG中无限循环，我们只回溯有限的步数（通常守卫检查就在前一个或前几个块中）
    std::queue<const llvm::BasicBlock*> worklist;
    std::set<const llvm::BasicBlock*> visited;
    worklist.push(currentBlock);
    visited.insert(currentBlock);
    
    int search_depth = 0;
    const int max_depth = 5; // 限制回溯深度

    while (!worklist.empty() && search_depth < max_depth) {
        const llvm::BasicBlock* block = worklist.front();
        worklist.pop();

        // 获取当前块的终止指令，通常是BranchInst
        const llvm::Instruction* terminator = block->getTerminator();
        if (const auto* branch = llvm::dyn_cast<llvm::BranchInst>(terminator)) {
            if (branch->isConditional()) {
                // 如果是条件分支，检查其条件
                if (const auto* cmp = llvm::dyn_cast<llvm::CmpInst>(branch->getCondition())) {
                    if (const auto* load = llvm::dyn_cast<llvm::LoadInst>(cmp->getOperand(0))) {
                        const llvm::Value* loadedVar = load->getPointerOperand();
                        if (loadedVar->hasName() && loadedVar->getName() == guardName) {
                            // 找到了！这个Store被对应的守卫变量检查所控制
                            return true;
                        }
                    }
                }
            }
        }
        
        // 如果当前块不是，则继续向上回溯
        if (block != &block->getParent()->getEntryBlock()) {
             for (const llvm::BasicBlock *Pred : llvm::predecessors(block)) {
                if (visited.find(Pred) == visited.end()) {
                    worklist.push(Pred);
                    visited.insert(Pred);
                }
            }
        }
        search_depth++;
    }

    return false;
}