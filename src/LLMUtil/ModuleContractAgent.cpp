#include "LLMUtil/ModuleContractAgent.h"
#include "LLMUtil/ModulePrompt.h"
#include "LLMUtil/SharedToolKit.h"
#include "Util/TargetPath.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstdlib>
namespace llm_client {
using J = nlohmann::json;
namespace {
Parameter list(const char* name,const char* type,const char* description,bool required=true) {
    return Parameter(name,J{{"type","array"},{"items",{{"type",type}}},{"description",description}},required);
}
Parameter choice(const char* name,J values) { return Parameter(name,J{{"type","string"},{"enum",values}},true); }
Parameter source(const char* name) { return list(name,"string","Validated module source-operation IDs; use instead of numeric operands when no exact operation is bound.",false); }
Parameter repo() { return list("repository_evidence_ids","string","Cited IDs returned by repository_read; separate from module operation IDs.",false); }
void nonempty(const J& a,const char* key) { if (a.at(key).get<std::string>().empty()) throw std::invalid_argument(std::string(key)+" must not be empty"); }
}
ModuleContractAgent::ModuleContractAgent(std::shared_ptr<LLMClient> client,CCPG* ccpg,const J& model)
    : Conversation(client,modulePrompt),ccpg_(ccpg),model_(model) {
    const char* configured=std::getenv("LACE_PROTOCOL_CATALOG");
    fs::path path=configured&&*configured?fs::path(configured):fs::path(PROJECT_PATH)/"config/protocol_catalog.json";
    std::ifstream in(path); if (!in) throw std::runtime_error("Cannot read protocol catalog: "+path.string()); in>>catalog_;
    if (catalog_.at("schema")!="lace-protocol-catalog-v1"||!catalog_.at("patterns").is_array()||catalog_["patterns"].empty()) throw std::invalid_argument("Invalid protocol catalog");
    std::set<std::string> patterns;
    for (const auto& p:catalog_["patterns"]) {
        if (p.at("id").get<std::string>().empty()||!patterns.insert(p.at("id")).second||!p.at("questions").is_array()||p.at("questions").empty()) throw std::invalid_argument("Invalid/duplicate catalog pattern");
        for (const auto& q:p.at("questions")) if (!q.is_string()||q.get<std::string>().empty()) throw std::invalid_argument("Catalog question must be nonempty text");
    }
    data_={{"schema","lace-source-contracts-v1"},{"requirements",J::array()},{"guarantees",J::array()},
        {"source_operations",J::object()},{"protocols",J::array()},{"pattern_reviews",J::object()},
        {"reviews",J::object()},{"checks",J::object()},{"gaps",J::array()},{"source_manifest",J::array()},
        {"harness","openai-agents"},{"observation_format","source-first-no-ast-v1"},{"agent_instructions",modulePrompt},{"protocol_catalog",catalog_},
        {"graph_coverage",model_.at("coverage")}};
    for (const auto& f:model_["functions"]) functions_.insert(f.at("name"));
    initializeRepositoryContext();
    if (const char* turns=std::getenv("LACE_MODULE_MAX_TURNS")) set_max_turns(std::stoi(turns));
    save();
}
std::string ModuleContractAgent::sourcePacket() {
    const auto root=TargetPath::getInstance()->getTargetAbsolutePath(); std::vector<fs::path> files;
    std::set<std::string> aliases;
    if (!data_["repository_context"].is_null()) for (const auto& f:data_["repository_context"]["source_verification"]["verified_files"])
        if (f.contains("alias_of")) aliases.insert(f.at("path"));
    data_["deduplicated_source_aliases"]=aliases;
    for (const auto& e:fs::recursive_directory_iterator(root)) {
        if (e.is_symlink()) { data_["gaps"].push_back("Source symlink not included: "+fs::relative(e.path(),root).string()); continue; }
        const auto ext=e.path().extension().string();
        if (e.is_regular_file()&&!aliases.count(fs::relative(e.path(),root).string())&&(ext==".c"||ext==".h"||ext==".cpp"||ext==".hpp"||ext==".hh"||ext==".cc"||ext==".cxx")) files.push_back(e.path());
    }
    std::sort(files.begin(),files.end()); std::ostringstream out;
    for (const auto& path:files) {
        std::ifstream in(path); if (!in) throw std::runtime_error("Cannot read source: "+path.string());
        std::string name=fs::relative(path,root).string(),line; size_t lines=0,bytes=0;
        out<<"\n<source file="<<J(name).dump()<<">\n";
        while (std::getline(in,line)) { sourceLines_[name].push_back(line); bytes+=line.size()+1; out<<++lines<<": "<<line<<'\n'; }
        out<<"</source>\n"; data_["source_manifest"].push_back({{"path",name},{"lines",lines},{"bytes",bytes},{"complete",true}});
    }
    if (files.empty()) throw std::runtime_error("No C/C++ module sources found"); return out.str();
}
void ModuleContractAgent::generate() {
    try {
        auto packet=sourcePacket(); save();
        send_message("Phase A. Read the complete module below. There are NO precomputed requirements, shared-object buckets, or inferred parallel entries. "
            "Use the extensible catalog as an auditing guide, pursue other source-supported protocols too, and declare actual concurrency Req/Gua. "
            "Graph coverage (navigation/checking only): "+model_["coverage"].dump()+"\nCatalog: "+catalog_.dump()+
            "\nRepository context: "+(data_["repository_context"].is_null()?std::string("not configured"):data_["repository_context"]["revision"].get<std::string>())+"\n"+packet);
        if (!generationFinished_) throw std::runtime_error("Agent did not finalize contract extraction"); save();
    } catch (const std::exception& e) { save(e.what()); throw; }
}
std::vector<Tool> ModuleContractAgent::get_available_tools() const {
    auto tools=SharedToolKit::get_source_tools(); auto rt=repositoryTools(); tools.insert(tools.end(),rt.begin(),rt.end());
    tools.push_back({"get_module_functions","Query all source/IR function representations; these are not parallel entries. Pagination retains all rows.",
        {{"name","string","Exact function name, or empty for all.",true},{"origin","string","source, ir, or empty for both.",true},{"offset","integer","Start at zero.",true},{"limit","integer","Page size 1..1000; follow next_offset.",true}}});
    tools.push_back({"get_operations","On-demand IR evidence only; never returns AST nodes. Filter narrowly by function/file/lines or a registered source_id. Coordinates are navigation, not proven source equivalence. Pagination retains every matching instruction.",
        {{"function_id","integer","Function representation ID, or -1 for any.",true},{"path","string","File path or suffix, or empty.",true},
         {"first_line","integer","Inclusive lower bound, or 0 for any.",true},{"last_line","integer","Upper bound, or 0 for any.",true},
         Parameter("origin",J{{"type","string"},{"enum",{"ir"}}},true),
         {"source_id","string","Optional registered module source ID to restrict the query to its coordinates.",false},
         {"offset","integer","Start at zero.",true},{"limit","integer","Page size 1..1000.",true}}});
    tools.push_back({"get_source_operation","Register a concrete source operation. Optional kind/symbol/function selectors bind it against compiler access/callee facts. Returns binding status and count, no AST/IR records.",
        {{"path","string","Exact module-relative path.",true},{"first_line","integer","First line, 1-based.",true},{"last_line","integer","Last line, inclusive.",true},
         {"operation_kind","string","read, write, call, atomic, fence or unspecified.",false},{"symbol","string","Exact source lvalue (e.g. p->field) or callee name, copied from the cited statement.",false},{"function","string","Containing source function name.",false}}});
    tools.push_back({"get_source_operations","Batch validated source anchors; no graph binding is silently invented.",
        {Parameter("operations",J{{"type","array"},{"items",{{"type","object"},{"properties",{{"path",{{"type","string"}}},{"first_line",{{"type","integer"}}},{"last_line",{{"type","integer"}}},{"operation_kind",{{"type","string"}}},{"symbol",{{"type","string"}}},{"function",{{"type","string"}}}}},{"required",{"path","first_line","last_line"}},{"additionalProperties",false}}}},true)}});
    tools.push_back({"record_gap","Record missing source/context, unresolved mapping or unsupported semantics.",{{"description","string","Missing evidence and affected protocol.",true}}});
    tools.push_back({"get_obligations","Query all requirements (empty IDs) or full selected records with E evidence and reviews.",{list("ids","string","IDs, or [] for all.")}});
    if (generating_) {
        tools.push_back({"record_protocol","Describe ordinary source facts, resource protocol and local control flow; these are not synchronization Gua.",
            {list("functions","string","Related module functions; not a coverage quota."),{"summary","string","Protocol, source evidence, assumptions and gaps.",true},choice("status",{"reviewed","unknown"})}});
        tools.push_back({"record_pattern_review","Record how a catalog question was investigated. This accounts for the checklist, not semantic completeness.",
            {{"pattern_id","string","Catalog pattern ID.",true},choice("status",{"reviewed","not_applicable","unknown"}),{"reason","string","Source-based audit position; describe unresolved evidence.",true}}});
        tools.push_back({"add_requirement","Record one concurrency safety relation; preserve all operands. No static object or entry IDs required.",
            {choice("form",{"MustPrecede","MustBeAtomic","MustBeMediated"}),{"resource","string","Concrete source resource expression.",true},
             {"scope","string","Stable domain label for same instance, lifetime/generation and activation pairing; use exactly this label on matching Gua.",true},
             list("actors","string","At least two potential execution activations; two invocations of one function are distinct actors."),
             list("a_nodes","integer","Exact inspected operation IDs; ordered sequence for atomicity."),list("b_nodes","integer","Second operand; [] for atomicity."),
             source("a_sources"),source("b_sources"),list("interference_nodes","integer","All identified interfering operations for atomicity; [] otherwise."),source("interference_sources"),
             list("assumptions","string","Unresolved applicability/coverage premises; [] if source-established."),repo(),{"note","string","Concurrent conflict, actors' source evidence, intended invariant and scope. Ordinary local order/API prerequisites are not Req.",true}}});
        tools.push_back({"add_guarantee","Record synchronization supplied by actual code, including scoped non-lock protocols. Pure local order is not an Order Gua.",
            {choice("form",{"Order","Wait","Exclude","AtomicOp"}),{"resource","string","Resource matching Req.",true},{"scope","string","Exact Req instance/activation domain label.",true},
             list("a_nodes","integer","Predecessor completion events, or protected/atomic operations."),list("b_nodes","integer","Post-handoff continuation operations for Order/Wait; [] otherwise."),
             source("a_sources"),source("b_sources"),{"token","string","Scoped protection/atomic token; empty for Order/Wait.",true},{"mode","string","exclusive/shared for Exclude; empty otherwise.",true},
             {"conditions","string","Established and unresolved instance/domain/matching/return/requeue premises.",true},{"unconditional","boolean","Applicability and scope established for these operands. Still subject to source review.",true},
             list("evidence_nodes","integer","Actual synchronization operation IDs."),source("evidence_sources"),
             Parameter("regions",J{{"type","array"},{"items",{{"type","object"},{"properties",{{"start_node",{{"type","integer"}}},{"end_node",{{"type","integer"}}}}},{"required",{"start_node","end_node"}},{"additionalProperties",false}}},{"description","Exclude: matching acquisition/release boundaries for each protected interval. E requires whole sequence continuously protected; opaque interior calls remain unsupported. [] for other Gua."}},true),
             Parameter("source_regions",J{{"type","array"},{"items",{{"type","object"},{"properties",{{"start_source",{{"type","string"}}},{"end_source",{{"type","string"}}}}},{"required",{"start_source","end_source"}},{"additionalProperties",false}}}},false),
             repo(),{"note","string","Actual mechanism and why it supplies the claimed synchronization effect on this domain.",true}}});
        tools.push_back({"finalize_contract","Finish after explicitly considering every catalog pattern; unknown positions are retained. No per-object/function quota.",{}});
    } else {
        tools.push_back({"review_obligations","Review every Req, including E discharges. Source-supported reports need a concrete applicable harmful interleaving.",
            {list("ids","string","Req IDs sharing this judgment."),choice("status",{"source_supported","satisfied","not_applicable","unknown"}),
             list("evidence_nodes","integer","Evidence IDs; [] permitted for unknown."),list("witness_nodes","integer","Concrete involved operations, [] except reports."),
             source("evidence_sources"),source("witness_sources"),repo(),{"reason","string","Justify applicable actors, instance/lifetime and harm, or coverage of all operands for exclusion.",true},
             {"interleaving","string","Concrete harmful execution for reports, empty otherwise.",true}}});
        tools.push_back({"finalize_review","Finish once every Req has an explicit source judgment, preserving unknowns.",{}});
    } return tools;
}
std::vector<int> ModuleContractAgent::validNodes(const J& args,const char* key,bool required) const {
    const auto& value=args.at(key); if (!value.is_array()) throw std::invalid_argument(std::string(key)+" must be an array");
    std::vector<int> out; std::set<int> seen;
    for (const auto& n:value) {
        if (!n.is_number_integer()||!ccpg_->getNodeByID(n.get<int>())) throw std::invalid_argument("Unknown operation ID");
        if (ccpg_->getNodeByID(n.get<int>())->getCPGNode()->getType()=="Method") throw std::invalid_argument("Function ID is not an operation operand");
        if (!seen.insert(n.get<int>()).second) throw std::invalid_argument("Duplicate operation ID"); out.push_back(n.get<int>());
    }
    if (required&&out.empty()) throw std::invalid_argument(std::string(key)+" must not be empty"); return out;
}
std::string ModuleContractAgent::execute_tool(const std::string& name,const J& args) {
    if (name=="get_module_functions"||name=="get_operations") {
        int offset=args.at("offset"),limit=args.at("limit"); if (offset<0||limit<1||limit>1000) throw std::invalid_argument("Invalid page");
        const auto origin=args.at("origin").get<std::string>(); if (origin!=""&&origin!="source"&&origin!="ir") throw std::invalid_argument("Invalid origin");
        if (name=="get_operations"&&origin!="ir") throw std::invalid_argument("Operation queries expose IR evidence only; read original source instead of AST nodes");
        J anchor;
        if (name=="get_operations"&&!args.value("source_id",std::string()).empty()) {
            auto id=args.at("source_id").get<std::string>();
            if (!data_["source_operations"].contains(id)) throw std::invalid_argument("Unknown module source operation");
            anchor=data_["source_operations"].at(id);
        }
        J rows=J::array();
        for (const auto& row:model_.at(name=="get_module_functions"?"functions":"nodes")) {
            if (!origin.empty()&&row.at("origin")!=origin) continue;
            if (name=="get_module_functions") { if (!args.at("name").get<std::string>().empty()&&row.at("name")!=args.at("name")) continue; }
            else {
                if (row.at("kind")=="Method") continue;
                if (!anchor.is_null()) {
                    int line=row.at("line");
                    if (!matchesSourcePath(row.at("file").get<std::string>(),anchor.at("path").get<std::string>())||
                        line<anchor.at("first_line").get<int>()||line>anchor.at("last_line").get<int>()) continue;
                }
                if (args.at("function_id").get<int>()>=0&&row.at("function_id")!=args.at("function_id")) continue;
                auto path=args.at("path").get<std::string>(),file=row.at("file").get<std::string>();
                if (!path.empty()&&file!=path&&(file.size()<=path.size()||file.substr(file.size()-path.size()-1)!="/"+path)) continue;
                int first=args.at("first_line"),last=args.at("last_line"),line=row.at("line");
                if ((first>0&&line<first)||(last>0&&line>last)) continue;
            } rows.push_back(row);
        }
        J page=J::array(); for (size_t i=offset;i<rows.size()&&i<size_t(offset+limit);++i) page.push_back(rows[i]);
        return J{{"rows",page},{"total",rows.size()},{"next_offset",size_t(offset)+page.size()<rows.size()?J(size_t(offset)+page.size()):J(nullptr)}}.dump();
    }
    if (name=="get_source_operation") return sourceOperation(args).dump();
    if (name=="get_source_operations") { J result=J::array(); if (args.at("operations").empty()) throw std::invalid_argument("No operations"); for (const auto& op:args.at("operations")) result.push_back(sourceOperation(op)); return result.dump(); }
    if (auto result=executeRepositoryTool(name,args)) return *result;
    if (name=="record_gap") { nonempty(args,"description"); data_["gaps"].push_back(args.at("description")); save(); return "Gap recorded"; }
    if (name=="get_obligations") {
        auto ids=args.at("ids").get<std::set<std::string>>(); J out=J::array();
        for (const auto& r:data_["requirements"]) {
            std::string id=r.at("id"); if (!ids.empty()&&!ids.count(id)) continue;
            J row=r; if (data_["checks"].contains(id)) row["checker_evidence"]=data_["checks"].at(id);
            if (data_["reviews"].contains(id)) row["review"]=data_["reviews"].at(id); out.push_back(row);
        }
        if (!ids.empty()&&out.size()!=ids.size()) throw std::invalid_argument("Unknown Req ID"); return out.dump();
    }
    if (name=="record_protocol"&&generating_) {
        nonempty(args,"summary"); if (args.at("status")!="reviewed"&&args.at("status")!="unknown") throw std::invalid_argument("Invalid protocol status");
        for (const auto& f:args.at("functions")) if (!functions_.count(f.get<std::string>())&&!hasSourceFunction(f)) throw std::invalid_argument("Unknown module function");
        data_["protocols"].push_back(args); save(); return "Protocol recorded";
    }
    if (name=="record_pattern_review"&&generating_) {
        nonempty(args,"reason"); std::string id=args.at("pattern_id"),status=args.at("status");
        if (status!="reviewed"&&status!="not_applicable"&&status!="unknown") throw std::invalid_argument("Invalid pattern status");
        bool known=false; for (const auto& p:catalog_["patterns"]) known|=p.at("id")==id;
        if (!known) throw std::invalid_argument("Unknown catalog pattern"); data_["pattern_reviews"][id]=args; save(); return "Audit position recorded";
    }
    if ((name=="add_requirement"||name=="add_guarantee")&&generating_) {
        bool req=name=="add_requirement"; const std::string form=args.at("form");
        if (req?(form!="MustPrecede"&&form!="MustBeAtomic"&&form!="MustBeMediated"):(form!="Order"&&form!="Wait"&&form!="Exclude"&&form!="AtomicOp")) throw std::invalid_argument("Invalid contract form");
        nonempty(args,"resource"); nonempty(args,"scope"); nonempty(args,"note");
        auto a=validNodes(args,"a_nodes",false),b=validNodes(args,"b_nodes",false); auto sa=validSources(args,"a_sources"),sb=validSources(args,"b_sources");
        if ((!a.empty()&&!sa.empty())||(!b.empty()&&!sb.empty())) throw std::invalid_argument("Use one namespace per operand");
        if (a.empty()&&sa.empty()) throw std::invalid_argument("Operand A required");
        if (((req&&form!="MustBeAtomic")||form=="Order"||form=="Wait")&&b.empty()&&sb.empty()) throw std::invalid_argument("Operand B required");
        if (form=="MustBeAtomic"&&(a.size()+sa.size()<2||!b.empty()||!sb.empty())) throw std::invalid_argument("Atomic Req needs ordered multi-operation sequence and empty B");
        validRepositoryEvidence(args.value("repository_evidence_ids",J::array()),false);
        J record=args;
        if (req) {
            auto actors=args.at("actors").get<std::set<std::string>>(); if (actors.size()<2||actors.count("")) throw std::invalid_argument("At least two distinct execution activations required");
            auto ni=validNodes(args,"interference_nodes",false); auto si=validSources(args,"interference_sources");
            if (!ni.empty()&&!si.empty()) throw std::invalid_argument("Use one interference namespace");
            for (const auto& premise:args.at("assumptions")) if (!premise.is_string()) throw std::invalid_argument("Assumptions must be text");
            record["id"]="req:"+std::to_string(data_["requirements"].size()); record["origin"]="source_agent";
            data_["requirements"].push_back(record);
        } else {
            auto ev=validNodes(args,"evidence_nodes",false); auto se=validSources(args,"evidence_sources");
            if (ev.empty()&&se.empty()) throw std::invalid_argument("Synchronization evidence required"); nonempty(args,"conditions");
            if (form=="Exclude"||form=="AtomicOp") nonempty(args,"token");
            if (form=="Exclude"&&args.at("mode")!="exclusive"&&args.at("mode")!="shared") throw std::invalid_argument("Invalid exclusion mode");
            if (!args.at("unconditional").is_boolean()) throw std::invalid_argument("unconditional must be boolean");
            for (const auto& region:args.at("regions")) { validNodes(J{{"boundary",{region.at("start_node"),region.at("end_node")}}},"boundary"); }
            for (const auto& region:args.value("source_regions",J::array())) validSources(J{{"bounds",{region.at("start_source"),region.at("end_source")}}},"bounds");
            record["id"]="gua:"+std::to_string(data_["guarantees"].size()); data_["guarantees"].push_back(record);
        }
        save(); return J{{"id",record.at("id")},{"recorded",true},{"semantics_verified",false}}.dump();
    }
    if (name=="finalize_contract"&&generating_) {
        J missing=J::array(); for (const auto& p:catalog_["patterns"]) if (!data_["pattern_reviews"].contains(p.at("id"))) missing.push_back(p.at("id"));
        if (!missing.empty()) return J{{"error","Unaccounted catalog patterns"},{"missing",missing}}.dump();
        generationFinished_=true; save(); return "finish";
    }
    if (name=="review_obligations"&&!generating_) {
        std::string status=args.at("status"); if (status!="source_supported"&&status!="satisfied"&&status!="not_applicable"&&status!="unknown") throw std::invalid_argument("Invalid judgment");
        auto ev=validNodes(args,"evidence_nodes",false),w=validNodes(args,"witness_nodes",false); auto se=validSources(args,"evidence_sources"),sw=validSources(args,"witness_sources");
        if (status!="unknown"&&ev.empty()&&se.empty()) throw std::invalid_argument("Evidence required"); nonempty(args,"reason");
        if (status=="source_supported") { nonempty(args,"interleaving"); if (w.empty()&&sw.empty()) throw std::invalid_argument("Concrete witness required"); }
        auto ids=args.at("ids").get<std::set<std::string>>(); if (ids.empty()) throw std::invalid_argument("No Req IDs"); std::set<std::string> found;
        validRepositoryEvidence(args.value("repository_evidence_ids",J::array()),false);
        for (const auto& r:data_["requirements"]) if (ids.count(r.at("id"))) {
            if (status=="source_supported") {
                auto touched=[&](const char* numeric,const char* sources) {
                    for (int n:w) for (const auto& a:r.value(numeric,J::array())) if (a==n) return true;
                    for (const auto& s:sw) for (const auto& a:r.value(sources,J::array())) if (a==s) return true; return false;
                };
                if (!touched("a_nodes","a_sources")||
                    (r.at("form")=="MustBeAtomic"?!touched("interference_nodes","interference_sources"):!touched("b_nodes","b_sources")))
                    throw std::invalid_argument("Witness must include Req operations and opposing/interfering operations");
            } found.insert(r.at("id"));
        }
        if (found!=ids) throw std::invalid_argument("Unknown Req ID"); for (const auto& id:ids) data_["reviews"][id]=args; save(); return "Source judgment recorded; not reproduced";
    }
    if (name=="finalize_review"&&!generating_) {
        J missing=J::array(); for (const auto& r:data_["requirements"]) if (!data_["reviews"].contains(r.at("id"))) missing.push_back(r.at("id"));
        if (!missing.empty()) return J{{"error","Unreviewed requirements"},{"missing",missing}}.dump(); reviewFinished_=true; save(); return "finish";
    }
    if (name=="read_file"||name=="grep_source"||name=="list_dir") if (auto result=SharedToolKit::handle_shared_tool(name,args,ccpg_)) return *result;
    throw std::invalid_argument("Unknown tool or wrong phase");
}
void ModuleContractAgent::review(const J& checks) {
    generating_=false; data_["checks"]=checks; data_["e_finished"]=true;save();
    try {
        send_message("Phase C. Continue in this SAME conversation. Review ALL Req using get_obligations, including E discharges. "
            "E implements sufficient deterministic rules under declared Gua premises. Undischarged does not mean a feasible counterexample; "
            "source-only operands remain explicit unknowns in E and must still be reviewed. Verify actual mechanisms, scope, same instance/generation, "
            "admission/requeue and whole-transaction coverage. Report concrete harmful interleavings or preserve unknowns. End with finalize_review. Requirements: "+std::to_string(data_["requirements"].size()));
        if (!reviewFinished_) throw std::runtime_error("Agent did not finalize review"); save();
    } catch (const std::exception& e) { save(e.what()); throw; }
}
void ModuleContractAgent::finishWithoutReview(const J& checks) {
    data_["checks"]=checks; data_["review_disabled"]=true;
    data_["e_finished"]=true;data_["partial_extraction"]=!generationFinished_;save();
}
std::vector<query::ModuleFinding> ModuleContractAgent::findings() const {
    std::vector<query::ModuleFinding> out;
    for (const auto& r:data_.at("requirements")) {
        std::string id=r.at("id");
        if (data_.value("review_disabled",false)) {
            const auto& e=data_.at("checks").at(id); if (!e.value("e_candidate",false)) continue;
            query::ModuleFinding f; f.id=id;f.resource=r.at("resource");f.severity="unverified";
            f.origin="deterministic_E";f.status="static_contract_candidate";
            f.bug_category=r.at("form")=="MustPrecede"?"order_violation":r.at("form")=="MustBeAtomic"?"atomicity_violation":"data_race";
            f.description="[E candidate; not a proved feasible interleaving] "+r.at("note").get<std::string>()+"\n"+e.dump();
            for (const char* operand:{"a_nodes","b_nodes","interference_nodes"}) {
                size_t i=0; for (int n:e.at("resolved_operands").at(operand)) f.nodes[std::string(operand)+std::to_string(i++)]=n;
            }
            for (const char* operand:{"a_sources","b_sources","interference_sources"}) for (const auto& s:r.value(operand,J::array())) f.source_operations.push_back(data_["source_operations"].at(s.get<std::string>()));
            out.push_back(f);continue;
        }
        if (!data_["reviews"].contains(id)) continue; const auto& v=data_["reviews"].at(id);
        if (v.at("status")!="source_supported") continue; query::ModuleFinding f; f.id=id; f.resource=r.at("resource"); f.severity="medium";
        f.bug_category=r.at("form")=="MustPrecede"?"order_violation":r.at("form")=="MustBeAtomic"?"atomicity_violation":"data_race";
        f.description="[source-supported; not reproduced] "+v.at("reason").get<std::string>()+"\n"+v.at("interleaving").get<std::string>();
        size_t i=0; for (int n:v.at("witness_nodes")) f.nodes["witness_"+std::to_string(i++)]=n;
        for (const auto& s:v.value("witness_sources",J::array())) f.source_operations.push_back(data_["source_operations"].at(s.get<std::string>())); out.push_back(f);
    } return out;
}
void ModuleContractAgent::save(const std::string& error) const {
    J report=data_; size_t unknown=0,unmapped=0;
    for (const auto& p:data_["pattern_reviews"].items()) if (p.value().at("status")=="unknown") ++unknown;
    for (const auto& p:data_["protocols"]) if (p.at("status")=="unknown") ++unknown;
    for (const auto& r:data_["requirements"]) { std::string id=r.at("id"); if (!data_["reviews"].contains(id)||data_["reviews"].at(id).at("status")=="unknown") ++unknown; }
    for (const auto& op:data_["source_operations"].items()) if (op.value().value("ir_candidate_count",size_t(0))==0) ++unmapped;
    report["coverage"]={{"requirements",data_["requirements"].size()},{"catalog_patterns",catalog_["patterns"].size()},
        {"accounted_patterns",data_["pattern_reviews"].size()},{"unknown_positions",unknown},{"source_anchors_without_ir_candidates",unmapped},
        {"generation_finished",generationFinished_},{"review_finished",reviewFinished_},
        {"review_disabled",data_.value("review_disabled",false)},
        {"e_finished",data_.value("e_finished",false)},
        {"workflow_complete",error.empty()&&generationFinished_&&(reviewFinished_||data_.value("review_disabled",false))},
        {"scope","Workflow accounting and supplied-input graph coverage; no guarantee that all concurrency requirements were extracted or proved"}};
    if (!error.empty()) report["error"]=error;
    auto path=TargetPath::getInstance()->getOutputDir()/"module_contracts.json";
    std::ofstream out(path.string()+".tmp"); out<<report.dump(2); out.close(); fs::rename(path.string()+".tmp",path);
}
}
