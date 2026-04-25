#include "Query/HypothesisVerifier.h"
#include "CCPG/CCPGNode.h"
#include "CCPG/AliasChecker.h"
#include "CCPG/LSAnalysis.h"
#include "CPG/Node.h"
#include <queue>
#include <set>
#include <sstream>
#include <iostream>
#include <algorithm>

namespace query {

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

HypothesisVerifier::HypothesisVerifier(CCPG* ccpg, ThreadCreationTree* tct)
    : ccpg_(ccpg), tct_(tct) {}

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

} // namespace query
