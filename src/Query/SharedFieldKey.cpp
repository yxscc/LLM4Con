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
#include "llvm/ADT/StringRef.h"

#include <queue>
#include <set>
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

// Try to recover the most likely struct type for an opaque-pointer SSA
// value by looking at its uses. Any `getelementptr T, ptr v, ...` user
// where v is the pointer operand pins the type to T (LLVM keeps the
// source-element-type explicit on every GEP even with opaque pointers).
//
// Returns the struct type if the same non-generic struct dominates the
// uses, or nullptr if either nothing typed uses v or there is genuine
// type ambiguity (in which case downstream code falls back to the
// Kind::Unknown bucket as before).
static const llvm::StructType*
inferStructTypeFromUsers(const llvm::Value* v) {
    if (!v) return nullptr;
    const llvm::Value* base = v->stripPointerCasts();
    if (!base) return nullptr;

    const llvm::StructType* best = nullptr;
    int considered = 0;
    constexpr int kMaxUsers = 32;
    for (const llvm::User* U : base->users()) {
        if (considered++ > kMaxUsers) break;
        const auto* gep = llvm::dyn_cast<llvm::GEPOperator>(U);
        if (!gep) continue;
        if (gep->getPointerOperand()->stripPointerCasts() != base) continue;
        const auto* ST = llvm::dyn_cast<llvm::StructType>(
            gep->getSourceElementType());
        if (!ST) continue;
        if (isGenericContainerType(ST)) continue;
        if (!best) {
            best = ST;
        } else if (best != ST) {
            // Multiple distinct struct types use this pointer; bail out
            // rather than risk an arbitrary pick.
            return nullptr;
        }
    }
    return best;
}

// Names of kernel publication sinks that turn a stack-local into a
// cross-thread sharing candidate. When an alloca's address (or some
// pointer derived from it) is passed to one of these, the object is
// reachable from another thread via the sink's data structure
// (waitqueue, RCU callback, linked list, completion, ...).
static bool isEscapingPublicationCallee(llvm::StringRef name) {
    if (name.empty()) return false;
    static const llvm::StringRef kSinks[] = {
        // waitqueue
        "add_wait_queue", "add_wait_queue_exclusive",
        "__add_wait_queue", "__add_wait_queue_exclusive",
        "__add_wait_queue_entry_tail",
        "prepare_to_wait", "prepare_to_wait_exclusive",
        "__prepare_to_wait_event", "prepare_to_wait_event",
        "init_waitqueue_entry", "init_wait_entry",
        // intrusive lists / hlist / llist
        "list_add", "list_add_tail", "list_add_rcu", "list_add_tail_rcu",
        "list_move", "list_move_tail",
        "hlist_add_head", "hlist_add_head_rcu", "hlist_add_tail_rcu",
        "hlist_add_behind", "hlist_add_before",
        "hlist_nulls_add_head_rcu",
        "llist_add", "llist_add_batch",
        // RCU
        "call_rcu", "call_srcu", "call_rcu_tasks",
        "queue_rcu_work",
        // workqueue / completion
        "queue_work", "queue_work_on", "queue_delayed_work",
        "queue_delayed_work_on", "schedule_work", "schedule_delayed_work",
        "init_completion", "init_completion_map",
        "complete", "complete_all",
        // rbtree / rhash
        "rb_link_node", "rb_link_node_rcu",
        "rhashtable_insert_fast", "rhltable_insert",
        // generic publication helpers
        "init_llist_head", "init_swait_queue_head",
    };
    for (llvm::StringRef s : kSinks) {
        if (name == s) return true;
    }
    return false;
}

