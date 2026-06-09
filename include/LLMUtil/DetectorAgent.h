#pragma once

#include "LLMUtil/Conversation.h"
#include "LLMUtil/Rule.h"
#include "LLMUtil/ThreadPair.h"
#include "Query/VulnerabilitySurfaceGenerator.h"
#include "Query/HypothesisVerifier.h"
#include "CCPG/CCPG.h"
#include "CCPG/ThreadCreationTree.h"
#include <memory>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <set>
#include <string>

namespace llm_client {

struct DetectorContext {
    // Mechanism-first rule-instantiation mode. The LLM must first submit
    // structured context analyses, then instantiate intent-level rules that
    // are grounded through HypothesisVerifier internally.
    std::vector<query::Hypothesis> confirmed_hypotheses;
    std::vector<nlohmann::json> context_analyses;
    query::HypothesisVerifier* verifier = nullptr;

    // Phase 4 dedupe: cache of already-accepted rule fingerprints of the form
    // "mechanism|sorted_node_ids" to prevent trivial variants of the same
    // underlying rule from flooding the report.
    std::unordered_set<std::string> accepted_fingerprints;

    const query::VulnerabilitySurface* surface = nullptr;
    CCPG* ccpg = nullptr;
    ThreadCreationTree* tct = nullptr;

    // External work memory for short-history ReAct runs. Conversation history
    // is intentionally pruned, so the detector keeps a compact ledger here and
    // injects it into the dynamic system prompt before each LLM turn.
    int total_tool_calls = 0;
    bool stop_requested = false;
    std::string stop_reason;
    bool surface_full_returned = false;
    std::unordered_map<std::string, int> tool_counts;
    std::unordered_map<int, std::string> object_decisions;
    std::unordered_map<int, std::string> object_mechanisms;
    std::set<int> inspected_objects;
    std::set<std::string> function_code_reads;
    std::set<int> function_ops_reads;
    std::set<int> lock_nodes_checked;
};

class DetectorAgent : public Conversation {
public:
    DetectorAgent(std::shared_ptr<LLMClient> client, CCPG* ccpg);

    struct DetectionResult {
        std::vector<query::Hypothesis> confirmed;
    };

    DetectionResult runDetection(const query::VulnerabilitySurface& surface);

private:
    CCPG* ccpg_;

    std::string build_system_prompt();
    std::string build_effective_system_prompt() override;
    std::string build_progress_memo(const DetectorContext* ctx) const;
    std::vector<Tool> get_available_tools() const override;
    std::string execute_tool(const std::string& tool_name, const nlohmann::json& arguments) override;
    std::string get_tool_choice() const override { return "required"; }
    std::string parseResult(const std::vector<ChatMessage>& history) override;
};

} // namespace llm_client
