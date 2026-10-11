#pragma once
#include "LLMUtil/LLMClient.h"
class CCPG;
namespace llm_client {
void runModuleAnalysis(CCPG* ccpg, std::shared_ptr<LLMClient> client, bool staticOnly = false);
}
