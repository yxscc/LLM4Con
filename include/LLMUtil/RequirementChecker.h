#pragma once
#include "nlohmann/json.hpp"
namespace llm_client {
// Pure, deterministic sufficient-discharge rules over the complete input model.
// Gua semantics/instance scope are explicit premises, not independently proved.
// All results, including discharges, undergo source review in the same agent.
nlohmann::json checkModuleRequirements(const nlohmann::json& requirements,
    const nlohmann::json& guarantees, const nlohmann::json& model,
    const nlohmann::json& sourceOperations = nlohmann::json::object());
}
