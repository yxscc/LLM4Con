#ifndef CONTRACTGENERATORAGENT_H
#define CONTRACTGENERATORAGENT_H

#include "LLMUtil/ConcurrencyContract.h"
#include "LLMUtil/Conversation.h"
#include "Query/VulnerabilitySurfaceGenerator.h"
#include <set>
#include <string>
#include <optional>
#include <unordered_map>
#include <vector>

class CCPG;
class Thread;

namespace llm_client {
class ContractGeneratorAgent : public Conversation {
public:
    ContractGeneratorAgent(CCPG* ccpg, std::shared_ptr<LLMClient> client);

    // Generates a single contract for a given thread entry function.
    //
    // `touchedObjects` (optional) are the shared objects this thread actually
    // accesses, already computed by the static vulnerability surface. When
    // supplied, the prompt is SEEDED with those objects, the thread's own
    // accesses (function/line/lock/snippet), and the preloaded source of the
    // accessing functions -- so the model rarely needs the call-graph read tools
    // and the per-contract exploration budget is tightened. When empty, the agent
    // falls back to the original blind call-graph exploration (legacy path).
    // `objectIds` (optional) is parallel to `touchedObjects`: objectIds[k] is the
    // surface shared-object index of touchedObjects[k]. When supplied, each object
    // is labelled `[obj#N]` in the prompt and the model is asked to set
    // report_clause.object_id, so the static-composition phase can match a clause
    // to its surface object deterministically (no fuzzy name matching).
    std::optional<LLM::ConcurrencyContract> generateContractForThread(
        Thread* thread,
        const std::vector<const query::SharedObject*>& touchedObjects = {},
        const std::vector<int>& objectIds = {});

private:
    // Overrides from Conversation
    std::string execute_tool(const std::string& tool_name, const nlohmann::json& arguments) override;
    std::vector<Tool> get_available_tools() const override;
    std::string parseResult(const std::vector<ChatMessage>& history) override;
    static std::string build_system_prompt();

    // Concatenate the source bodies of the named functions (deduped) up to a char
    // budget, for one-shot preloading into the contract prompt.
    std::string preloadSource(const std::set<std::string>& funcNames,
                              std::size_t charBudget) const;

    // Contract COMPLETENESS pass. After the initial generation, drive up to a few
    // focused repair rounds so that every HIGH-RISK surface object this thread
    // touches (unprotected write / free / list mutation / self-race) is explicitly
    // addressed -- either with a real assume/guarantee or an explicit
    // no_order_needed declaration. A silently-omitted dangerous object is the recall
    // hole that leaves Phase B static composition with no candidate for the real bug.
    // Seeded mode only; gated by coverageEnabled().
    void repairContractCoverage(LLM::ConcurrencyContract& contract, int tid,
                                const std::vector<const query::SharedObject*>& touchedObjects,
                                const std::vector<int>& objectIds);

    // Per-call cap on contract-completeness repair rounds (bounds cost).
    static constexpr int kCoverageRepairRounds = 2;

    CCPG* ccpg_;

    // Per-contract exploration (read/navigation) budget. The base send_message
    // loop has no round cap, so a thread whose call graph is large can make the
    // model read functions forever without ever emitting clauses. We count only
    // the shared read/navigation tools (report_clause/confirm/finalize are never
    // capped): past SOFT we nudge it to stop reading and emit clauses; past HARD
    // we deterministically end the session. The budget is set per call:
    // tight when the prompt is seeded from the surface, generous for the blind
    // fallback. Reset per generateContractForThread.
    int explore_calls_ = 0;
    int exploreSoft_ = 30;
    int exploreHard_ = 60;
    static constexpr int kSeededSoftBudget = 8;
    static constexpr int kSeededHardBudget = 16;
    static constexpr int kBlindSoftBudget = 30;
    static constexpr int kBlindHardBudget = 60;

    // Reporting-loop guard. report_clause/confirm_role/report_ordering are NOT
    // subject to the read budget, so a thread seeded with many objects can loop
    // forever: the token window prunes the clauses the model already emitted, so it
    // re-sees the full (pinned) task each turn, restarts (re-calling confirm_role),
    // and never finalizes. Contracts are now SELECTIVE (a clause only for resources
    // with a real order/sync obligation), so we no longer push the model to cover
    // every object; we just track the seeded object count and the number of reporting
    // rounds and hard-stop the session past a cap (the safety net for a runaway loop).
    std::set<int> seededObjectIds_;
    std::unordered_map<int, const query::SharedObject*> seededObjectsById_;
    int reportRounds_ = 0;
    int reportHardCap() const {
        int n = static_cast<int>(seededObjectIds_.size());
        int cap = n + 20;
        if (cap < 30) cap = 30;
        if (cap > 70) cap = 70;
        return cap;
    }
};
}

#endif // CONTRACTGENERATORAGENT_H
