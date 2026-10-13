#include "LLMUtil/ModuleContractAgent.h"
#include "LLMUtil/ContractArtifact.h"
#include "Util/TargetPath.h"
#include <fstream>
#include <algorithm>
#include <stdexcept>

namespace llm_client {
using J=nlohmann::json;
J ModuleContractAgent::artifactFeedback(const std::string& id) const {
    const J* claim=nullptr;
    for (const char* collection:{"requirements","guarantees"}) for (const auto& item:data_.at(collection))
        if (item.at("id")==id) claim=&item;
    if (!claim) throw std::invalid_argument("Unknown imported contract");
    std::set<std::string> sources;
    for (const char* key:{"a_sources","b_sources","interference_sources","evidence_sources"})
        for (const auto& source:claim->value(key,J::array())) sources.insert(source.get<std::string>());
    for (const auto& region:claim->value("source_regions",J::array()))
        for (const char* key:{"start_source","end_source"}) sources.insert(region.at(key).get<std::string>());
    size_t bound=0;J unknown=J::array();
    for (const auto& source:sources) {
        const auto& op=data_.at("source_operations").at(source);
        if (op.at("binding")=="bound") ++bound;
        else {
            J location={{"path",op.at("path")},{"line",op.at("first_line")},
                {"kind",op.value("operation_kind",std::string("unspecified"))},
                {"expression",op.value("symbol",std::string())}};
            if (op.contains("function")) location["function"]=op.at("function");
            if (op.at("first_line")!=op.at("last_line")) location["end_line"]=op.at("last_line");
            unknown.push_back({{"operation",location},{"status",op.at("binding")},
                {"ir_candidate_count",op.at("ir_candidate_count")}});
        }
    }
    return {{"id",id},{"bound_operations",bound},{"unknown_operations",unknown},
        {"semantics_verified",false},{"unknown_permitted",true}};
}
J ModuleContractAgent::syncContractArtifacts() {
    if (!artifactMode_) return {{"enabled",false}};
    if (!generating_) throw std::invalid_argument("Contract artifacts are read-only during review");
    const auto workspace=TargetPath::getInstance()->getOutputDir()/"agent_workspace";
    const auto directory=workspace/"contracts";
    fs::create_directories(directory);
    if (fs::is_symlink(directory)) throw std::invalid_argument("Contract directory cannot be a symlink");
    if (!data_.contains("contract_artifacts")) data_["contract_artifacts"]=J::object();
    if (!data_.contains("artifact_revisions")) data_["artifact_revisions"]=J::array();
    J accepted=J::array(),errors=J::object();std::set<std::string> present;
    std::vector<fs::path> files;
    for (const auto& entry:fs::recursive_directory_iterator(directory)) {
        if (entry.path().extension()==".json") files.push_back(entry.path());
    }
    std::sort(files.begin(),files.end());
    for (const auto& path:files) {
        const auto name=path.lexically_relative(directory).generic_string();present.insert(name);
        const auto previous=data_["contract_artifacts"].value(name,J(nullptr));
        J backup=data_;
        try {
            if (fs::is_symlink(path)||!fs::is_regular_file(path))
                throw std::invalid_argument("Contract artifacts must be regular JSON files");
            std::ifstream in(path);J artifact;in>>artifact;
            if (!previous.is_null()&&previous.at("artifact")==artifact) continue;
            importingArtifacts_=true;
            auto arguments=translateContractArtifact(artifact,[&](const J& op) {
                auto registered=sourceOperation(op);
                if (registered.at("binding")=="selector_not_in_source")
                    throw std::invalid_argument("Operation expression is not in the cited source statement");
                return registered.at("source_id").get<std::string>();
            });
            const auto kind=artifact.at("kind").get<std::string>();
            if (!previous.is_null()) {
                if (previous.at("kind")!=kind) throw std::invalid_argument("Use a new file to change contract kind");
                arguments["replace_id"]=previous.at("id");
                arguments["correction_reason"]="Updated source contract artifact "+name;
            }
            const auto reply=J::parse(execute_tool(kind=="requirement"?"_artifact_requirement":"_artifact_guarantee",arguments));
            const auto id=reply.at("id").get<std::string>();
            const size_t revision=previous.is_null()?1:previous.at("revision").get<size_t>()+1;
            data_["contract_artifacts"][name]={{"id",id},{"kind",kind},{"revision",revision},{"artifact",artifact}};
            data_["artifact_revisions"].push_back({{"file",name},{"event",previous.is_null()?"created":"updated"},
                {"id",id},{"revision",revision},{"previous",previous},{"artifact",artifact}});
            J row=artifactFeedback(id);row["file"]=name;row["revision"]=revision;accepted.push_back(row);
            importingArtifacts_=false;
        } catch (const std::exception& error) {
            importingArtifacts_=false;data_=std::move(backup);errors[name]=error.what();
        }
    }
    std::vector<std::string> removed;
    for (const auto& entry:data_["contract_artifacts"].items()) if (!present.count(entry.key())) removed.push_back(entry.key());
    for (const auto& name:removed) {
        const auto previous=data_["contract_artifacts"].at(name);const auto id=previous.at("id");
        auto& claims=data_[previous.at("kind")=="requirement"?"requirements":"guarantees"];
        for (auto item=claims.begin();item!=claims.end();++item) if (item->at("id")==id) {claims.erase(item);break;}
        data_["artifact_revisions"].push_back({{"file",name},{"event","deleted"},{"previous",previous}});
        data_["contract_artifacts"].erase(name);accepted.push_back({{"file",name},{"id",id},{"deleted",true}});
    }
    data_["artifact_errors"]=errors;save();
    J status={{"schema","lace-contract-artifact-status-v1"},{"files",J::object()},{"errors",errors},
        {"unknown_permitted",true},{"semantic_completeness_proved",false}};
    for (const auto& entry:data_["contract_artifacts"].items()) {
        auto row=artifactFeedback(entry.value().at("id"));row["revision"]=entry.value().at("revision");
        status["files"][entry.key()]=row;
    }
    fs::create_directories(workspace);
    const auto output=workspace/"contract_status.json";
    {std::ofstream out(output.string()+".tmp");out<<status.dump(2);}
    fs::rename(output.string()+".tmp",output);
    return {{"accepted",accepted},{"errors",errors},{"status_file","contract_status.json"},
        {"unknown_permitted",true},{"semantic_completeness_proved",false}};
}
}
