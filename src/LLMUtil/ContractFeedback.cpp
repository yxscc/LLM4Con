#include "LLMUtil/ContractFeedback.h"
#include "LLMUtil/OperationBinding.h"
#include <map>
#include <queue>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

namespace llm_client {
using J = nlohmann::json;
J contractFeedback(const J& claim, const J& model, const J& sources,
                   const J& manifest) {
    std::map<int, J> nodes, functions;
    std::map<int, std::vector<int>> next;
    for (const auto& n : model.at("nodes")) nodes.emplace(n.at("node_id"), n);
    for (const auto& f : model.at("functions")) functions.emplace(f.at("function_id"), f);
    for (const auto& e : model.at("cfg_edges")) {
        int a = e.at(0), b = e.at(1);
        if (nodes.count(a) && nodes.count(b) &&
            nodes.at(a).at("function_id") == nodes.at(b).at("function_id")) next[a].push_back(b);
    }
    auto available = [&](int n) {
        if (!nodes.count(n) || nodes.at(n).at("origin") != "ir") return false;
        int f = nodes.at(n).at("function_id");
        return functions.count(f) && functions.at(f).value("cfg_complete", false);
    };
    auto reachable = [&](int from, int to) {
        std::queue<int> todo; std::set<int> seen;
        todo.push(from); seen.insert(from);
        while (!todo.empty()) {
            int n = todo.front(); todo.pop();
            if (n == to) return true;
            for (int x : next[n]) if (seen.insert(x).second) todo.push(x);
        }
        return false;
    };
    J feedback = {{"bindings", J::array()}, {"sequence_pairs", J::array()},
        {"semantics_verified", false}, {"synchronization_established", false}};
    std::vector<int> sequence;
    auto sourceBinding = [&](J& row, const J& id) {
        int n = -1;
        if (!sources.contains(id.get<std::string>())) row["status"] = "unknown_source_id";
        else {
            auto binding = bindSourceOperation(sources.at(id.get<std::string>()), model, manifest);
            row["status"] = binding.at("status");
            row["semantic_match_count"] = binding.at("matching_candidates").size();
            if (binding.contains("binding_origin")) {
                row["binding_origin"]=binding.at("binding_origin");
                row["automatic_status"]=binding.at("automatic_status");
                row["selected_node_id"]=binding.at("node_id");
                if (binding.contains("selection_reason")) row["selection_reason"]=binding.at("selection_reason");
                if (binding.contains("selection_error")) row["selection_error"]=binding.at("selection_error");
            }
            if (binding.at("status") == "bound") n = binding.at("node_id");
            row["cfg_available"] = n >= 0 && available(n);
        }
        return n >= 0 && available(n) ? n : -1;
    };
    auto addOperand = [&](const char* prefix) {
        std::string numeric = std::string(prefix) + "_nodes", source = std::string(prefix) + "_sources";
        size_t index = 0;
        for (const auto& id : claim.value(numeric, J::array())) {
            int n = id;
            feedback["bindings"].push_back({{"operand", prefix}, {"index", index++},
                {"operation_id", n}, {"status", available(n) ? "bound_ir" : "graph_unavailable"}});
            if (std::string(prefix) == "a") sequence.push_back(available(n) ? n : -1);
        }
        for (const auto& id : claim.value(source, J::array())) {
            J row = {{"operand", prefix}, {"index", index++}, {"source_id", id}};
            int n = sourceBinding(row, id);
            feedback["bindings"].push_back(row);
            if (std::string(prefix) == "a") sequence.push_back(n);
        }
    };
    for (const char* prefix : {"a", "b", "interference", "evidence"}) addOperand(prefix);
    size_t regionIndex = 0;
    for (const auto& region : claim.value("source_regions", J::array())) {
        for (const char* endpoint : {"start_source", "end_source"}) {
            J row = {{"operand", endpoint}, {"index", regionIndex}, {"source_id", region.at(endpoint)}};
            sourceBinding(row, region.at(endpoint)); feedback["bindings"].push_back(row);
        }
        ++regionIndex;
    }
    if (claim.at("form") == "MustBeAtomic") {
        for (size_t i = 1; i < sequence.size(); ++i) {
            int a = sequence[i-1], b = sequence[i];
            std::string status;
            if (a < 0 || b < 0) status = "unbound_or_incomplete_cfg";
            else if (nodes.at(a).at("function_id") != nodes.at(b).at("function_id")) status = "interprocedural_not_checked";
            else if (a == b) status = "same_resolved_operation";
            else if (!reachable(a, b)) status = "no_local_path";
            else if (reachable(b, a)) status = "cyclic_local_path";
            else status = "local_path";
            J pair = {{"from_index", i-1}, {"to_index", i}, {"status", status}};
            if (a >= 0 && b >= 0) pair["same_resolved_operation"] = a == b;
            // Reachability alone is not the prefix order used by E. Diagnose
            // compiler-arm bypasses while the Agent can still correct anchors.
            std::string order = "not_established";
            if (status == "local_path") {
                const auto& fn = functions.at(nodes.at(b).at("function_id"));
                if (!fn.contains("entry")) order = "cfg_entry_unavailable";
                else {
                    int entry = fn.at("entry");
                    std::queue<int> todo; std::set<int> seen;
                    if (entry != a) { todo.push(entry); seen.insert(entry); }
                    bool bypass = false;
                    while (!todo.empty()) {
                        int n = todo.front(); todo.pop();
                        if (n == b) { bypass = true; break; }
                        for (int x : next[n]) if (x != a && seen.insert(x).second) todo.push(x);
                    }
                    order = !reachable(entry,b) ? "unreachable_from_entry" :
                        bypass ? "not_dominating" : "established";
                }
            } else if (status == "cyclic_local_path") order = "occurrence_order_unsupported";
            else if (status == "same_resolved_operation") order = "same_resolved_operation";
            pair["e_sequence_order"] = order;
            feedback["sequence_pairs"].push_back(pair);
        }
        feedback["sequence_interpretation"] = "Local reachability in one function invocation only. A local_path is not must-order or HB. same_resolved_operation identifies one IR effect; different read/write effects may share one source line. cyclic_local_path concerns distinct effects and does not establish iteration order: it is not a duplicate-step diagnosis. Inspect no_local_path for alternative branches or distinct helper invocations; retain unsupported context as a gap.";
    }
    if (claim.contains("unconditional")) {
        feedback["declared_applicability"] = claim.at("unconditional").get<bool>() ? "established_within_declared_scope" : "unresolved_by_agent";
        feedback["applicability_interpretation"] = "This flag is the Agent's declaration, not a backend semantic judgment. A stated same-instance/protection domain is not itself an unresolved premise. False requires a concrete unresolved premise; true still requires actual scoped synchronization evidence.";
    }
    return feedback;
}

std::string extractionState(const J& ledger) {
    return J{{"requirements",ledger.at("requirements")},
        {"guarantees",ledger.at("guarantees")},
        {"source_operations",ledger.at("source_operations")}}.dump();
}
J extractionCompletionIssues(const J& ledger,const J& model,
    const std::set<std::string>& inspectedSources,const std::string& diagnosticState) {
    J issues=J::array();
    if (diagnosticState != extractionState(ledger)) issues.push_back({
        {"kind","current_contract_diagnostics_not_inspected"},
        {"tool","get_contract_diagnostics"},{"ids",J::array()}});
    std::map<std::string,std::set<std::string>> references;
    for (const char* kind:{"requirements","guarantees"}) for (const auto& claim:ledger.at(kind)) {
        for (const char* key:{"a_sources","b_sources","interference_sources","evidence_sources"})
            for (const auto& source:claim.value(key,J::array())) references[source.get<std::string>()].insert(claim.at("id"));
        for (const auto& region:claim.value("source_regions",J::array()))
            for (const char* key:{"start_source","end_source"}) references[region.at(key).get<std::string>()].insert(claim.at("id").get<std::string>());
    }
    for (const auto& [id,contracts]:references) {
        if (!ledger.at("source_operations").contains(id)) continue; // schema validation handles absent IDs
        const auto& op=ledger.at("source_operations").at(id);
        auto binding=bindSourceOperation(op,model,ledger.at("source_manifest"));
        if (binding.at("status")=="bound"||binding.at("coordinate_candidates").empty()) continue;
        if (!inspectedSources.count(id)) {
            issues.push_back({{"kind","unresolved_source_not_inspected"},{"source_id",id},
                {"contract_ids",contracts},{"status",binding.at("status")},{"tool","get_operations"}});
            continue;
        }
        const auto gaps=ledger.value("source_binding_gaps",J::object());
        if (!gaps.contains(id)||gaps.at(id).value("source_snapshot",J())!=op||
            gaps.at(id).value("reason",std::string()).find_first_not_of(" \t\r\n")==std::string::npos)
            issues.push_back({{"kind","unresolved_source_needs_reason"},{"source_id",id},
                {"contract_ids",contracts},{"status",binding.at("status")},{"tool","record_gap"}});
    }
    return issues;
}

J storeContract(J& ledger, const J& arguments, bool requirement) {
    const char* collection = requirement ? "requirements" : "guarantees";
    const std::string prefix = requirement ? "req:" : "gua:";
    const auto replacement = arguments.value("replace_id", std::string());
    const auto reason = arguments.value("correction_reason", std::string());
    J record = arguments;
    record.erase("replace_id"); record.erase("correction_reason");
    size_t position = ledger.at(collection).size();
    if (!replacement.empty()) {
        if (reason.find_first_not_of(" \t\r\n") == std::string::npos)
            throw std::invalid_argument("A replacement needs a concrete correction_reason");
        bool found = false;
        for (size_t i = 0; i < ledger.at(collection).size(); ++i)
            if (ledger.at(collection).at(i).at("id") == replacement) { position = i; found = true; break; }
        if (!found) throw std::invalid_argument("Unknown replacement ID for this contract kind");
    } else if (!reason.empty()) throw std::invalid_argument("correction_reason requires replace_id");
    record["id"] = replacement.empty() ? prefix + std::to_string(position) : replacement;
    if (requirement) record["origin"] = "source_agent";
    if (!replacement.empty()) {
        J revision = {{"contract_id", replacement}, {"reason", reason},
            {"previous", ledger.at(collection).at(position)}, {"replacement", record}};
        if (!ledger.contains("contract_revisions")) ledger["contract_revisions"] = J::array();
        ledger["contract_revisions"].push_back(revision);
        ledger[collection][position] = record;
    } else ledger[collection].push_back(record);
    return record;
}
}
