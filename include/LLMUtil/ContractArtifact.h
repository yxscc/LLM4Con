#pragma once
#include "nlohmann/json.hpp"
#include <functional>

namespace llm_client {
// Agent-facing source locators. IDs and canonical E operands stay backend-owned.
nlohmann::json artifactOperationArguments(const nlohmann::json&);
nlohmann::json translateContractArtifact(const nlohmann::json&,
    const std::function<std::string(const nlohmann::json&)>& registerOperation);
}
