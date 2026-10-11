#include "CCPG/CCPG.h"
#include "PhasarUtil/AnalysisManager.h"
#include "PhasarUtil/PhasarPointerAnalysis.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/raw_ostream.h"
#include <algorithm>
#include <queue>
#include <map>
#include <tuple>
#include <iostream>
#include <stdexcept>

using J = nlohmann::json;

// Preserve original source and IR representations separately. A source-line
// match is navigation, never an invented IR equivalence or a synchronization.
void CCPG::build() {
    moduleModel_ = {{"schema", "lace-module-graph-v1"}, {"functions", J::array()},
        {"nodes", J::array()}, {"cfg_edges", J::array()}, {"call_edges", J::array()},
        {"gaps", J::array()}, {"parallel_entries_inferred", false}};
    auto methods = cpg->getNodesByType("Method");
    std::vector<Node*> ordered(methods.begin(), methods.end());
    std::sort(ordered.begin(), ordered.end(), [](auto* a, auto* b) {
        return std::make_tuple(a->getFileName(), a->getLineNumber(), a->getName(), a->id) <
               std::make_tuple(b->getFileName(), b->getLineNumber(), b->getName(), b->id);
    });
    size_t sourceFunctions = 0, sourceNodes = 0;
    std::multimap<std::string, ccpg::Function*> sourceByName;
    for (Node* method : ordered) {
        if (method->properties["IS_EXTERNAL"] == "true" || method->getCode() == "<empty>") continue;
        auto* entry = createCCPGNode(method);
        auto* fn = new ccpg::Function(entry); addFunction(fn);
        std::queue<Node*> work; std::unordered_set<Node*> seen;
        work.push(method);
        while (!work.empty()) {
            auto* source = work.front(); work.pop();
            if (!seen.insert(source).second) continue;
            if (source != method && source->getType() == "Method") continue;
            auto* node = createCCPGNode(source); node->setFunction(fn); fn->addNode(node);
            const auto file = source->getFileName().empty() ? method->getFileName() : source->getFileName();
            node->setNodeLoc(NodeLoc(file, source->getLineNumber(), fn));
            addNodeByLoc(node->getNodeLoc(), node); fn->addNodeByLoc(node->getNodeLoc(), node);
            J row = {{"node_id", node->getId()}, {"function_id", fn->getId()}, {"origin", "source"},
                {"code", source->getCode()}, {"api", source->getName()}, {"kind", source->getType()},
                {"file", node->getNodeLoc().getFileName()}, {"line", source->getLineNumber()}};
            moduleModel_["nodes"].push_back(row); ++sourceNodes;
            std::vector<Node*> children;
            for (auto* edge : source->outEdges)
                if (edge->getType() == "Ast" || edge->getType() == "Cfg") children.push_back(edge->getToNode());
            std::sort(children.begin(), children.end(), [](auto* a, auto* b) { return a->id < b->id; });
            for (auto* child : children) work.push(child);
        }
        for (Node* source : seen) {
            auto* from = getCCPGNodeByCPGNode(source);
            if (!from || from->getFunction() != fn) continue;
            for (auto* edge : source->outCFGEdges) {
                auto* to = getCCPGNodeByCPGNode(edge->getToNode());
                if (!to || to->getFunction() != fn) continue;
                auto* e = createCCPGEdge(from, to); fn->addEdge(e);
                moduleModel_["cfg_edges"].push_back({from->getId(), to->getId()});
            }
        }
        moduleModel_["functions"].push_back({{"function_id", fn->getId()}, {"name", method->getName()},
            {"origin", "source"}, {"file", entry->getNodeLoc().getFileName()}, {"line", method->getLineNumber()},
            {"entry", entry->getId()}, {"exits", J::array()}, {"cfg_complete", false}});
        sourceByName.emplace(method->getName(), fn); ++sourceFunctions;
    }

    auto* pa = dynamic_cast<PhasarPointerAnalysis*>(AnalysisManager::getInstance()->getPointerAnalyzer());
    const llvm::Module* ir = pa ? pa->getModule() : nullptr;
    if (!ir) throw std::runtime_error("Complete module graph requires matching LLVM IR");
    std::map<const llvm::Function*, ccpg::Function*> irFunctions;
    std::map<const llvm::Instruction*, CCPGNode*> instructions;
    size_t irCount = 0, irNodes = 0, withoutDebug = 0;
    auto ownedNode = [&](const std::string& id, const std::string& type,
                         const std::unordered_map<std::string,std::string>& props) {
        auto owned = std::make_unique<Node>(id,type,props); auto* raw = owned.get();
        ownedIRNodes_.push_back(std::move(owned)); return createCCPGNode(raw);
    };
    for (const auto& lf : *ir) {
        if (lf.isDeclaration()) continue;
        std::string file; int line = 0;
        if (auto* sp = lf.getSubprogram()) { file = sp->getFilename().str(); line = sp->getLine(); }
        auto* entry = ownedNode("ir-function:" + lf.getName().str(), "METHOD",
            {{"NAME",lf.getName().str()}, {"CODE",lf.getName().str()}, {"FILENAME",file}, {"LINE_NUMBER",std::to_string(line)}});
        auto* fn = new ccpg::Function(entry); fn->setLLVMFunction(&lf); addFunction(fn);
        irFunctions[&lf] = fn; entry->setFunction(fn); fn->addNode(entry);
        entry->setNodeLoc(NodeLoc(file,line,fn));
        moduleModel_["nodes"].push_back({{"node_id",entry->getId()}, {"function_id",fn->getId()},
            {"origin","ir"}, {"kind","Method"}, {"code",lf.getName().str()}, {"api",""},
            {"file",file}, {"line",line}});
        J exits = J::array(); unsigned ordinal = 0;
        for (const auto& bb : lf) for (const auto& inst : bb) {
            std::string text; llvm::raw_string_ostream stream(text); inst.print(stream); stream.flush();
            std::string instFile = file; int instLine = 0;
            if (auto dl = inst.getDebugLoc()) { instFile = dl->getFilename().str(); instLine = dl.getLine(); }
            else ++withoutDebug;
            auto* cb = llvm::dyn_cast<llvm::CallBase>(&inst);
            auto* callee = cb ? cb->getCalledFunction() : nullptr;
            const std::string api = callee ? callee->getName().str() : "";
            auto* node = ownedNode("ir-op:" + lf.getName().str() + ":" + std::to_string(ordinal++), cb ? "CALL" : "IR_OPERATION",
                {{"NAME",api}, {"CODE",text}, {"FILENAME",instFile}, {"LINE_NUMBER",std::to_string(instLine)}});
            node->setCallSite(cb != nullptr); node->setFunction(fn); fn->addNode(node);
            node->setNodeLoc(NodeLoc(instFile,instLine,fn)); addNodeByLoc(node->getNodeLoc(),node); fn->addNodeByLoc(node->getNodeLoc(),node);
            if (auto* ci = llvm::dyn_cast<llvm::CallInst>(&inst)) node->setLLVMCallInst(ci);
            instructions[&inst] = node;
            bool atomic = llvm::isa<llvm::AtomicRMWInst>(inst) || llvm::isa<llvm::AtomicCmpXchgInst>(inst);
            if (auto* load = llvm::dyn_cast<llvm::LoadInst>(&inst)) atomic |= load->isAtomic();
            if (auto* store = llvm::dyn_cast<llvm::StoreInst>(&inst)) atomic |= store->isAtomic();
            moduleModel_["nodes"].push_back({{"atomic",atomic}, {"node_id",node->getId()}, {"function_id",fn->getId()},
                {"origin","ir"}, {"kind",inst.getOpcodeName()}, {"code",text}, {"api",api},
                {"file",instFile}, {"line",instLine}, {"ordinal",ordinal-1}});
            if (llvm::isa<llvm::ReturnInst>(inst) || llvm::isa<llvm::ResumeInst>(inst) || llvm::isa<llvm::UnreachableInst>(inst)) exits.push_back(node->getId());
            ++irNodes;
        }
        moduleModel_["functions"].push_back({{"function_id",fn->getId()}, {"name",lf.getName().str()},
            {"origin","ir"}, {"file",file}, {"line",line}, {"entry",entry->getId()},
            {"exits",exits}, {"cfg_complete",true}}); ++irCount;
    }
    auto cfg = [&](CCPGNode* a, CCPGNode* b) {
        auto* edge = createCCPGEdge(a,b); a->getFunction()->addEdge(edge);
        moduleModel_["cfg_edges"].push_back({a->getId(),b->getId()});
    };
    for (const auto& [lf,fn] : irFunctions) {
        cfg(fn->getFuncNode(),instructions.at(&lf->getEntryBlock().front()));
        for (const auto& bb : *lf) {
            CCPGNode* previous = nullptr;
            for (const auto& inst : bb) {
                auto* node = instructions.at(&inst); if (previous) cfg(previous,node); previous = node;
                if (auto* call = llvm::dyn_cast<llvm::CallBase>(&inst)) {
                    if (auto* target = call->getCalledFunction(); target && irFunctions.count(target)) {
                        auto* edge = createCCPGEdge(node,irFunctions.at(target)->getFuncNode()); edge->setType(CCPGEdge::CALL);
                        moduleModel_["call_edges"].push_back({node->getId(),edge->getDst()->getId()});
                    } else if (!target) moduleModel_["gaps"].push_back({{"kind","indirect_call_target"},{"node_id",node->getId()}});
                }
            }
            for (auto* successor : llvm::successors(&bb)) cfg(previous,instructions.at(&successor->front()));
        }
    }
    for (const auto& [name,fn] : sourceByName) for (auto* node : fn->getNodes()) {
        auto* source = node->getCPGNode(); if (source->getType() != "Call") continue;
        auto matches = sourceByName.equal_range(source->getName());
        for (auto it = matches.first; it != matches.second; ++it) {
            auto* edge = createCCPGEdge(node,it->second->getFuncNode()); edge->setType(CCPGEdge::CALL);
            moduleModel_["call_edges"].push_back({node->getId(),edge->getDst()->getId()});
        }
    }
    moduleModel_["coverage"] = {{"source_functions",sourceFunctions}, {"source_nodes",sourceNodes},
        {"ir_functions",irCount}, {"ir_instructions",irNodes}, {"ir_instructions_without_debug_location",withoutDebug},
        {"scope","all source methods retained by Joern and all defined functions/instructions in supplied IR; external declarations have no body"}};
    std::cout << "[Module Graph] " << moduleModel_["coverage"].dump() << std::endl;
}
J CCPG::moduleModel() const { return moduleModel_; }
