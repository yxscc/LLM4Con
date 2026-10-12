#include "LLMUtil/ModuleAnalysis.h"
#include "LLMUtil/ModuleContractAgent.h"
#include "LLMUtil/RequirementChecker.h"
#include "Util/TargetPath.h"
#include <exception>
#include <fstream>
#include <string>
namespace llm_client {
void runModuleAnalysis(CCPG* ccpg, std::shared_ptr<LLMClient> client, bool staticOnly, bool noReview) {
    const auto model = ccpg->moduleModel();
    const auto output = TargetPath::getInstance()->getOutputDir();
    std::ofstream(output/"module_model.json") << model.dump(2);
    if (staticOnly) return;
    ModuleContractAgent agent(std::move(client),ccpg,model);
    try {
        agent.generate();
    } catch (const std::exception& e) {
        // Budget exhaustion does not invalidate contracts already accepted by
        // the tools. Run E on that partial ledger, retain the failure/coverage
        // gap, and never start Phase C or claim extraction completed.
        if (std::string(e.what()).find("MaxTurnsExceeded")==std::string::npos) throw;
        auto checks=checkModuleRequirements(agent.requirements(),agent.guarantees(),model,agent.sourceOperations(),agent.sourceManifest());
        agent.finishWithoutReview(checks);agent.save(e.what());
        query::writeModuleFindings(agent.findings(),ccpg,output);
        throw;
    }
    auto checks = checkModuleRequirements(agent.requirements(),agent.guarantees(),model,agent.sourceOperations(),agent.sourceManifest());
    if (noReview) agent.finishWithoutReview(checks);
    else agent.review(checks);
    query::writeModuleFindings(agent.findings(),ccpg,output);
}
}
