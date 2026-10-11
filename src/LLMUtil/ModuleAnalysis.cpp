#include "LLMUtil/ModuleAnalysis.h"
#include "LLMUtil/ModuleContractAgent.h"
#include "LLMUtil/RequirementChecker.h"
#include "Util/TargetPath.h"
#include <fstream>
namespace llm_client {
void runModuleAnalysis(CCPG* ccpg, std::shared_ptr<LLMClient> client, bool staticOnly) {
    const auto model = ccpg->moduleModel();
    const auto output = TargetPath::getInstance()->getOutputDir();
    std::ofstream(output/"module_model.json") << model.dump(2);
    if (staticOnly) return;
    ModuleContractAgent agent(std::move(client),ccpg,model);
    agent.generate();
    auto checks = checkModuleRequirements(agent.requirements(),agent.guarantees(),model,agent.sourceOperations());
    agent.review(checks);
    query::writeModuleFindings(agent.findings(),ccpg,output);
}
}
