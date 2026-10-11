#pragma once
#include "LLMUtil/Conversation.h"
#include "LLMUtil/ConcurrencyContract.h"
#include "Query/VulnerabilitySurfaceGenerator.h"
#include "Query/HypothesisVerifier.h"
#include <optional>

namespace llm_client {
class ModuleContractAgent : public Conversation {
public:
    ModuleContractAgent(std::shared_ptr<LLMClient> client, CCPG* ccpg,
                        const query::VulnerabilitySurface& surface);
    void generate();
    std::map<int, LLM::ConcurrencyContract> contracts() const;
    const nlohmann::json& requirements() const { return data_.at("requirements"); }
    void review(const nlohmann::json& checks);
    std::vector<query::Hypothesis> findings() const;
    void save(const std::string& error = "") const;
private:
    CCPG* ccpg_;
    const query::VulnerabilitySurface& surface_;
    nlohmann::json data_, surfaceJson_;
    std::set<std::string> functions_, coveredFunctions_;
    std::map<std::string, std::vector<std::string>> sourceLines_;
    bool generating_ = true, generationFinished_ = false, reviewFinished_ = false;
    std::string sourcePacket();
    std::string execute_tool(const std::string&, const nlohmann::json&) override;
    std::vector<Tool> get_available_tools() const override;
    // Both phases terminate through a validated finalize_* tool, not prose.
    std::string get_tool_choice() const override { return "required"; }
    std::vector<int> validNodes(const nlohmann::json&, const char*, bool required = true) const;
    void initializeRepositoryContext();
    nlohmann::json buildCallInventory() const;
    std::vector<Tool> repositoryTools() const;
    std::optional<std::string> executeRepositoryTool(const std::string&, const nlohmann::json&);
    void validRepositoryEvidence(const nlohmann::json&, bool required) const;
    nlohmann::json sourceOperation(const nlohmann::json&);
    std::vector<std::string> validSources(const nlohmann::json&, const char*) const;
    bool hasSourceFunction(const std::string&) const;
};
bool moduleContractsEnabled();
}
