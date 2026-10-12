#pragma once
#include "nlohmann/json.hpp"

namespace llm_client {
// Extraction diagnostics only: no HB, synchronization claim or E discharge.
nlohmann::json contractFeedback(const nlohmann::json& claim,
    const nlohmann::json& model, const nlohmann::json& sourceOperations,
    const nlohmann::json& sourceManifest);
// Called after schema/operand validation. Full replacements preserve IDs and
// keep the previous and replacement records in an append-only revision history.
nlohmann::json storeContract(nlohmann::json& ledger,
    const nlohmann::json& arguments, bool requirement);
}
