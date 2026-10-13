#include "LLMUtil/ContractArtifact.h"
#include <set>
#include <stdexcept>
#include <string>

namespace llm_client {
using J=nlohmann::json;
namespace {
void keys(const J& value,const std::set<std::string>& allowed) {
    if (!value.is_object()) throw std::invalid_argument("Expected a JSON object");
    for (const auto& item:value.items()) if (!allowed.count(item.key()))
        throw std::invalid_argument("Unknown artifact field: "+item.key());
}
void text(const J& value,const char* name) {
    if (!value.contains(name)||!value.at(name).is_string()||
        value.at(name).get<std::string>().find_first_not_of(" \t\r\n")==std::string::npos)
        throw std::invalid_argument(std::string("Nonblank text required: ")+name);
}
J strings(const J& value,const char* name) {
    auto result=value.value(name,J::array());
    if (!result.is_array()) throw std::invalid_argument(std::string("Array required: ")+name);
    for (const auto& item:result) if (!item.is_string())
        throw std::invalid_argument(std::string("Text entries required: ")+name);
    return result;
}
}
J artifactOperationArguments(const J& value) {
    keys(value,{"path","line","end_line","kind","expression","function"});
    text(value,"path");text(value,"kind");text(value,"expression");
    if (!value.contains("line")||!value.at("line").is_number_integer())
        throw std::invalid_argument("Integer source line required");
    if (value.contains("end_line")&&!value.at("end_line").is_number_integer())
        throw std::invalid_argument("Integer end_line required");
    const int first=value.at("line"),last=value.value("end_line",first);
    if (first<1||last<first) throw std::invalid_argument("Invalid source line span");
    const std::string kind=value.at("kind");
    if (!std::set<std::string>{"read","write","call","atomic","fence"}.count(kind))
        throw std::invalid_argument("Invalid source operation kind");
    std::string expression=value.at("expression");
    expression.erase(0,expression.find_first_not_of(" \t\r\n"));
    // A call's source callee is enough for correspondence; argument expressions
    // remain in the original line and in the unmodified artifact for auditing.
    if (kind=="call"||kind=="atomic") {
        const auto bracket=expression.find('(');
        if (bracket!=std::string::npos&&bracket>0) expression.resize(bracket);
        const auto end=expression.find_last_not_of(" \t");
        if (end!=std::string::npos) expression.resize(end+1);
    }
    J result={{"path",value.at("path")},{"first_line",first},{"last_line",last},
        {"operation_kind",kind},{"symbol",expression},{"statement_binding",true}};
    if (value.contains("function")) {text(value,"function");result["function"]=value.at("function");}
    return result;
}
J translateContractArtifact(const J& value,
    const std::function<std::string(const J&)>& registerOperation) {
    keys(value,{"schema","kind","form","resource","scope","a","b","interference",
        "actors","assumptions","evidence","regions","token","mode","conditions",
        "unconditional","repository_evidence_ids","note"});
    if (value.value("schema",std::string("lace-source-contract-v1"))!="lace-source-contract-v1")
        throw std::invalid_argument("Unsupported contract artifact schema");
    for (const char* key:{"kind","form","resource","scope","note"}) text(value,key);
    const std::string kind=value.at("kind"),form=value.at("form");
    const bool req=kind=="requirement";
    if (!req&&kind!="guarantee") throw std::invalid_argument("kind must be requirement or guarantee");
    if (!(req?std::set<std::string>{"MustPrecede","MustBeMediated","MustBeAtomic"}:
             std::set<std::string>{"Order","Wait","Exclude","AtomicOp"}).count(form))
        throw std::invalid_argument("Invalid contract form for this kind");
    J result={{"form",form},{"resource",value.at("resource")},{"scope",value.at("scope")},{"note",value.at("note")},
        {"a_nodes",J::array()},{"b_nodes",J::array()},{"interference_nodes",J::array()},
        {"repository_evidence_ids",strings(value,"repository_evidence_ids")}};
    auto operations=[&](const char* key) {
        auto items=value.value(key,J::array());
        if (!items.is_array()) throw std::invalid_argument(std::string("Operation array required: ")+key);
        J ids=J::array();
        for (const auto& op:items) ids.push_back(registerOperation(artifactOperationArguments(op)));
        return ids;
    };
    result["a_sources"]=operations("a");result["b_sources"]=operations("b");
    if (req) {
        for (const char* key:{"evidence","regions","token","mode","conditions","unconditional"})
            if (value.contains(key)) throw std::invalid_argument(std::string("Guarantee-only field: ")+key);
        result["actors"]=strings(value,"actors");result["assumptions"]=strings(value,"assumptions");
        result["interference_sources"]=operations("interference");
    } else {
        for (const char* key:{"actors","assumptions","interference"})
            if (value.contains(key)) throw std::invalid_argument(std::string("Requirement-only field: ")+key);
        text(value,"conditions");
        if (!value.contains("unconditional")||!value.at("unconditional").is_boolean())
            throw std::invalid_argument("Explicit boolean unconditional required");
        result["conditions"]=value.at("conditions");result["unconditional"]=value.at("unconditional");
        result["token"]=value.value("token",std::string());result["mode"]=value.value("mode",std::string());
        result["evidence_nodes"]=J::array();result["evidence_sources"]=operations("evidence");
        result["regions"]=J::array();result["source_regions"]=J::array();
        const auto regions=value.value("regions",J::array());
        if (!regions.is_array()) throw std::invalid_argument("regions must be an array");
        for (const auto& region:regions) {
            keys(region,{"start","end"});
            result["source_regions"].push_back({
                {"start_source",registerOperation(artifactOperationArguments(region.at("start")))},
                {"end_source",registerOperation(artifactOperationArguments(region.at("end")))}});
        }
    }
    if (result.at("a_sources").empty()) throw std::invalid_argument("Operand A required");
    if ((req&&form!="MustBeAtomic")||form=="Order"||form=="Wait")
        if (result.at("b_sources").empty()) throw std::invalid_argument("Operand B required");
    if (req) {
        const auto actors=result.at("actors").get<std::set<std::string>>();
        if (actors.size()<2||actors.count("")) throw std::invalid_argument("At least two distinct actors required");
        if (form=="MustBeAtomic"&&(result.at("a_sources").size()<2||!result.at("b_sources").empty()))
            throw std::invalid_argument("Atomic Req needs ordered multi-operation sequence and empty B");
    } else {
        if (result.at("evidence_sources").empty()) throw std::invalid_argument("Synchronization evidence required");
        if (form=="Exclude"||form=="AtomicOp") text(result,"token");
        if (form=="Exclude"&&result.at("mode")!="exclusive"&&result.at("mode")!="shared")
            throw std::invalid_argument("Invalid exclusion mode");
    }
    return result;
}
}
