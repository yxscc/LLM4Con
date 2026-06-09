#pragma once

#include "LLMUtil/Conversation.h"
#include "LLMUtil/ConcurrencyContract.h"
#include "Query/VulnerabilitySurfaceGenerator.h"
#include "Query/HypothesisVerifier.h"
#include "CCPG/CCPG.h"
#include "CCPG/ThreadCreationTree.h"

#include <map>
#include <memory>
#include <set>
#include <string>
#include <unordered_set>
#include <vector>

namespace llm_client {

// Per-shared-object interleaving analysis agent (thread-contract / "old story"
// entry). For ONE shared object touched by a small cluster of concurrent
// threads, the agent reasons FREELY about the harmful interleaving of those
// threads' operations, anchored on their per-thread ConcurrencyContracts and
// the object's statically-collected accesses, then proposes race hypotheses
// that are grounded through the shared HypothesisVerifier.
//
// Deliberately EXCLUDED to keep the two papers' stories distinct:
//   - No MechanismKnowledgeBase / retrieve_mechanism_priors (new-story signature).
//   - No fixed Rule 6-pattern templates / start_rule role-filling. The bug
//     category emerges from interleaving reasoning, not template selection.
class InterleavingAnalysisAgent : public Conversation {
public:
    InterleavingAnalysisAgent(std::shared_ptr<LLMClient> client, CCPG* ccpg,
                              ThreadCreationTree* tct,
                              query::HypothesisVerifier* verifier);

    // Analyze one shared object. `contracts` maps thread_id -> contract for the
    // threads that touch this object (may be empty when running with the
    // contract ablation off). Returns the hypotheses confirmed by the verifier.
    std::vector<query::Hypothesis> analyzeObject(
        const query::SharedObject& object,
        const query::VulnerabilitySurface& surface,
        const std::map<int, LLM::ConcurrencyContract>& contracts);

    // Analyze ONE concurrent context, centered on a focus thread: thread `focusTid`'s
    // contract (the orders it REQUIRES) checked against ALL concurrent accesses, by
    // any thread, to the shared objects `focusTid` touches -- in a single LLM session.
    // This collapses the per-object/per-pair session fan-out: sessions scale with the
    // number of threads (bounded), not objects or O(threads^2) pairs, and every
    // interaction involving `focusTid` is covered without a lossy cap. A bug may span
    // several objects (e.g. free of objA and a dangling use via objB), so all of the
    // focus thread's objects are reasoned about together. Relevant source is preloaded
    // into the prompt so the model rarely needs read tools.
    std::vector<query::Hypothesis> analyzeThread(
        int focusTid,
        const std::vector<const query::SharedObject*>& objects,
        const query::VulnerabilitySurface& surface,
        const std::map<int, LLM::ConcurrencyContract>& contracts);

    // Analyze a CLUSTER of shared objects that are all touched by the SAME set of
    // threads (`threadSet`) in one LLM session. Co-accessed fields are reasoned
    // about together: the shared source is loaded once instead of re-sent per
    // object, and a multi-field bug (e.g. a producer index + the buffer it
    // guards) is visible in a single context. This collapses the per-object
    // session fan-out -- sessions scale with the number of distinct thread-sets,
    // not the number of objects -- without dropping any object (recall-safe).
    // `useContractFraming` selects the reasoning method: when true, the session
    // first derives each thread's order/synchronization obligations (assume:
    // prec/atomic/count_guarded; guarantee: serialize/order/counts) for THIS
    // cluster's objects -- i.e. the per-thread concurrency contract, scoped to what
    // this cluster needs and derived inline from the preloaded source -- and a bug
    // is a required order violated by a concurrent access with no covering
    // guarantee. When false (contract ablation), it reasons about interleavings
    // directly without the assume/guarantee framing.
    // When `precomputedContracts` is non-null, the session runs in CALIBRATION
    // mode (the static-composition pipeline): instead of deriving contracts
    // inline, it is shown the per-thread contracts already derived in Phase A and
    // the `staticVerdict` -- the deterministic single-mismatch composition's
    // candidate violations (and discharged pairs). The model then CONFIRMS or
    // REJECTS each candidate and may ADD a hazard it sees that the composition
    // missed, emitting hypotheses through the same grounded propose path. When
    // null, behaviour is unchanged (inline-derivation folded mode).
    std::vector<query::Hypothesis> analyzeCluster(
        const std::vector<const query::SharedObject*>& objects,
        const std::set<int>& threadSet,
        const query::VulnerabilitySurface& surface,
        bool useContractFraming,
        const std::map<int, LLM::ConcurrencyContract>* precomputedContracts = nullptr,
        const std::string* staticVerdict = nullptr);

private:
    CCPG* ccpg_;
    ThreadCreationTree* tct_;
    query::HypothesisVerifier* verifier_;

