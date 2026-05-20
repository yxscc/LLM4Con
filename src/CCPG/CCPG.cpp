#include "phasar.h"
#include <cxxabi.h>
#include <regex>
#include <filesystem>
#include <unordered_set>
#include <set>
#include <fstream>
#include <tuple>
#include <queue>
#include <iomanip>
#include <sstream>
#include <limits>
#include <cstring>
#include <vector>

#include "CCPG/HB.h"
#include "PhasarUtil/LLVMAnalyzer.h"
#include "CCPG/AliasChecker.h"
#include "CCPG/LSAnalysis.h"
#include "Util/ExecutionTimer.h"
#include "PhasarUtil/AnalysisManager.h"
#include "PhasarUtil/PhasarPointerAnalysis.h"

#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Operator.h"

using namespace ccpg;
using namespace psr;

namespace fs = std::filesystem;

void handleContext(CCPGNode * caller, ccpg::Function * f);

namespace {

// P8a helpers: walk constant initializers to harvest llvm::Function
// pointers stored in kernel callback tables (device_attribute,
// attribute_group, file_operations, ...).
constexpr int kP8aMaxConstDepth = 8;
constexpr unsigned kP8aMaxOperands = 256;

void p8aCollectFns(const llvm::Constant* c,
                   std::vector<const llvm::Function*>& out,
                   std::unordered_set<const llvm::Constant*>& visited,
                   int depth) {
    if (!c || depth > kP8aMaxConstDepth) return;
    if (!visited.insert(c).second) return;
    const llvm::Value* stripped = c->stripPointerCasts();
    if (!stripped) return;
    if (const auto* fn = llvm::dyn_cast<llvm::Function>(stripped)) {
        out.push_back(fn);
        return;
    }
    const auto* cAfter = llvm::dyn_cast<llvm::Constant>(stripped);
    if (!cAfter) return;
    unsigned n = cAfter->getNumOperands();
    if (n > kP8aMaxOperands) n = kP8aMaxOperands;
    for (unsigned i = 0; i < n; ++i) {
        if (const auto* sub =
                llvm::dyn_cast<llvm::Constant>(cAfter->getOperand(i))) {
            p8aCollectFns(sub, out, visited, depth + 1);
        }
    }
}

// Whether a (possibly anonymous-suffixed) struct type-name looks like a
// known kernel callback / ops table whose function-pointer members
// should be promoted to thread entries. We match both the canonical
// name and `<name>.<digits>` LLVM-internal anonymized variants.
bool p8aIsCallbackTableTypeName(const std::string& sname) {
    static const std::vector<std::string> kPatterns = {
        // sysfs / kobject attribute groups and individual attributes
        "struct.attribute_group",
        "struct.device_attribute",
        "struct.driver_attribute",
        "struct.bus_attribute",
        "struct.class_attribute",
        "struct.kobj_attribute",
        "struct.bin_attribute",
        // VFS / chardev / procfs / debugfs callback tables
        "struct.file_operations",
        "struct.kernfs_ops",
        "struct.proc_ops",
        "struct.seq_operations",
        // PM / power management callback tables
        "struct.dev_pm_ops",
        "struct.platform_driver",
        // Driver / bus / class registration tables containing callbacks
        "struct.bus_type",
        "struct.device_driver",
        "struct.notifier_block",
        // Networking
        "struct.net_proto_family",
        "struct.proto_ops",
        "struct.ethtool_ops",
        "struct.net_device_ops",
        "struct.tty_operations",
        // Subsystem-specific
        "struct.iio_info",
        "struct.regmap_bus",
        "struct.nft_object_type",
        "struct.nft_expr_ops",
        "struct.nft_chain_type",
    };
    for (const auto& p : kPatterns) {
        if (sname == p) return true;
        // LLVM appends ".<n>" to anonymized struct types from C; allow it.
        if (sname.size() > p.size() + 1 &&
            sname[p.size()] == '.' &&
            sname.compare(0, p.size(), p) == 0) {
            return true;
        }
    }
    return false;
}

// Walk through array wrappers down to the first non-array element
// type. Many ops globals are declared as arrays of attribute structs.
const llvm::Type* p8aPeelArray(const llvm::Type* t) {
    while (t) {
        if (auto* arr = llvm::dyn_cast<llvm::ArrayType>(t)) {
            t = arr->getElementType();
            continue;
        }
        break;
    }
    return t;
}

// ---------------------------------------------------------------------------
// v23 P9a — Dynamic callback registration scan.
//
// P8a covers callbacks installed via STATIC kernel globals (`.data`
// constant initializers). But many subsystem callbacks are installed at
// runtime by storing a function pointer into a member of a
// dynamically-allocated struct, then handing that struct off to a
// register_*_notifier / INIT_WORK / timer_setup / hrtimer_init helper.
// E.g.:
//
//     tusb->psy_nb.notifier_call = tusb1210_psy_notifier;
//     power_supply_reg_notifier(&tusb->psy_nb);
//
// Phasar's entry detector doesn't see `tusb1210_psy_notifier` as a
// thread, ThreadCreationTree doesn't model `power_supply_reg_notifier`
// as a fork API, so the callback's accesses on the patched object
// (`tusb->charger`) never make it into the surface and we lose the bug
// even though the IR is right there.
//
// Fix: walk every StoreInst in the module; if it stores a defined
// function pointer into a GEP whose source-element type is one of the
// known kernel callback-host structs, promote that function to an
// entry. We match on the OWNING struct type (not field index), which is
// robust to LLVM GEP fusion / opaque pointers and covers nested
// containers like delayed_work (struct.delayed_work.work.func).
bool p9aIsCallbackHostTypeName(llvm::StringRef sname) {
    static const char* const kPatterns[] = {
        "struct.notifier_block",         // .notifier_call
        "struct.work_struct",            // .func
        "struct.delayed_work",           // .work.func
        "struct.rcu_work",               // wraps work_struct
        "struct.timer_list",             // .function
        "struct.hrtimer",                // .function
        "struct.tasklet_struct",         // .func / .callback
        "struct.tasklet_hrtimer",
        "struct.wait_queue_entry",       // .func (wake_up_func_t)
        "struct.callback_head",          // .func (RCU)
        "struct.irq_work",               // .func
        "struct.completion",             // no .func; harmless skip
    };
    for (const char* p : kPatterns) {
        llvm::StringRef pref(p);
        if (sname == pref) return true;
        // Accept "<pattern>.<digits>" LLVM-anonymous-disambiguator variant.
        if (sname.size() > pref.size() + 1 &&
            sname[pref.size()] == '.' &&
            sname.startswith(pref)) {
            return true;
        }
    }
    return false;
}

// Walk back through bitcasts/pointer-equivalences and return the first
// GEPOperator we find, or nullptr if the pointer doesn't look like a
// GEP into a known struct.
const llvm::GEPOperator* p9aFindOwningGEP(const llvm::Value* p) {
    if (!p) return nullptr;
    for (int guard = 0; guard < 8 && p != nullptr; ++guard) {
        if (auto* gep = llvm::dyn_cast<llvm::GEPOperator>(p)) {
            return gep;
        }
        if (auto* bc = llvm::dyn_cast<llvm::BitCastOperator>(p)) {
            p = bc->getOperand(0);
            continue;
        }
        if (auto* asc = llvm::dyn_cast<llvm::AddrSpaceCastOperator>(p)) {
            p = asc->getOperand(0);
            continue;
        }
        break;
    }
    return nullptr;
}

// Returns the local llvm::Function whose address is stored by `SI`,
// when the destination memory is a GEP into a known callback-host
// struct. Returns nullptr when this isn't a callback installation.
const llvm::Function* p9aExtractCallback(const llvm::StoreInst* SI) {
    if (!SI) return nullptr;
    const llvm::Value* val = SI->getValueOperand();
    if (!val) return nullptr;
    val = val->stripPointerCasts();
    const auto* fn = llvm::dyn_cast<llvm::Function>(val);
    if (!fn || fn->isDeclaration()) return nullptr;
    // Skip llvm.dbg.* and asm helpers.
    llvm::StringRef fname = fn->getName();
    if (fname.startswith("llvm.") || fname.empty()) return nullptr;

    const llvm::GEPOperator* gep = p9aFindOwningGEP(SI->getPointerOperand());
    if (!gep) return nullptr;
    const auto* srcTy =
        llvm::dyn_cast_or_null<llvm::StructType>(gep->getSourceElementType());
    if (!srcTy || !srcTy->hasName()) return nullptr;
    if (!p9aIsCallbackHostTypeName(srcTy->getName())) return nullptr;
    return fn;
}

}  // namespace

