#pragma once
#include "nlohmann/json.hpp"
namespace llm_client {
bool sourceCoordinateMatches(const nlohmann::json& node,
    const nlohmann::json& operation, const nlohmann::json& sourceManifest);
// Debug coordinates + compiler access/callee facts. No instance/alias proof.
nlohmann::json bindSourceOperation(const nlohmann::json& operation,
    const nlohmann::json& model, const nlohmann::json& sourceManifest);
}
