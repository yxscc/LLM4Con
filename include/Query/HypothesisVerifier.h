#pragma once

#include "CCPG/CCPG.h"
#include "CCPG/ThreadCreationTree.h"
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <map>

class HBGraph;  // forward decl: M7 Phase A happens-before graph

namespace query {

struct VerificationConstraint {
    std::string predicate;
    nlohmann::json args;
};

struct Hypothesis {
    std::string id;
    std::string description;
    std::string bug_category;
    std::string severity;
    std::map<std::string, int> nodes;  // role_name -> CCPG node_id
    std::vector<VerificationConstraint> constraints;

    nlohmann::json toJson() const;
    std::string toReportString(CCPG* ccpg) const;
};

struct ConstraintEvalResult {
    int index;
    std::string predicate;
    bool satisfied;
    std::string detail;
};

struct VerificationResult {
    bool all_satisfied = false;
    std::vector<ConstraintEvalResult> results;

    nlohmann::json toFeedbackJson() const;
};

// M7 Phase B: operation kind for op_kind() predicate. Free-standing so it
// can be parsed from JSON without exposing LLVM headers to callers.
enum class OpKind {
    READ,    // llvm::LoadInst
    WRITE,   // llvm::StoreInst
    RMW,     // llvm::AtomicRMWInst / llvm::AtomicCmpXchgInst
    CALL,    // llvm::CallInst (incl. invoke)
    OTHER
};
const char* opKindName(OpKind k);
OpKind opKindFromString(const std::string& s);

class HypothesisVerifier {
public:
    // Backward-compatible: hb may be null, in which case eval_hb falls back
    // to a coarse "reachable" check (legacy mode). When hb is provided,
    // eval_hb / eval_concurrent / eval_unsafe_atomic_block use the
    // happens-before synchronization graph.
    HypothesisVerifier(CCPG* ccpg, ThreadCreationTree* tct, HBGraph* hb = nullptr);

    VerificationResult verify(const Hypothesis& h);

private:
    CCPG* ccpg_;
    ThreadCreationTree* tct_;
    HBGraph* hb_;  // M7 Phase A: nullable for legacy compatibility

    int resolveNodeRef(const nlohmann::json& val, const Hypothesis& h);

    // ---- Legacy 6 predicates (M0-M6). Kept for backward compatibility;
    // DetectorAgent prompt may still emit them. New prompt prefers the
    // M7 5+3 vocabulary below. ----
    bool eval_in_thread(int node_id, int thread_id, std::string& detail);
    bool eval_may_run_concurrently(int t1, int t2, std::string& detail);
    bool eval_reachable(int from_id, int to_id, std::string& detail);
    bool eval_not_lock_protected(int node_id, std::string& detail);
    bool eval_same_lock(int n1, int n2, std::string& detail);
    bool eval_alias(int n1, int n2, std::string& detail);

    // ---- M7 Phase B: 5 primitives + 3 sugars (HYPOTHESIS_DSL_DESIGN.md) ----
    // Primitives:
    //   in_thread / reachable already exist above (re-used as DSL primitives).
    bool eval_same_location(int n1, int n2, std::string& detail);
    bool eval_op_kind(int node_id, OpKind expected, std::string& detail);
    bool eval_hb(int n1, int n2, bool expected, std::string& detail);
    // Sugars (verifier-side expansions; LLM may also use them directly):
    bool eval_conflicts(int n1, int n2, std::string& detail);
    bool eval_concurrent(int n1, int n2, std::string& detail);
    bool eval_unsafe_atomic_block(int start_id, int end_id, int witness_id,
                                  std::string& detail);
};

} // namespace query
