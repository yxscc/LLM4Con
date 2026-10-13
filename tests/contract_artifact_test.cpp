#include "LLMUtil/ContractArtifact.h"
#include "LLMUtil/ContractFeedback.h"
#include "LLMUtil/OperationBinding.h"
#include <iostream>
#include <stdexcept>
using J=nlohmann::json;
void check(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
int main(){
    J op={{"path","test.c"},{"line",10},{"kind","call"},{"expression","spin_lock(&p->lock)"},{"function","reader"}};
    auto anchor=llm_client::artifactOperationArguments(op);
    check(anchor["symbol"]=="spin_lock"&&anchor["first_line"]==10&&anchor["statement_binding"]==true,"Locator not normalized");
    J req={{"kind","requirement"},{"form","MustBeMediated"},{"resource","p.field"},{"scope","same-p"},
        {"a",{op}},{"b",{op}},{"actors",{"reader","writer"}},{"note","Actual relation"}};
    auto translated=llm_client::translateContractArtifact(req,[](const J&){return "op";});
    check(translated["a_nodes"].empty()&&translated["a_sources"]==J({"op"})&&translated["b_sources"]==J({"op"}),"Source operands not retained");
    check(translated["assumptions"].empty(),"Defaults fabricated premises");
    J gua={{"kind","guarantee"},{"form","Exclude"},{"resource","p.field"},{"scope","same-p"},
        {"a",{op}},{"evidence",{op}},{"regions",{{{"start",op},{"end",op}}}},
        {"token","p.lock"},{"mode","exclusive"},{"conditions","Listed domain"},{"unconditional",true},{"note","Actual exclusion"}};
    auto g=llm_client::translateContractArtifact(gua,[](const J&){return "op";});
    check(g["source_regions"][0]["start_source"]=="op"&&g["evidence_sources"]==J({"op"}),"Gua endpoints/evidence not translated");
    for(auto bad:J::array({J{{"form","Other"}},J{{"actors",{"one"}}},J{{"b",J::array()}},J{{"extra","typo"}},J{{"a_nodes",{42}}}})){
        auto copy=req;copy.update(bad);bool rejected=false;
        try{llm_client::translateContractArtifact(copy,[](const J&){return "op";});}catch(const std::exception&){rejected=true;}
        check(rejected,"Malformed or graph-ID artifact accepted");
    }
    J node={{"node_id",1},{"origin","ir"},{"kind","call"},{"memory_role","call"},{"api","_raw_spin_lock"},
        {"callee_api","_raw_spin_lock"},{"file","test.c"},{"line",10},{"source_function","reader"},
        {"code","call @_raw_spin_lock"},{"atomic",false}};
    anchor["content"]="10: spin_lock(&p->lock);";
    J model={{"nodes",J::array({node})}},manifest={{{"path","test.c"}}};
    auto binding=llm_client::bindSourceOperation(anchor,model,manifest);
    check(binding["status"]=="bound"&&binding["node_id"]==1,"Unique typed source call not bound");
    auto legacy=anchor;legacy.erase("statement_binding");
    check(llm_client::bindSourceOperation(legacy,model,manifest)["status"]=="operation_not_resolved","Frozen legacy selector behavior changed");
    model["nodes"][0]["inline_calls"]={{{"api","spin_lock"},{"file","test.c"},{"line",10},{"callee","_raw_spin_lock"}}};
    check(llm_client::bindSourceOperation(legacy,model,manifest)["status"]=="bound","Compiler inline wrapper identity ignored");
    J second=model["nodes"][0];second["node_id"]=2;model["nodes"].push_back(second);
    check(llm_client::bindSourceOperation(anchor,model,manifest)["status"]!="bound","Two effects reduced to an arbitrary first node");
    model["nodes"]=J::array({node});anchor["content"]="10: spin_lock(other());";
    check(llm_client::bindSourceOperation(anchor,model,manifest)["status"]!="bound","Nested source calls collapsed by coordinate fallback");
    auto write=anchor;write["operation_kind"]="write";write["symbol"]="p->field";write["content"]="10: p->field=1;";
    node["kind"]="store";node["memory_role"]="write";node["access_paths"]=J::array();node["local_memory"]=false;model["nodes"]=J::array({node});
    check(llm_client::bindSourceOperation(write,model,manifest)["status"]=="bound","Unique write with missing DWARF name not bound");
    model["nodes"][0]["access_paths"]={"p.other"};
    check(llm_client::bindSourceOperation(write,model,manifest)["status"]!="bound","Known conflicting field ignored");
    J ledger={{"requirements",J::array()},{"guarantees",J::array()}};
    check(llm_client::storeContract(ledger,translated,true)["id"]=="req:0","First ID changed");
    ledger["requirements"].clear();
    check(llm_client::storeContract(ledger,translated,true)["id"]=="req:1","Deleted ID recycled");
    std::cout<<"Source contract translation, typed coordinates, inline calls and stable IDs passed\n";
}
