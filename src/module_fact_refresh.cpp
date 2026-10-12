#include "CCPG/IRSourceFacts.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"
#include <fstream>
#include <iostream>
#include <regex>
#include <map>
#include <set>
#include <filesystem>

// Refresh compiler correspondence facts on an unchanged frozen graph. Reject
// mismatching IR; preserve node IDs, CFG/call edges and original contracts.
std::string signature(std::string text) {
    text=std::regex_replace(text,std::regex(", !(dbg|psr\\.id) ![0-9]+"),"");
    text=std::regex_replace(text,std::regex("![0-9]+"),"!metadata");
    return text;
}
int main(int argc,char** argv) {
    if (argc!=4) {std::cerr<<"usage: module_fact_refresh input.ll frozen_model.json derived_model.json\n";return 2;}
    try {
        if (std::filesystem::weakly_canonical(argv[3])==std::filesystem::weakly_canonical(argv[1])||
            std::filesystem::weakly_canonical(argv[3])==std::filesystem::weakly_canonical(argv[2]))
            throw std::runtime_error("derived output must not overwrite either input");
        llvm::LLVMContext context;llvm::SMDiagnostic diagnostic;
        auto ir=llvm::parseIRFile(argv[1],diagnostic,context);
        if (!ir) {diagnostic.print(argv[0],llvm::errs());return 2;}
        nlohmann::json model;std::ifstream input(argv[2]);input>>model;
        std::map<int,std::string> functions;
        std::map<std::pair<std::string,unsigned>,nlohmann::json*> nodes;
        for (const auto& f:model.at("functions")) if (f.at("origin")=="ir") functions.emplace(f.at("function_id"),f.at("name"));
        for (auto& node:model.at("nodes")) if (node.at("origin")=="ir"&&node.at("kind")!="Method") {
            auto key=std::make_pair(functions.at(node.at("function_id")),node.at("ordinal").get<unsigned>());
            if (!nodes.emplace(key,&node).second) throw std::runtime_error("duplicate frozen IR ordinal");
        }
        ccpg::IRSourceFacts facts(*ir);std::set<std::string> definitions;size_t refreshed=0;
        for (const auto& f:*ir) if (!f.isDeclaration()) {
            definitions.insert(f.getName().str());unsigned ordinal=0;
            for (const auto& bb:f) for (const auto& instruction:bb) {
                auto found=nodes.find({f.getName().str(),ordinal++});
                if (found==nodes.end()) throw std::runtime_error("IR instruction absent from frozen graph: "+f.getName().str());
                std::string code;llvm::raw_string_ostream stream(code);instruction.print(stream);stream.flush();
                auto& node=*found->second;
                if (node.at("kind")!=instruction.getOpcodeName()||signature(node.at("code"))!=signature(code))
                    throw std::runtime_error("IR instruction mismatch: "+f.getName().str()+":"+std::to_string(ordinal-1));
                node.update(facts.describe(instruction));++refreshed;
            }
        }
        std::set<std::string> frozen;for (const auto& f:functions) frozen.insert(f.second);
        if (refreshed!=nodes.size()||definitions!=frozen) throw std::runtime_error("IR function/instruction coverage mismatch");
        model["compiler_facts_refresh"]={{"kind","derived_offline_correspondence"},{"validated_instructions",refreshed},
            {"graph_identity_preserved",true},{"model_calls",0}};
        std::ofstream output(argv[3]);if (!output) throw std::runtime_error("cannot write derived model");output<<model.dump(2)<<'\n';
        std::cout<<"Refreshed "<<refreshed<<" compiler facts; graph IDs and edges unchanged, zero model calls\n";
    } catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 2;}
}
