#include "CCPG/IRSourceFacts.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Operator.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/InlineAsm.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/BinaryFormat/Dwarf.h"
namespace ccpg {
namespace {
std::string order(llvm::AtomicOrdering o) {
    switch(o) {
        case llvm::AtomicOrdering::NotAtomic:return "none";
        case llvm::AtomicOrdering::Unordered:return "unordered";
        case llvm::AtomicOrdering::Monotonic:return "monotonic";
        case llvm::AtomicOrdering::Acquire:return "acquire";
        case llvm::AtomicOrdering::Release:return "release";
        case llvm::AtomicOrdering::AcquireRelease:return "acq_rel";
        case llvm::AtomicOrdering::SequentiallyConsistent:return "seq_cst";
    } return "unknown";
}
}
IRSourceFacts::IRSourceFacts(const llvm::Module& m):module_(m) {
    llvm::DebugInfoFinder finder; finder.processModule(m);
    for (auto* t:finder.types()) if (auto* c=llvm::dyn_cast<llvm::DICompositeType>(t))
        if (!c->getName().empty()) types_[c->getName().str()].insert(c);
    for (const auto& f:m) for (const auto& b:f) for (const auto& i:b) {
        auto record=[&](const llvm::Value* value,const llvm::DILocalVariable* variable) {
            names_.emplace(value,variable->getName().str());
            if (f.getSubprogram()&&variable->getScope()->getSubprogram()==f.getSubprogram())
                callerNames_.emplace(value,variable->getName().str());
        };
        if (auto* d=llvm::dyn_cast<llvm::DbgDeclareInst>(&i))
            if (d->getAddress() && d->getExpression()->getNumElements()==0) record(d->getAddress(),d->getVariable());
        if (auto* d=llvm::dyn_cast<llvm::DbgValueInst>(&i))
            if (auto* v=d->getValue(); v && !llvm::isa<llvm::Constant>(v) && d->getExpression()->getNumElements()==0)
                record(v,d->getVariable());
    }
}
// Recover only a non-escaping pointer spill with one stored value and a
// dominating store. This is expression provenance, not general alias analysis.
const llvm::Value* IRSourceFacts::spillSource(const llvm::Value* value) const {
    auto* load=llvm::dyn_cast<llvm::LoadInst>(value);
    if (!load||!load->getType()->isPointerTy()) return nullptr;
    auto* slot=llvm::dyn_cast<llvm::AllocaInst>(load->getPointerOperand());
    if (!slot) return nullptr;
    const llvm::StoreInst* store=nullptr;
    for (auto* user:slot->users()) {
        if (auto* s=llvm::dyn_cast<llvm::StoreInst>(user)) {
            if (s->getPointerOperand()!=slot||store) return nullptr;
            store=s;
        } else if (auto* l=llvm::dyn_cast<llvm::LoadInst>(user)) {
            if (l->getPointerOperand()!=slot) return nullptr;
        } else if (auto* intrinsic=llvm::dyn_cast<llvm::IntrinsicInst>(user)) {
            if (!llvm::isa<llvm::DbgInfoIntrinsic>(intrinsic)&&
                intrinsic->getIntrinsicID()!=llvm::Intrinsic::lifetime_start&&
                intrinsic->getIntrinsicID()!=llvm::Intrinsic::lifetime_end) return nullptr;
        } else return nullptr;
    }
    if (!store||!store->getValueOperand()->getType()->isPointerTy()) return nullptr;
    auto& dom=dominators_[load->getFunction()];
    if (!dom) dom=std::make_unique<llvm::DominatorTree>(*const_cast<llvm::Function*>(load->getFunction()));
    return dom->dominates(store,load)?store->getValueOperand():nullptr;
}
std::string IRSourceFacts::path(const llvm::Value* v,std::set<const llvm::Value*>& active) const {
    if (!v||!active.insert(v).second) return "";
    struct Pop { std::set<const llvm::Value*>& a; const llvm::Value* v; ~Pop(){a.erase(v);} } pop{active,v};
    // Keep expressions in the containing source function's vocabulary. Follow
    // spills for inlined parameter temporaries, not past a caller's named alias
    // (e.g. a container pointer or tc6 = data).
    if (callerNames_.count(v)) return callerNames_.at(v);
    if (auto* g=llvm::dyn_cast<llvm::GlobalVariable>(v)) return g->getName().str();
    if (auto* l=llvm::dyn_cast<llvm::LoadInst>(v)) {
        if (callerNames_.count(l->getPointerOperand())) return callerNames_.at(l->getPointerOperand());
        if (auto* stored=spillSource(l)) if (auto p=path(stored,active);!p.empty()) return p;
        return path(l->getPointerOperand(),active);
    }
    if (auto* op=llvm::dyn_cast<llvm::Operator>(v)) {
        if (op->getOpcode()==llvm::Instruction::BitCast||op->getOpcode()==llvm::Instruction::AddrSpaceCast)
            return path(op->getOperand(0),active);
    }
    auto* gep=llvm::dyn_cast<llvm::GEPOperator>(v);
    if (!gep) return names_.count(v)?names_.at(v):"";
    auto result=path(gep->getPointerOperand(),active); if (result.empty()) return "";
    llvm::Type* type=gep->getSourceElementType(); bool first=true;
    for (auto it=gep->idx_begin();it!=gep->idx_end();++it) {
        auto* index=llvm::dyn_cast<llvm::ConstantInt>(it->get()); if (!index) return "";
        if (first) {first=false; if (!index->isZero()) return ""; continue;}
        if (auto* s=llvm::dyn_cast<llvm::StructType>(type)) {
            const auto n=index->getZExtValue(); if (n>=s->getNumElements()||!s->hasName()) return "";
            auto name=s->getName().str(); for (auto prefix:{"struct.","union."}) if (name.rfind(prefix,0)==0) name=name.substr(std::string(prefix).size());
            auto found=types_.find(name); if (found==types_.end()) return "";
            const auto offset=module_.getDataLayout().getStructLayout(s)->getElementOffsetInBits(n);
            std::set<std::string> fields;
            for (auto* dt:found->second) for (auto* e:dt->getElements())
                if (auto* member=llvm::dyn_cast<llvm::DIDerivedType>(e);member&&member->getTag()==llvm::dwarf::DW_TAG_member&&member->getOffsetInBits()==offset&&!member->getName().empty()) fields.insert(member->getName().str());
            if (fields.size()!=1) return "";
            result+='.'+*fields.begin(); type=s->getElementType(n);
        } else if (auto* a=llvm::dyn_cast<llvm::ArrayType>(type)) {
            result+='['+std::to_string(index->getZExtValue())+']'; type=a->getElementType();
        } else return "";
    } return result;
}
nlohmann::json IRSourceFacts::describe(const llvm::Instruction& i) const {
    using J=nlohmann::json; std::string role="other",ordering="none"; const llvm::Value* address=nullptr; bool vol=false;
    if (auto* l=llvm::dyn_cast<llvm::LoadInst>(&i)) {role="read";address=l->getPointerOperand();ordering=order(l->getOrdering());vol=l->isVolatile();}
    else if (auto* s=llvm::dyn_cast<llvm::StoreInst>(&i)) {role="write";address=s->getPointerOperand();ordering=order(s->getOrdering());vol=s->isVolatile();}
    else if (auto* a=llvm::dyn_cast<llvm::AtomicRMWInst>(&i)) {role="atomic";address=a->getPointerOperand();ordering=order(a->getOrdering());}
    else if (auto* a=llvm::dyn_cast<llvm::AtomicCmpXchgInst>(&i)) {role="atomic";address=a->getPointerOperand();ordering=order(a->getSuccessOrdering());}
    else if (auto* f=llvm::dyn_cast<llvm::FenceInst>(&i)) {role="fence";ordering=order(f->getOrdering());}
    else if (llvm::isa<llvm::CallBase>(i)) role="call";
    J paths=J::array(),locations=J::array(),inlineEffects=J::array(),inlineCalls=J::array(),callees=J::array(); std::set<const llvm::Value*> active;
    if (auto p=path(address,active);!p.empty()) paths.push_back(p);
    const llvm::Value* root=address; std::set<const llvm::Value*> visited;
    while (root&&visited.insert(root).second) {
        const auto* underlying=llvm::getUnderlyingObject(root);
        if (underlying!=root) root=underlying;
        else if (auto* stored=spillSource(root)) root=stored;
        else break;
    }
    const bool local=root&&llvm::isa<llvm::AllocaInst>(root);
    auto* call=llvm::dyn_cast<llvm::CallBase>(&i);
    const bool assembly=call&&llvm::isa<llvm::InlineAsm>(call->getCalledOperand());
    std::string calleeApi;
    if (call) {
        if (auto* f=llvm::dyn_cast<llvm::Function>(call->getCalledOperand()->stripPointerCasts())) calleeApi=f->getName().str();
        else if (!assembly) if (auto p=path(call->getCalledOperand(),active);!p.empty()) callees.push_back(p);
    }
    // Ordinary nested calls may be instrumentation or opaque helpers. Do not
    // select them as an inline API's memory effect merely from a debug frame.
    const bool concreteEffect=(!local&&(role=="read"||role=="write"||role=="atomic"))||role=="fence"||assembly;
    for (auto* d=i.getDebugLoc().get();d&&d->getInlinedAt();d=d->getInlinedAt()) {
        auto* sp=d->getScope()->getSubprogram(); auto* site=d->getInlinedAt();
        if (concreteEffect&&sp) inlineEffects.push_back({{"api",sp->getName().str()},
            {"file",site->getFilename().str()},{"line",site->getLine()},{"column",site->getColumn()},
            {"effect_role",role},{"inline_assembly",assembly}});
        // Correspondence of a source wrapper to an actual nested call is a
        // separate fact from its memory/atomic effects. Do not rewrite the
        // callee, ordering or atomic flags and do not count debug intrinsics.
        if (call&&sp&&!assembly&&calleeApi.rfind("llvm.dbg.",0)!=0&&calleeApi.rfind("llvm.lifetime.",0)!=0)
            inlineCalls.push_back({{"api",sp->getName().str()},{"file",site->getFilename().str()},
                {"line",site->getLine()},{"column",site->getColumn()},{"callee",calleeApi}});
    }
    for (auto* d=i.getDebugLoc().get();d;d=d->getInlinedAt()) locations.push_back({{"file",d->getFilename().str()},{"line",d->getLine()},{"column",d->getColumn()}});
    auto* sp=i.getFunction()->getSubprogram();
    return {{"memory_role",role},{"access_paths",paths},{"memory_ordering",ordering},{"volatile",vol},
        {"source_locations",locations},{"source_function",sp?sp->getName().str():i.getFunction()->getName().str()},
        {"local_memory",local},{"inline_effects",inlineEffects},{"inline_calls",inlineCalls},
        {"callee_api",calleeApi},{"callee_access_paths",callees}};
}
}
