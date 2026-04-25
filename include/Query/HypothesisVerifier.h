#pragma once

#include "CCPG/CCPG.h"
#include "CCPG/ThreadCreationTree.h"
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <map>

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

class HypothesisVerifier {
public:
    HypothesisVerifier(CCPG* ccpg, ThreadCreationTree* tct);

    VerificationResult verify(const Hypothesis& h);

private:
    CCPG* ccpg_;
    ThreadCreationTree* tct_;

    int resolveNodeRef(const nlohmann::json& val, const Hypothesis& h);

    bool eval_in_thread(int node_id, int thread_id, std::string& detail);
    bool eval_may_run_concurrently(int t1, int t2, std::string& detail);
    bool eval_reachable(int from_id, int to_id, std::string& detail);
    bool eval_not_lock_protected(int node_id, std::string& detail);
    bool eval_same_lock(int n1, int n2, std::string& detail);
    bool eval_alias(int n1, int n2, std::string& detail);
};

} // namespace query