CCPGEdge * CCPG::hasCallEdge(CCPGNode * node){
    for(CCPGEdge * edge : node->getOutEdges()){
        if(edge->getType() == CCPGEdge::EdgeType::CALL){
            return edge;
        }
    }
    return nullptr;
}

bool CCPG::hasHBEdge(CCPGNode * node){
    for(CCPGEdge * edge : node->getOutEdges()){
        if(edge->getType() == CCPGEdge::EdgeType::HB){
            return true;
        }
    }
    return false;
}

void CCPG::build(){

    const CPG* cpg = this->getCPG();
    ThreadCreationTree* tree = ThreadCreationTree::getInstance();
    tree->setCPG(cpg);
    tree->setCCPG(this);
    
    std::queue<ccpg::Function *> functionQueue;
    std::set<ccpg::Function *> visited;

    CCPGNode* main = getMain();
    if(main != nullptr){
        ccpg::Function * f = createFunction(main);
        entryFunctions.insert(f);
        functionQueue.push(f);
    }
    
    // NEW: For kernel modules without explicit main/thread creation,
    // treat all discovered entry points as potential parallel entry points
    auto pointerAnalyzer = dynamic_cast<PhasarPointerAnalysis*>(
        AnalysisManager::getInstance()->getPointerAnalyzer());
    if (pointerAnalyzer) {
        auto allEntries = pointerAnalyzer->getAllEntryPointInfos();
        if (allEntries.size() > 1) {
            std::cout << "[Kernel Module Mode] Adding " << allEntries.size() 
                      << " entry points as parallel entries" << std::endl;
            for (const auto& entryInfo : allEntries) {
                // Skip the main entry point we already added
                if (main != nullptr && main->getCPGNode()->getName() == entryInfo.functionName) {
                    std::cout << "  - Skipping (already main): " << entryInfo.functionName << std::endl;
                    continue;
                }
                
                // Demangle the function name for CPG lookup
                std::string demangledName = LLVMAnalyzer::getInstance()->demangle(entryInfo.functionName.c_str());
                
                // Extract short function name from demangled name
                // e.g., "leveldb::DBImpl::Get(leveldb::ReadOptions const&, ...)" -> "Get"
                std::string shortName = demangledName;
                
                // Remove parameters (everything after '(')
                size_t parenPos = shortName.find('(');
                if (parenPos != std::string::npos) {
                    shortName = shortName.substr(0, parenPos);
                }
                
                // Extract the last component after '::'
                size_t lastColon = shortName.rfind("::");
                if (lastColon != std::string::npos) {
                    shortName = shortName.substr(lastColon + 2);
                }
                
                // Handle destructor (remove leading '~' for lookup, will match ~ClassName)
                std::string lookupName = shortName;
                
                std::cout << "  - Looking for: " << entryInfo.functionName 
                          << " -> demangled: " << demangledName 
                          << " -> shortName: " << shortName << std::endl;
                
                // Find the method node in CPG using short name
                Node* methodNode = cpg->findMethod(shortName);
                if (methodNode == nullptr && shortName != demangledName) {
                    // Try full demangled name as fallback
                    methodNode = cpg->findMethod(demangledName);
                }
                if (methodNode == nullptr) {
                    // Try original mangled name as last resort
                    methodNode = cpg->findMethod(entryInfo.functionName);
                }
                // Kernel syscall / interrupt wrapper name fallback:
                // LLVM bitcode for `SYSCALL_DEFINE*(foo,...)` carries arch-
                // decorated symbols like `__x64_sys_foo`, `__arm64_sys_foo`,
                // `__ia32_sys_foo`, `__se_sys_foo`, `__do_sys_foo`,
                // `__sys_foo`, but Joern's CPG (which parses preprocessed C)
                // sees the un-decorated `sys_foo` or even just `foo`.
                // Try progressively stripping the arch/ABI prefix until we
                // find a match so the syscall becomes reachable as an entry.
                if (methodNode == nullptr) {
                    static const char* const kSyscallPrefixes[] = {
                        "__x64_sys_", "__x32_sys_", "__ia32_sys_",
                        "__arm64_sys_", "__arm_sys_", "__mips_sys_",
                        "__riscv_sys_", "__s390_sys_", "__s390x_sys_",
                        "__powerpc_sys_", "__powerpc64_sys_",
                        "__se_sys_", "__se_compat_sys_",
                        "__do_sys_", "__do_compat_sys_",
                        "__sys_"
                    };
                    std::vector<std::string> candidates;
                    for (const char* pfx : kSyscallPrefixes) {
                        std::size_t plen = std::strlen(pfx);
                        if (shortName.size() > plen &&
                            shortName.compare(0, plen, pfx) == 0) {
                            std::string stripped = shortName.substr(plen);
                            candidates.push_back("sys_" + stripped);
                            candidates.push_back(stripped);
                            break;
                        }
                    }
                    // Also try adding/removing a leading "sys_" on the
                    // shortName so e.g. `sys_move_pages` vs `move_pages`
                    // can cross-match.
                    if (shortName.rfind("sys_", 0) == 0) {
                        candidates.push_back(shortName.substr(4));
                    } else {
                        candidates.push_back("sys_" + shortName);
                    }
                    for (const auto& cand : candidates) {
                        methodNode = cpg->findMethod(cand);
                        if (methodNode != nullptr) {
                            std::cout << "  - Kernel syscall name fallback: "
                                      << shortName << " -> " << cand << std::endl;
                            break;
                        }
                    }
                }
                if (methodNode == nullptr) {
                    std::cout << "  - Not found in CPG: " << shortName << " (tried: " << demangledName << ")" << std::endl;
                    continue;
                }
                
                std::cout << "  - Found in CPG: " << methodNode->getName() 
                          << " at " << methodNode->getFileName() << ":" << methodNode->getLineNumber() << std::endl;
                
                // Check if already added before creating
                if (containsCPGNode(methodNode)) {
                    std::cout << "  - Already exists: " << entryInfo.functionName << std::endl;
                    continue;
                }
                
                CCPGNode* entryNode = createCCPGNode(methodNode);
                if (entryNode != nullptr) {
                    ccpg::Function* f = createFunction(entryNode);
                    if (f != nullptr) {
                        entryFunctions.insert(f);
                        functionQueue.push(f);
                        std::cout << "  - Added entry: " << entryInfo.functionName << std::endl;
                    }
                }
            }
            std::cout << "[Kernel Module Mode] Total entry functions: " << entryFunctions.size() << std::endl;
        }
    }

    // P8a: Sysfs / file_operations / driver-ops callback discovery.
    //
    // Many kernel callbacks (sysfs show/store, fops .read/.write/.poll,
    // chardev/procfs/debugfs ops, driver_pm callbacks, …) are static
    // helpers that are only referenced through function pointers stored
    // in well-known ops tables. Phasar's entry-point heuristic only
    // promotes EXPORT_SYMBOL'd / externally-visible functions, so these
    // static helpers never become threads and the surface generator
    // never sees the user-space side of any field they touch.
    //
    // We walk the LLVM module looking for globals whose type matches a
    // known callback table (or an array of such), descend into the
    // constant initializer to collect every llvm::Function pointer it
    // contains, then add any function we can map back into the CPG as
    // an additional entry. Downstream ThreadCreationTree::build() picks
    // them up automatically as kernel-entry threads (concurrent with
    // every other entry that shares data, exactly the model we want).
    if (pointerAnalyzer) {
        const llvm::Module* M = pointerAnalyzer->getModule();
        if (M) {
            std::vector<const llvm::Function*> harvested;
            std::unordered_set<const llvm::Constant*> visited;
            int globalsScanned = 0;
            for (const llvm::GlobalVariable& gv : M->globals()) {
                if (!gv.hasInitializer()) continue;
                const llvm::Type* ty = p8aPeelArray(gv.getValueType());
                const auto* st = llvm::dyn_cast_or_null<llvm::StructType>(ty);
                if (!st || !st->hasName()) continue;
                std::string structName = st->getName().str();
                if (!p8aIsCallbackTableTypeName(structName)) continue;
                ++globalsScanned;
                visited.clear();
                p8aCollectFns(gv.getInitializer(), harvested, visited, 0);
            }

            std::unordered_set<const llvm::Function*> uniq(
                harvested.begin(), harvested.end());
            int added = 0, alreadyKnown = 0, notInCPG = 0, isDecl = 0;
            for (const llvm::Function* fn : uniq) {
                if (!fn || fn->isDeclaration()) {
                    if (fn) ++isDecl;
                    continue;
                }
                std::string name = fn->getName().str();
                if (name.empty()) continue;

                Node* methodNode = cpg->findMethod(name);
                if (methodNode == nullptr) {
                    for (const std::string& v : CPG::demangleVariants(name)) {
                        methodNode = cpg->findMethod(v);
                        if (methodNode) break;
                    }
                }
                if (methodNode == nullptr) {
                    ++notInCPG;
                    continue;
                }
                if (containsCPGNode(methodNode)) {
                    ++alreadyKnown;
                    continue;
                }
                CCPGNode* entryNode = createCCPGNode(methodNode);
                if (entryNode == nullptr) continue;
                ccpg::Function* f = createFunction(entryNode);
                if (f == nullptr) continue;
                entryFunctions.insert(f);
                functionQueue.push(f);
                ++added;
            }

            if (globalsScanned > 0 || added > 0) {
                std::cout << "[P8a] Sysfs/callback discovery: scanned "
                          << globalsScanned << " ops globals, harvested "
                          << uniq.size() << " functions, added " << added
                          << " new entries (" << alreadyKnown
                          << " already known, " << notInCPG
                          << " not in CPG, " << isDecl << " declarations)"
                          << std::endl;
            }
        }
    }

    // P9a: Dynamic callback registration discovery.
    //
    // Scan every StoreInst in the module for installations of the form
    //     gep into struct.{notifier_block|work_struct|delayed_work|
    //                       timer_list|hrtimer|tasklet_struct|...} = @fn
    // and promote @fn to an entry function. This covers the runtime
    // INIT_WORK / timer_setup / register_*_notifier / etc. paths that
    // P8a misses because their callback pointer never sits in a static
    // global.
    if (pointerAnalyzer) {
        const llvm::Module* M = pointerAnalyzer->getModule();
        if (M) {
            std::unordered_set<const llvm::Function*> harvested;
            long instsScanned = 0;
            for (const llvm::Function& F : *M) {
                if (F.isDeclaration()) continue;
                for (const llvm::BasicBlock& BB : F) {
                    for (const llvm::Instruction& I : BB) {
                        const auto* SI = llvm::dyn_cast<llvm::StoreInst>(&I);
                        if (!SI) continue;
                        ++instsScanned;
                        if (const llvm::Function* fn = p9aExtractCallback(SI)) {
                            harvested.insert(fn);
                        }
                    }
                }
            }

            int added = 0, alreadyKnown = 0, notInCPG = 0;
            std::vector<std::string> notInCPGNames;  // first 10 for debug
            for (const llvm::Function* fn : harvested) {
                if (!fn) continue;
                std::string name = fn->getName().str();
                if (name.empty()) continue;
                // Layer 1: exact name + demangle variants (matches P8a).
                Node* methodNode = cpg->findMethod(name);
                if (methodNode == nullptr) {
                    for (const std::string& v : CPG::demangleVariants(name)) {
                        methodNode = cpg->findMethod(v);
                        if (methodNode) break;
                    }
                }
                // Layer 2: fall back to (file, line) matching via DWARF
                // debug info, which can find a method even when its NAME
                // attribute in the CPG dot file has been mangled or when
                // LLVM-side suffix decoration differs from the C-side name.
                if (methodNode == nullptr) {
                    methodNode = cpg->findMethodByLLVMFunction(fn);
                }
                if (methodNode == nullptr) {
                    ++notInCPG;
                    if (notInCPGNames.size() < 10) {
                        notInCPGNames.push_back(name);
                    }
                    continue;
                }
                if (containsCPGNode(methodNode)) {
                    ++alreadyKnown;
                    continue;
                }
                CCPGNode* entryNode = createCCPGNode(methodNode);
                if (entryNode == nullptr) continue;
                ccpg::Function* f = createFunction(entryNode);
                if (f == nullptr) continue;
                entryFunctions.insert(f);
                functionQueue.push(f);
                ++added;
            }

            if (!harvested.empty() || added > 0) {
                std::cout << "[P9a] Dynamic callback discovery: scanned "
                          << instsScanned << " stores, harvested "
                          << harvested.size() << " unique callbacks, added "
                          << added << " new entries ("
                          << alreadyKnown << " already known, "
                          << notInCPG << " not in CPG)" << std::endl;
                if (!notInCPGNames.empty()) {
                    std::cout << "[P9a-DBG] not-in-CPG sample:";
                    for (const auto& s : notInCPGNames) std::cout << " " << s;
                    std::cout << std::endl;
                }
            }
        }
    }

    int i = 0;
    // create function for each call node
    while (!functionQueue.empty()) {
        ccpg::Function * function = functionQueue.front();
        functionQueue.pop();
        if(visited.count(function)){
            continue;
        }
        visited.insert(function);

        for(CCPGNode * objectInitNode : function->getNodesByType(ThreadAPIUtil::TYPE::OBJECT_INIT)){
            if (hasCallEdge(objectInitNode)) {
                continue;
            }
            ccpg::Function * f = createFunctionByCaller(objectInitNode);
            if(f == nullptr){
                continue;
            }
            functionQueue.push(f);
        }

        for(CCPGNode * callNode : function->getNodesByType(ThreadAPIUtil::TYPE::OTHER_CALL)){
            if (hasCallEdge(callNode)) {
                continue;
            }
            ccpg::Function * f = createFunctionByCaller(callNode);
            if(f == nullptr){
                continue;
            }
            functionQueue.push(f);
        }

        for(CCPGNode * forkNode : function->getNodesByType(ThreadAPIUtil::TYPE::FORK)){
            Node * functionCPGNode = ThreadCreationTree::getInstance()->findThreadEntryInCPG(forkNode);
            if(functionCPGNode == nullptr){
                continue;
            }
            CCPGNode * functionNode = createCCPGNode(functionCPGNode);
            ccpg::Function * f = createFunction(functionNode);
            CCPGEdge * edge = createCCPGEdge(forkNode, functionNode);
            edge->setType(CCPGEdge::EdgeType::HB);
            this->addEdge(edge);
            functionQueue.push(f);
            handleContext(forkNode, f);
        }
    }
    labelForkPotential();

    ExecutionTimer::getInstance()->start("Building thread creation tree");
    tree->build();
    ExecutionTimer::getInstance()->stop("Building thread creation tree");

    labelAPI();

    tree->handleJoins();

    ExecutionTimer::getInstance()->start("LockSet Analysis");
    LSAnalysis * lsAnalysis = LSAnalysis::getInstance();
    lsAnalysis->setCCPG(this);
    lsAnalysis->build();
    ExecutionTimer::getInstance()->stop("LockSet Analysis");
}

