#include "LLMUtil/RequirementChecker.h"
#include "LLMUtil/OperationBinding.h"
#include <stdexcept>
#include <iostream>
using J=nlohmann::json;
void check(bool ok,const char* why) { if (!ok) throw std::runtime_error(why); }
J baseModel() {
    J nodes=J::array();
    for (int n:{1,2,3,4,10,11,12,13}) nodes.push_back({{"node_id",n},{"function_id",n<10?1:10},{"origin","ir"},{"kind","load"},{"atomic",false}});
    return {{"nodes",nodes},{"functions",{{{"function_id",1},{"entry",1},{"exits",{4}},{"cfg_complete",true}},
        {{"function_id",10},{"entry",10},{"exits",{13}},{"cfg_complete",true}}}},{"cfg_edges",{{1,2},{2,3},{3,4},{10,11},{11,12},{12,13}}}};
}
int main() {
    J model=baseModel();
    J r={{"id","r"},{"form","MustPrecede"},{"resource","obj"},{"scope","obj-instance/lifetime"},{"a_nodes",{2}},{"b_nodes",{12}}};
    J g={{"form","Wait"},{"resource","obj"},{"scope","obj-instance/lifetime"},{"a_nodes",{3}},{"b_nodes",{11}},
        {"evidence_nodes",{3,11}},{"unconditional",true},{"conditions","Matched completion of this activation, closed admission"},{"note","Actual signal/wait protocol"}};
    auto result=[&](const J& gs=J::array()) { return llm_client::checkModuleRequirements(J::array({r}),gs,model).at("r"); };
    check(result()["status"]=="undischarged","Local CFG must not synchronize activations");
    check(result(J::array({g}))["status"]=="discharged","Completion closure and target prefix did not compose with Wait");
    model["cfg_edges"].push_back({2,4});
    check(result(J::array({g}))["status"]=="undischarged","Source can skip synchronization completion");
    check(result(J::array({g}))["rule_failures"][0]["cfg_bypass"]==J({2,4}),"Missing source completion bypass evidence");
    model=baseModel(); model["cfg_edges"].push_back({10,12});
    check(result(J::array({g}))["status"]=="undischarged","Target can bypass wait");
    model=baseModel(); model["cfg_edges"]={{1,2},{2,3},{3,4},{10,12},{12,11},{11,13}};
    check(result(J::array({g}))["status"]=="undischarged","Wait placed after free credited");
    model=baseModel(); model["cfg_edges"].push_back({3,2});
    check(result(J::array({g}))["status"]=="undischarged","Loop iterations treated as unique occurrences");
    model=baseModel(); J conditional=g; conditional["unconditional"]=false;
    check(result(J::array({conditional}))["status"]=="undischarged","Conditional Gua auto-credited");
    check(result(J::array({conditional}))["guarantee_diagnostics"][0]["rejection_reasons"][0]=="unresolved_guarantee_conditions","Gua rejection reason lost");
    J wrongScope=g; wrongScope["scope"]="another-object-instance";
    check(result(J::array({wrongScope}))["status"]=="undischarged","Different Gua scope mixed");
    r["assumptions"]={"May be a different object generation"};
    check(result(J::array({g}))["status"]=="requires_semantic_review","Unknown applicability silently proved");
    check(!result(J::array({g}))["facts"].empty(),"Assumptions prevented independent graph relation check");r.erase("assumptions");
    r["b_nodes"]={2}; r["form"]="MustBeMediated";
    check(result()["status"]=="undischarged","Same node means distinct invocations synchronized");
    r["b_nodes"]={12}; r["a_nodes"]={999};
    check(result()["status"]=="requires_semantic_review","Missing graph node dropped");
    r["a_nodes"]=J::array(); r["a_sources"]={"module-op:orphan.c:3:3"};
    check(result()["status"]=="requires_semantic_review","Source-only Req disappeared"); r.erase("a_sources");
    r["a_nodes"]={2}; model["nodes"][1]["origin"]="source";
    check(result(J::array({g}))["status"]=="requires_semantic_review","Source/IR equivalence assumed");
    model=baseModel(); r["form"]="MustBeAtomic"; r["a_nodes"]={2,3}; r["b_nodes"]=J::array(); r["interference_nodes"]={12};
    J ag=g; ag["form"]="AtomicOp"; ag["token"]="obj"; ag["a_nodes"]={2,3,12}; ag["b_nodes"]=J::array();
    for (auto& n:model["nodes"]) n["atomic"]=true;
    check(result(J::array({ag}))["status"]=="undischarged","Individual atomic operations discharge whole transaction");
    r["a_nodes"]={2};
    check(result()["e_candidate"]==false,"One remaining step reported as an atomicity defect");
    r["a_nodes"]={2,12};
    check(result()["e_candidate"]==false,"Different activation functions treated as one atomic sequence");
    r["a_nodes"]={2,3};
    r["a_sources"]={"unbound-step"};
    check(result()["e_candidate"]==false,"Dropped source sequence step still produced an atomicity candidate");
    r.erase("a_sources");
    r["interference_nodes"]=J::array();
    check(result(J::array({ag}))["status"]=="requires_semantic_review","Empty interference domain means safe transaction");
    r["form"]="MustBeMediated"; r["a_nodes"]={2}; r["b_nodes"]={12};
    check(result(J::array({ag}))["status"]=="discharged","Compatible actual atomic operation pair lost");
    model=baseModel(); J ex=g; ex["form"]="Exclude"; ex["token"]="L"; ex["mode"]="exclusive"; ex["a_nodes"]={2,3,12}; ex["b_nodes"]=J::array(); ex["regions"]={{{"start_node",1},{"end_node",4}},{{"start_node",11},{"end_node",13}}};
    r["form"]="MustBeAtomic"; r["a_nodes"]={2,3}; r["b_nodes"]=J::array(); r["interference_nodes"]={12};
    check(result(J::array({ex}))["status"]=="discharged","Whole continuous exclusion interval lost");
    ex["regions"]={{{"start_node",1},{"end_node",3}},{{"start_node",2},{"end_node",4}},{{"start_node",11},{"end_node",13}}};
    check(result(J::array({ex}))["status"]=="undischarged","Separate protected steps made one transaction atomic");
    ex["regions"]={{{"start_node",1},{"end_node",4}},{{"start_node",11},{"end_node",13}}}; model["nodes"][2]["kind"]="call"; model["nodes"][2]["api"]="opaque_helper";
    check(result(J::array({ex}))["status"]=="undischarged","Opaque call assumed to preserve lock");
    check(result()["counterexample_constructed"]==false&&result()["proof"]==false,"Failure described as proved bug");
    model=baseModel();r["form"]="MustBeMediated";r["a_nodes"]=J::array();r["b_nodes"]={12};r["interference_nodes"]=J::array();r["a_sources"]={"s"};
    model["nodes"][1].update({{"file","/build/foo.c"},{"line",7},{"memory_role","read"},{"source_function","reader"},{"access_paths",{"p.field"}}});
    J sources={{"s",{{"source_id","s"},{"path","foo.c"},{"first_line",7},{"last_line",7},{"content","7: x = p->field;"},{"operation_kind","read"},{"symbol","p->field"},{"function","reader"}}}};
    J manifest={{{"path","foo.c"}}};
    auto bound=llm_client::checkModuleRequirements(J::array({r}),J::array(),model,sources,manifest).at("r");
    check(bound["bindings"][0]["status"]=="bound"&&bound["e_candidate"]==true,"Source access did not reach deterministic E");
    sources["s"]["symbol"]="p->missing";
    check(llm_client::bindSourceOperation(sources["s"],model,manifest)["status"]=="selector_not_in_source","Invented source selector accepted");
    sources["s"]["symbol"]="p->field";model["nodes"][2].update(model["nodes"][1]);model["nodes"][2]["node_id"]=3;
    check(llm_client::bindSourceOperation(sources["s"],model,manifest)["status"]=="ambiguous","Ambiguous access selected arbitrarily");
    manifest.push_back({{"path","nested/foo.c"}});
    check(llm_client::bindSourceOperation(sources["s"],model,manifest)["status"]=="no_ir_candidate","Duplicate basename conflated");
    std::cout<<"Complete-model requirement checker regressions passed\n";
}
