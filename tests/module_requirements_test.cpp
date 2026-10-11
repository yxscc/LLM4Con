#include "LLMUtil/ModuleRequirements.h"
#include <iostream>
#include <stdexcept>
using J = nlohmann::json;
static void check(bool value, const char* message) { if (!value) throw std::runtime_error(message); }
static J access(int t, int n, const char* type) { return {{"thread_id", t}, {"node_id", n}, {"access_type", type}}; }
int main() {
    J surface = {{"shared_objects", J::array()}};
    for (int i = 0; i < 35; ++i) surface["shared_objects"].push_back({{"accesses", {access(1, 2*i, "Read"), access(2, 2*i+1, "Free")}}});
    auto all = llm_client::buildModuleRequirements(surface);
    check(all["requirements"].size() == 35, "objects after the old top-20 boundary disappeared");
    J uses = J::array(); for (int i = 0; i < 120; ++i) uses.push_back(access(1, i, "Read"));
    uses.push_back(access(2, 999, "Free"));
    surface["shared_objects"] = J::array({{{"accesses", uses}}});
    all = llm_client::buildModuleRequirements(surface);
    check(all["requirements"].size() == 1, "pair enumeration instead of a quantified lifetime group");
    check(all["requirements"][0]["a_nodes"].size() == 120, "access sampling lost uses");
    check(all["requirements"][0]["b_nodes"] == J::array({999}), "use/free ordering reversed");
    surface["shared_objects"] = J::array({{{"accesses", {access(1, 1, "Read"), access(1, 2, "Free")}}}});
    check(llm_client::buildModuleRequirements(surface)["requirements"].empty(), "invented cross-context concurrency");
    surface["shared_objects"][0]["is_self_race"] = true;
    check(llm_client::buildModuleRequirements(surface)["requirements"].size() == 1, "lost reentrant lifetime obligation");
    surface["shared_objects"][0]["is_self_race"] = false;
    surface["threads"] = J::array({{{"thread_id", 1}, {"reentrant_entry", true}}});
    check(llm_client::buildModuleRequirements(surface)["requirements"].size() == 1, "per-entry reentrancy lost behind object self-race flag");
    // Two activations of one callback remain relevant even when another entry
    // touches the same object (the CAN RX/TX field-merging failure).
    surface["shared_objects"][0]["accesses"] = {access(1, 10, "Write"), access(1, 11, "Read"), access(2, 12, "Read")};
    all = llm_client::buildModuleRequirements(surface);
    bool selfPair = false;
    for (const auto& r : all["requirements"]) if (r["owner_thread"] == 1 && r["a_nodes"] == J::array({10}))
        selfPair = std::find(r["b_nodes"].begin(), r["b_nodes"].end(), J(11)) != r["b_nodes"].end();
    check(selfPair, "same-entry write/read disappeared in a multi-entry object");
    surface.erase("threads");
    surface["shared_objects"][0]["accesses"] = {access(1, 1, "Read"), access(1, 2, "Free")};
    surface["shared_objects"][0]["accesses"].push_back(access(2, -1, "Write"));
    check(llm_client::buildModuleRequirements(surface)["unanchored_accesses"].size() == 1, "silent loss of missing node anchor");
    surface["shared_objects"] = J::array({{{"accesses", {access(1, 1, "Read"), access(2, 2, "Read")}}}});
    check(llm_client::buildModuleRequirements(surface)["requirements"].empty(), "read/read is not a data race");
    surface["shared_objects"][0]["accesses"][1]["access_type"] = "Write";
    check(!llm_client::buildModuleRequirements(surface)["requirements"].empty(), "missing read/write obligation");
    std::cout << "module requirement coverage tests passed\n";
}
