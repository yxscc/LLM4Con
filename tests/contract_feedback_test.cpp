#include "LLMUtil/ContractFeedback.h"
#include <iostream>
#include <stdexcept>
using J = nlohmann::json;
void check(bool ok, const char* reason) { if (!ok) throw std::runtime_error(reason); }
int main() {
    J nodes = J::array();
    for (int id : {1,2,3,4,10,11}) nodes.push_back({{"node_id", id}, {"function_id", id < 10 ? 1 : 10},
        {"origin", "ir"}, {"kind", "load"}, {"file", "test.c"}, {"line", id},
        {"memory_role", "read"}, {"source_function", "f"}, {"access_paths", {"p.field"}}});
    J model = {{"nodes", nodes}, {"functions", {{{"function_id",1},{"cfg_complete",true},{"entry",1}},
        {{"function_id",10},{"cfg_complete",true},{"entry",10}}}}, {"cfg_edges", {{1,2},{1,3},{2,4},{3,4},{10,11}}}};
    J atomic = {{"form","MustBeAtomic"}, {"a_nodes",{2,3}}, {"b_nodes",J::array()}, {"interference_nodes",{11}}};
    const J empty = J::object();
    auto feedback = [&](const J& r) { return llm_client::contractFeedback(r,model,empty,J::array()); };
    const J original = model;
    auto d = feedback(atomic);
    check(d["sequence_pairs"][0]["status"] == "no_local_path", "Alternative branches silently accepted as one local path");
    check(model == original, "Extraction feedback changed the static model");
    check(d["synchronization_established"] == false && d["semantics_verified"] == false, "Reachability claimed synchronization or semantics");
    atomic["a_nodes"] = {2,4};
    check(feedback(atomic)["sequence_pairs"][0]["status"] == "local_path", "Valid forward path lost");
    check(feedback(atomic)["sequence_pairs"][0]["e_sequence_order"] == "not_dominating",
        "A compiler-arm bypass was hidden by reachability feedback");
    atomic["a_nodes"] = {1,4};
    check(feedback(atomic)["sequence_pairs"][0]["e_sequence_order"] == "established",
        "A dominating local prefix was lost");
    atomic["a_nodes"] = {2,4};
    model["cfg_edges"].push_back({4,1});
    check(feedback(atomic)["sequence_pairs"][0]["status"] == "cyclic_local_path", "Loop occurrence was treated as unique order");
    check(feedback(atomic)["sequence_pairs"][0]["same_resolved_operation"] == false, "Distinct loop effects called the same operation");
    model["nodes"][3].update({{"line",2},{"kind","store"},{"memory_role","write"},{"access_paths",{"p.dest"}}});
    J read = {{"path","test.c"},{"first_line",2},{"last_line",2},{"content","p->dest = p->field;"},
        {"operation_kind","read"},{"symbol","p->field"},{"function","f"}};
    J write = read; write["operation_kind"] = "write"; write["symbol"] = "p->dest";
    J sameLineSources = {{"read",read},{"write",write},{"read-alias",read}};
    J sameLine = atomic; sameLine["a_nodes"] = J::array(); sameLine["a_sources"] = {"read","write"};
    d = llm_client::contractFeedback(sameLine,model,sameLineSources,J::array({J{{"path","test.c"}}}));
    check(d["sequence_pairs"][0]["status"] == "cyclic_local_path" && d["sequence_pairs"][0]["same_resolved_operation"] == false,
        "Same-line read/write effects conflated because of a loop");
    sameLine["a_sources"] = {"read","read-alias"};
    d = llm_client::contractFeedback(sameLine,model,sameLineSources,J::array({J{{"path","test.c"}}}));
    check(d["sequence_pairs"][0]["status"] == "same_resolved_operation" && d["sequence_pairs"][0]["same_resolved_operation"] == true,
        "Two source anchors for the same effect were mistaken for distinct loop steps");
    atomic["a_nodes"] = {2,11};
    check(feedback(atomic)["sequence_pairs"][0]["status"] == "interprocedural_not_checked", "Call/return context invented");
    model["functions"][0]["cfg_complete"] = false;
    atomic["a_nodes"] = {2,4};
    check(feedback(atomic)["sequence_pairs"][0]["status"] == "unbound_or_incomplete_cfg", "Incomplete CFG got a definitive path judgment");
    model = original;
    J op = {{"path","test.c"},{"first_line",2},{"last_line",3},{"content","x = p->field; y = p->field;"},
        {"operation_kind","read"},{"symbol","p->field"},{"function","f"}};
    J sources = {{"wide",op}};
    atomic["a_nodes"] = J::array(); atomic["a_sources"] = {"wide","absent"};
    d = llm_client::contractFeedback(atomic,model,sources,J::array({J{{"path","test.c"}}}));
    check(d["bindings"][0]["status"] == "ambiguous" && d["bindings"][0]["semantic_match_count"] == 2,
        "A wide anchor chose an arbitrary node");
    check(!d["bindings"][0].contains("operation_id"), "Source feedback exposed an invented exact operation ID");
    check(d["bindings"][1]["status"] == "unknown_source_id", "Unknown source operand disappeared");

    J completion = {{"requirements", {{{"id","req:0"},{"a_sources",{"wide"}},{"b_sources",{"no-ir"}}}}},
        {"guarantees",J::array()},{"source_operations",{{"wide",op},{"orphan",op}}},
        {"source_manifest",J::array({J{{"path","test.c"}}})}};
    J noIR=op; noIR["first_line"]=90; noIR["last_line"]=90;
    completion["source_operations"]["no-ir"]=noIR;
    auto pending=[&](std::set<std::string> seen,const std::string& state) {
        return llm_client::extractionCompletionIssues(completion,model,seen,state);
    };
    check(pending({}, "").size()==2, "Uninspected diagnostics/source correspondence silently finished");
    auto state=llm_client::extractionState(completion);
    check(pending({}, state)[0]["kind"]=="unresolved_source_not_inspected", "Missing IR inspection not reported");
    check(pending({"wide"}, state)[0]["kind"]=="unresolved_source_needs_reason", "Inspection silently turned unknown into completion");
    completion["source_binding_gaps"]["wide"]={{"reason","Two effects inspected; correspondence remains unresolved"},{"source_snapshot",op}};
    check(pending({"wide"},state).empty(), "Documented unknown, no-IR operand or unused anchor blocked completion");
    check(llm_client::extractionState(completion)==state, "Recording a gap modified accepted contracts");
    completion["requirements"][0]["note"]="Corrected invariant";
    check(pending({"wide"},state)[0]["kind"]=="current_contract_diagnostics_not_inspected", "Stale diagnostic inspection survived a Req edit");
    completion["source_operations"]["wide"]["content"]="x = p->field; y = p->field; /* revised source evidence */";
    state=llm_client::extractionState(completion);
    check(pending({"wide"},state)[0]["kind"]=="unresolved_source_needs_reason", "A stale gap snapshot excused a changed source anchor");

    J ledger = {{"requirements",J::array()},{"guarantees",J::array()}};
    J r = {{"form","MustBeAtomic"},{"a_sources",{"old"}},{"note","old mutually exclusive sequence"}};
    check(llm_client::storeContract(ledger,r,true)["id"] == "req:0", "Initial Req ID changed");
    J replacement = r; replacement["a_sources"] = {"read","use"};
    replacement["replace_id"] = "req:0"; replacement["correction_reason"] = "Separate the alternative branch after reading source";
    auto revised = llm_client::storeContract(ledger,replacement,true);
    check(revised["id"] == "req:0" && ledger["requirements"].size() == 1, "Correction created a duplicate active requirement");
    check(!revised.contains("replace_id") && !revised.contains("correction_reason"), "Revision controls became contract operands");
    check(ledger["contract_revisions"][0]["previous"]["a_sources"] == J({"old"}), "Original extraction was lost");
    check(ledger["contract_revisions"][0]["replacement"] == revised, "Correction history differs from active contract");
    check(llm_client::storeContract(ledger,r,true)["id"] == "req:1", "New ID collides after correction");
    J g = {{"form","Exclude"},{"unconditional",false},{"note","source-established lock within scope"}};
    auto stored = llm_client::storeContract(ledger,g,false);
    check(stored["unconditional"] == false, "Backend inferred applicability from prose");
    check(feedback(stored)["declared_applicability"] == "unresolved_by_agent", "Conditional guarantee auto-credited");
    g["replace_id"] = "gua:0"; g["correction_reason"] = "Pinned source establishes the claimed scoped effect"; g["unconditional"] = true;
    check(llm_client::storeContract(ledger,g,false)["unconditional"] == true, "Explicit applicability correction was ignored");
    J snapshot = ledger;
    for (const auto& bad : {J{{"replace_id","req:0"},{"correction_reason","wrong contract kind"}},
                            J{{"replace_id","gua:0"},{"correction_reason","   "}},
                            J{{"replace_id","gua:99"},{"correction_reason","absent"}}}) {
        J args = g; args.update(bad);
        bool rejected = false;
        try { llm_client::storeContract(ledger,args,false); }
        catch (const std::invalid_argument&) { rejected = true; }
        check(rejected && ledger == snapshot, "Invalid correction mutated or silently added a record");
    }
    std::cout << "Contract extraction feedback and revision regressions passed\n";
}
