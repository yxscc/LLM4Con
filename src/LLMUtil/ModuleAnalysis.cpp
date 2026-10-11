#include "LLMUtil/ModuleAnalysis.h"
#include "LLMUtil/ModuleContractAgent.h"
#include "LLMUtil/RequirementChecker.h"
#include "CCPG/HBGraph.h"
#include "Util/TargetPath.h"
#include <fstream>
namespace llm_client {
void runModuleAnalysis(CCPG* ccpg, std::shared_ptr<LLMClient> client, bool staticOnly) {
    auto surface = query::VulnerabilitySurfaceGenerator(ccpg,ThreadCreationTree::getInstance()).generate();
    const auto output = TargetPath::getInstance()->getOutputDir();
    const auto json = surface.toJson();
    std::ofstream(output/"vulnerability_surface.json") << json.dump(2);
    if (staticOnly) return;
    ModuleContractAgent agent(std::move(client),ccpg,surface);
    agent.generate();
    auto hb = HBGraph::getInstance();
    CheckerRelations relations {
        [=](int a,int b) { return hb->hbReachable(ccpg->getNodeByID(a),ccpg->getNodeByID(b),0,false); },
        [=](int a,int b) { return !ccpg->contextsIncomplete() && hb->hbReachable(ccpg->getNodeByID(a),ccpg->getNodeByID(b),0,true,HBGraph::SyncPolicy::StructuralOnly); }
    };
    auto checks = checkModuleRequirements(agent.requirements(),agent.guarantees(),json,relations);
    agent.review(checks);
    query::writeModuleFindings(agent.findings(),ccpg,output);
}
}