CCPGNode * CCPG::getCallSiteInFunction(const ccpg::Function * caller, const ccpg::Function * callee){
    CCPGNodeSet callSites = callee->getCallSites();
    for(CCPGNode * node : callSites){
        if(node->getFunction() == caller){
            return node;
        }
    }
    return nullptr;
}

void handleContext(CCPGNode * caller, ccpg::Function * f){
    Function * callerFunction = caller->getFunction();
    ContextSet contextSet = callerFunction->getContextSet();
    for(Context * context : contextSet){
        if(context->contains(caller)){
            continue;
        }
        f->addContext(context->extend(caller));
    }
}

ccpg::Function * CCPG::createFunctionByCaller(CCPGNode * caller){
    CCPGNode * callee = findCalleeByCaller(caller);
    if(callee == nullptr){
        return nullptr;
    }
    ccpg::Function * f = createFunction(callee);
    CCPGEdge * edge = createCCPGEdge(caller, callee);
    edge->setType(CCPGEdge::EdgeType::CALL);
    this->addEdge(edge);
    handleContext(caller, f);
    return f;
}

void CCPG::labelForkPotential(){
    std::queue<ccpg::Function *> functionQueue;
    for(ccpg::Function * function : functions){
        if(function->getNodesByType(ThreadAPIUtil::TYPE::FORK).size() > 0){
            functionQueue.push(function);
        }
    }

    while (!functionQueue.empty()) {
        ccpg::Function * function = functionQueue.front();
        functionQueue.pop();

        if(function->isForkPotential()){
            continue;
        }

        function->setForkPotential(true);

        FunctionSet callers = function->getCallers();
        for(ccpg::Function * caller : callers){
            if(!caller->isForkPotential()){
                functionQueue.push(caller);
            }
        }
        
    }
}

