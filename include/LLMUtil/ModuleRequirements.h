#pragma once
#include "nlohmann/json.hpp"
#include <algorithm>
#include <map>
#include <set>
#include <string>

namespace llm_client {
// Factorized obligations: O(thread/object/access buckets), not a list of access
// pairs. Every operand set is retained; no top-K or representative sampling.
// These are conditional safety obligations over MAY-alias surface objects.
// Applicability (same allocation/lifetime, feasible execution) remains explicit.
inline nlohmann::json buildModuleRequirements(const nlohmann::json& surface) {
    using J = nlohmann::json;
    J result = {{"requirements", J::array()}, {"unanchored_accesses", J::array()}};
    size_t oid = 0;
    std::set<int> reentrant;
    if (surface.contains("threads")) for (const auto& thread : surface.at("threads"))
        if (thread.value("reentrant_entry", false)) reentrant.insert(thread.at("thread_id").get<int>());
    for (const auto& object : surface.at("shared_objects")) {
        std::map<int, std::map<std::string, std::set<int>>> buckets;
        size_t ai = 0;
        for (const auto& access : object.at("accesses")) {
            int node = access.value("node_id", -1);
            if (node < 0) result["unanchored_accesses"].push_back({{"object_id", oid}, {"access_index", ai}});
            else buckets[access.at("thread_id").get<int>()][access.at("access_type").get<std::string>()].insert(node);
            ++ai;
        }
        auto add = [&](int tid, const std::string& form, const std::set<int>& a,
                       const std::set<int>& b, const std::string& category) {
            if (a.empty() || b.empty()) return;
            result["requirements"].push_back({{"id", "static:" + std::to_string(oid) + ":" + std::to_string(tid) + ":" + category},
                {"origin", "static"}, {"form", form}, {"object_id", oid}, {"owner_thread", tid},
                {"a_nodes", a}, {"b_nodes", b}, {"category", category},
                {"applicability", "same object instance and lifetime; jointly feasible concurrent activations"},
                {"note", "Universally quantified over the retained operand sets; applicability is not proven."}});
        };
        for (const auto& [tid, own] : buckets) {
            std::set<int> uses, writes, frees, otherUses, otherWrites;
            for (const auto& [type, nodes] : own) {
                if (type != "Free") uses.insert(nodes.begin(), nodes.end());
                if (type == "Write" || type == "ReadWrite") writes.insert(nodes.begin(), nodes.end());
            }
            for (const auto& [otherTid, other] : buckets) {
                if (otherTid == tid && !object.value("is_self_race", false) && !reentrant.count(tid)) continue;
                for (const auto& [type, nodes] : other) {
                    if (type == "Free") frees.insert(nodes.begin(), nodes.end());
                    else otherUses.insert(nodes.begin(), nodes.end());
                    if (type == "Write" || type == "ReadWrite") otherWrites.insert(nodes.begin(), nodes.end());
                }
            }
            add(tid, "MustPrecede", uses, frees, "lifetime");
            add(tid, "MustBeMediated", writes, otherUses, "write_conflict");
            // A read-only thread still receives the dual obligation. Duplicate
            // directions are retained as context-specific coverage, not sessions.
            add(tid, "MustBeMediated", uses, otherWrites, "read_write_conflict");
        }
        ++oid;
    }
    return result;
}
}
