#ifndef CONTRACTGENERATORAGENT_H
#define CONTRACTGENERATORAGENT_H

#include "LLMUtil/ConcurrencyContract.h"
#include "LLMUtil/Conversation.h"
#include "CCPG/CCPG.h"
#include <string>
#include <optional>
#include <set>


namespace llm_client {
class ContractGeneratorAgent : public llm_client::Conversation {
public:
    ContractGeneratorAgent(CCPG* ccpg, std::shared_ptr<llm_client::LLMClient> client);

    // Generates a single contract for a given thread entry function
    std::optional<LLM::ConcurrencyContract> generateContractForThread(CCPGNode * forkNode, int threadEntryPointNodeID);

    std::string get_generated_contract() const { return last_generated_contract_; }

private:
    // Overrides from Conversation
    std::string execute_tool(const std::string& tool_name, const nlohmann::json& arguments) override;
    std::vector<Tool> get_available_tools() const override;
    std::string parseResult(const std::vector<llm_client::ChatMessage>& history) override;
    static std::string build_system_prompt();

    // Tool implementations
    std::string get_function_body(const nlohmann::json& args);
    std::string confirm_contract(const nlohmann::json& args);

    CCPG* ccpg;
    std::string last_generated_contract_;
};
}

#endif // CONTRACTGENERATORAGENT_H
