#pragma once
#include "nlohmann/json.hpp"
#include <set>
#include <string>

namespace llm_client {
// Extraction diagnostics only: no HB, synchronization claim or E discharge.
nlohmann::json contractFeedback(const nlohmann::json& claim,
    const nlohmann::json& model, const nlohmann::json& sourceOperations,
    const nlohmann::json& sourceManifest);
// Called after schema/operand validation. Full replacements preserve IDs and
// keep the previous and replacement records in an append-only revision history.
nlohmann::json storeContract(nlohmann::json& ledger,
    const nlohmann::json& arguments, bool requirement);
// Completion checks only accepted contracts and evidence actually inspected.
// Unknown is legal after inspection; this never proves extraction completeness.
std::string extractionState(const nlohmann::json& ledger);
nlohmann::json extractionCompletionIssues(const nlohmann::json& ledger,
    const nlohmann::json& model, const std::set<std::string>& inspectedSources,
    const std::string& inspectedDiagnosticState);
}