void CCPG::labelAPI(){

    for(ccpg::Function * function : functions){
        if(function->getNodesByType(ThreadAPIUtil::TYPE::ACQUIRE).size() == 1 && function->getNodesByType(ThreadAPIUtil::TYPE::RELEASE).size() == 0){
            function->setAcquirePotential(true);
        }
        if(function->getNodesByType(ThreadAPIUtil::TYPE::RELEASE).size() == 1 && function->getNodesByType(ThreadAPIUtil::TYPE::ACQUIRE).size() == 0){
            function->setReleasePotential(true);
        }
    }
    
}


void CCPG::deleteNode(CCPGNode * node){
    for(CCPGEdge * edge : node->getInEdges()){
        edge->getSrc()->removeOutEdge(edge);
        edges.erase(edge);
    }

    for(CCPGEdge * edge : node->getOutEdges()){
        edge->getDst()->removeInEdge(edge);
        edges.erase(edge);
    }

    nodes.erase(node);
    cpgNodeToCCPGNodeMap.erase(node->getCPGNode());
}

CCPGNodeSet CCPG::getEntries(){
    CCPGNodeSet entries;
    const CPG* cpg = this->getCPG();

    auto potentialEntries = AnalysisManager::getInstance()->getPointerAnalyzer()->getPotentialEntryPoints();

    for (const auto& entryInfo : potentialEntries) {
        
        const std::string& fileName = entryInfo.fileName;
        int lineNumberFromPhasar = entryInfo.lineNumber;
        
        if (fileName == "N/A" || lineNumberFromPhasar == 0) {
            continue; // 跳过无效的入口点信息
        }

        CPGNodeSet methods = cpg->getMethodsByFileName(fileName);
        if(methods.empty()){
            methods = cpg->getMethodsByFileName(NodeLoc::extractBaseFileName(fileName));
            if(methods.empty()){
                continue;
            }
        }

        for(Node* methodNode : methods){
            if( methodNode->getLineNumber() != -1 && abs(methodNode->getLineNumber() - lineNumberFromPhasar) <= 3){
                entries.insert(createCCPGNode(methodNode));
            }
        }
    }
    return entries;
}

