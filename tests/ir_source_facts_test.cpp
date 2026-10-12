#include "CCPG/IRSourceFacts.h"
#include "LLMUtil/OperationBinding.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/SourceMgr.h"
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <filesystem>
using J=nlohmann::json;
void check(bool ok,const char* why) {if (!ok) throw std::runtime_error(why);}
int main(int argc,char** argv) {
    try {
        check(argc==3,"fixture IR and source required");llvm::LLVMContext context;llvm::SMDiagnostic diagnostic;
        auto ir=llvm::parseIRFile(argv[1],diagnostic,context);check(bool(ir),"fixture IR did not parse");
        const auto sourcePath=std::filesystem::path(argv[2]).filename().string();
        ccpg::IRSourceFacts facts(*ir);J model={{"nodes",J::array()}},manifest={{{"path",sourcePath}}};int id=0;
        for (const auto& f:*ir) for (const auto& bb:f) for (const auto& i:bb) {
            J n={{"node_id",++id},{"origin","ir"},{"kind",i.getOpcodeName()},{"api",""},{"atomic",false}};
            if (auto* call=llvm::dyn_cast<llvm::CallBase>(&i)) if (auto* callee=call->getCalledFunction()) n["api"]=callee->getName().str();
            n.update(facts.describe(i));model["nodes"].push_back(n);
        }
        std::vector<std::string> lines;std::ifstream source(argv[2]);std::string line;
        while (std::getline(source,line)) lines.push_back(line);
        auto op=[&](const char* marker,const char* role,const char* symbol,const char* function) {
            for (size_t i=0;i<lines.size();++i) if (lines[i].find(marker)!=std::string::npos)
                return J{{"path",sourcePath},{"first_line",i+1},{"last_line",i+1},{"content",lines[i]},
                    {"operation_kind",role},{"symbol",symbol},{"function",function}};
            throw std::runtime_error("fixture marker absent");
        };
        auto bind=[&](const J& o) {return llm_client::bindSourceOperation(o,model,manifest);};
        auto read=op("ATOMIC_READ","atomic","atomic_read","check_then_call");auto bound=bind(read);
        if (bound.at("status")!="bound") {
            std::cerr<<bound.dump(2)<<'\n';
            for (const auto& n:model["nodes"]) if (!n.at("inline_effects").empty()) std::cerr<<n.dump()<<'\n';
        }
        check(bound.at("status")=="bound","inlined read effect not uniquely bound");
        const J* readNode=nullptr;
        for (const auto& n:model["nodes"]) if (n.at("node_id")==bound.at("node_id")) readNode=&n;
        check(readNode&&readNode->at("memory_role")=="read"&&readNode->at("volatile")&&!readNode->at("atomic").get<bool>(),"inline read was misrepresented as LLVM atomic");
        if (readNode->at("access_paths")!=J({"psy.use_cnt.value"})) std::cerr<<readNode->dump(2)<<'\n';
        check(readNode->at("access_paths")==J({"psy.use_cnt.value"}),"inlined pointer spill lost provider expression");
        check(llm_client::sourceCoordinateMatches(*readNode,read,manifest),"inline call site missing from source query coordinates");
        check(bind(op("ATOMIC_READ","call","atomic_read","check_then_call"))["status"]=="bound","source call did not map to inline effect");
        check(bind(op("ATOMIC_DEC","atomic","atomic_dec_return","retire"))["status"]=="bound","inlined assembly effect not uniquely bound");
        check(bind(op("INDIRECT_CALL","call","psy->desc->get_property","check_then_call"))["status"]=="bound","indirect callee expression not bound");
        auto alias=bind(op("ALIAS_INLINE","atomic","atomic_read","aliased"));
        check(alias["status"]=="bound","caller alias broke inline effect binding");
        for (const auto& n:model["nodes"]) if (n["node_id"]==alias["node_id"])
            check(n["access_paths"]==J({"psy.use_cnt.value"}),"caller source alias replaced by incoming pointer name");
        check(bind(op("ALIAS_CALLEE","call","psy->desc->get_property","aliased"))["status"]=="bound","caller source alias lost from indirect callee");
        check(bind(op("AMBIGUOUS_INLINE","atomic","atomic_read","twice"))["status"]=="ambiguous","two inline invocations arbitrarily selected");
        check(bind(op("AMBIGUOUS_FIELD","read","psy->use_cnt.value","repeated_field"))["status"]=="ambiguous","two field reads arbitrarily selected");
        auto escaped=op("ESCAPED_CALLBACK","call","psy->desc->get_property","escaped_callback");
        escaped["first_line"]=escaped.at("last_line").get<int>()-2;
        escaped["content"]="callback = psy->desc->get_property; change(&callback); return callback(psy);";
        check(bind(escaped)["status"]=="operation_not_resolved","escaped callee spill invented a stable target");
        auto wrong=read;wrong["function"]="retire";check(bind(wrong)["status"]!="bound","wrong containing function accepted");
        wrong=read;wrong["symbol"]="missing_api";check(bind(wrong)["status"]=="selector_not_in_source","invented source API accepted");
        std::cout<<"Compiler-derived inline/callee correspondence regressions passed\n";
    } catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
}
