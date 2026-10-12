#pragma once
#include "nlohmann/json.hpp"
#include <string>
namespace llm_client {
bool sourceCoordinateMatches(const nlohmann::json& node,
    const nlohmann::json& operation, const nlohmann::json& sourceManifest);
// Debug coordinates + compiler access/callee facts. No instance/alias proof.
nlohmann::json bindSourceOperation(const nlohmann::json& operation,
    const nlohmann::json& model, const nlohmann::json& sourceManifest);
// Explicit Agent recovery of an unresolved correspondence. The selected IR
// instruction must belong to the source coordinates/function; semantics remain
// an Agent judgment. -1 withdraws an earlier selection. Does not mutate inputs.
nlohmann::json selectSourceIRBinding(const nlohmann::json& operation, int nodeID,
    const std::string& reason, const nlohmann::json& model,
    const nlohmann::json& sourceManifest);
}
