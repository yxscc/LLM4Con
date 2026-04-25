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
#include <string>

namespace llm_client {

struct DetectorContext {
    // Open hypothesis mode
    std::vector<query::Hypothesis> confirmed_hypotheses;
    query::HypothesisVerifier* verifier = nullptr;

    // Phase 4 dedupe: cache of already-accepted hypothesis fingerprints of
    // the form "bug_category|sorted_node_ids" to prevent the LLM from
    // flooding us with trivial variants of the same underlying hypothesis.
    std::unordered_set<std::string> accepted_fingerprints;

    const query::VulnerabilitySurface* surface;
    CCPG* ccpg;
    ThreadCreationTree* tct;
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
    std::vector<Tool> get_available_tools() const override;
    std::string execute_tool(const std::string& tool_name, const nlohmann::json& arguments) override;
    std::string get_tool_choice() const override { return "required"; }
    std::string parseResult(const std::vector<ChatMessage>& history) override;
};

} // namespace llm_client