// Walk forward uses of `start` to see if any reaches a publication sink
// from `isEscapingPublicationCallee`. Cheap, bounded BFS through pointer
// equivalences (GEP / bitcast / addrspacecast / phi / select).
//
// Distinct from getUnderlyingObject in the OTHER direction: this asks
// "does the alloca ESCAPE forward?" rather than "what does v point to
// upward?".
static bool allocaEscapesViaKnownSink(const llvm::AllocaInst* AI) {
    if (!AI) return false;
    std::queue<const llvm::Value*> q;
    std::set<const llvm::Value*> seen;
    q.push(AI);
    seen.insert(AI);
    int budget = 128;
    while (!q.empty() && budget-- > 0) {
        const llvm::Value* v = q.front(); q.pop();
        for (const llvm::User* U : v->users()) {
            if (const auto* CI = llvm::dyn_cast<llvm::CallBase>(U)) {
                // Match callee by name, even when called via a function
                // pointer (rare for these helpers but cheap to handle).
                if (const auto* callee = CI->getCalledFunction()) {
                    if (callee->hasName() &&
                        isEscapingPublicationCallee(callee->getName())) {
                        return true;
                    }
                }
                // Even if we don't recognise the callee, treat passing
                // the alloca to an EXTERNAL (decl-only) function with a
                // call-by-pointer argument as a potential escape — kernel
                // helpers like `add_wait_queue` are usually declared but
                // not defined in the bitcode. We approximate this by
                // accepting any callee that is a declaration without
                // body and is not on a small intra-procedural deny list.
                if (const auto* callee = CI->getCalledFunction()) {
                    if (callee->isDeclaration()) {
                        llvm::StringRef n = callee->getName();
                        // Deny list: cheap noisy helpers that never publish.
                        if (n.startswith("llvm.")) continue;
                        if (n == "memset" || n == "memcpy" || n == "memmove" ||
                            n == "__memset" || n == "__memcpy" ||
                            n == "__memcpy_chk" || n == "__memset_chk" ||
                            n.startswith("__builtin_") ||
                            n.startswith("__sanitizer_") ||
                            n.startswith("__asan_") ||
                            n.startswith("__msan_") ||
                            n.startswith("__tsan_") ||
                            n == "__kmsan_check_memory") continue;
                        // Conservatively accept any other external callee
                        // that takes our pointer as a typed pointer arg.
                        return true;
                    }
                }
                continue;
            }
            // Track pointer equivalences forward.
            if (llvm::isa<llvm::GetElementPtrInst>(U) ||
                llvm::isa<llvm::BitCastInst>(U) ||
                llvm::isa<llvm::AddrSpaceCastInst>(U) ||
                llvm::isa<llvm::PHINode>(U) ||
                llvm::isa<llvm::SelectInst>(U)) {
                if (seen.insert(U).second) q.push(U);
                continue;
            }
            // Stores of the alloca address into a heap pointer also
            // escape (the value is now reachable from another thread
            // through that pointer). Detect: `store ptr %alloca, ptr %p`
            // where %p is NOT itself an alloca.
            if (const auto* SI = llvm::dyn_cast<llvm::StoreInst>(U)) {
                if (SI->getValueOperand() == v) {
                    const llvm::Value* dst = llvm::getUnderlyingObject(
                        SI->getPointerOperand());
                    if (dst && !llvm::isa<llvm::AllocaInst>(dst)) return true;
                }
            }
        }
    }
    return false;
}

// ---- Public API ------------------------------------------------------------

std::optional<SharedFieldKey>
SharedFieldKey::fromValue(const llvm::Value* v, const llvm::Module& M,
                          bool is_whole_object_access) {
    if (!v) return std::nullopt;

    const llvm::DataLayout& DL = M.getDataLayout();

    // Discover the logical root object. getUnderlyingObject peels casts,
    // GEPs, phi/select (best-effort), and collapses to the raw source
    // pointer. This is authoritative for "is it stack-local?".
    const llvm::Value* root = llvm::getUnderlyingObject(v);
    if (!root) root = v;

    // Stack-local storage: by default reject (not a legitimate cross-thread
    // sharing candidate). Exception: alloca-escape via a known kernel
    // publication sink (waitqueue / list / RCU / completion / queue_work).
    // When that happens, the stack object becomes a legitimate shared
    // instance — the consumer thread will reach the same struct through
    // container_of(...) and observe the SAME (struct.T, offset) key.
    bool alloca_is_escaped = false;
    if (auto* AI = llvm::dyn_cast<llvm::AllocaInst>(root)) {
        if (!allocaEscapesViaKnownSink(AI)) return std::nullopt;
        alloca_is_escaped = true;
        // Fall through with `root = AI`, the GEP walk below still applies.
    }

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

    // Alloca-escaped struct: the GEP walk reached no canonicalStruct
    // (e.g. directly &alloca with no field GEP). Use the alloca's
    // allocated type so the consumer thread's container_of(...) access,
    // which already keys as (struct.T, offset), aggregates with this
    // publish-side access.
    if (alloca_is_escaped) {
        const auto* AI = llvm::cast<llvm::AllocaInst>(root);
        if (auto* ST = llvm::dyn_cast<llvm::StructType>(AI->getAllocatedType())) {
            key.kind = Kind::StructField;
            key.type_name = structTypeName(ST);
            key.field_offset = offset;
            return key;
        }
    }

    // Whole-object accesses (free-like calls) often land here under
    // opaque pointers because the call site discards the struct type
    // and the freed pointer is not behind a GEP. Recover the struct
    // type from the GEP users of the same SSA value so the free can
    // aggregate with field-level accesses of the same struct in other
    // threads. Without this, kfree(nlk) and Read(nlk->field) sit in
    // disjoint buckets and the cross-thread UAF is invisible to the
    // Surface/LLM.
    if (is_whole_object_access) {
        if (const llvm::StructType* inferred = inferStructTypeFromUsers(v)) {
            key.kind = Kind::StructField;
            key.type_name = structTypeName(inferred);
            key.field_offset = 0;
            return key;
        }
    }

    // Fallback: neither a struct-field access nor a global — classify by
    // root identity so equivalent accesses from the same source still
    // bucket together, but don't over-merge unrelated pointers.
    key.kind = Kind::Unknown;
    key.type_name = rootIdentifier(root);
    key.field_offset = offset;
    return key;
}

