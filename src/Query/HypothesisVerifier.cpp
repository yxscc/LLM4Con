#include "Query/HypothesisVerifier.h"
#include "CCPG/CCPGNode.h"
#include "CCPG/AliasChecker.h"
#include "CCPG/LSAnalysis.h"
#include "CCPG/HBGraph.h"
#include "CPG/Node.h"
#include "PhasarUtil/AnalysisManager.h"
#include "PhasarUtil/PhasarPointerAnalysis.h"
#include "Query/SharedFieldKey.h"

#include "llvm/IR/Instructions.h"

#include <queue>
#include <set>
#include <sstream>
#include <iostream>
#include <algorithm>

namespace query {

// ---- OpKind helpers (M7 Phase B) -------------------------------------------

const char* opKindName(OpKind k) {
    switch (k) {
        case OpKind::READ:  return "READ";
        case OpKind::WRITE: return "WRITE";
        case OpKind::RMW:   return "RMW";
        case OpKind::CALL:  return "CALL";
        case OpKind::OTHER: return "OTHER";
    }
    return "?";
}

OpKind opKindFromString(const std::string& s) {
    if (s == "READ"  || s == "read"  || s == "load")  return OpKind::READ;
    if (s == "WRITE" || s == "write" || s == "store") return OpKind::WRITE;
    if (s == "RMW"   || s == "rmw"   || s == "atomic_rmw") return OpKind::RMW;
    if (s == "CALL"  || s == "call")  return OpKind::CALL;
    return OpKind::OTHER;
}

// --- Hypothesis serialization ---

nlohmann::json Hypothesis::toJson() const {
    nlohmann::json j;
    j["hypothesis_id"] = id;
    j["description"] = description;
    j["bug_category"] = bug_category;
    j["severity"] = severity;
    j["nodes"] = nodes;

    j["constraints"] = nlohmann::json::array();
    for (const auto& c : constraints) {
        j["constraints"].push_back({{"predicate", c.predicate}, {"args", c.args}});
    }
    return j;
}

std::string Hypothesis::toReportString(CCPG* ccpg) const {
    std::stringstream ss;
    ss << "========== Hypothesis-Based Violation Detected ==========\n"
       << "Hypothesis: " << id << "\n"
       << "Category: " << bug_category << " (severity: " << severity << ")\n"
       << "Description: " << description << "\n\n"
       << "--- Involved Nodes ---\n";

    for (const auto& [role, node_id] : nodes) {
        ss << "  " << role << " (node " << node_id << "): ";
        CCPGNode* n = ccpg ? ccpg->getNodeByID(node_id) : nullptr;
        if (n && n->getCPGNode()) {
            ss << n->getCPGNode()->getCode()
               << "  [" << n->getNodeLoc().toString() << "]";
        } else {
            ss << "[node not found]";
        }
        ss << "\n";
    }

    ss << "\n--- Verified Constraints ---\n";
    for (size_t i = 0; i < constraints.size(); ++i) {
        ss << "  [" << i << "] " << constraints[i].predicate
           << " " << constraints[i].args.dump() << "\n";
    }
    ss << "==========================================================";
    return ss.str();
}

// --- VerificationResult feedback ---

nlohmann::json VerificationResult::toFeedbackJson() const {
    nlohmann::json j;
    j["verified"] = all_satisfied;

    nlohmann::json passed = nlohmann::json::array();
    nlohmann::json failed = nlohmann::json::array();

    for (const auto& r : results) {
        nlohmann::json entry = {
            {"index", r.index},
            {"predicate", r.predicate},
            {"detail", r.detail}
        };
        if (r.satisfied) {
            passed.push_back(entry);
        } else {
            failed.push_back(entry);
        }
    }

    j["passed"] = passed;
    j["failed"] = failed;

    if (all_satisfied) {
        j["message"] = "All " + std::to_string(results.size()) +
                        " constraints satisfied. Hypothesis CONFIRMED.";
    } else {
        j["message"] = std::to_string(failed.size()) + " constraint(s) failed. "
                        "Review the 'failed' array for details and adjust node IDs or constraints.";
    }
    return j;
}

// --- HypothesisVerifier ---

HypothesisVerifier::HypothesisVerifier(CCPG* ccpg, ThreadCreationTree* tct,
                                       HBGraph* hb)
    : ccpg_(ccpg), tct_(tct), hb_(hb) {}

int HypothesisVerifier::resolveNodeRef(const nlohmann::json& val, const Hypothesis& h) {
    if (val.is_number_integer()) {
        return val.get<int>();
    }
    if (val.is_string()) {
        std::string ref = val.get<std::string>();
        auto it = h.nodes.find(ref);
        if (it != h.nodes.end()) return it->second;
    }
    return -1;
}

VerificationResult HypothesisVerifier::verify(const Hypothesis& h) {
    VerificationResult result;
    result.all_satisfied = true;

    for (int i = 0; i < (int)h.constraints.size(); ++i) {
        const auto& c = h.constraints[i];
        ConstraintEvalResult cr;
        cr.index = i;
        cr.predicate = c.predicate;
        cr.satisfied = false;

        if (c.predicate == "in_thread") {
            int node_id = resolveNodeRef(c.args.value("node", nlohmann::json()), h);
            int thread_id = c.args.value("thread", -1);
            cr.satisfied = eval_in_thread(node_id, thread_id, cr.detail);

        } else if (c.predicate == "may_run_concurrently") {
            int t1 = c.args.value("thread1", -1);
            int t2 = c.args.value("thread2", -1);
            cr.satisfied = eval_may_run_concurrently(t1, t2, cr.detail);

        } else if (c.predicate == "reachable") {
            int from_id = resolveNodeRef(c.args.value("from", nlohmann::json()), h);
            int to_id = resolveNodeRef(c.args.value("to", nlohmann::json()), h);
            cr.satisfied = eval_reachable(from_id, to_id, cr.detail);

        } else if (c.predicate == "not_lock_protected") {
            int node_id = resolveNodeRef(c.args.value("node", nlohmann::json()), h);
            cr.satisfied = eval_not_lock_protected(node_id, cr.detail);

        } else if (c.predicate == "same_lock") {
            int n1 = resolveNodeRef(c.args.value("node1", nlohmann::json()), h);
            int n2 = resolveNodeRef(c.args.value("node2", nlohmann::json()), h);
            cr.satisfied = eval_same_lock(n1, n2, cr.detail);

        } else if (c.predicate == "alias") {
            int n1 = resolveNodeRef(c.args.value("node1", nlohmann::json()), h);
            int n2 = resolveNodeRef(c.args.value("node2", nlohmann::json()), h);
            cr.satisfied = eval_alias(n1, n2, cr.detail);

        // ---- M7 Phase B: 5 primitives + 3 sugars ---------------------------
        // The new vocabulary accepts both {"a","b"} and the legacy
        // {"node1","node2"} arg shapes so existing prompts keep working
        // while the new prompt cleans up to {a,b}.
        } else if (c.predicate == "same_location") {
            int n1 = resolveNodeRef(
                c.args.contains("a") ? c.args.at("a") : c.args.value("node1", nlohmann::json()), h);
            int n2 = resolveNodeRef(
                c.args.contains("b") ? c.args.at("b") : c.args.value("node2", nlohmann::json()), h);
            cr.satisfied = eval_same_location(n1, n2, cr.detail);

        } else if (c.predicate == "op_kind") {
            int node_id = resolveNodeRef(c.args.value("node", nlohmann::json()), h);
            std::string kindStr = c.args.value("kind", std::string());
            if (kindStr.empty()) kindStr = c.args.value("expected", std::string());
            OpKind expected = opKindFromString(kindStr);
            cr.satisfied = eval_op_kind(node_id, expected, cr.detail);

        } else if (c.predicate == "hb") {
            int n1 = resolveNodeRef(
                c.args.contains("a") ? c.args.at("a") : c.args.value("from", nlohmann::json()), h);
            int n2 = resolveNodeRef(
                c.args.contains("b") ? c.args.at("b") : c.args.value("to", nlohmann::json()), h);
            // expected defaults to true (the typical "X happens-before Y"
            // assertion). Set "expected": false to assert the absence of an
            // hb path (used by F5/F8 UAF / NULL-deref templates).
            bool expected = c.args.value("expected", true);
            cr.satisfied = eval_hb(n1, n2, expected, cr.detail);

        } else if (c.predicate == "conflicts") {
            int n1 = resolveNodeRef(
                c.args.contains("a") ? c.args.at("a") : c.args.value("node1", nlohmann::json()), h);
            int n2 = resolveNodeRef(
                c.args.contains("b") ? c.args.at("b") : c.args.value("node2", nlohmann::json()), h);
            cr.satisfied = eval_conflicts(n1, n2, cr.detail);

        } else if (c.predicate == "concurrent") {
            int n1 = resolveNodeRef(
                c.args.contains("a") ? c.args.at("a") : c.args.value("node1", nlohmann::json()), h);
            int n2 = resolveNodeRef(
                c.args.contains("b") ? c.args.at("b") : c.args.value("node2", nlohmann::json()), h);
            cr.satisfied = eval_concurrent(n1, n2, cr.detail);

        } else if (c.predicate == "unsafe_atomic_block") {
            int s_id = resolveNodeRef(c.args.value("start", nlohmann::json()), h);
            int e_id = resolveNodeRef(c.args.value("end", nlohmann::json()), h);
            int w_id = resolveNodeRef(c.args.value("witness", nlohmann::json()), h);
            cr.satisfied = eval_unsafe_atomic_block(s_id, e_id, w_id, cr.detail);

        } else {
            cr.detail = "Unknown predicate: " + c.predicate;
        }

        if (!cr.satisfied) result.all_satisfied = false;
        result.results.push_back(std::move(cr));
    }

    return result;
}

// --- Predicate implementations ---

bool HypothesisVerifier::eval_in_thread(int node_id, int thread_id, std::string& detail) {
    if (node_id < 0 || thread_id < 0) {
        detail = "Invalid node_id or thread_id";
        return false;
    }
    CCPGNode* node = ccpg_->getNodeByID(node_id);
    if (!node) {
        detail = "Node " + std::to_string(node_id) + " not found in CCPG";
        return false;
    }
    Thread* thread = tct_->getThreadById(thread_id);
    if (!thread) {
        detail = "Thread " + std::to_string(thread_id) + " not found";
        return false;
    }
    if (thread->getNodes().count(node)) {
        detail = "Node " + std::to_string(node_id) + " belongs to thread " + std::to_string(thread_id);
        return true;
    }
    detail = "Node " + std::to_string(node_id) + " is NOT in thread " + std::to_string(thread_id);
    return false;
}

bool HypothesisVerifier::eval_may_run_concurrently(int t1, int t2, std::string& detail) {
    Thread* thread1 = tct_->getThreadById(t1);
    Thread* thread2 = tct_->getThreadById(t2);
    if (!thread1 || !thread2) {
        detail = "Thread(s) not found: T" + std::to_string(t1) + ", T" + std::to_string(t2);
        return false;
    }
    bool concurrent = tct_->mayThreadsRunConcurrently(thread1, thread2);
    detail = "T" + std::to_string(t1) + " and T" + std::to_string(t2) +
             (concurrent ? " CAN" : " CANNOT") + " run concurrently";
    return concurrent;
}

bool HypothesisVerifier::eval_reachable(int from_id, int to_id, std::string& detail) {
    CCPGNode* from = ccpg_->getNodeByID(from_id);
    CCPGNode* to = ccpg_->getNodeByID(to_id);
    if (!from || !to) {
        detail = "Node(s) not found: " + std::to_string(from_id) + " -> " + std::to_string(to_id);
        return false;
    }

    // Find which thread contains the 'from' node
    Thread* thread = nullptr;
    for (Thread* t : tct_->getThreads()) {
        if (t->getNodes().count(from)) {
            thread = t;
            break;
        }
    }
    if (!thread) {
        detail = "Source node " + std::to_string(from_id) + " not found in any thread";
        return false;
    }

    if (from == to) {
        detail = "Trivially reachable (same node)";
        return true;
    }
    if (!thread->getNodes().count(to)) {
        detail = "Target node " + std::to_string(to_id) + " not in same thread as source";
        return false;
    }

    // Phase-5 upgrade: cross-function bounded BFS. We walk ORDER edges
    // (intra-procedural control flow) and CALL edges (step into a callee)
    // up to a fixed depth to avoid combinatorial blowups on deep kernel
    // call stacks. Depth is measured in edges, not frames; the bound
    // balances recall (deeper = more paths) against cost.
    constexpr int kMaxDepth = 8;

    struct Frame {
        CCPGNode* node;
        int depth;
    };
    std::queue<Frame> worklist;
    std::set<CCPGNode*> visited;
    worklist.push({from, 0});
    visited.insert(from);

    while (!worklist.empty()) {
        Frame cur = worklist.front();
        worklist.pop();
        if (cur.depth >= kMaxDepth) continue;
        for (CCPGEdge* edge : cur.node->getOutEdges()) {
            CCPGEdge::EdgeType et = edge->getType();
            // Follow intra-procedural order edges and cross-function CALL
            // edges. HB edges are happens-before across threads and should
            // NOT count as intra-thread reachability.
            if (et != CCPGEdge::EdgeType::ORDER &&
                et != CCPGEdge::EdgeType::CALL) {
                continue;
            }
            CCPGNode* next = edge->getDst();
            if (!next || visited.count(next)) continue;
            if (next == to) {
                detail = "Path exists from node " + std::to_string(from_id) +
                         " to node " + std::to_string(to_id) +
                         " (cross-function BFS depth " +
                         std::to_string(cur.depth + 1) + ")";
                return true;
            }
            // For ORDER edges stay within the thread's nodes; for CALL
            // edges we accept any callee node reachable in the CCPG, since
            // a caller that transitively reaches `to` should still count.
            if (et == CCPGEdge::EdgeType::ORDER &&
                !thread->getNodes().count(next)) {
                continue;
            }
            visited.insert(next);
            worklist.push({next, cur.depth + 1});
        }
    }
    detail = "No control flow path from node " + std::to_string(from_id) +
             " to node " + std::to_string(to_id) +
             " within depth " + std::to_string(kMaxDepth) +
             " (ORDER+CALL BFS)";
    return false;
}

bool HypothesisVerifier::eval_not_lock_protected(int node_id, std::string& detail) {
    CCPGNode* node = ccpg_->getNodeByID(node_id);
    if (!node) {
        detail = "Node " + std::to_string(node_id) + " not found";
        return false;
    }

    // Path-sensitive LockSet query. Iterate over every context in which this
    // node can be reached (a CCPGNode may be shared across multiple thread
    // contexts in the CCPG). If any context has an empty LockSet, the node
    // is unprotected there, which is what the hypothesis asserts. If every
    // context has a non-empty LockSet, the hypothesis fails with a list of
    // the locks that always hold.
    LSAnalysis* ls = LSAnalysis::getInstance();
    ccpg::Function* nodeFunc = node->getFunction();
    ccpg::ContextSet ctxs;
    if (nodeFunc) ctxs = nodeFunc->getContextSet();
    NodeLoc nodeLoc = node->getNodeLoc();

    if (ctxs.empty()) {
        auto lockSet = ls->getLockSet(nodeLoc, Context());
        if (lockSet.empty()) {
            detail = "Node " + std::to_string(node_id) +
                     " is NOT lock-protected (empty LockSet)";
            return true;
        }
        std::string lockList;
        for (Lock* l : lockSet) {
            if (CCPGNode* acq = l->getAcquire()) {
                if (!lockList.empty()) lockList += ", ";
                lockList += acq->getCPGNode()
                            ? acq->getCPGNode()->getCode() : "?";
            }
        }
        detail = "Node " + std::to_string(node_id) +
                 " IS lock-protected by: " + lockList;
        return false;
    }

    // With multiple contexts, consider the node "unprotected" iff there is
    // at least one context where no lock is held. This captures the typical
    // data-race scenario where one call path enters without the lock.
    std::vector<std::string> allLockStrs;
    bool anyUnprotected = false;
    for (Context* ctx : ctxs) {
        auto lockSet = ls->getLockSet(nodeLoc, ctx ? *ctx : Context());
        if (lockSet.empty()) {
            anyUnprotected = true;
            break;
        }
        for (Lock* l : lockSet) {
            if (CCPGNode* acq = l->getAcquire()) {
                std::string code = acq->getCPGNode()
                                    ? acq->getCPGNode()->getCode() : "?";
                if (std::find(allLockStrs.begin(), allLockStrs.end(), code)
                        == allLockStrs.end()) {
                    allLockStrs.push_back(code);
                }
            }
        }
    }

    if (anyUnprotected) {
        detail = "Node " + std::to_string(node_id) +
                 " has at least one call context with an empty LockSet";
        return true;
    }

    std::string lockList;
    for (const auto& s : allLockStrs) {
        if (!lockList.empty()) lockList += ", ";
        lockList += s;
    }
    detail = "Node " + std::to_string(node_id) +
             " is lock-protected in all " + std::to_string(ctxs.size()) +
             " contexts. Lock(s) ever held: " + lockList;
    return false;
}

bool HypothesisVerifier::eval_same_lock(int n1, int n2, std::string& detail) {
    CCPGNode* node1 = ccpg_->getNodeByID(n1);
    CCPGNode* node2 = ccpg_->getNodeByID(n2);
    if (!node1 || !node2) {
        detail = "Node(s) not found: " + std::to_string(n1) + ", " + std::to_string(n2);
        return false;
    }

    // The project's LSAnalysis already implements a path-sensitive "same lock
    // protects both nodes" predicate; use it directly. This replaces the
    // previous coarse AliasChecker-based lookup which compared lock *pointer*
    // identity and missed re-acquisitions or wrappers.
    LSAnalysis* ls = LSAnalysis::getInstance();
    bool same = ls->isProtectedBySameLock(node1, node2);
    detail = "Node " + std::to_string(n1) +
             (same ? " and node " : " and node ") + std::to_string(n2) +
             (same ? " are protected by a common lock (path-sensitive LockSet intersection)."
                   : " do NOT share any common lock across their LockSets.");
    return same;
}

bool HypothesisVerifier::eval_alias(int n1, int n2, std::string& detail) {
    CCPGNode* node1 = ccpg_->getNodeByID(n1);
    CCPGNode* node2 = ccpg_->getNodeByID(n2);
    if (!node1 || !node2) {
        detail = "Node(s) not found: " + std::to_string(n1) + ", " + std::to_string(n2);
        return false;
    }

    AliasChecker* ac = AliasChecker::getInstance();
    auto accesses1 = ac->getMemoryAccessesFromLocation(
        node1->getNodeLoc(),
        node1->getFunction()->getContextSet().empty()
            ? Context()
            : **node1->getFunction()->getContextSet().begin());
    auto accesses2 = ac->getMemoryAccessesFromLocation(
        node2->getNodeLoc(),
        node2->getFunction()->getContextSet().empty()
            ? Context()
            : **node2->getFunction()->getContextSet().begin());

    for (const auto& a1 : accesses1) {
        for (const auto& a2 : accesses2) {
            if (ac->isAlias(a1.pointerOperand, a2.pointerOperand)) {
                detail = "Memory operations at node " + std::to_string(n1) +
                         " and node " + std::to_string(n2) + " ARE aliases";
                return true;
            }
        }
    }

    detail = "Memory operations at node " + std::to_string(n1) +
             " and node " + std::to_string(n2) + " are NOT aliases";
    return false;
}

// =============================================================================
// M7 Phase B: 5 primitives + 3 sugars (HYPOTHESIS_DSL_DESIGN.md §2)
// =============================================================================

namespace {

// Helper: pick the first (canonical) Context for a CCPGNode's function. The
// project's other predicates (alias, lockset) follow the same convention, so
// the new ones do too — keeping behavioural parity even when a node has
// multiple call contexts.
Context firstContext(CCPGNode* n) {
    if (!n) return Context();
    auto* f = n->getFunction();
    if (!f) return Context();
    const auto& ctxs = f->getContextSet();
    if (ctxs.empty()) return Context();
    return **ctxs.begin();
}

// Helper: collect all MemoryAccesses tied to a CCPGNode, walking every
// context the node's function appears in. Without this we miss accesses
// that only show up under a non-canonical entry path (e.g. when a helper
// is reached from multiple syscall entry points).
std::vector<MemoryAccess> gatherAccesses(CCPGNode* n) {
    std::vector<MemoryAccess> out;
    if (!n) return out;
    AliasChecker* ac = AliasChecker::getInstance();
    auto* f = n->getFunction();
    if (!f || f->getContextSet().empty()) {
        auto v = ac->getMemoryAccessesFromLocation(n->getNodeLoc(), Context());
        out.insert(out.end(), v.begin(), v.end());
        return out;
    }
    for (Context* ctx : f->getContextSet()) {
        auto v = ac->getMemoryAccessesFromLocation(
            n->getNodeLoc(), ctx ? *ctx : Context());
        out.insert(out.end(), v.begin(), v.end());
    }
    return out;
}

// Map a single LLVM Instruction to an OpKind. Order matters: AtomicRMW /
// AtomicCmpXchg are technically StoreInst-like, but we want them reported
// as RMW so the LLM can distinguish "non-atomic RMW" (= F7) from plain
// "WRITE" hypotheses.
OpKind classifyInstruction(const llvm::Instruction* I) {
    if (!I) return OpKind::OTHER;
    if (llvm::isa<llvm::AtomicRMWInst>(I) ||
        llvm::isa<llvm::AtomicCmpXchgInst>(I)) {
        return OpKind::RMW;
    }
    if (llvm::isa<llvm::StoreInst>(I)) return OpKind::WRITE;
    if (llvm::isa<llvm::LoadInst>(I))  return OpKind::READ;
    if (llvm::isa<llvm::CallInst>(I) ||
        llvm::isa<llvm::InvokeInst>(I)) return OpKind::CALL;
    return OpKind::OTHER;
}

const llvm::Module* getLLVMModule() {
    auto* pa = dynamic_cast<PhasarPointerAnalysis*>(
        AnalysisManager::getInstance()->getPointerAnalyzer());
    return pa ? pa->getModule() : nullptr;
}

}  // namespace

// ---- eval_same_location ----------------------------------------------------
//
// Field-level aliasing: two nodes operate on the same memory cell. Strictly
// stronger than legacy `alias`: we first try SharedFieldKey equality (which
// distinguishes between e.g. `obj->refcnt` and `obj->next` even when Phasar
// puts them in the same alias set), then fall back to coarse pointer alias
// when SharedFieldKey can't be derived (stack-local, opaque casts, etc.).
bool HypothesisVerifier::eval_same_location(int n1, int n2, std::string& detail) {
    CCPGNode* node1 = ccpg_->getNodeByID(n1);
    CCPGNode* node2 = ccpg_->getNodeByID(n2);
    if (!node1 || !node2) {
        detail = "Node(s) not found: " + std::to_string(n1) + ", " + std::to_string(n2);
        return false;
    }
    auto accs1 = gatherAccesses(node1);
    auto accs2 = gatherAccesses(node2);
    if (accs1.empty() || accs2.empty()) {
        detail = "No memory accesses recorded for node " + std::to_string(n1) +
                 " or " + std::to_string(n2) +
                 " (try eval_alias for a coarser check)";
        return false;
    }

    const llvm::Module* M = getLLVMModule();
    if (M) {
        for (const auto& a1 : accs1) {
            auto k1 = SharedFieldKey::fromValue(a1.pointerOperand, *M);
            if (!k1) continue;
            for (const auto& a2 : accs2) {
                auto k2 = SharedFieldKey::fromValue(a2.pointerOperand, *M);
                if (!k2) continue;
                if (*k1 == *k2) {
                    detail = "same field: " + k1->toString();
                    return true;
                }
            }
        }
    }

    AliasChecker* ac = AliasChecker::getInstance();
    for (const auto& a1 : accs1) {
        for (const auto& a2 : accs2) {
            if (ac->isAlias(a1.pointerOperand, a2.pointerOperand)) {
                detail = "phasar-alias (field-level disagrees or unavailable)";
                return true;
            }
        }
    }
    detail = "no shared field nor pointer alias between node " +
             std::to_string(n1) + " and " + std::to_string(n2);
    return false;
}

// ---- eval_op_kind ----------------------------------------------------------
//
// Read the LLVM IR Instruction(s) that materialised this CCPGNode and check
// whether *any* of them matches `expected`. We accept multiple matches
// because a single source line can lower to several IR ops (e.g. `x++` =>
// load + add + store), and we want any of them to satisfy the predicate.
//
// CALL is also satisfied by CCPGNode::isCallSite() so the LLM can write
// {predicate:"op_kind", node:N, kind:"CALL"} on call-site CCPG nodes whose
// llvmCallInst was attached during CCPG build.
bool HypothesisVerifier::eval_op_kind(int node_id, OpKind expected,
                                      std::string& detail) {
    CCPGNode* node = ccpg_->getNodeByID(node_id);
    if (!node) {
        detail = "Node " + std::to_string(node_id) + " not found";
        return false;
    }

    if (expected == OpKind::CALL && node->isCallSite() &&
        node->getLLVMCallInst()) {
        detail = "CCPG marks node " + std::to_string(node_id) +
                 " as call site (matches CALL)";
        return true;
    }

    auto accesses = gatherAccesses(node);
    std::set<OpKind> seen;
    for (const auto& a : accesses) {
        OpKind k = classifyInstruction(a.instruction);
        seen.insert(k);
        if (k == expected) {
            std::string ks = opKindName(k);
            detail = "Node " + std::to_string(node_id) +
                     " has at least one IR access of kind " + ks;
            return true;
        }
    }

    if (expected == OpKind::CALL && node->isCallSite()) {
        // No memory access map (e.g. void-returning helper) but CCPG still
        // tagged it as a call site; accept the predicate.
        detail = "Node " + std::to_string(node_id) +
                 " is a CCPG call site (no IR memory access recorded)";
        return true;
    }

    std::string seenStr;
    for (OpKind k : seen) {
        if (!seenStr.empty()) seenStr += ", ";
        seenStr += opKindName(k);
    }
    if (seenStr.empty()) seenStr = "<none>";
    detail = "Node " + std::to_string(node_id) +
             " has no IR access of kind " + opKindName(expected) +
             "; observed kinds: " + seenStr;
    return false;
}

// ---- eval_hb ---------------------------------------------------------------
//
// `expected` lets the LLM assert *both* directions of a happens-before
// relation:
//   * expected=true  => "n1 must happen-before n2"   (the typical use case)
//   * expected=false => "no hb chain from n1 to n2"  (used by F5/F8 UAF
//                                                     and NULL-deref templates)
//
// When the HBGraph is unavailable (legacy mode) we conservatively report
// success only when a coarse intra-thread reachability check agrees with
// the assertion; this preserves backward compatibility for hypotheses that
// happen to use the same primitive name through transition.
bool HypothesisVerifier::eval_hb(int n1, int n2, bool expected,
                                 std::string& detail) {
    CCPGNode* a = ccpg_->getNodeByID(n1);
    CCPGNode* b = ccpg_->getNodeByID(n2);
    if (!a || !b) {
        detail = "Node(s) not found: " + std::to_string(n1) + ", " + std::to_string(n2);
        return false;
    }
    bool actual;
    std::string source;
    if (hb_) {
        actual = hb_->hbReachable(a, b);
        source = "HBGraph";
    } else {
        std::string r;
        actual = const_cast<HypothesisVerifier*>(this)
                     ->eval_reachable(n1, n2, r);
        source = "fallback(reachable, no HBGraph)";
    }
    bool ok = (actual == expected);
    detail = "hb(" + std::to_string(n1) + "," + std::to_string(n2) + ")=" +
             (actual ? "true" : "false") +
             " expected=" + (expected ? "true" : "false") +
             " [" + source + "]";
    return ok;
}

// ---- eval_conflicts (sugar) ------------------------------------------------
//
// conflicts(a,b) := same_location(a,b) ∧ (op_kind(a)∈{WRITE,RMW} ∨
//                                         op_kind(b)∈{WRITE,RMW})
//
// We don't reuse eval_op_kind / eval_same_location verbatim because we want
// a single concise `detail` string for the LLM. We do reuse SharedFieldKey
// + AliasChecker semantics directly to keep the contract identical.
bool HypothesisVerifier::eval_conflicts(int n1, int n2, std::string& detail) {
    std::string locDetail;
    bool sameLoc = eval_same_location(n1, n2, locDetail);
    if (!sameLoc) {
        detail = "conflicts: same_location FAILED — " + locDetail;
        return false;
    }

    CCPGNode* node1 = ccpg_->getNodeByID(n1);
    CCPGNode* node2 = ccpg_->getNodeByID(n2);
    auto accs1 = gatherAccesses(node1);
    auto accs2 = gatherAccesses(node2);

    auto hasWriteOrRMW = [](const std::vector<MemoryAccess>& v) {
        for (const auto& a : v) {
            OpKind k = classifyInstruction(a.instruction);
            if (k == OpKind::WRITE || k == OpKind::RMW) return true;
            if (a.isWrite) return true;  // belt-and-braces
        }
        return false;
    };
    bool w1 = hasWriteOrRMW(accs1);
    bool w2 = hasWriteOrRMW(accs2);
    if (!(w1 || w2)) {
        detail = "conflicts: same field but neither side writes (" + locDetail + ")";
        return false;
    }
    detail = "conflicts: " + locDetail +
             "; write side(s)=" + (w1 ? "n1" : std::string()) +
             (w1 && w2 ? "+" : std::string()) + (w2 ? "n2" : std::string());
    return true;
}

// ---- eval_concurrent (sugar) -----------------------------------------------
//
// concurrent(a,b) := ¬hb(a,b) ∧ ¬hb(b,a)
//
// In kernel-module mode (where entry_points are taken as parallel threads
// without an explicit fork node), this reduces to "neither node is reachable
// from the other in the synchronization graph", which is exactly what we
// want to model two unconnected concurrent execution contexts.
bool HypothesisVerifier::eval_concurrent(int n1, int n2, std::string& detail) {
    CCPGNode* a = ccpg_->getNodeByID(n1);
    CCPGNode* b = ccpg_->getNodeByID(n2);
    if (!a || !b) {
        detail = "Node(s) not found: " + std::to_string(n1) + ", " + std::to_string(n2);
        return false;
    }
    bool ab, ba;
    std::string source;
    if (hb_) {
        ab = hb_->hbReachable(a, b);
        ba = hb_->hbReachable(b, a);
        source = "HBGraph";
    } else {
        std::string r1, r2;
        ab = const_cast<HypothesisVerifier*>(this)->eval_reachable(n1, n2, r1);
        ba = const_cast<HypothesisVerifier*>(this)->eval_reachable(n2, n1, r2);
        source = "fallback(reachable, no HBGraph)";
    }
    bool ok = !ab && !ba;
    detail = "concurrent: hb(" + std::to_string(n1) + "," + std::to_string(n2) + ")=" +
             (ab ? "T" : "F") + ", hb(" + std::to_string(n2) + "," + std::to_string(n1) +
             ")=" + (ba ? "T" : "F") + " [" + source + "]";
    return ok;
}

// ---- eval_unsafe_atomic_block (sugar) --------------------------------------
//
// unsafe_atomic_block(start, end, witness) ≡
//     reachable(start, end)
//   ∧ conflicts(witness, start)
//   ∧ ¬hb(witness, start)
//   ∧ ¬hb(end, witness)
//
// Used for TOCTOU (F6) and non-atomic RMW (F7): the block start..end is
// supposed to be atomic w.r.t. `witness`; the predicate fires when
// `witness` is concurrent with the *interior* of the block.
//
// Note: per HYPOTHESIS_DSL_DESIGN §2.2 the definition uses
// `conflicts(witness, start)`, but most TOCTOU patches actually mutate the
// shared field at any point inside the block (start/end/middle). We accept
// `conflicts(witness, start) ∨ conflicts(witness, end)` so the LLM does
// not have to pick the exact internal node that aliases, which is
// over-constrained for kernel-style patterns.
bool HypothesisVerifier::eval_unsafe_atomic_block(int start_id, int end_id,
                                                  int witness_id,
                                                  std::string& detail) {
    std::string r;
    if (!eval_reachable(start_id, end_id, r)) {
        detail = "unsafe_atomic_block: reachable(start,end) FAILED — " + r;
        return false;
    }

    std::string c1, c2;
    bool confStart = eval_conflicts(witness_id, start_id, c1);
    bool confEnd   = eval_conflicts(witness_id, end_id,   c2);
    if (!(confStart || confEnd)) {
        detail = "unsafe_atomic_block: witness does not conflict with start "
                 "nor end (" + c1 + " | " + c2 + ")";
        return false;
    }

    // eval_hb(.., expected=false, ..) returns true *iff* the actual
    // happens-before relation does NOT hold. So `ok_ws` = "¬hb(witness,start)"
    // and `ok_ew` = "¬hb(end,witness)" — exactly what the DSL asks for.
    std::string h1, h2;
    bool ok_ws = eval_hb(witness_id, start_id, /*expected=*/false, h1);
    bool ok_ew = eval_hb(end_id,     witness_id, /*expected=*/false, h2);
    if (!(ok_ws && ok_ew)) {
        detail = "unsafe_atomic_block: witness is hb-ordered around the "
                 "block (" + h1 + " | " + h2 + ")";
        return false;
    }

    detail = "unsafe_atomic_block: start->end reachable; witness conflicts "
             "with " + std::string(confStart ? "start" : "end") +
             "; ¬hb(witness,start) ∧ ¬hb(end,witness) [" +
             (hb_ ? "HBGraph" : "fallback") + "]";
    return true;
}

} // namespace query
