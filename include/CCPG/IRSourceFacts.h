#pragma once
#include "nlohmann/json.hpp"
#include "llvm/IR/Module.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/Dominators.h"
#include <memory>
#include <map>
#include <set>
#include <string>
namespace ccpg {
// Compiler-derived navigation facts, not points-to/instance equivalence.
class IRSourceFacts {
    const llvm::Module& module_;
    std::map<const llvm::Value*,std::string> names_;
    std::map<const llvm::Value*,std::string> callerNames_;
    std::map<std::string,std::set<const llvm::DICompositeType*>> types_;
    mutable std::map<const llvm::Function*,std::unique_ptr<llvm::DominatorTree>> dominators_;
    const llvm::Value* spillSource(const llvm::Value*) const;
    std::string path(const llvm::Value*,std::set<const llvm::Value*>&) const;
public:
    explicit IRSourceFacts(const llvm::Module&);
    nlohmann::json describe(const llvm::Instruction&) const;
};
}
