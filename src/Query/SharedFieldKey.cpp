// src/Query/SharedFieldKey.cpp
#include "Query/SharedFieldKey.h"

#include "llvm/IR/Instructions.h"
#include "llvm/IR/Operator.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/ADT/APInt.h"

#include <sstream>
#include <string>
#include <vector>

namespace query {

// ---- Local helpers ---------------------------------------------------------

// Extract a stable C-level name from an llvm::StructType. Anonymous types
// get synthesised names tied to their module pointer; the important thing
// is that two accesses to the same struct type in the same module hash
// equal.
static std::string structTypeName(const llvm::StructType* ST) {
    if (!ST) return "";
    if (ST->hasName()) return ST->getName().str();
    std::stringstream ss;
    ss << "anon.struct@" << static_cast<const void*>(ST);
    return ss.str();
}

// Kernel "container-node" / primitive wrapper struct types whose name is
// by itself insufficient to identify the conceptual storage. E.g. an access
// to a `struct list_head` embedded inside `struct netns_nftables` should
// NOT aggregate together with every other embedded `list_head` in the
// kernel — the distinguishing identity is the *owner* struct (or the
// owning global variable). We therefore skip these types when choosing the
// canonical struct name in `fromValue`.
static bool isGenericContainerType(const llvm::StructType* ST) {
    if (!ST || !ST->hasName()) return false;
    llvm::StringRef n = ST->getName();
    // Match names with or without the ".<number>" LLVM disambiguator suffix.
    auto is = [&](llvm::StringRef base) {
        return n == base ||
               (n.startswith(base) && n.drop_front(base.size()).startswith("."));
    };
    return is("struct.list_head")         || is("struct.hlist_head") ||
           is("struct.hlist_node")        || is("struct.hlist_bl_head") ||
           is("struct.hlist_bl_node")     || is("struct.hlist_nulls_head") ||
           is("struct.hlist_nulls_node")  ||
           is("struct.llist_head")        || is("struct.llist_node") ||
           is("struct.rb_root")           || is("struct.rb_root_cached") ||
           is("struct.rb_node")           ||
           is("struct.rhash_head")        || is("struct.rhashtable") ||
           is("struct.rhlist_head")       ||
           is("struct.callback_head")     ||  // RCU head
           is("struct.atomic_t")          || is("struct.atomic64_t") ||
           is("struct.atomic_long_t")     ||
           is("struct.refcount_struct")   || is("struct.kref") ||
           is("struct.raw_spinlock")      || is("struct.spinlock") ||
           is("struct.raw_spinlock_t")    || is("struct.spinlock_t") ||
           is("struct.mutex")             || is("struct.rw_semaphore") ||
           is("struct.semaphore")         || is("struct.completion") ||
           is("struct.seqcount")          || is("struct.seqcount_spinlock") ||
           is("struct.lockdep_map");
}

// Walk from `v` upwards through zero or more GEPs, accumulating the total
// constant byte offset into `offsetOut` and collecting every struct type
// encountered along the way (closest-to-leaf first). Also returns the
// outermost pointer through `baseOut`.
//
// The outermost non-generic struct is what the programmer typically
// "sees" as the owner of the field; intermediate types like
// `struct list_head` are embeddings, not identities.
static void climbGEPs(const llvm::Value* v,
                      const llvm::DataLayout& DL,
                      int64_t& offsetOut,
                      const llvm::Value*& baseOut,
                      std::vector<const llvm::StructType*>& structStack) {
    offsetOut = 0;
    baseOut = v;
    structStack.clear();
    if (!v) return;

    const llvm::Value* cur = v;

    // Bound the walk for pathological IR.
    for (int guard = 0; guard < 32 && cur != nullptr; ++guard) {
        if (auto* gep = llvm::dyn_cast<llvm::GEPOperator>(cur)) {
            if (auto* ST = llvm::dyn_cast<llvm::StructType>(gep->getSourceElementType())) {
                structStack.push_back(ST);
            }
            llvm::APInt off(DL.getIndexTypeSizeInBits(gep->getType()), 0, true);
            if (gep->accumulateConstantOffset(DL, off)) {
                offsetOut += off.getSExtValue();
            }
            cur = gep->getPointerOperand();
            baseOut = cur;
            continue;
        }
        if (auto* bc = llvm::dyn_cast<llvm::BitCastOperator>(cur)) {
            cur = bc->getOperand(0);
            baseOut = cur;
            continue;
        }
        if (auto* asc = llvm::dyn_cast<llvm::AddrSpaceCastOperator>(cur)) {
            cur = asc->getOperand(0);
            baseOut = cur;
            continue;
        }
        break;
    }
}

// Produce a short, canonical identifier for a "root" value (a pointer
// that has been stripped of casts and GEPs by getUnderlyingObject). Used
// when the root is neither a global nor an alloca — e.g. a function
// parameter or a heap allocation.
static std::string rootIdentifier(const llvm::Value* root) {
    if (!root) return "(null)";
    if (auto* GV = llvm::dyn_cast<llvm::GlobalVariable>(root)) {
        return GV->hasName() ? GV->getName().str() : "anon.global";
    }
    if (auto* A = llvm::dyn_cast<llvm::Argument>(root)) {
        std::stringstream ss;
        const llvm::Function* F = A->getParent();
        ss << "param:" << (F && F->hasName() ? F->getName().str() : "?")
           << "#" << A->getArgNo();
        return ss.str();
    }
    if (root->hasName()) return root->getName().str();
    std::stringstream ss;
    ss << "val@" << static_cast<const void*>(root);
    return ss.str();
}

// ---- Public API ------------------------------------------------------------

std::optional<SharedFieldKey>
SharedFieldKey::fromValue(const llvm::Value* v, const llvm::Module& M) {
    if (!v) return std::nullopt;

    const llvm::DataLayout& DL = M.getDataLayout();

    // Discover the logical root object. getUnderlyingObject peels casts,
    // GEPs, phi/select (best-effort), and collapses to the raw source
    // pointer. This is authoritative for "is it stack-local?".
    const llvm::Value* root = llvm::getUnderlyingObject(v);
    if (!root) root = v;

    // Stack-local storage: reject (not a legitimate cross-thread sharing
    // candidate). This single check alone eliminates whole classes of
    // false positives like `timespec64 tm` or `snd_seq_event ev` on the
    // stack being reported as "shared".
    if (llvm::isa<llvm::AllocaInst>(root)) return std::nullopt;

    // Walk the GEP chain starting at v to pick up the struct types and
    // field-byte-offset that the programmer "sees" at this access.
    // `structStack` is ordered innermost-first (closest to the leaf GEP).
    int64_t offset = 0;
    const llvm::Value* gepBase = v;
    std::vector<const llvm::StructType*> structStack;
    climbGEPs(v, DL, offset, gepBase, structStack);

    // Choose a canonical struct type. We prefer the outermost non-generic
    // struct on the walk — that is the semantic owner of the field. If
    // every struct on the path is a generic container node (list_head,
    // rb_node, etc.) we still fall back to the innermost so downstream
    // code sees *something* and global/alloca logic below can take over.
    const llvm::StructType* ownerStruct = nullptr;   // outermost non-generic
    const llvm::StructType* innermost = structStack.empty() ? nullptr : structStack.front();
    for (auto it = structStack.rbegin(); it != structStack.rend(); ++it) {
        if (!isGenericContainerType(*it)) {
            ownerStruct = *it;
            break;
        }
    }
    const llvm::StructType* canonicalStruct = ownerStruct ? ownerStruct : innermost;

    SharedFieldKey key;

    if (auto* GV = llvm::dyn_cast<llvm::GlobalVariable>(root)) {
        key.kind = Kind::Global;
        key.type_name = GV->hasName() ? GV->getName().str() : "anon.global";
        key.field_offset = offset; // offset inside the global
        return key;
    }

    if (canonicalStruct) {
        key.kind = Kind::StructField;
        key.type_name = structTypeName(canonicalStruct);
        key.field_offset = offset;
        return key;
    }

    // Fallback: neither a struct-field access nor a global — classify by
    // root identity so equivalent accesses from the same source still
    // bucket together, but don't over-merge unrelated pointers.
    key.kind = Kind::Unknown;
    key.type_name = rootIdentifier(root);
    key.field_offset = offset;
    return key;
}

std::size_t SharedFieldKey::hash() const {
    std::size_t h1 = std::hash<int>()(static_cast<int>(kind));
    std::size_t h2 = std::hash<std::string>()(type_name);
    std::size_t h3 = std::hash<int64_t>()(field_offset);
    // mix
    std::size_t h = h1;
    h ^= h2 + 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2);
    h ^= h3 + 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2);
    return h;
}

std::string SharedFieldKey::toString() const {
    std::stringstream ss;
    switch (kind) {
        case Kind::Global:      ss << "global:"; break;
        case Kind::StructField: ss << "field:";  break;
        case Kind::Unknown:     ss << "obj:";    break;
    }
    ss << type_name;
    if (field_offset > 0) ss << "+" << field_offset;
    return ss.str();
}

} // namespace query
