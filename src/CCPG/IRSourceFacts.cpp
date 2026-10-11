#include "CCPG/IRSourceFacts.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Operator.h"
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
        if (auto* d=llvm::dyn_cast<llvm::DbgDeclareInst>(&i))
            if (d->getAddress() && d->getExpression()->getNumElements()==0) names_.emplace(d->getAddress(),d->getVariable()->getName().str());
        if (auto* d=llvm::dyn_cast<llvm::DbgValueInst>(&i))
            if (auto* v=d->getValue(); v && !llvm::isa<llvm::Constant>(v) && d->getExpression()->getNumElements()==0)
                names_.emplace(v,d->getVariable()->getName().str());
    }
}
std::string IRSourceFacts::path(const llvm::Value* v,std::set<const llvm::Value*>& active) const {
    if (!v||!active.insert(v).second) return "";
    struct Pop { std::set<const llvm::Value*>& a; const llvm::Value* v; ~Pop(){a.erase(v);} } pop{active,v};
    if (names_.count(v)) return names_.at(v);
    if (auto* g=llvm::dyn_cast<llvm::GlobalVariable>(v)) return g->getName().str();
    if (auto* l=llvm::dyn_cast<llvm::LoadInst>(v)) return path(l->getPointerOperand(),active);
    if (auto* op=llvm::dyn_cast<llvm::Operator>(v)) {
        if (op->getOpcode()==llvm::Instruction::BitCast||op->getOpcode()==llvm::Instruction::AddrSpaceCast)
            return path(op->getOperand(0),active);
    }
    auto* gep=llvm::dyn_cast<llvm::GEPOperator>(v); if (!gep) return "";
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
    J paths=J::array(),locations=J::array(); std::set<const llvm::Value*> active;
    if (auto p=path(address,active);!p.empty()) paths.push_back(p);
    for (auto* d=i.getDebugLoc().get();d;d=d->getInlinedAt()) locations.push_back({{"file",d->getFilename().str()},{"line",d->getLine()},{"column",d->getColumn()}});
    auto* sp=i.getFunction()->getSubprogram();
    return {{"memory_role",role},{"access_paths",paths},{"memory_ordering",ordering},{"volatile",vol},
        {"source_locations",locations},{"source_function",sp?sp->getName().str():i.getFunction()->getName().str()}};
}
}
