#include "LLMUtil/ModuleAPISummary.h"
#include <cassert>
#include <iostream>
using J = nlohmann::json;
int main() {
    J calls = J::array({
        {{"api", "external_access"}, {"node_id", 10}, {"arguments", {{{"index", 0}, {"code", "&a->cache"}}}}},
        {{"api", "external_access"}, {"node_id", 20}, {"arguments", {{{"index", 0}, {"code", "&b->cache"}}}}},
        {{"api", "other"}, {"node_id", 30}, {"arguments", J::array()}}
    });
    J summary = {{"api", "external_access"}, {"basis", "documented"}, {"conditions", "Valid object lifetime"},
        {"explanation", "Offline summary binding test"}, {"evidence_ids", {"src:test"}},
        {"effects", {{{"argument_index", 0}, {"resource_path", "pointed-to object"}, {"access", "ReadWrite"}}}},
        {"preconditions", {"Caller must supply protection"}}, {"guarantees", J::array()}};
    auto result = llm_client::instantiateAPISummary(summary, calls);
    assert(result["requirements"].size() == 1 && result["resources"].size() == 2);
    const auto& req = result["requirements"][0];
    assert(req["object_id"] == -1 && req["form"] == "CallPrecondition");
    assert(req["a_nodes"] == J::array({10, 20}));
    assert(result["resources"][0]["actual_argument"] == "&a->cache");
    assert(result["resources"][1]["actual_argument"] == "&b->cache");
    assert(result["resources"][0]["alias_status"] == "not_established");
    for (const auto& field : {"argument", "basis", "api"}) {
        auto invalid = summary;
        if (std::string(field) == "argument") invalid["effects"][0]["argument_index"] = 1;
        else if (std::string(field) == "basis") invalid["basis"] = "proven";
        else invalid["api"] = "missing";
        bool rejected = false;
        try { llm_client::instantiateAPISummary(invalid, calls); }
        catch (const std::invalid_argument&) { rejected = true; }
        assert(rejected);
    }
    summary["preconditions"] = J::array();
    assert(llm_client::instantiateAPISummary(summary, calls)["requirements"].empty());
    std::cout << "API summary binding tests passed\n";
}
