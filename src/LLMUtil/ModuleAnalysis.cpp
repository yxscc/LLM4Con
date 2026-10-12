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
        // Extraction errors do not invalidate contracts already accepted by
        // the tools. Check the partial ledger without starting Phase C, retain
        // the incomplete state, and propagate the original extraction error.
        const std::string extractionError=e.what();
        try {
            auto checks=checkModuleRequirements(agent.requirements(),agent.guarantees(),model,agent.sourceOperations(),agent.sourceManifest());
            agent.finishWithoutReview(checks);agent.save(extractionError);
            query::writeModuleFindings(agent.findings(),ccpg,output);
        } catch (const std::exception& partialError) {
            // A failed fallback must not replace the original failure. Preserve
            // its diagnostic and the actual E state when storage permits.
            try { agent.save(extractionError+"\nPartial E/report recovery failed: "+partialError.what()); }
            catch (...) {} // Preserve the original exception even if saving fails.
        }
        throw;
    }
    auto checks = checkModuleRequirements(agent.requirements(),agent.guarantees(),model,agent.sourceOperations(),agent.sourceManifest());
    if (noReview) agent.finishWithoutReview(checks);
    else agent.review(checks);
    query::writeModuleFindings(agent.findings(),ccpg,output);
}
}