CCPGNode* CCPG::getMain() {
    const CPG* cpg = this->getCPG();
    auto mainInfo = AnalysisManager::getInstance()->getPointerAnalyzer()->getMainFunction();
    if (mainInfo.fileName == "N/A" || mainInfo.lineNumber == 0) {
        std::cerr << "Warning: No valid main function found." << std::endl;
        return nullptr;
    }

    const std::string& mainFuncName = mainInfo.functionName;
    std::string fileName = mainInfo.fileName;
    int lineNumber = mainInfo.lineNumber;
    std::string fileName_last = NodeLoc::extractBaseFileName(fileName);

    std::cerr << "[DEBUG getMain] Looking for: funcName=" << mainFuncName 
              << ", fileName=" << fileName << ", fileName_last=" << fileName_last 
              << ", lineNumber=" << lineNumber << std::endl;

    CPGNodeSet methods = cpg->getMethodsByFileName(fileName);
    std::cerr << "[DEBUG getMain] methods from full path: " << methods.size() << std::endl;
    if(methods.empty()){
        methods = cpg->getMethodsByFileName(fileName_last);
        std::cerr << "[DEBUG getMain] methods from filename only: " << methods.size() << std::endl;
        if(methods.empty()){
            std::cerr << "Warning: No methods found in file " << fileName << " or " << fileName_last << std::endl;
            // Layer 5 diagnostic: list closest-name methods so it is
            // obvious whether the entry exists under a different file
            // path or a slightly different name (LLVM mangling, ops_table
            // members renamed by the optimizer, etc.).
            auto suggestions = cpg->findMethodSuggestions(mainFuncName, 5);
            if (!suggestions.empty()) {
                std::cerr << "[DEBUG getMain] Closest method names in CPG:";
                for (const auto& s : suggestions) std::cerr << " " << s;
                std::cerr << std::endl;
            }
            return nullptr;
        }
    }

    std::cerr << "[DEBUG getMain] Iterating " << methods.size() << " methods" << std::endl;
    // Pre-compute the demangle/syscall name variants once so each method
    // node only does an O(K) lookup instead of O(K*M). Only used after
    // the strict equality check fails, preserving the baseline path.
    std::vector<std::string> nameCandidates = CPG::allNameCandidates(mainFuncName);
    std::unordered_set<std::string> nameCandidateSet(nameCandidates.begin(),
                                                     nameCandidates.end());
    Node* bestMatch = nullptr;
    int bestDelta = std::numeric_limits<int>::max();
    for(auto it = methods.begin(); it != methods.end(); it++){
        Node * methodNode = *it;
        std::cerr << "[DEBUG getMain] Checking method: name=" << methodNode->getName()
                  << ", line=" << methodNode->getLineNumber() << std::endl;
        if (methodNode->getName() != mainFuncName &&
            nameCandidateSet.count(methodNode->getName()) == 0) {
            continue;
        }
        if(methodNode->getLineNumber() != -1) {
            int delta = abs(methodNode->getLineNumber() - lineNumber);
            if (delta <= 3) {
                std::cerr << "[DEBUG getMain] Found main! Creating CCPGNode" << std::endl;
                return createCCPGNode(methodNode);
            }
            if (delta < bestDelta) {
                bestDelta = delta;
                bestMatch = methodNode;
            }
        } else if (bestMatch == nullptr) {
            bestMatch = methodNode;
        }
    }

    if (bestMatch != nullptr) {
        std::cerr << "[DEBUG getMain] Main line mismatch; using closest match (delta="
                  << bestDelta << "). Creating CCPGNode" << std::endl;
        return createCCPGNode(bestMatch);
    }

    std::cerr << "[DEBUG getMain] Main not found after iteration" << std::endl;
    auto suggestions = cpg->findMethodSuggestions(mainFuncName, 5);
    if (!suggestions.empty()) {
        std::cerr << "[DEBUG getMain] Closest method names in CPG:";
        for (const auto& s : suggestions) std::cerr << " " << s;
        std::cerr << std::endl;
    }
    return nullptr;
}

