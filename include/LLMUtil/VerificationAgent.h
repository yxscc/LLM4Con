#ifndef VERIFICATION_AGENT_H
#define VERIFICATION_AGENT_H

#include "LLMUtil/Conversation.h"
#include "Query/StatefulBugDetector.h"
#include "Query/HypothesisVerifier.h"
#include <string>
#include <vector>
#include <memory>
#include <unordered_set>

class CCPG;

namespace llm_client {

class VerificationAgent : public Conversation {
public:
    VerificationAgent(std::shared_ptr<LLMClient> client);

    // Returns true if the bug is verified as a True Positive, false otherwise.
    bool verifyBug(const query::StatefulBug& bug);

    // Agent-mode counterpart of verifyBug: takes a constraint-confirmed
    // open Hypothesis (from DetectorAgent) and decides whether the LLM
    // believes it is a real bug. Returns false on FALSE_POSITIVE / no
    // verdict — caller should drop the hypothesis.
    bool verifyHypothesis(const query::Hypothesis& h, CCPG* ccpg);

protected:
    // Override from Conversation
    std::vector<Tool> get_available_tools() const override;
    std::string execute_tool(const std::string& tool_name, const nlohmann::json& arguments) override;

private:
    CCPG* ccpg_;
    const query::StatefulBug* current_bug_ = nullptr;
    const query::Hypothesis* current_hypothesis_ = nullptr;
    bool verdict_ = false; // Stores the final verdict
    bool has_verdict_ = false; // Flag to indicate if verdict is reached

    // Tracks (tool_name + canonical args) for the *current* hypothesis/bug
    // to short-circuit the LLM's tendency to redundantly re-call the same
    // tool with identical arguments and burn the round-trip budget.
    std::unordered_set<std::string> seen_tool_calls_;

    // Hard caps on the verifier's runaway behaviour. The base Conversation
    // class only prunes history; it has no real round-trip limit. We count
    // duplicate-tool warnings and total tool calls inside this verification
    // turn, and force-finish via the magic "finish" return string once
    // either threshold is hit so the loop in Conversation::send_message
    // exits cleanly. has_verdict_ stays false → default-KEEP kicks in.
    int dup_call_count_ = 0;
    int total_tool_calls_ = 0;
    static constexpr int MAX_DUP_CALLS = 5;
    static constexpr int MAX_TOTAL_TOOL_CALLS = 30;

    std::string buildVerificationSystemPrompt();
    std::string buildBugReportPrompt(const query::StatefulBug& bug);
    std::string buildHypothesisReportPrompt(const query::Hypothesis& h, CCPG* ccpg);
};

} // namespace llm_client

#endif // VERIFICATION_AGENT_H
