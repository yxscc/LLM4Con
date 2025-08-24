#ifndef CONTRACTGENERATORAGENT_H
#define CONTRACTGENERATORAGENT_H

#include "LLMUtil/ConcurrencyContract.h"
#include "LLMUtil/Conversation.h"
#include <string>
#include <optional>

class CCPG;
class Thread;

namespace llm_client {
class ContractGeneratorAgent : public Conversation {
public:
    ContractGeneratorAgent(CCPG* ccpg, std::shared_ptr<LLMClient> client);

    // Generates a single contract for a given thread entry function
    std::optional<LLM::ConcurrencyContract> generateContractForThread(Thread* thread);

private:
    // Overrides from Conversation
    std::string execute_tool(const std::string& tool_name, const nlohmann::json& arguments) override;
    std::vector<Tool> get_available_tools() const override;
    std::string parseResult(const std::vector<ChatMessage>& history) override;
    static std::string build_system_prompt();

    CCPG* ccpg_;
};
}

#endif // CONTRACTGENERATORAGENT_H
