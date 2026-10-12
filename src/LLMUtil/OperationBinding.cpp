#include "LLMUtil/OperationBinding.h"
#include <filesystem>
#include <regex>
#include <set>
namespace llm_client {
using J=nlohmann::json;
namespace {
std::string norm(std::string s) {
    s=std::regex_replace(s,std::regex("->"),".");
    s=std::regex_replace(s,std::regex("[[:space:]()]"),""); return s;
}
bool matches(const std::string& file,const std::string& path,const J& manifest) {
    namespace fs=std::filesystem;
    auto f=fs::path(file).lexically_normal(),p=fs::path(path).lexically_normal();
    if (f==p) return true;
    if (p.is_absolute()||f.filename()!=p.filename()) return false;
    size_t count=0; for (const auto& row:manifest) count+=fs::path(row.at("path").get<std::string>()).filename()==p.filename();
    return count==1;
}
}
bool sourceCoordinateMatches(const J& n,const J& op,const J& manifest) {
    J locs=n.value("source_locations",J::array());
    if (locs.empty()) locs.push_back({{"file",n.value("file","")},{"line",n.value("line",0)}});
    for (const auto& loc:locs) if (loc.at("line").get<int>()>=op.at("first_line").get<int>()&&
        loc.at("line").get<int>()<=op.at("last_line").get<int>()&&
        (op.at("path")==""||matches(loc.at("file"),op.at("path"),manifest))) return true;
    return false;
}
J bindSourceOperation(const J& op,const J& model,const J& manifest) {
    J coordinates=J::array(),selected=J::array(),correspondence=J::array();
    auto role=op.value("operation_kind",std::string("unspecified")),symbol=op.value("symbol",std::string()),fn=op.value("function",std::string());
    auto text=norm(op.value("content",std::string()));
    const bool selectorInSource=symbol.empty()||text.find(norm(symbol))!=std::string::npos;
    for (const auto& n:model.at("nodes")) {
        if (n.at("origin")!="ir"||n.at("kind")=="Method") continue;
        if (!sourceCoordinateMatches(n,op,manifest)) continue; coordinates.push_back(n.at("node_id"));
        if (!selectorInSource||(!fn.empty()&&n.value("source_function",std::string())!=fn)) continue;
        const auto actualRole=n.value("memory_role",std::string());
        bool semantic=false;
        std::string basis;
        const bool roleMatches=role=="unspecified"||actualRole==role||(role=="atomic"&&n.value("atomic",false));
        if (roleMatches&&(actualRole=="call"||n.at("kind")=="call"||n.at("kind")=="invoke")) {
            auto api=n.value("api",std::string()); if (api.empty()) api=n.value("callee_api",std::string());
            semantic=!api.empty()&&api.rfind("llvm.",0)!=0&&(symbol.empty()?text.find(api+"(")!=std::string::npos:api==symbol);
            // norm strips parentheses; use original text to identify call syntax.
            if (symbol.empty()&&!api.empty()&&api.rfind("llvm.",0)!=0) semantic=std::regex_search(op.value("content",std::string()),std::regex("\\b"+api+"\\s*\\("));
            if (semantic) basis="direct_callee";
            for (const auto& p:n.value("callee_access_paths",J::array())) if (!symbol.empty()&&norm(p.get<std::string>())==norm(symbol)) {semantic=true;basis="indirect_callee_expression";}
        } else if (roleMatches&&!symbol.empty()) {
            for (const auto& p:n.value("access_paths",J::array())) semantic|=norm(p.get<std::string>())==norm(symbol);
            if (semantic) basis="compiler_access_path";
        }
        // A source API invocation can lower to a non-call/non-LLVM-atomic
        // instruction. Match its inlined call site and concrete external effect;
        // this establishes correspondence only, never an atomicity guarantee.
        if (!semantic&&(role=="call"||role=="atomic")) for (const auto& effect:n.value("inline_effects",J::array())) {
            if (!sourceCoordinateMatches(J{{"file",effect.at("file")},{"line",effect.at("line")}},op,manifest)) continue;
            if (!symbol.empty()&&effect.at("api")==symbol) {semantic=true;basis="compiler_inline_effect";}
        }
        if (semantic) { selected.push_back(n.at("node_id"));correspondence.push_back({{"node_id",n.at("node_id")},{"kind",basis}}); }
    }
    std::string status;
    if (!selectorInSource) status="selector_not_in_source";
    else if (coordinates.empty()) status="no_ir_candidate";
    else if (selected.empty()) status="operation_not_resolved";
    else if (selected.size()!=1) status="ambiguous";
    else status="bound";
    return {{"status",status},{"coordinate_candidates",coordinates},{"matching_candidates",selected},
        {"node_id",status=="bound"?selected.front():J(nullptr)},
        {"basis","debug location plus source selector and compiler callee/access facts"},
        {"correspondence",correspondence},
        {"instance_equivalence_proved",false},{"semantic_correctness_verified",false}};
}
}
