#pragma once
#include "nlohmann/json.hpp"
#include <map>
#include <string>
#include <vector>
#include <filesystem>
class CCPG;
namespace query {
struct ModuleFinding {
    std::string id, resource, description, bug_category, severity;
    std::map<std::string,int> nodes;
    nlohmann::json source_operations = nlohmann::json::array();
};
// Only exact duplicate witnesses for the same resource/category are combined.
// Keep every contributing requirement and explanation in the structured report.
void writeModuleFindings(const std::vector<ModuleFinding>& findings, CCPG* ccpg,
    const std::filesystem::path& output);
}
