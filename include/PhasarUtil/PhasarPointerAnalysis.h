// PhasarPointerAnalysis.h

#pragma once

#include "PointerAnalysisInterface.h"
#include "CCPG/CCPGNode.h"
#include <memory>

// Forward-declare Phasar classes to reduce header dependencies
namespace psr {
class LLVMProjectIRDB;
class DIBasedTypeHierarchy;
class LLVMBasedICFG;
class LLVMAliasSet;
} // namespace psr

namespace llvm {
class Value;
class Function;
class Instruction;
class CallInst;
class LoadInst;
class StoreInst;
} // namespace llvm

class PhasarPointerAnalysis : public PointerAnalysisInterface {
public:
    // Constructor with optional entry point list (for kernel modules)
    explicit PhasarPointerAnalysis(const std::string &bitcodeFilePath, 
                                   const std::vector<std::string>& userEntryPoints = {});
    ~PhasarPointerAnalysis(); // Required for unique_ptr with forward-declared types

    bool areAliases(const llvm::Value *V1, const llvm::Value *V2) override;
    std::set<const llvm::Value *> getPointsToSet(const llvm::Value *Ptr) override;
    const llvm::Value* getValueByName(const std::string &funcName, const std::string &varName) override;
    std::vector<EntryPointInfo> getPotentialEntryPoints() override;

    EntryPointInfo getMainFunction() const override;
    std::vector<EntryPointInfo> getAllEntryPointInfos() const;  // NEW: Get all entry points for kernel modules
    std::vector<const llvm::Function *> getAllLLVMFunctions() const override;

    std::vector<const llvm::CallInst *> getCallInstsByLoc(const NodeLoc &Loc) const;
    std::vector<const llvm::LoadInst *> getLoadInstsByLoc(const NodeLoc &Loc) const;
    std::vector<const llvm::StoreInst *> getStoreInstsByLoc(const NodeLoc &Loc) const;
    
    psr::LLVMBasedICFG* getICFG() const  { return ICFG.get(); }
    std::vector<const llvm::Function*> getCalleesOfCallAt(const llvm::Instruction* callInst) const override;
    std::vector<const llvm::GlobalVariable*> getAllGlobalVariables() const override;
    bool isGuardedByStaticInitializer(const llvm::StoreInst* storeInst) const override;

    // Get all discovered entry points (for kernel modules)
    const std::vector<std::string>& getDiscoveredEntryPoints() const { return discoveredEntryPoints_; }

private:
    std::unique_ptr<psr::LLVMProjectIRDB> DB;
    std::unique_ptr<psr::DIBasedTypeHierarchy> TH;
    std::unique_ptr<psr::LLVMBasedICFG> ICFG;
    std::unique_ptr<psr::LLVMAliasSet> PTA;
    std::unordered_map<NodeLoc, std::vector<const llvm::CallInst *>, NodeLocHash> LocToCallInstsMap;
    std::unordered_map<NodeLoc, std::vector<const llvm::LoadInst *>, NodeLocHash> LocToLoadInstsMap;
    std::unordered_map<NodeLoc, std::vector<const llvm::StoreInst *>, NodeLocHash> LocToStoreInstsMap;
    std::vector<std::string> discoveredEntryPoints_;  // Discovered kernel entry points
};