#include "LLMUtil/RequirementChecker.h"
#include <fstream>
#include <iostream>
int main(int argc,char** argv) {
    if (argc!=4) { std::cerr<<"usage: contract_replay module_model.json module_contracts.json checks.json\n";return 2; }
    try {
        nlohmann::json model,contracts;
        std::ifstream m(argv[1]),c(argv[2]);m>>model;c>>contracts;
        auto checks=llm_client::checkModuleRequirements(contracts.at("requirements"),contracts.at("guarantees"),model,
            contracts.value("source_operations",nlohmann::json::object()),contracts.value("source_manifest",nlohmann::json::array()));
        std::ofstream out(argv[3]);if (!out) throw std::runtime_error("cannot write replay output");out<<checks.dump(2)<<'\n';
        std::cout<<"Replayed "<<checks.size()<<" requirements without model calls or final review\n";
    } catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 2;}
}
