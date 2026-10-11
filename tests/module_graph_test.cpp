#include "CCPG/CCPG.h"
#include "PhasarUtil/AnalysisManager.h"
#include "PhasarUtil/PhasarPointerAnalysis.h"
#include "Util/TargetPath.h"
#include "Util/ExecutionTimer.h"
#include <fstream>
#include <stdexcept>
#include <set>
#include <unistd.h>
TargetPath* TargetPath::instance=nullptr;
ExecutionTimer* ExecutionTimer::instance=nullptr;
void check(bool ok,const char* why) { if (!ok) throw std::runtime_error(why); }
int main() {
    const auto root=std::filesystem::temp_directory_path()/("lace-module-graph-test-"+std::to_string(getpid()));
    std::filesystem::create_directories(root/"src");
    std::ofstream(root/"src/module.c")<<"void present(int *p) { *p=1; }\n#ifdef CONFIG_NOT_COMPILED\nvoid source_only(int *p) { *p=2; }\n#endif\n";
    std::ofstream(root/"module.ll")<<R"IR(
define void @present(ptr %p) {
entry:
  store i32 1, ptr %p
  ret void
}
define void @plain_external_helper(ptr %p) {
entry:
  %v = load i32, ptr %p
  %next = add i32 %v, 1
  store i32 %next, ptr %p
  ret void
}
declare void @outside_declaration(ptr)
)IR";
    setenv("LACE_DUMP_ROOT",(root/"artifacts").c_str(),1);
    TargetPath::getInstance()->setTargetAbsolutePath(root/"src");
    AnalysisManager::getInstance()->initialize(std::make_unique<PhasarPointerAnalysis>((root/"module.ll").string()));
    // Deliberately incomplete source frontend: no Joern method retained. This
    // is a test input, not fabricated graph operations or inferred thread roots.
    CPG emptySource; CCPG ccpg(&emptySource); ccpg.build();
    const auto m=ccpg.moduleModel();
    std::set<std::string> functions;
    for (const auto& f:m.at("functions")) { check(f.at("origin")=="ir","Invented source representation"); functions.insert(f.at("name")); check(f.at("cfg_complete"),"IR CFG not retained"); }
    check(functions==std::set<std::string>{"present","plain_external_helper"},"Unreferenced IR function lost or external declaration given invented body");
    check(m.at("coverage").at("source_functions")==0,"Incomplete source frontend was concealed");
    check(m.at("coverage").at("ir_functions")==2,"IR function coverage incorrect");
    check(m.at("coverage").at("ir_instructions")==6,"Some actual IR instructions were dropped");
    check(m.at("coverage").at("ir_instructions_without_debug_location")==6,"Missing debug locations not accounted");
    check(m.at("nodes").size()==8&&m.at("cfg_edges").size()==6,"CFG or function entries incomplete");
    check(m.at("parallel_entries_inferred")==false&&ccpg.getEntryFunctions().empty(),"All-function coverage inferred parallel entries");
    std::filesystem::remove_all(root);
}
