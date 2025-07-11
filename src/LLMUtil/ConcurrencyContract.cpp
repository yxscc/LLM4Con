#include "LLMUtil/ConcurrencyContract.h"
#include <stdexcept>

namespace LLM {

// Helper to convert ThreadRole enum to string
std::string threadRoleToString(ConcurrencyContract::ThreadRole role) {
    switch (role) {
        case ConcurrencyContract::ThreadRole::WORKER: return "WORKER";
        case ConcurrencyContract::ThreadRole::LISTENER: return "LISTENER";
        case ConcurrencyContract::ThreadRole::PERIODIC_TASK: return "PERIODIC_TASK";
        case ConcurrencyContract::ThreadRole::IO_BOUND: return "IO_BOUND";
        case ConcurrencyContract::ThreadRole::CPU_BOUND: return "CPU_BOUND";
        default: return "UNKNOWN";
    }
}

// Helper to convert string to ThreadRole enum
ConcurrencyContract::ThreadRole stringToThreadRole(const std::string& roleStr) {
    if (roleStr == "WORKER") return ConcurrencyContract::ThreadRole::WORKER;
    if (roleStr == "LISTENER") return ConcurrencyContract::ThreadRole::LISTENER;
    if (roleStr == "PERIODIC_TASK") return ConcurrencyContract::ThreadRole::PERIODIC_TASK;
    if (roleStr == "IO_BOUND") return ConcurrencyContract::ThreadRole::IO_BOUND;
    if (roleStr == "CPU_BOUND") return ConcurrencyContract::ThreadRole::CPU_BOUND;
    return ConcurrencyContract::ThreadRole::UNKNOWN;
}

// Constructor
ConcurrencyContract::ConcurrencyContract(NodeID entryPoint) 
    : threadEntryPointFunctionID(entryPoint), role(ThreadRole::UNKNOWN), confidenceScore(0.0), version(1) {}

// Comparison operator for LockIdentifier
bool operator<(const ConcurrencyContract::LockIdentifier& lhs, const ConcurrencyContract::LockIdentifier& rhs) {
    return lhs.cpgNodeID < rhs.cpgNodeID;
}

// JSON Serialization
void to_json(nlohmann::json& j, const ConcurrencyContract::SharedResource& r) {
    j = {{"cpgNodeID", r.cpgNodeID}, {"variableName", r.variableName}, {"typeName", r.typeName}};
}

void from_json(const nlohmann::json& j, ConcurrencyContract::SharedResource& r) {
    j.at("cpgNodeID").get_to(r.cpgNodeID);
    j.at("variableName").get_to(r.variableName);
    j.at("typeName").get_to(r.typeName);
}

void to_json(nlohmann::json& j, const ConcurrencyContract::LockIdentifier& l) {
    j = {{"cpgNodeID", l.cpgNodeID}, {"lockName", l.lockName}, {"isCustom", l.isCustom}};
}

void from_json(const nlohmann::json& j, ConcurrencyContract::LockIdentifier& l) {
    j.at("cpgNodeID").get_to(l.cpgNodeID);
    j.at("lockName").get_to(l.lockName);
    j.at("isCustom").get_to(l.isCustom);
}

void to_json(nlohmann::json& j, const ConcurrencyContract::DataLockBinding& b) {
    j = {{"lock", b.lock}, {"protectedResources", b.protectedResources}};
}

void from_json(const nlohmann::json& j, ConcurrencyContract::DataLockBinding& b) {
    j.at("lock").get_to(b.lock);
    j.at("protectedResources").get_to(b.protectedResources);
}

void to_json(nlohmann::json& j, const ConcurrencyContract::LockOrderConstraint& o) {
    j = {{"orderedLocks", o.orderedLocks}};
}

void from_json(const nlohmann::json& j, ConcurrencyContract::LockOrderConstraint& o) {
    j.at("orderedLocks").get_to(o.orderedLocks);
}

void to_json(nlohmann::json& j, const ConcurrencyContract::CustomSyncProtocol& p) {
    j = {{"primitiveIdentifier", p.primitiveIdentifier}, {"lockFunctionID", p.lockFunctionID}, {"unlockFunctionID", p.unlockFunctionID}};
}

void from_json(const nlohmann::json& j, ConcurrencyContract::CustomSyncProtocol& p) {
    j.at("primitiveIdentifier").get_to(p.primitiveIdentifier);
    j.at("lockFunctionID").get_to(p.lockFunctionID);
    j.at("unlockFunctionID").get_to(p.unlockFunctionID);
}

void to_json(nlohmann::json& j, const ConcurrencyContract::Synchronization& s) {
    j = {{"dataLockBindings", s.dataLockBindings}, {"lockOrders", s.lockOrders}, {"customProtocols", s.customProtocols}};
}

void from_json(const nlohmann::json& j, ConcurrencyContract::Synchronization& s) {
    j.at("dataLockBindings").get_to(s.dataLockBindings);
    j.at("lockOrders").get_to(s.lockOrders);
    j.at("customProtocols").get_to(s.customProtocols);
}

void to_json(nlohmann::json& j, const ConcurrencyContract::StateMachineModel::Transition& t) {
    j = {{"fromState", t.fromState}, {"toState", t.toState}, {"triggerActionNodeID", t.triggerActionNodeID}};
}

void from_json(const nlohmann::json& j, ConcurrencyContract::StateMachineModel::Transition& t) {
    j.at("fromState").get_to(t.fromState);
    j.at("toState").get_to(t.toState);
    j.at("triggerActionNodeID").get_to(t.triggerActionNodeID);
}

void to_json(nlohmann::json& j, const ConcurrencyContract::StateMachineModel& m) {
    j = {{"stateVariable", m.stateVariable}, {"initialState", m.initialState}, {"transitions", m.transitions}};
}

void from_json(const nlohmann::json& j, ConcurrencyContract::StateMachineModel& m) {
    j.at("stateVariable").get_to(m.stateVariable);
    j.at("initialState").get_to(m.initialState);
    j.at("transitions").get_to(m.transitions);
}

void to_json(nlohmann::json& j, const ConcurrencyContract::HappensBeforeConstraint& c) {
    j = {{"sourceActionNodeID", c.sourceActionNodeID}, {"sinkActionNodeID", c.sinkActionNodeID}, {"logicDescription", c.logicDescription}};
}

void from_json(const nlohmann::json& j, ConcurrencyContract::HappensBeforeConstraint& c) {
    j.at("sourceActionNodeID").get_to(c.sourceActionNodeID);
    j.at("sinkActionNodeID").get_to(c.sinkActionNodeID);
    j.at("logicDescription").get_to(c.logicDescription);
}


std::string ConcurrencyContract::toJson() const {
    nlohmann::json j;
    j["threadEntryPointFunctionID"] = this->threadEntryPointFunctionID;
    j["threadName"] = this->threadName;
    j["role"] = threadRoleToString(this->role);
    j["coreSharedState"] = this->coreSharedState;
    j["syncDiscipline"] = this->syncDiscipline;
    if (this->stateModel.has_value()) {
        j["stateModel"] = this->stateModel.value();
    }
    j["happensBeforeConstraints"] = this->happensBeforeConstraints;
    j["confidenceScore"] = this->confidenceScore;
    j["reasoning"] = this->reasoning;
    j["criticFeedback"] = this->criticFeedback;
    j["version"] = this->version;
    return j.dump(4); // Pretty print
}

ConcurrencyContract ConcurrencyContract::fromJson(const std::string& jsonString) {
    auto j = nlohmann::json::parse(jsonString);
    
    if (!j.contains("threadEntryPointFunctionID")) {
        throw std::invalid_argument("JSON must contain 'threadEntryPointFunctionID'");
    }
    ConcurrencyContract contract(j.at("threadEntryPointFunctionID").get<NodeID>());

    contract.threadName = j.at("threadName").get<std::string>();
    contract.role = stringToThreadRole(j.at("role").get<std::string>());
    contract.coreSharedState = j.at("coreSharedState").get<std::vector<SharedResource>>();
    contract.syncDiscipline = j.at("syncDiscipline").get<Synchronization>();

    if (j.contains("stateModel") && !j["stateModel"].is_null()) {
        contract.stateModel = j.at("stateModel").get<StateMachineModel>();
    }

    contract.happensBeforeConstraints = j.at("happensBeforeConstraints").get<std::vector<HappensBeforeConstraint>>();
    contract.confidenceScore = j.at("confidenceScore").get<double>();
    contract.reasoning = j.at("reasoning").get<std::string>();
    contract.criticFeedback = j.at("criticFeedback").get<std::vector<std::string>>();
    contract.version = j.at("version").get<int>();

    return contract;
}

} // namespace LLM
