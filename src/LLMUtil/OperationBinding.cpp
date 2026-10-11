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
J bindSourceOperation(const J& op,const J& model,const J& manifest) {
    J coordinates=J::array(),selected=J::array();
    auto role=op.value("operation_kind",std::string("unspecified")),symbol=op.value("symbol",std::string()),fn=op.value("function",std::string());
    auto text=norm(op.value("content",std::string()));
    const bool selectorInSource=symbol.empty()||text.find(norm(symbol))!=std::string::npos;
    for (const auto& n:model.at("nodes")) {
        if (n.at("origin")!="ir"||n.at("kind")=="Method") continue;
        J locs=n.value("source_locations",J::array());
        if (locs.empty()) locs.push_back({{"file",n.value("file","")},{"line",n.value("line",0)}});
        bool location=false;
        for (const auto& loc:locs) if (loc.at("line").get<int>()>=op.at("first_line").get<int>()&&loc.at("line").get<int>()<=op.at("last_line").get<int>()&&matches(loc.at("file"),op.at("path"),manifest)) location=true;
        if (!location) continue; coordinates.push_back(n.at("node_id"));
        if (!selectorInSource||(!fn.empty()&&n.value("source_function",std::string())!=fn)) continue;
        const auto actualRole=n.value("memory_role",std::string());
        if (role!="unspecified"&&actualRole!=role&&!(role=="atomic"&&n.value("atomic",false))) continue;
        bool semantic=false;
        if (actualRole=="call"||n.at("kind")=="call"||n.at("kind")=="invoke") {
            const auto api=n.value("api",std::string());
            semantic=!api.empty()&&api.rfind("llvm.",0)!=0&&(symbol.empty()?text.find(api+"(")!=std::string::npos:api==symbol);
            // norm strips parentheses; use original text to identify call syntax.
            if (symbol.empty()&&!api.empty()&&api.rfind("llvm.",0)!=0) semantic=std::regex_search(op.value("content",std::string()),std::regex("\\b"+api+"\\s*\\("));
        } else if (!symbol.empty()) {
            for (const auto& p:n.value("access_paths",J::array())) semantic|=norm(p.get<std::string>())==norm(symbol);
        }
        if (semantic) selected.push_back(n.at("node_id"));
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
        {"instance_equivalence_proved",false},{"semantic_correctness_verified",false}};
}
}
