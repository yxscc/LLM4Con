#ifndef VERIFICATION_AGENT_H
#define VERIFICATION_AGENT_H

#include "LLMUtil/Conversation.h"
#include "Query/StatefulBugDetector.h"
#include <string>
#include <vector>
#include <memory>

class CCPG;

namespace llm_client {

class VerificationAgent : public Conversation {
public:
    VerificationAgent(std::shared_ptr<LLMClient> client);

    // Returns true if the bug is verified as a True Positive, false otherwise.
    bool verifyBug(const query::StatefulBug& bug);

protected:
    // Override from Conversation
    std::vector<Tool> get_available_tools() const override;
    std::string execute_tool(const std::string& tool_name, const nlohmann::json& arguments) override;

private:
    CCPG* ccpg_;
    const query::StatefulBug* current_bug_ = nullptr;
    bool verdict_ = false; // Stores the final verdict
    bool has_verdict_ = false; // Flag to indicate if verdict is reached

    std::string buildVerificationSystemPrompt();
    std::string buildBugReportPrompt(const query::StatefulBug& bug);
};

} // namespace llm_client

#endif // VERIFICATION_AGENT_H