CCPGNode * CCPG::createCCPGNode(Node* n) {
    if(n == nullptr){
        return nullptr;
    }

    if(n->getCode() == "ret = task->func(task->func_arg)"){
        int i = 1;
    }

    // 检查n是否已经存在CCPG中
    if(containsCPGNode(n)){
        return cpgNodeToCCPGNodeMap[n];
    }

    CCPGNode* node = new CCPGNode(n, ThreadAPIUtil::getInstance()->getType(n));
    node->setId(nodes.size() + 1);
    if(node->getType() == ThreadAPIUtil::TYPE::DUMMY 
    || node->getType() == ThreadAPIUtil::TYPE::LOOP
    || node->getType() == ThreadAPIUtil::TYPE::BRANCH
    || node->getType() == ThreadAPIUtil::TYPE::GOTO
    || node->getType() == ThreadAPIUtil::TYPE::GOTOTARGET
    || node->getType() == ThreadAPIUtil::TYPE::CONTROLSTRUCTURE
    || node->getType() == ThreadAPIUtil::TYPE::RETURN
    || node->getType() == ThreadAPIUtil::TYPE::ASSIGNMENT
    || node->getType() == ThreadAPIUtil::TYPE::HARE_PAR_FOR
    || node->getType() == ThreadAPIUtil::TYPE::GLOBAL)
    {
        node->setCallSite(false);
    }
    else{
        node->setCallSite(true);
    }
    //node->setFunction(f); 
    addNode(node);
    return node;
}

CCPGEdge* CCPG::createCCPGEdge(CCPGNode* from, CCPGNode* to) {
    if(from == nullptr || to == nullptr){
        return nullptr;
    }
    CCPGEdge* edge = new CCPGEdge(from, to);
    from->addOutEdge(edge);
    to->addInEdge(edge);
    addEdge(edge);
    return edge;
}

