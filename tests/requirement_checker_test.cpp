#include "LLMUtil/RequirementChecker.h"
#include <stdexcept>
#include <iostream>
using J = nlohmann::json;
void check(bool ok,const char* why) { if (!ok) throw std::runtime_error(why); }
J access(int n,int tid,const char* lock="") { return {{"node_id",n},{"thread_id",tid},{"access_type","Write"},{"lock_protected",*lock != 0},{"lock",lock}}; }
int main() {
    J surface = {{"threads", {{{"thread_id",1},{"reentrant_entry",true}}}},
        {"shared_objects",{{{"accesses",{access(10,1),access(11,1),access(12,2)}}}}}};
    J req = {{"id","r"},{"object_id",0},{"owner_thread",1},{"form","MustBeMediated"},{"a_nodes",{10}},{"b_nodes",{10}}};
    llm_client::CheckerRelations relations;
    auto status = [&](const J& gs=J::array()) { return llm_client::checkModuleRequirements(J::array({req}),gs,surface,relations).at("r").at("status"); };
    check(status()=="undischarged","Same code node was mistaken for ordering between activations");
    req["form"]="MustBeAtomic"; req["a_nodes"]={10,11}; req["b_nodes"]=J::array();
    surface["shared_objects"][0]["accesses"]={access(10,1),access(11,1)};
    check(status()=="undischarged","Reentrant interference disappeared from a compound region");
    surface["shared_objects"][0]["accesses"]={access(10,1,"L"),access(11,1),access(12,2,"L")};
    surface["threads"][0]["reentrant_entry"]=false;
    check(status()=="undischarged","One protected step discharged an entire transaction");
    surface["shared_objects"][0]["accesses"]={access(10,1,"L"),access(11,1,"M"),access(12,2,"L")};
    check(status()=="undischarged","Different locks discharged one atomic region");
    surface["shared_objects"][0]["accesses"]={access(10,1,"L"),access(11,1,"L"),access(12,2,"L")};
    check(status()=="checker_suggests_satisfied","Whole-region same lock hint lost");
    J atomic={{{"form","AtomicOp"},{"unconditional",true},{"a_nodes",{10,11,12}},{"b_nodes",J::array()},{"token","state"}}};
    surface["shared_objects"][0]["accesses"]={access(10,1),access(11,1),access(12,2)};
    check(status(atomic)=="undischarged","Individual atomics made a compound transaction atomic");
    req["form"]="MustBeMediated"; req["a_nodes"]={10}; req["b_nodes"]={12};
    atomic[0]["unconditional"]=false;
    check(status(atomic)=="undischarged","Conditional guarantee was credited automatically");
    surface["shared_objects"][0]["accesses"]={access(10,1,"Lock"),access(12,2,"lock")};
    check(status()=="undischarged","Case-sensitive lock identities were conflated");
    req["object_id"]=-1;
    check(status()=="requires_semantic_review","Unmapped resource credited as safe");
    req["object_id"]=0; req["form"]="MustPrecede";
    relations.control_reachable=[](int,int){return true;};
    check(status()=="undischarged","Thread-local control flow was used as synchronization");
    auto checks=llm_client::checkModuleRequirements(J::array({req}),J::array(),surface,relations);
    check(checks["r"]["proof"]==false,"Static hint claimed a proof");
    std::cout << "requirement checker regression tests passed\n";
}