    std::string build_system_prompt();
    std::vector<Tool> get_available_tools() const override;
    std::string execute_tool(const std::string& tool_name,
                             const nlohmann::json& arguments) override;
    std::string parseResult(const std::vector<ChatMessage>& history) override;

    // Concatenate the source bodies of the named functions (deduped) up to a char
    // budget, for one-shot preloading into a session prompt. Functions beyond the
    // budget are left for the read tools.
    std::string preloadSource(const std::set<std::string>& funcNames,
                              std::size_t charBudget) const;

    // Resolve one role reference from propose_race_hypothesis into a CCPG node
    // id: prefers `access_index` (into the current object's accesses, which
    // already carry node_id), then explicit `node_id`, then a source-ref
    // (file/line/symbol/snippet) via query::resolveSourceRef. Returns -1 on
    // failure and fills `err`.
    int resolveRoleNode(const nlohmann::json& site, std::string& err) const;

    // Per-session read/navigation budget. With the racing source preloaded, a
    // focused session needs few reads; this caps the deep-exploration tail that
    // otherwise lets one session run for many minutes. Reporting tools
    // (propose_race_hypothesis / finish_analysis) are never capped. The budget is
    // set per call: generous for inline-derivation/ablation (the session must build
    // contracts from source), TIGHT for calibration (Phase A already derived the
    // contracts and Phase B handed over the concrete candidates -- the session is a
    // reviewer, not an investigator -- but it keeps a few reads to resolve a doubt).
    int explore_calls_ = 0;
    int exploreSoft_ = 22;
    int exploreHard_ = 45;
    static constexpr int kInlineSoftBudget = 22;
    static constexpr int kInlineHardBudget = 45;
    static constexpr int kCalibrateSoftBudget = 5;
    static constexpr int kCalibrateHardBudget = 12;

    // Cross-session source cache. The same calAgent instance runs every session, so
    // a function read while analysing one thread-set is reused when a later
    // overlapping session touches the same thread: we record, per thread, the
    // function names the model read, and preload their bodies up front next time so
    // the model does not re-issue the same read. `curThreads_` is the set under
    // analysis in the active session (for attributing reads). Persists across
    // sessions (NOT cleared by reset()).
    std::map<int, std::set<std::string>> threadReadFuncs_;
    std::set<int> curThreads_;
};

// Per-session work memory passed via Conversation::context_for_tools. Used by
// both analyzeObject (single object) and analyzeThreadPair (multiple objects).
struct InterleavingContext {
    const query::SharedObject* object = nullptr;     // single-object mode only
    const query::VulnerabilitySurface* surface = nullptr;
    // Flat [A#] list the model references via `access_index`; spans all objects in
    // the session (one object in analyzeObject, the pair's objects in analyzeThreadPair).
    std::vector<const query::ThreadAccess*> accesses;
    std::vector<query::Hypothesis> confirmed;
    std::unordered_set<std::string> accepted_fingerprints;
    int total_tool_calls = 0;
};

} // namespace llm_client