ccpg::Function * CCPG::createFunction(CCPGNode * funcNode) {
    ccpg::Function * function = getFunctionByCCPGNode(funcNode);
    if (function != nullptr) {
        return function;
    }
    
    function = new ccpg::Function(funcNode);
    addFunction(function);

    std::queue<CCPGNode *> nodeQueue;
    nodeQueue.push(funcNode);
    funcNode->setFunction(function);

    std::unordered_set<CCPGNode*> visitedNodes; // 防止因循环等造成重复处理
    visitedNodes.insert(funcNode);

    while (!nodeQueue.empty()) {

        CCPGNode* node = nodeQueue.front();
        nodeQueue.pop();
        function->addNode(node);

        Node* cpgNode = node->getCPGNode();
        std::unordered_set<Node*> children = findChildren(cpgNode);

        for(Node* child : children){
            CCPGNode* childNode = getCCPGNodeByCPGNode(child);
            if (!childNode) {
                childNode = createCCPGNode(child);
                childNode->setFunction(function); // 新节点也需要设置其所属函数
            }
            CCPGEdge* edge = createCCPGEdge(node, childNode);
            this->addEdge(edge);
            function->addEdge(edge);

            if (visitedNodes.find(childNode) == visitedNodes.end()) {
                visitedNodes.insert(childNode);
                nodeQueue.push(childNode);
            }
        }
    }

    const std::string& funcFileName = function->getFuncNode()->getCPGNode()->getFileName();
    for (CCPGNode* node : function->getNodes()) {
        const std::string& nodeFileName = node->getCPGNode()->getFileName();
        const std::string& effectiveFileName = nodeFileName.empty() ? funcFileName : nodeFileName;
        NodeLoc loc(effectiveFileName, node->getCPGNode()->getLineNumber(), function);
        node->setNodeLoc(loc);
        locToNodeSetMap[loc].insert(node);
        function->addNodeByLoc(loc, node);
    }

    PointerAnalysisInterface* analyzer = AnalysisManager::getInstance()->getPointerAnalyzer();
    PhasarPointerAnalysis* phasarAnalyzer = static_cast<PhasarPointerAnalysis*>(analyzer);
    if (!phasarAnalyzer) {
        std::cerr << "Error: Phasar analysis backend is not initialized." << std::endl;
        return function;
    }

    const llvm::Function* llvmFunc = AliasChecker::getInstance()->getLLVMFunction(function);
    if (llvmFunc) {
        function->setLLVMFunction(llvmFunc);
    } else {
        std::cerr << "Warning: Could not map CPG function '" << function->getFuncNode()->getCPGNode()->getName()
                  << "' to an llvm::Function." << std::endl;
        return function;
    }

    for(CCPGNode* node : function->getNodes()){
        if(node->isCallSite()){
            NodeLoc loc = node->getNodeLoc();
            std::string cpgCallName = node->getCPGNode()->getName();
            auto candidateCallInsts = phasarAnalyzer->getCallInstsByLoc(loc);
            
            for (const llvm::CallInst* candidateInst : candidateCallInsts) {
                const llvm::Function* calledFunc = candidateInst->getCalledFunction();
                if (!calledFunc) {
                    continue; 
                }

                std::string llvmCallName = LLVMAnalyzer::getInstance()->demangle(calledFunc->getName().str().c_str());
                if (cpgCallName == llvmCallName) {
                    // 找到了完美的匹配！
                    node->setLLVMCallInst(candidateInst);
                    break; 
                }
            }
        }
    }

    return function;    

}

/*std::unordered_set<Node*> CCPG::findChildren(Node* node, std::unordered_set<Node*> visited_node){
    
    if(visited_node.find(node) != visited_node.end()){
        return std::unordered_set<Node*>();
    }
    else{
        visited_node.insert(node);
    }

    const CPG * cpg = this->getCPG();
    std::unordered_set<Node*> children;

    for(Edge* edge : node->outCFGEdges){

        Node* toNode = edge->getToNode();

        if(ThreadAPIUtil::getInstance()->isCCPGNode(toNode)){
            children.insert(toNode);
        }
        else{
            std::unordered_set<Node*> temp = findChildren(edge->getToNode(), visited_node);
            children.insert(temp.begin(), temp.end());
        }
    }

    return children;
}
*/

std::unordered_set<Node*> CCPG::findChildren(Node* startNode) {
    // Check cache first
    auto cachedResult = findChildrenCache.find(startNode);
    if (cachedResult != findChildrenCache.end()) {
        return cachedResult->second;
    }

    std::unordered_set<Node*> final_children;
    std::queue<Node*> worklist;
    std::unordered_set<Node*> visited_in_this_search;
    for (Edge* edge : startNode->outCFGEdges) {
        worklist.push(edge->getToNode());
    }
    visited_in_this_search.insert(startNode);
    while (!worklist.empty()) {
        Node* currentNode = worklist.front();
        worklist.pop();

        if (visited_in_this_search.count(currentNode)) {
            continue;
        }
        visited_in_this_search.insert(currentNode);

        if (ThreadAPIUtil::getInstance()->isCCPGNode(currentNode)) {
            final_children.insert(currentNode);
        } else {
            for (Edge* edge : currentNode->outCFGEdges) {
                worklist.push(edge->getToNode());
            }
        }
    }

    // Store result in cache before returning
    findChildrenCache[startNode] = final_children;
    return final_children;
}

ccpg::Function * CCPG::getFunctionByCCPGNode(CCPGNode * node){
    int id = node->getId();
    auto it = IDToFunction.find(id);
    if (it != IDToFunction.end()) {
        return it->second;
    }
    return nullptr;
}

CCPGNode * CCPG::findCalleeByCaller(CCPGNode * caller){
    AliasChecker * aliasChecker = AliasChecker::getInstance();

    if(hasCallEdge(caller)){
        for(CCPGEdge * edge : caller->getOutEdges()){
            if(edge->getType() == CCPGEdge::EdgeType::CALL){
                return edge->getDst();
            }
        }
    }

    const llvm::CallInst* callInst = caller->getLLVMCallInst();
    if (callInst) {
        auto potentialCallees = AnalysisManager::getInstance()->getPointerAnalyzer()->getCalleesOfCallAt(callInst);

        for (const llvm::Function* llvmFunc : potentialCallees) {
            Node* methodNode = cpg->findMethodByLLVMFunction(llvmFunc);
            if (methodNode) {
                return createCCPGNode(methodNode);
            }
        }
    }

    if (caller->getCPGNode()->getName() == "<operator>.pointerCall" && caller->getCPGNode()->getCode() == "task->func(task->func_arg)") {
        Node * method = cpg->findMethod("muxer_thread");
        return createCCPGNode(method);
        //
    }

    if (caller->getType() == ThreadAPIUtil::TYPE::OBJECT_INIT){
        Node * objectInitCPGNode = caller->getCPGNode();
        Node * object = objectInitCPGNode->getArgument(1);
        if(object == nullptr){
            return nullptr;
        }
        std::string name = object->getName();
        Node * method = cpg->findMethod(name);
        return createCCPGNode(method);
    }

    else if (caller->getType() == ThreadAPIUtil::TYPE::OTHER_CALL){
        Node * callCPGNode = caller->getCPGNode();
        Node * method = cpg->findMethod(callCPGNode);
        return createCCPGNode(method);
    }
    
    return nullptr;
}

