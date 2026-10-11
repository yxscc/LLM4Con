#include "Query/ModuleFinding.h"
#include "CCPG/CCPG.h"
#include <fstream>
#include <iostream>
#include <set>
namespace query {
void writeModuleFindings(const std::vector<ModuleFinding>& findings, CCPG* ccpg, const std::filesystem::path& output) {
    using J = nlohmann::json;
    J reports = J::array(); std::map<std::string,size_t> signatures;
    for (const auto& f : findings) {
        J nodes = J::array(); std::set<std::string> locations;
        for (const auto& [role,id] : f.nodes) {
            auto n = ccpg->getNodeByID(id);
            J row = {{"role",role},{"node_id",id}};
            if (n && n->getCPGNode()) {
                row["code"] = n->getCPGNode()->getCode(); row["location"] = n->getNodeLoc().toString();
                locations.insert(row.at("location").get<std::string>() + ":" + row.at("code").get<std::string>());
            } else locations.insert("node:" + std::to_string(id));
            nodes.push_back(row);
        }
        for (const auto& s : f.source_operations) locations.insert(s.at("path").get<std::string>() + ":" + s.at("first_line").dump() + ":" + s.at("last_line").dump());
        const auto sig = J({{"resource",f.resource},{"category",f.bug_category},{"locations",locations},{"origin",f.origin}}).dump();
        J angle = {{"requirement_id",f.id},{"description",f.description},{"nodes",nodes},{"source_operations",f.source_operations}};
        if (signatures.count(sig)) reports[signatures.at(sig)]["evidence"].push_back(angle);
        else {
            signatures[sig] = reports.size();
            reports.push_back({{"report_id","finding:" + std::to_string(reports.size())},{"status",f.status},{"origin",f.origin},
                {"reproduced",false},{"resource",f.resource},{"bug_category",f.bug_category},{"severity",f.severity},{"evidence",J::array({angle})}});
        }
    }
    const auto dir = output / "stateful_bugs"; std::filesystem::create_directories(dir);
    std::ofstream structured(dir/"reports.json"); structured << reports.dump(2) << '\n';
    std::ofstream text(dir/"bugs.txt");
    for (const auto& r : reports) {
        text << "========== Concurrency Defect Candidate ==========\nReport: " << r.at("report_id").get<std::string>()
             << "\nCategory: " << r.at("bug_category").get<std::string>() << "\nStatus: " << r.at("status").get<std::string>() << " (not reproduced)\n";
        for (const auto& e : r.at("evidence")) {
            text << "Requirement: " << e.at("requirement_id").get<std::string>() << "\nDescription: " << e.at("description").get<std::string>() << '\n';
            for (const auto& n : e.at("nodes")) text << "  " << n.dump() << '\n';
            for (const auto& s : e.at("source_operations")) text << "  " << s.at("source_id").get<std::string>() << " " << s.at("path").get<std::string>() << ':' << s.at("first_line") << '-' << s.at("last_line") << '\n' << s.at("content").get<std::string>() << '\n';
        }
        text << "========================================================\n\n";
    }
    if (reports.empty()) std::cout << "No bugs detected. Check module_contracts.json for unresolved obligations.\n";
    std::cout << "Final candidate reports: " << reports.size() << std::endl;
}
}
