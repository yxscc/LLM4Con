#include "LLMUtil/RequirementChecker.h"
#include "LLMUtil/OperationBinding.h"
#include <map>
#include <set>
#include <queue>
#include <algorithm>
#include <functional>
namespace llm_client {
using J = nlohmann::json;
namespace {
// Call edges are navigation only. No caller/return pairing or parallel-entry
// inference is hidden in these intraprocedural control-flow predicates.
class Graph {
public:
    std::map<int,J> nodes, functions;
    std::map<int,std::vector<int>> next, functionNodes;
    mutable std::map<std::tuple<int,int,int>,std::vector<int>> paths;
    mutable std::map<std::pair<int,int>,bool> beforeFacts, afterFacts;
    explicit Graph(const J& m) {
        for (const auto& n:m.at("nodes")) { nodes.emplace(n.at("node_id"),n); functionNodes[n.at("function_id")].push_back(n.at("node_id")); }
        for (const auto& f:m.at("functions")) functions.emplace(f.at("function_id"),f);
        for (const auto& e:m.at("cfg_edges")) {
            int a=e.at(0),b=e.at(1);
            if (nodes.count(a)&&nodes.count(b)&&nodes.at(a).at("function_id")==nodes.at(b).at("function_id")) next[a].push_back(b);
        }
    }
    bool available(int n) const {
        if (!nodes.count(n)) return false;
        const auto& row=nodes.at(n);
        return row.at("origin")=="ir" && functions.at(row.at("function_id")).value("cfg_complete",false);
    }
    bool same(int a,int b) const { return available(a)&&available(b)&&nodes.at(a).at("function_id")==nodes.at(b).at("function_id"); }
    int entry(int n) const { return functions.at(nodes.at(n).at("function_id")).at("entry"); }
    std::vector<int> path(int a,int b,int avoid=-1) const {
        auto key=std::make_tuple(a,b,avoid);
        if (paths.count(key)) return paths.at(key);
        std::vector<int> result;
        if (!same(a,b)||a==avoid||b==avoid) return paths[key]=result;
        std::queue<int> todo; std::map<int,int> pred; todo.push(a); pred[a]=-1;
        while (!todo.empty()) {
            int n=todo.front(); todo.pop();
            if (n==b) { for (int x=b;x!=-1;x=pred.at(x)) result.push_back(x); std::reverse(result.begin(),result.end()); break; }
            auto it=next.find(n); if (it==next.end()) continue;
            for (int x:it->second) if (x!=avoid&&!pred.count(x)) { pred[x]=n; todo.push(x); }
        }
        return paths[key]=result;
    }
    bool dominates(int a,int b) const {
        return same(a,b)&&!path(entry(b),b).empty()&&path(entry(b),b,a).empty();
    }
    bool cycleFrom(int a,int avoid) const {
        std::map<int,int> color;
        std::function<bool(int)> visit=[&](int n) {
            if (n==avoid) return false;
            if (color[n]==1) return true;
            if (color[n]==2) return false;
            color[n]=1;
            auto it=next.find(n);
            if (it!=next.end()) for (int x:it->second) if (visit(x)) return true;
            color[n]=2; return false;
        };
        return visit(a);
    }
    // Prefix: every activation reaching b has passed a, in this occurrence
    // order. Cyclic CFGs cannot establish iteration-sensitive occurrence order.
    bool before(int a,int b) const {
        const auto key=std::make_pair(a,b);
        if (beforeFacts.count(key)) return beforeFacts.at(key);
        bool value=same(a,b)&&!path(a,b).empty()&&dominates(a,b)&&(a==b||path(b,a).empty());
        // Internal reflexivity, never cross-activation Req discharge.
        return beforeFacts[key]=value;
    }
    // Completion closure: every continuation after a passes b before any exit.
    // Infinite bypass cycles are conservatively retained, not assumed terminating.
    bool after(int a,int b) const {
        const auto key=std::make_pair(a,b);
        if (afterFacts.count(key)) return afterFacts.at(key);
        auto establish=[&]() {
            if (!same(a,b)||path(a,b).empty()||path(entry(a),a).empty()) return false;
            if (a==b) return true;
            const auto& exits=functions.at(nodes.at(a).at("function_id")).at("exits");
            if (exits.empty()||cycleFrom(a,b)||!path(b,a).empty()) return false;
            for (const auto& e:exits) if (!path(a,e,b).empty()) return false;
            return true;
        };
        return afterFacts[key]=establish();
    }
    bool region(int a,const J& region) const {
        int start=region.at("start_node"), end=region.at("end_node");
        if (a==start||a==end||!before(start,a)||!after(a,end)) return false;
        // The Gua declares matching token acquisition/release. The graph checks
        // continuous control flow; opaque calls or another acquisition/release
        // inside the interval would require interprocedural effect summaries.
        for (int id:functionNodes.at(nodes.at(start).at("function_id"))) {
            const auto& n=nodes.at(id);
            if (id==start||id==end||!same(id,start)||path(start,id).empty()||path(id,end).empty()) continue;
            if (n.value("kind",std::string{})=="call"||n.value("kind",std::string{})=="invoke") {
                const auto api=n.value("api",std::string{});
                if (api.rfind("llvm.dbg.",0)!=0&&api.rfind("llvm.lifetime.",0)!=0) return false;
            }
        }
        return true;
    }
};
std::vector<int> operand(const J& claim,const char* key) {
    return claim.value(key,J::array()).get<std::vector<int>>();
}
}
J checkModuleRequirements(const J& requirements,const J& inputGuarantees,const J& model,const J& sourceOperations,const J& sourceManifest) {
    Graph graph(model); J result=J::object();
    auto resolve=[&](const J& claim,bool req) {
        J resolved=claim,bindings=J::array();
        for (const char* prefix:req?std::vector<const char*>{"a","b","interference"}:std::vector<const char*>{"a","b","evidence"}) {
            auto sources=std::string(prefix)+"_sources",nodes=std::string(prefix)+"_nodes";
            if (!resolved.contains(nodes)) resolved[nodes]=J::array();
            for (const auto& id:claim.value(sources,J::array())) {
                J binding={{"status","source_snapshot_missing"},{"node_id",nullptr}};
                if (sourceOperations.contains(id.get<std::string>())) binding=bindSourceOperation(sourceOperations.at(id.get<std::string>()),model,sourceManifest);
                binding["source_id"]=id; binding["operand"]=prefix; bindings.push_back(binding);
                if (binding.at("status")=="bound") resolved[nodes].push_back(binding.at("node_id"));
            }
        }
        if (!req) for (const auto& region:claim.value("source_regions",J::array())) {
            J numeric;
            for (const char* side:{"start","end"}) {
                auto source=region.at(std::string(side)+"_source").get<std::string>();
                J binding={{"status","source_snapshot_missing"},{"node_id",nullptr}};
                if (sourceOperations.contains(source)) binding=bindSourceOperation(sourceOperations.at(source),model,sourceManifest);
                binding["source_id"]=source;binding["operand"]=std::string("region_")+side;bindings.push_back(binding);
                if (binding.at("status")=="bound") numeric[std::string(side)+"_node"]=binding.at("node_id");
            }
            if (numeric.size()==2) {if (!resolved.contains("regions")) resolved["regions"]=J::array();resolved["regions"].push_back(numeric);}
        }
        resolved["bindings"]=bindings; return resolved;
    };
    J guarantees=J::array(); for (const auto& g:inputGuarantees) guarantees.push_back(resolve(g,false));
    for (const auto& inputRequirement:requirements) {
        const auto r=resolve(inputRequirement,true);
        const auto id=r.at("id").get<std::string>(); J facts=J::array(), premises=J::array(), gaps=J::array(), failures=J::array();
        std::set<std::string> seenFailures;
        auto failure=[&](const J& f) { if (seenFailures.insert(f.dump()).second) failures.push_back(f); };
        std::set<size_t> used;
        auto a=operand(r,"a_nodes"),b=operand(r,"b_nodes"),interference=operand(r,"interference_nodes");
        const bool conditional=!r.value("assumptions",J::array()).empty();
        bool missing=false;
        if (conditional) gaps.push_back({{"kind","unresolved_requirement_premises"},{"assumptions",r.at("assumptions")}});
        for (const auto& binding:r.at("bindings")) if (binding.at("status")!="bound") {
            missing=true; gaps.push_back({{"kind","source_operand_not_bound_to_ir"},{"snapshot_present",sourceOperations.contains(binding.at("source_id").get<std::string>())},{"binding",binding}});
        }
        for (int n:a) if (!graph.available(n)) { missing=true; gaps.push_back({{"kind","exact_cfg_unavailable"},{"node_id",n}}); }
        for (int n:b) if (!graph.available(n)) { missing=true; gaps.push_back({{"kind","exact_cfg_unavailable"},{"node_id",n}}); }
        for (int n:interference) if (!graph.available(n)) { missing=true; gaps.push_back({{"kind","exact_cfg_unavailable"},{"node_id",n}}); }
        J guaranteeDiagnostics=J::array();
        for (size_t i=0;i<guarantees.size();++i) {
            const auto& g=guarantees[i]; J reasons=J::array();
            if (!g.value("unconditional",false)) reasons.push_back("unresolved_guarantee_conditions");
            if (g.value("resource","")!=r.value("resource","")) reasons.push_back("resource_mismatch");
            if (g.value("scope","")!=r.value("scope","")||r.value("scope","").empty()) reasons.push_back("scope_mismatch");
            for (const auto& binding:g.at("bindings")) if (binding.at("status")!="bound") reasons.push_back("source_operand_not_bound_to_ir");
            const auto evidence=operand(g,"evidence_nodes");
            if (evidence.empty()) reasons.push_back("synchronization_evidence_missing");
            for (int n:evidence) if (!graph.available(n)) reasons.push_back("exact_cfg_unavailable");
            guaranteeDiagnostics.push_back({{"guarantee_index",i},{"guarantee_id",g.value("id","gua:"+std::to_string(i))},
                {"eligible",reasons.empty()},{"rejection_reasons",reasons},{"bindings",g.at("bindings")},{"semantics_verified",false}});
        }
        auto credited=[&](const J& g) {
            for (size_t i=0;i<guarantees.size();++i) if (&guarantees[i]==&g) return guaranteeDiagnostics[i].at("eligible").get<bool>();
            return false;
        };
        // A successful cross-activation chain must contain at least one Gua.
        // Same-function/local order and a==b alone cannot discharge any Req.
        auto ordered=[&](int from,int to) {
            struct State { int point; std::vector<size_t> chain; J local; };
            std::queue<State> todo; todo.push({from,{},J::array()}); std::set<std::pair<int,bool>> seen;
            while (!todo.empty()) {
                auto state=todo.front(); todo.pop();
                if (!seen.insert({state.point,!state.chain.empty()}).second) continue;
                if (!state.chain.empty()&&!graph.before(state.point,to)) {
                    J bypass=J::array();
                    if (graph.same(state.point,to)) bypass=graph.path(graph.entry(to),to,state.point);
                    failure({{"rule","target_prefix"},{"from",state.point},{"to",to},{"cfg_bypass",bypass},
                        {"note","CFG navigation path only; feasibility and synchronization semantics require source review"}});
                }
                if (!state.chain.empty()&&graph.before(state.point,to)) {
                    for (auto index:state.chain) used.insert(index);
                    state.local.push_back({{"kind","target_prefix"},{"from",state.point},{"to",to}});
                    facts.push_back({{"kind","synchronized_order"},{"a",from},{"b",to},{"guarantee_indices",state.chain},{"cfg_premises",state.local}}); return true;
                }
                for (size_t i=0;i<guarantees.size();++i) {
                    const auto& g=guarantees[i]; if (!credited(g)||(g.at("form")!="Order"&&g.at("form")!="Wait")) continue;
                    if (std::find(state.chain.begin(),state.chain.end(),i)!=state.chain.end()) continue;
                    for (int x:operand(g,"a_nodes")) for (int y:operand(g,"b_nodes")) {
                        bool closure=state.chain.empty()?graph.after(state.point,x):graph.before(state.point,x);
                        if (!closure) {
                            J bypass=J::array();
                            if (graph.same(state.point,x)) {
                                if (state.chain.empty()) {
                                    for (const auto& exit:graph.functions.at(graph.nodes.at(state.point).at("function_id")).at("exits")) {
                                        auto path=graph.path(state.point,exit,x); if (!path.empty()) { bypass=path; break; }
                                    }
                                } else bypass=graph.path(graph.entry(x),x,state.point);
                            }
                            failure({{"rule",state.chain.empty()?"source_completion":"handoff_prefix"},{"from",state.point},{"to",x},
                                {"guarantee_index",i},{"cfg_bypass",bypass},{"note","Missing must-order premise; an empty bypass may mean a loop, missing cross-function summary or unreachable endpoint"}});
                        }
                        if (!closure||!graph.available(y)) continue;
                        auto chain=state.chain; chain.push_back(i); J local=state.local;
                        local.push_back({{"kind",state.chain.empty()?"source_completion":"handoff_prefix"},{"from",state.point},{"to",x}});
                        todo.push({y,std::move(chain),std::move(local)});
                    }
                }
            }
            failure({{"rule","no_synchronization_chain"},{"from",from},{"to",to}});
            return false;
        };
        auto excluded=[&](const std::vector<int>& sequence,int other) {
            for (size_t i=0;i<guarantees.size();++i) {
                const auto& g=guarantees[i];
                if (!credited(g)||g.at("form")!="Exclude"||g.value("mode","")!="exclusive"||g.value("token","").empty()) continue;
                const auto protectedOps=operand(g,"a_nodes");
                if (std::find(protectedOps.begin(),protectedOps.end(),other)==protectedOps.end()||
                    !std::all_of(sequence.begin(),sequence.end(),[&](int n){return std::find(protectedOps.begin(),protectedOps.end(),n)!=protectedOps.end();})) {
                    failure({{"rule","exclusion_operand_coverage"},{"guarantee_index",i},{"sequence",sequence},{"other",other}});continue;
                }
                const auto regions=g.value("regions",J::array()); bool transaction=false, contender=false;
                for (const auto& region:regions) {
                    if (std::all_of(sequence.begin(),sequence.end(),[&](int n){return graph.region(n,region);})) transaction=true;
                    if (graph.region(other,region)) contender=true;
                }
                if (transaction&&contender) {
                    used.insert(i); facts.push_back({{"kind","continuous_exclusion"},{"sequence",sequence},{"other",other},{"guarantee_index",i},{"regions",regions}}); return true;
                }
                failure({{"rule","continuous_exclusion"},{"guarantee_index",i},{"sequence",sequence},{"other",other},
                    {"transaction_region_established",transaction},{"contender_region_established",contender},{"regions",regions}});
            }
            return false;
        };
        auto atomicPair=[&](int x,int y) {
            if (!graph.available(x)||!graph.available(y)||!graph.nodes.at(x).value("atomic",false)||!graph.nodes.at(y).value("atomic",false)) return false;
            for (size_t i=0;i<guarantees.size();++i) {
                const auto& g=guarantees[i]; if (!credited(g)||g.at("form")!="AtomicOp"||g.value("token","").empty()) continue;
                auto ops=operand(g,"a_nodes");
                if (std::find(ops.begin(),ops.end(),x)!=ops.end()&&std::find(ops.begin(),ops.end(),y)!=ops.end()) {
                    used.insert(i); facts.push_back({{"kind","compatible_atomic_operations"},{"a",x},{"b",y},{"guarantee_index",i}}); return true;
                }
            } return false;
        };
        bool success=!missing&&!a.empty(); J failed=J::array();
        const std::string form=r.at("form");
        if (form=="MustBeAtomic") {
            if (a.size()<2||interference.empty()) { success=false; missing=true; gaps.push_back({{"kind","atomic_sequence_or_interference_domain_missing"}}); }
            // Local sequence order is a premise, not synchronization.
            for (size_t i=1;i<a.size();++i) if (!graph.before(a[i-1],a[i])) { success=false; gaps.push_back({{"kind","sequence_order_not_established"},{"a",a[i-1]},{"b",a[i]}}); }
            if (!a.empty()) for (int n:interference) if (graph.available(n)&&std::all_of(a.begin(),a.end(),[&](int x){return graph.available(x);}))
                if (!ordered(a.back(),n)&&!ordered(n,a.front())&&!excluded(a,n)) { success=false; failed.push_back({{"sequence",a},{"interference",n}}); }
        } else {
            if (b.empty()) { missing=true; success=false; gaps.push_back({{"kind","second_operand_missing"}}); }
            for (int x:a) for (int y:b) if (graph.available(x)&&graph.available(y)) {
                bool ok=ordered(x,y);
                if (form=="MustBeMediated") ok=ok||ordered(y,x)||excluded({x},y)||atomicPair(x,y);
                if (!ok) { success=false; failed.push_back({{"a",x},{"b",y}}); }
            }
        }
        for (size_t i:used) premises.push_back({{"guarantee_index",i},{"resource",guarantees[i].at("resource")},
            {"scope",guarantees[i].at("scope")},{"conditions",guarantees[i].at("conditions")},{"note",guarantees[i].at("note")}});
        for (size_t i=0;i<guaranteeDiagnostics.size();++i) guaranteeDiagnostics[i]["used_in_discharge"]=used.count(i)>0;
        result[id]={{"status",missing||conditional?"requires_semantic_review":success?"discharged":"undischarged"},
            {"bindings",r.at("bindings")},{"resolved_operands",{{"a_nodes",a},{"b_nodes",b},{"interference_nodes",interference}}},
            {"guarantee_diagnostics",guaranteeDiagnostics},{"evaluated_combinations",facts.size()+failed.size()},
            {"e_candidate",!failed.empty()},{"candidate_origin","deterministic_E"},{"applicability_unresolved",conditional},
            {"requirement_semantics_verified",false},
            {"proof",false},{"model_relative",true},{"facts",facts},{"guarantee_premises",premises},{"gaps",gaps},
            {"unresolved_combinations",failed},{"rule_failures",failures},{"counterexample_constructed",false},
            {"meaning",success?"Sufficient rule holds under the declared Gua semantics and instance/activation scope; mandatory source review remains":"No sufficient discharge established; failure is not a feasible interleaving or proof of a defect"}};
    }
    return result;
}
}