// Walk the GEP chain like `climbGEPs` but record per-GEP info:
// (sourceElementType T_i, offset_within_T_i_added_by_gep_i). The
// access at the leaf sits at byte
//   off_in_T_i = sum_{j<=i} offset_added_by_gep_j
// inside T_i. We use that to emit alias keys for every inner
// non-generic struct in an embedding chain.
namespace {
struct GEPLevel {
    const llvm::StructType* sourceST = nullptr;
    int64_t offsetAddedByThisGEP = 0;
};

static void climbGEPsLeveled(const llvm::Value* v,
                             const llvm::DataLayout& DL,
                             const llvm::Value*& baseOut,
                             std::vector<GEPLevel>& levelsOut) {
    baseOut = v;
    levelsOut.clear();
    if (!v) return;
    const llvm::Value* cur = v;
    for (int guard = 0; guard < 32 && cur != nullptr; ++guard) {
        if (auto* gep = llvm::dyn_cast<llvm::GEPOperator>(cur)) {
            GEPLevel lvl;
            if (auto* ST = llvm::dyn_cast<llvm::StructType>(
                    gep->getSourceElementType())) {
                lvl.sourceST = ST;
            }
            llvm::APInt off(
                DL.getIndexTypeSizeInBits(gep->getType()), 0, true);
            if (gep->accumulateConstantOffset(DL, off)) {
                lvl.offsetAddedByThisGEP = off.getSExtValue();
            }
            levelsOut.push_back(lvl);
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
}  // anonymous namespace

std::vector<SharedFieldKey>
SharedFieldKey::fromValueAllAliases(const llvm::Value* v,
                                    const llvm::Module& M,
                                    bool is_whole_object_access) {
    std::vector<SharedFieldKey> out;
    auto canonical = fromValue(v, M, is_whole_object_access);
    if (canonical) out.push_back(*canonical);
    if (!v) return out;

    const llvm::DataLayout& DL = M.getDataLayout();
    const llvm::Value* base = v;
    std::vector<GEPLevel> levels;
    climbGEPsLeveled(v, DL, base, levels);
    if (levels.size() < 2) return out;  // no embedding chain

    // Aliases for inner-but-not-canonical structs.
    // levels[0] = innermost GEP (closest to the leaf v), each successive
    // level wraps it. Cumulative offset within levels[i].sourceST is the
    // sum of offsets added at depths [0..i]. (Inner-to-outer.)
    int64_t cumOffsetWithinThisST = 0;
    for (size_t i = 0; i < levels.size(); ++i) {
        cumOffsetWithinThisST += levels[i].offsetAddedByThisGEP;
        const llvm::StructType* ST = levels[i].sourceST;
        if (!ST) continue;
        if (isGenericContainerType(ST)) continue;
        // Skip if this struct produces the same key as the canonical.
        SharedFieldKey alias;
        alias.kind = Kind::StructField;
        alias.type_name = structTypeName(ST);
        alias.field_offset = cumOffsetWithinThisST;
        bool dup = false;
        for (const auto& k : out) {
            if (k == alias) { dup = true; break; }
        }
        if (!dup) out.push_back(alias);
    }
    return out;
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
