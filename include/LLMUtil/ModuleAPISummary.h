#pragma once
#include "nlohmann/json.hpp"
#include <set>
#include <stdexcept>

namespace llm_client {
// A source-backed API precondition is evaluated at concrete module callsites.
// It does not require an object from the legacy shared-object surface, and is
// never automatically discharged by the legacy ordering/lock checker.
inline nlohmann::json instantiateAPISummary(const nlohmann::json& summary,
                                           const nlohmann::json& inventory) {
    using J = nlohmann::json;
    std::string api = summary.at("api"), confidence = summary.at("basis");
    if (confidence != "documented" && confidence != "inferred")
        throw std::invalid_argument("basis must be documented or inferred; neither means formally verified");
    if (summary.at("conditions").get<std::string>().empty() || summary.at("explanation").get<std::string>().empty())
        throw std::invalid_argument("Conditions and explanation are required");
    J calls = J::array(), resources = J::array(), requirements = J::array();
    std::set<int> nodes;
    for (const auto& c : inventory) if (c.at("api") == api) {
        calls.push_back(c); nodes.insert(c.at("node_id").get<int>());
    }
    if (calls.empty()) throw std::invalid_argument("API has no callsite in the module inventory");
    if (!summary.at("effects").is_array() || !summary.at("preconditions").is_array() || !summary.at("guarantees").is_array())
        throw std::invalid_argument("effects, preconditions and guarantees must be arrays");
    for (const auto& p : summary.at("preconditions"))
        if (!p.is_string() || p.get<std::string>().empty()) throw std::invalid_argument("Preconditions must be nonempty strings");
    for (const auto& g : summary.at("guarantees"))
        if (!g.is_string() || g.get<std::string>().empty()) throw std::invalid_argument("Guarantees must be nonempty strings");
    for (size_t i = 0; i < summary.at("effects").size(); ++i) {
        const auto& effect = summary.at("effects")[i];
        if (!effect.at("argument_index").is_number_integer()) throw std::invalid_argument("argument_index must be an integer");
        int argument = effect.at("argument_index");
        std::string access = effect.at("access"), path = effect.at("resource_path");
        if (argument < 0 || path.empty() || (access != "Read" && access != "Write" && access != "ReadWrite" && access != "Free" && access != "Escape" && access != "Unknown"))
            throw std::invalid_argument("Invalid argument-relative resource effect");
        for (const auto& c : calls) {
            J actual;
            for (const auto& a : c.at("arguments")) if (a.at("index") == argument) actual = a.at("code");
            if (actual.is_null()) throw std::invalid_argument("Argument index not available at every matching callsite; inspect get_module_calls");
            resources.push_back({{"id", "call:" + std::to_string(c.at("node_id").get<int>()) + ":effect:" + std::to_string(i)},
                {"api", api}, {"call_node", c.at("node_id")}, {"argument_index", argument}, {"actual_argument", actual},
                {"resource_path", path}, {"access", access}, {"origin", "llm_api_summary"},
                {"evidence_ids", summary.at("evidence_ids")}, {"alias_status", "not_established"},
                {"note", "Callsite-bound effect, not a proven shared-object identity or automatically expanded access pair."}});
        }
    }
    for (size_t i = 0; i < summary.at("preconditions").size(); ++i) {
        requirements.push_back({{"id", "api:" + api + ":precondition:" + std::to_string(i)},
            {"origin", "api_summary"}, {"form", "CallPrecondition"}, {"object_id", -1}, {"owner_thread", -1},
            {"api", api}, {"a_nodes", nodes}, {"b_nodes", J::array()},
            {"predicate", summary.at("preconditions")[i]}, {"conditions", summary.at("conditions")},
            {"evidence_ids", summary.at("evidence_ids")}, {"basis", confidence},
            {"note", "Check at all represented module callsites and feasible caller contexts. API documentation is a requirement, not a caller-side guarantee."}});
    }
    return {{"calls", calls}, {"resources", resources}, {"requirements", requirements}};
}
}
