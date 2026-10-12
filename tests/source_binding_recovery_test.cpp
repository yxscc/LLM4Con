#include "LLMUtil/OperationBinding.h"
#include "LLMUtil/ContractFeedback.h"
#include "LLMUtil/RequirementChecker.h"
#include <iostream>
#include <stdexcept>
using J=nlohmann::json;
void check(bool ok,const char* why) { if (!ok) throw std::runtime_error(why); }
int main() {
    J nodes=J::array();
    for (int base:{0,10}) for (int offset:{1,2,3}) {
        const bool call=offset!=2;
        nodes.push_back({{"node_id",base+offset},{"function_id",base+1},{"origin","ir"},
            {"kind",call?"call":"load"},{"memory_role",call?"call":"read"},
            {"api",offset==1?"mutex_lock_nested":offset==3?"mutex_unlock":""},
            {"code",offset==1?"call @mutex_lock_nested(%lock, 0)":offset==3?"call @mutex_unlock(%lock)":"load %p.field"},
            {"file","test.c"},{"line",base+offset},{"source_function",base==0?"reader":"other"},
            {"access_paths",call?J::array():J({"p.field"})},{"atomic",false}});
    }
    J model={{"nodes",nodes},{"functions",{{{"function_id",1},{"entry",1},{"exits",{3}},{"cfg_complete",true}},
        {{"function_id",11},{"entry",11},{"exits",{13}},{"cfg_complete",true}}}},
        {"cfg_edges",{{1,2},{2,3},{11,12},{12,13}}}};
    const auto original=model;
    J manifest={{{"path","test.c"}}};
    J lock={{"path","test.c"},{"first_line",1},{"last_line",1},{"content","mutex_lock(&p->lock);"},
        {"operation_kind","call"},{"symbol","mutex_lock"},{"function","reader"}};
    check(llm_client::bindSourceOperation(lock,model,manifest)["status"]=="operation_not_resolved","Macro rename unexpectedly matched automatically");
    auto selected=llm_client::selectSourceIRBinding(lock,1,"Source mutex_lock macro lowers to inspected mutex_lock_nested with subclass 0",model,manifest);
    auto binding=llm_client::bindSourceOperation(selected,model,manifest);
    check(binding["status"]=="bound"&&binding["node_id"]==1&&binding["binding_origin"]=="agent_selected","Explicit macro correspondence not used");
    check(binding["automatic_status"]=="operation_not_resolved"&&binding["matching_candidates"].empty(),"Agent choice mislabeled as a compiler semantic match");
    check(!binding["semantic_correctness_verified"].get<bool>()&&!binding["instance_equivalence_proved"].get<bool>(),"Selection claimed semantic or instance proof");
    J restored=J::parse(selected.dump());
    check(llm_client::bindSourceOperation(restored,model,manifest)==binding,"Persisted correspondence lost on offline replay");

    J sources={{"lock",selected}};
    J r={{"id","r"},{"form","MustBeMediated"},{"resource","p.field"},{"scope","same-p"},
        {"a_sources",{"lock"}},{"a_nodes",J::array()},{"b_nodes",{12}}};
    auto e=llm_client::checkModuleRequirements(J::array({r}),J::array(),model,sources,manifest).at("r");
    check(e["resolved_operands"]["a_nodes"]==J({1})&&e["e_candidate"]==true,"Recovered Req did not reach E");
    auto feedback=llm_client::contractFeedback(r,model,sources,manifest);
    check(feedback["bindings"][0]["binding_origin"]=="agent_selected"&&feedback["bindings"][0]["selected_node_id"]==1,"Extraction feedback lost selection provenance");

    J secondLock=lock; secondLock["first_line"]=11;secondLock["last_line"]=11;secondLock["function"]="other";
    sources["second-lock"]=secondLock;
    J g={{"id","g"},{"form","Exclude"},{"resource","p.field"},{"scope","same-p"},{"a_nodes",{2,12}},
        {"token","p.lock"},{"mode","exclusive"},{"unconditional",true},{"conditions","Same scoped mutex"},{"note","Two protected accesses"},
        {"evidence_sources",{"lock","second-lock"}},{"source_regions",{{{"start_source","lock"},{"end_source","unlock"}},
            {{"start_source","second-lock"},{"end_source","second-unlock"}}}}};
    J unlock=lock;unlock["first_line"]=3;unlock["last_line"]=3;unlock["content"]="mutex_unlock(&p->lock);";unlock["symbol"]="mutex_unlock";
    sources["unlock"]=unlock;unlock["first_line"]=13;unlock["last_line"]=13;unlock["function"]="other";sources["second-unlock"]=unlock;
    r["a_sources"]=J::array();r["a_nodes"]={2};
    auto evaluate=[&]() {return llm_client::checkModuleRequirements(J::array({r}),J::array({g}),model,sources,manifest).at("r");};
    check(evaluate()["status"]=="undischarged","Unresolved Gua lock credited");
    sources["second-lock"]=llm_client::selectSourceIRBinding(secondLock,11,"Same inspected mutex_lock macro lowering in other",model,manifest);
    check(evaluate()["status"]=="discharged","Recovered Gua evidence/region endpoints did not reach E");
    model["functions"][0]["cfg_complete"]=false;
    check(evaluate()["status"]=="requires_semantic_review","Agent binding bypassed missing CFG");model=original;

    // A macro expansion can contain alternative read effects of one field.
    model["nodes"][0]=model["nodes"][1];model["nodes"][0]["node_id"]=1;
    J read=lock;read["first_line"]=2;read["last_line"]=2;read["operation_kind"]="read";read["symbol"]="p->field";read["content"]="value = READ_ONCE(p->field);";
    check(llm_client::bindSourceOperation(read,model,manifest)["status"]=="ambiguous","Ambiguity silently resolved");
    auto chosenRead=llm_client::selectSourceIRBinding(read,2,"Inspected field-sized load represents this macro read",model,manifest);
    check(llm_client::bindSourceOperation(chosenRead,model,manifest)["node_id"]==2,"Ambiguous read recovery failed");
    auto withdrawn=llm_client::selectSourceIRBinding(chosenRead,-1,"Further evidence leaves the branch unresolved",model,manifest);
    check(llm_client::bindSourceOperation(withdrawn,model,manifest)["status"]=="ambiguous","Withdrawal did not restore unknown");
    auto revised=llm_client::selectSourceIRBinding(chosenRead,1,"Corrected effect selection after inspecting branch context",model,manifest);
    check(llm_client::bindSourceOperation(revised,model,manifest)["node_id"]==1,"Selection correction ignored");
    auto changed=chosenRead;changed["content"]="different source";
    check(llm_client::bindSourceOperation(changed,model,manifest)["status"]=="agent_selection_invalid","Stale source choice accepted");
    model["nodes"][1]["code"]="different instruction";
    check(llm_client::bindSourceOperation(chosenRead,model,manifest)["status"]=="agent_selection_invalid","Stale IR choice accepted");model=original;

    for (auto bad:J::array({J{{"node",12}},J{{"node",999}},J{{"node",1},{"reason","  "}},
            J{{"node",1},{"function","wrong"}},J{{"node",1},{"first_line",100},{"last_line",100}},J{{"node",1},{"symbol","invented"}}})) {
        auto op=lock;
        for (auto item:bad.items()) if (item.key()!="node"&&item.key()!="reason") op[item.key()]=item.value();
        bool rejected=false;
        try {llm_client::selectSourceIRBinding(op,bad.at("node"),bad.value("reason",std::string("Candidate evidence")),model,manifest);}
        catch (const std::invalid_argument&) {rejected=true;}
        check(rejected,"Invalid or absent IR correspondence accepted");
    }
    check(model==original,"Binding recovery mutated static compiler facts");
    std::cout<<"Agent source/IR recovery, persistence, Req/Gua E and unknown regressions passed\n";
}