bool CCPG::existsEdge(CCPGNode * src, CCPGNode * dst) const {
    for (CCPGEdge * edge : src->getOutEdges()) {
        if (edge->getDst() == dst) {
            return true;
        }
    }
    return false;
}

std::string escapeSpecialCharacters(const std::string& input) {
    std::string result;
    for (char c : input) {
        switch (c) {
            case '\n': result += "\\n"; break;  // 换行符
            case '\t': result += "\\t"; break;  // 制表符
            case '\r': result += "\\r"; break;  // 回车符
            case '\\': result += "\\\\"; break; // 反斜杠
            case '"': result += "\\\""; break; // 双引号
            case '\'': result += "\\'"; break;  // 单引号
            case '\0': result += "\\0"; break;  // 空字符
            case '\b': result += "\\b"; break;  // 退格符
            case '\f': result += "\\f"; break;  // 换页符
            case '\v': result += "\\v"; break;  // 垂直制表符
            default: result += c; break;        // 其他字符保持不变
        }
    }
    return result;
}

void CCPG::dump(fs::path outputDir) {

    std::ofstream file(outputDir / "CCPG.dot");
    file << "digraph G {" << std::endl;
    for(CCPGNode* node : nodes) {
        std::string code;
        code = node->getCPGNode()->properties.at("CODE");
        // 如果cpg的methodNodes中包含node，则将code置为node的name
        if(node->getCPGNode()->getType() == "Method") {
            code = node->getCPGNode()->getName();
        }
        code = escapeSpecialCharacters(code);
        file << node->getId() << " [label=\"" << node->getId() << "<" + ThreadAPIUtil::getInstance()->getTypeString(node->getType()) + "> " + code;
        /*for(auto pair : node->getContextLockSet()) {
            file << "\\n";
            file << "Context: " << pair.first->toString() << "\\n";
            for(Lock* lock : pair.second) {
                if(lock == nullptr) continue;
                file << "lock" << lock->getId() << " ";
            }
        }*/
        file << "\"];" << std::endl;
        
    }
    for(CCPGEdge* edge : edges) {
        file << edge->getSrc()->getId() << " -> " << edge->getDst()->getId() << " [label=\"" << edge->getTypeString() << "\"];" << std::endl;
    }
    file << "}" << std::endl;
    file.close();

    // create dot for every function
    fs::path functionsOutputDir = outputDir / "functions";
    if (!fs::exists(functionsOutputDir)) {
        fs::create_directory(functionsOutputDir);
    }
    for(ccpg::Function* function : functions) {
        std::ofstream file(functionsOutputDir / (function->getFuncNode()->getCPGNode()->getName() + ".dot"));
        file << "digraph G {" << std::endl;
        for(CCPGNode* node : function->getNodes()) {
            std::string code;
            code = node->getCPGNode()->properties.at("CODE");
            // 如果cpg的methodNodes中包含node，则将code置为node的name
            if(node->getCPGNode()->getType() == "Method") {
                code = node->getCPGNode()->getName();
            }
            code = escapeSpecialCharacters(code);
            file << node->getId() << " [label=\"" << node->getId() << "<" + ThreadAPIUtil::getInstance()->getTypeString(node->getType()) + ">" + code;
            file << "\"];" << std::endl;
        }
        for(CCPGEdge* edge : function->getEdges()) {
            file << edge->getSrc()->getId() << " -> " << edge->getDst()->getId() << " [label=\"" << edge->getTypeString() << "\"];" << std::endl;
        }
        file << "}" << std::endl;
        file.close();
    }

    // create dot for every thread
    fs::path threadsOutputDir = outputDir / "threads";
    if (!fs::exists(threadsOutputDir)) {
        fs::create_directory(threadsOutputDir);
    }
    for(Thread* thread : ThreadCreationTree::getInstance()->getThreads()) {
        std::ofstream file(threadsOutputDir / (std::to_string(thread->getId()) + ".dot"));
        file << "digraph G {" << std::endl;
        for(CCPGNode* node : thread->getNodes()) {
            std::string code;
            code = node->getCPGNode()->properties.at("CODE");
            // 如果cpg的methodNodes中包含node，则将code置为node的name
            if(node->getCPGNode()->getType() == "Method") {
                code = node->getCPGNode()->getName();
            }
            code = escapeSpecialCharacters(code);
            file << node->getId() << " [label=\"" << node->getId() << "<" + ThreadAPIUtil::getInstance()->getTypeString(node->getType()) + ">" + code;
            file << "\"];" << std::endl;
        }
        for(CCPGEdge* edge : thread->getEdges()) {
            file << edge->getSrc()->getId() << " -> " << edge->getDst()->getId() << " [label=\"" << edge->getTypeString() << "\"];" << std::endl;
        }
        file << "}" << std::endl;
        file.close();
    }

    ThreadCreationTree::getInstance()->printThreadCreationTree(outputDir);
}