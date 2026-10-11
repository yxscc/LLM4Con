#pragma once
#include "nlohmann/json.hpp"
#include <functional>
namespace llm_client {
// Static adapters are injected, so the checking policy can be tested without
// LLVM/Joern or a model. Every result remains a hint for mandatory agent review.
struct CheckerRelations {
    std::function<bool(int,int)> control_reachable;
    std::function<bool(int,int)> synchronized_order;
};
nlohmann::json checkModuleRequirements(const nlohmann::json& requirements,
    const nlohmann::json& guarantees, const nlohmann::json& surface,
    const CheckerRelations& relations);
}
