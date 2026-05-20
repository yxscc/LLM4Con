// src/PhasarUtil/PhasarPointerAnalysis.cpp

#include "PhasarUtil/PhasarPointerAnalysis.h"
#include "Util/PathUtils.h"

#include "phasar.h"
#include <algorithm>
#include <iomanip>
#include <unordered_map>
#include <cstring>

using namespace psr;

// ---------------------------------------------------------------------------
// Name-based entry heuristic (LAST-RESORT fallback only).
//
// Prior versions of this file matched any function whose name contained
// `_init`, `_exit`, `_work`, `_handler`, `_callback`, or `_thread`. That
// list recklessly scooped up inline helpers like `sema_init`,
// `__init_waitqueue_head`, `list_head_init`, anything ending in `_thread`
// (including non-thread getters like `get_task_struct_thread`), etc.,
// which then polluted the shared-object surface with garbage entries.
//
// The structural strategies below (section attribute, EXPORT_SYMBOL via
// __addressable, _eil_addr_ syscall wrappers, ops-table membership) are
// now authoritative. This residual name heuristic is only consulted when
// all structural signals yielded zero entries AND we are unwilling to
// fall back to "whole module", which would be intractable.
// ---------------------------------------------------------------------------
static int getKernelEntryPriority(const std::string& name) {
    // Priority 1: explicit syscall wrappers emitted by SYSCALL_DEFINE*.
    if (name.find("__x64_sys_") == 0 ||
        name.find("__ia32_sys_") == 0 ||
        name.find("__arm64_sys_") == 0 ||
        name.find("__se_sys_") == 0 ||
        name.find("__do_sys_") == 0 ||
        name.find("compat_sys_") == 0) {
        return 1;
    }
    // Priority 2: `sys_foo` top-level (must also start with sys_ but we
    // reject the generic names that shadow it, e.g. `syscall_return_slowpath`).
    if (name.find("sys_") == 0 && name.size() >= 5 &&
        name != "system_state" && name != "syscall_nr_to_meta") {
        return 1;
    }
    // Priority 3: module init/exit stubs specifically.
    if (name == "init_module" || name == "cleanup_module") {
        return 2;
    }
    // No more `_init` / `_exit` / `_handler` / `_work` / `_callback` /
    // `_thread` name-matching: it is unreliable and is superseded by
    // structural signals.
    return 0;
}

// Helper: recursively collect Function references from an LLVM Constant
static void collectFunctionPointersFromConstant(const llvm::Constant* C,
                                                 std::vector<const llvm::Function*>& out,
                                                 std::set<const llvm::Constant*>& visited) {
    if (!C || !visited.insert(C).second) return;

    if (auto* F = llvm::dyn_cast<llvm::Function>(C)) {
        if (!F->isDeclaration()) {
            out.push_back(F);
        }
        return;
    }
    if (llvm::isa<llvm::ConstantPointerNull>(C) ||
        llvm::isa<llvm::UndefValue>(C) ||
        llvm::isa<llvm::ConstantAggregateZero>(C) ||
        llvm::isa<llvm::ConstantDataSequential>(C) ||
        llvm::isa<llvm::ConstantInt>(C) ||
        llvm::isa<llvm::ConstantFP>(C)) {
        return;
    }
    if (auto* GV = llvm::dyn_cast<llvm::GlobalVariable>(C)) {
        if (GV->hasInitializer())
            collectFunctionPointersFromConstant(GV->getInitializer(), out, visited);
        return;
    }
    for (unsigned i = 0; i < C->getNumOperands(); ++i) {
        if (auto* op = llvm::dyn_cast<llvm::Constant>(C->getOperand(i)))
            collectFunctionPointersFromConstant(op, out, visited);
    }
}

// Helper function to check if a function name matches Linux kernel entry patterns
static bool isKernelEntryFunction(const std::string& name) {
    return getKernelEntryPriority(name) > 0;
}

// ---------------------------------------------------------------------------
// Structural entry-point signals (kernel-specific but entirely derived from
// LLVM IR structure, not name matching). Each signal below corresponds to a
// pattern observed directly in kernel bitcode for the dataset in
// kernel_experiment/, cross-checked against ground-truth patched functions.
//
//   S1  .init.text / .exit.text   section on a Function
//       (module_init / module_exit / subsys_initcall / ... all land here)
//
//   S2  EXPORT_SYMBOL / EXPORT_SYMBOL_GPL
//       The kernel emits `__UNIQUE_ID___addressable_<name><num>` globals
//       holding `ptr @<name>`. Presence of such a global is an unambiguous
//       "this function is an exported kernel API surface".
//
//   S3  Syscall wrappers
//       `_eil_addr_<name>` globals (kernel error injection list) hold
//       `ptr @<syscall_wrapper>` and are emitted for every SYSCALL_DEFINE*.
//       `__syscall_meta__<name>` globals also witness a syscall.
//
//   S4  Function-pointer membership in any struct global initializer
//       (ops tables, notifier blocks, work_struct, nfnl_callback arrays,
//        pernet_operations, file_operations, ...).
//
// A function is considered an entry iff ANY of S1..S4 holds for it.
// This replaces name-based pattern matching (which caused false positives
// like `sema_init`, `list_head_init`, `list_lru_walk` being picked as
// entries) AND catches previously-missed entries in modules that had no
// name-matching hits (e.g. led-triggers.c, blk-rq-qos.c, ring.c).
// ---------------------------------------------------------------------------

enum StructuralSignal {
    SIG_NONE          = 0,
    SIG_SECTION_INIT  = 1 << 0,  // .init.text or .exit.text
    SIG_EXPORT_SYMBOL = 1 << 1,  // EXPORT_SYMBOL[_GPL]
    SIG_SYSCALL       = 1 << 2,  // _eil_addr_ / __syscall_meta__
    SIG_OPS_MEMBER    = 1 << 3,  // function pointer in struct global
    // M7 P2: function passed to a known indirect-fork / callback-registration
    // sink such as kthread_create / kthread_run / request_irq /
    // request_threaded_irq / single_open / seq_open / proc_create_seq*.
    // Catches threads that the kernel actually spawns but that are not
    // stored in any struct global (e.g. io_wq_worker, kthread bodies).
    SIG_INDIRECT_FORK = 1 << 4,
};

// Scan a single (already stripped of casts / wrapper constants) Constant
// and, for every Function reference inside, OR the bit-set `sig` onto the
// per-function map.
static void tagFunctionRefs(const llvm::Constant* C,
                            unsigned sig,
                            std::unordered_map<const llvm::Function*, unsigned>& out) {
    std::vector<const llvm::Function*> funcs;
    std::set<const llvm::Constant*> visited;
    collectFunctionPointersFromConstant(C, funcs, visited);
    for (const llvm::Function* F : funcs) {
        if (!F || F->isDeclaration()) continue;
        out[F] |= sig;
    }
}

// Core structural scan: compute the per-function signal bitmap for every
// function defined in the module. The caller decides what to do with it.
static std::unordered_map<const llvm::Function*, unsigned>
computeStructuralEntrySignals(const llvm::Module* M) {
    std::unordered_map<const llvm::Function*, unsigned> sigMap;
    if (!M) return sigMap;

    // S1: section attributes on functions.
    for (const llvm::Function& F : M->functions()) {
        if (F.isDeclaration()) continue;
        if (F.hasSection()) {
            llvm::StringRef sec = F.getSection();
            if (sec.startswith(".init.text") ||
                sec.startswith(".exit.text") ||
                sec == ".cpuidle.text" || sec == ".ref.text") {
                sigMap[&F] |= SIG_SECTION_INIT;
            }
        }
    }

    // S2, S3, S4: scan all globals and classify by name prefix.
    for (const llvm::GlobalVariable& GV : M->globals()) {
        if (!GV.hasInitializer()) continue;
        if (!GV.hasName()) continue;
        llvm::StringRef n = GV.getName();

        // LLVM internals and kernel metadata tables that don't represent
        // real call-target lists.
        if (n.startswith("llvm.")) continue;
        if (n.startswith("__ksymtab") || n.startswith("__kstrtab") ||
            n.startswith("____versions")) continue;

        const llvm::Constant* Init = GV.getInitializer();

        // S2: EXPORT_SYMBOL — `__UNIQUE_ID___addressable_<fn><num>`.
        // These are single-pointer globals storing `ptr @fn`. They are
        // the most trustworthy "this function is a public entry" marker.
        if (n.startswith("__UNIQUE_ID___addressable_")) {
            tagFunctionRefs(Init, SIG_EXPORT_SYMBOL, sigMap);
            continue;
        }

        // S3a: syscall — `_eil_addr_<name>` holds ptr @__x64_sys_<name>.
        if (n.startswith("_eil_addr_")) {
            tagFunctionRefs(Init, SIG_SYSCALL, sigMap);
            continue;
        }
        // S3b: syscall metadata global — presence witnesses that both
        // `__x64_sys_<NAME>` and `__ia32_sys_<NAME>` (whichever is defined
        // in this module) are syscall entries. Look them up by name.
        if (n.startswith("__syscall_meta__")) {
            std::string syscall_name = n.substr(std::strlen("__syscall_meta__")).str();
            for (const char* prefix : {"__x64_sys_", "__ia32_sys_",
                                       "__arm64_sys_", "__se_sys_",
                                       "__do_sys_", "sys_"}) {
                std::string cand = std::string(prefix) + syscall_name;
                if (const llvm::Function* F = M->getFunction(cand)) {
                    if (!F->isDeclaration()) sigMap[F] |= SIG_SYSCALL;
                }
            }
            continue;
        }

        // Skip other book-keeping globals that happen to mention functions.
        if (n.startswith("__param") || n.startswith("__mod_") ||
            n.startswith("__initcall") || n.startswith("__exitcall")) {
            // NOTE: some of these *do* hold pointers to init functions.
            // For __initcall/__exitcall arrays specifically, the pointer
            // target IS an init callback — but those targets are already
            // caught by SIG_SECTION_INIT. For __param, the registered
            // getters/setters are legitimate entries; tag them.
            if (n.startswith("__param")) {
                tagFunctionRefs(Init, SIG_OPS_MEMBER, sigMap);
            }
            continue;
        }

        // Linker section blacklist: globals relegated to discard/debug
        // sections are not runtime call-target lists.
        if (GV.hasSection()) {
            llvm::StringRef sec = GV.getSection();
            if (sec.startswith(".discard") ||
                sec.startswith(".debug") ||
                sec == ".modinfo" ||
                sec.startswith(".note") ||
                sec == "llvm.metadata") {
                continue;
            }
        }

        // S4: any other global whose initializer contains function
        // pointers. Almost always an ops table / callback array.
        tagFunctionRefs(Init, SIG_OPS_MEMBER, sigMap);
    }

    // S5: indirect-fork / callback-registration sinks. For each call
    // instruction whose callee name matches a known sink, tag the
    // function-pointer argument as an indirect-fork entry. The sink
    // table below covers the kernel APIs that take a callback as an
    // argument *outside* of any struct global (otherwise S4 already
    // catches it). Keeps the surface tight: only direct
    // ConstantExpr/Function arguments are tagged, no aliasing chase.
    struct SinkSpec {
        const char* name;
        // bit i set ⇒ argument i is a callback function pointer to tag.
        unsigned argMask;
    };
    static const SinkSpec kSinks[] = {
        // kthread_create / kthread_run family: first arg is the thread fn.
        {"kthread_create",              1u << 0},
        {"kthread_create_on_node",      1u << 0},
        {"kthread_create_on_cpu",       1u << 0},
        {"kthread_create_worker",       0},          // workers, not fn
        {"kthread_run",                 1u << 0},
        {"kthread_run_on_cpu",          1u << 0},
        // single_open / seq_open variants: arg 1 is the show callback.
        {"single_open",                 1u << 1},
        {"single_open_size",            1u << 1},
        {"single_open_net",             1u << 1},
        {"seq_open",                    1u << 1},
        {"seq_open_private",            1u << 1},
        // proc_create variants that take a callback directly.
        // proc_create_single*(name, mode, parent, show)
        {"proc_create_single",          1u << 3},
        {"proc_create_single_data",     1u << 3},
        // request_irq(irq, handler, flags, name, dev)
        {"request_irq",                 1u << 1},
        {"request_any_context_irq",     1u << 1},
        // request_threaded_irq(irq, handler, thread_fn, flags, name, dev)
        {"request_threaded_irq",        (1u << 1) | (1u << 2)},
        // tasklet / timer init: callback is arg 1.
        {"tasklet_init",                1u << 1},
        {"tasklet_setup",               1u << 1},
        {"timer_setup",                 1u << 1},
        // workqueue: INIT_WORK / queue_work_on take a work_struct, but
        // the bare callback variant exists too.
        {"create_singlethread_workqueue", 0},  // no direct callback arg
        // RCU callbacks: callback is arg 1.
        {"call_rcu",                    1u << 1},
        {"call_srcu",                   1u << 2},
        {"call_rcu_tasks",              1u << 1},
    };

    for (const llvm::Function& F : M->functions()) {
        if (F.isDeclaration()) continue;
        for (const auto& BB : F) {
            for (const auto& I : BB) {
                const auto* CI = llvm::dyn_cast<llvm::CallBase>(&I);
                if (!CI) continue;
                const llvm::Function* callee = CI->getCalledFunction();
                if (!callee || !callee->hasName()) continue;
                llvm::StringRef name = callee->getName();
                for (const SinkSpec& spec : kSinks) {
                    if (spec.argMask == 0) continue;
                    if (name != spec.name) continue;
                    for (unsigned i = 0;
                         i < CI->arg_size() && i < 32; ++i) {
                        if (((spec.argMask >> i) & 1u) == 0) continue;
                        const llvm::Value* arg =
                            CI->getArgOperand(i)->stripPointerCasts();
                        if (const auto* CF = llvm::dyn_cast<llvm::Function>(arg)) {
                            if (!CF->isDeclaration()) {
                                sigMap[CF] |= SIG_INDIRECT_FORK;
                            }
                        }
                    }
                    break;
                }
            }
        }
    }

    // S6: function address stored into any pointer — i.e. the function
    // is captured as a callback in a stack-allocated struct (which
    // never appears in a global initializer, so S4 misses it). This
    // catches the kernel waitqueue / completion / tasklet pattern
    // where the callback is stored into a stack-local struct whose
    // address is then published via `add_wait_queue` / `tasklet_init`
    // / etc. CVE-2024-50082's `data.wq.func = rq_qos_wake_function`
    // is the canonical example. We deliberately accept a small amount
    // of imprecision here (any address-taken function counts) because
    // address-taken functions in kernel code are overwhelmingly
    // callbacks — the false-positive cost of analysing a few extra
    // entries is bounded by entry-pair pruning further down the line,
    // whereas missing the actual callback is a hard recall loss.
    for (const llvm::Function& F : M->functions()) {
        if (F.isDeclaration()) continue;
        bool addressTaken = false;
        for (const llvm::User* U : F.users()) {
            if (const auto* CB = llvm::dyn_cast<llvm::CallBase>(U)) {
                // F is being CALLED directly — not an address-take.
                if (CB->getCalledOperand() == &F) continue;
                // F is passed as an argument: this IS an address-take,
                // and S5 has already covered the well-known sinks.
                // Still mark, in case the sink isn't in our list.
                addressTaken = true;
                break;
            }
            if (llvm::isa<llvm::StoreInst>(U)) {
                addressTaken = true;
                break;
            }
            // ConstantExpr / Constant uses: already covered by S4 if
            // they end up inside a global initializer. Skip here to
            // avoid double-tagging.
        }
        if (addressTaken) sigMap[&F] |= SIG_INDIRECT_FORK;
    }

    return sigMap;
}

// Pretty name for a signal bitset (debug output).
static std::string signalsToString(unsigned s) {
    std::string r;
    auto add = [&](const char* n){ if (!r.empty()) r += "+"; r += n; };
    if (s & SIG_SECTION_INIT)  add("section");
    if (s & SIG_EXPORT_SYMBOL) add("export");
    if (s & SIG_SYSCALL)       add("syscall");
    if (s & SIG_OPS_MEMBER)    add("ops");
    if (s & SIG_INDIRECT_FORK) add("indirect_fork");
    return r.empty() ? "-" : r;
}

// True if F's debug-info primary source file is a header (.h / .hh).
// Inline helpers from headers are repeatedly "defined" in every .o that
// includes them and, when picked as entry points, just pollute the
// shared-object surface. We drop them unless they also carry a strong
// structural signal (exported / ops member / in a section) which
// indicates the kernel explicitly registered them.
static bool isDefinedInHeader(const llvm::Function& F) {
    if (auto* sp = F.getSubprogram()) {
        llvm::StringRef fname = sp->getFilename();
        if (fname.endswith(".h") || fname.endswith(".hh") ||
            fname.endswith(".hpp") || fname.endswith(".hxx")) {
            return true;
        }
    }
    return false;
}

PhasarPointerAnalysis::PhasarPointerAnalysis(const std::string &bitcodeFilePath,
                                             const std::vector<std::string>& userEntryPoints) {
    DB = std::make_unique<psr::LLVMProjectIRDB>(bitcodeFilePath);

    // Try to find entry points: user-specified > 'main' > auto-detected kernel entries
    std::vector<std::string> entryPoints;
    
    // Priority 1: User-specified entry points (from config file)
    if (!userEntryPoints.empty()) {
        std::cout << "Using " << userEntryPoints.size() << " user-specified entry point(s):" << std::endl;
        for (const auto& ep : userEntryPoints) {
            // Verify the function exists in the bitcode
            if (DB && DB->getFunctionDefinition(ep)) {
                entryPoints.push_back(ep);
                std::cout << "  - " << ep << " (found)" << std::endl;
            } else {
                std::cerr << "  - " << ep << " (NOT FOUND in bitcode, skipping)" << std::endl;
            }
        }
    }
    // Priority 2: Check for 'main'
    else if (DB && DB->getFunctionDefinition("main")) {
        entryPoints.push_back("main");
        std::cout << "Found 'main' function as entry point." << std::endl;
    } else {
        // No 'main' found — discover kernel entry points STRUCTURALLY.
        //
        // We compute, for every function defined in the module, a bitmap of
        // the structural signals that mark it as an entry point:
        //   * placed in .init.text / .exit.text (module_init, subsys_init…)
        //   * referenced by __UNIQUE_ID___addressable_* (EXPORT_SYMBOL[_GPL])
        //   * referenced by _eil_addr_* / __syscall_meta__* (syscalls)
        //   * stored into any struct global (ops tables, notifier_block,
        //     work_struct, file_operations, pernet_operations, …)
        //
        // Any function with ≥1 signal becomes an entry. This is strictly
        // more precise than the old `_init/_exit/_work/_handler/_callback/
        // _thread` substring match, which mis-classified inline helpers
        // like `sema_init` and `list_head_init` as entries.
        std::cout << "No 'main' function found. Auto-discovering kernel entry points..." << std::endl;
        std::set<std::string> entrySet;

        if (DB) {
            const llvm::Module* M = DB->getModule();
            auto sigMap = computeStructuralEntrySignals(M);

            // Counters for reporting.
            int nSection = 0, nExport = 0, nSyscall = 0, nOps = 0, nIndirect = 0;

            for (const auto& [F, sig] : sigMap) {
                if (!F || sig == SIG_NONE) continue;

                // Drop inline-in-header definitions that only made it into
                // this .o by duplication and carry no explicit "registered
                // with the kernel" signal. They pollute the surface.
                if (isDefinedInHeader(*F) &&
                    (sig & (SIG_EXPORT_SYMBOL | SIG_SYSCALL |
                            SIG_SECTION_INIT | SIG_INDIRECT_FORK)) == 0) {
                    // Only an ops-table reference — likely a spurious
                    // match (address stored into a debug struct). Drop.
                    if ((sig & SIG_OPS_MEMBER) && F->hasLocalLinkage()) continue;
                }

                std::string name = F->getName().str();
                if (entrySet.insert(name).second) {
                    entryPoints.push_back(name);
                    if (sig & SIG_SECTION_INIT)  nSection++;
                    if (sig & SIG_EXPORT_SYMBOL) nExport++;
                    if (sig & SIG_SYSCALL)       nSyscall++;
                    if (sig & SIG_OPS_MEMBER)    nOps++;
                    if (sig & SIG_INDIRECT_FORK) nIndirect++;
                }
            }

            std::cout << "[Auto-Entry Structural] "
                      << nSection << " init/exit section, "
                      << nExport  << " EXPORT_SYMBOL, "
                      << nSyscall << " syscall wrapper, "
                      << nOps     << " ops-table member, "
                      << nIndirect << " indirect-fork sink"
                      << " (" << entrySet.size() << " unique function(s))"
                      << std::endl;

            // Name-heuristic pass — ADDITIVE on top of the structural
            // signals (previously this only ran when entrySet was
            // empty, which broke modules that produced 1-2 structural
            // hits and missed dozens of obvious name-heuristic entries:
            // e.g. CVE-2024-50082 went from 16 entries to 1 the moment
            // SIG_INDIRECT_FORK added rq_qos_wake_function alone). The
            // `getKernelEntryPriority(...) > 0` filter is narrow enough
            // (`main`, explicit SYSCALL wrappers, `init_module` /
            // `cleanup_module`) that unioning it never floods the
            // surface.
            const size_t structuralCount = entrySet.size();
            for (const llvm::Function* F : DB->getAllFunctions()) {
                if (F->isDeclaration()) continue;
                std::string funcName = F->getName().str();
                if (getKernelEntryPriority(funcName) > 0 &&
                    entrySet.insert(funcName).second) {
                    entryPoints.push_back(funcName);
                }
            }
            const size_t nameAdded = entrySet.size() - structuralCount;
            if (nameAdded > 0) {
                std::cout << "[Auto-Entry Name-Heuristic] +"
                          << nameAdded
                          << " entry point(s) on top of structural signals."
                          << std::endl;
            }

            // Final fallback: single-file TUs (e.g. mm/ksm.c compiled
            // standalone) expose NO structural signals because the real
            // EXPORT_SYMBOL / syscall-meta / ops-table globals live in
            // other translation units that were not part of this build.
            // In that case, every externally-linkable `define` is a
            // public entry point from the rest of the kernel's point of
            // view. We gate this so we don't flood regular modules.
            //
            // M7 P2: relax the gate from `entrySet.empty()` to
            // `entrySet.size() < 5`. With the SIG_INDIRECT_FORK signal
            // we now pick up isolated callbacks (e.g. rq_qos_wake_function
            // in CVE-2024-50082) which previously left entrySet empty;
            // those single-callback hits would silently disable the
            // externally-linkable supplement, which is precisely what
            // makes a single-TU module analysable. Threshold 5 is well
            // below any real kernel module's structural-entry count
            // (the smallest real-module run on the benchmark surfaces
            // 13 entries; the next smallest 22) so we don't promote
            // any module that already has a healthy structural surface.
            if (entrySet.size() < 5) {
                std::cout << "[Auto-Entry Fallback] No structural or "
                             "name-heuristic entries; promoting every "
                             "externally-linkable function in this TU."
                          << std::endl;
                std::size_t nPromoted = 0;
                for (const llvm::Function* F : DB->getAllFunctions()) {
                    if (!F || F->isDeclaration()) continue;
                    // Skip LLVM intrinsics and compiler-generated helpers.
                    if (F->getName().startswith("llvm.") ||
                        F->getName().startswith("__asan") ||
                        F->getName().startswith("__kasan") ||
                        F->getName().startswith("__ubsan") ||
                        F->getName().startswith("__tsan") ||
                        F->getName().startswith("__msan")) {
                        continue;
                    }
                    // Externally-linkable = the linker can see it from
                    // another TU. internal/private/linkonce_odr static
                    // helpers do not qualify (they can only be reached
                    // via in-TU calls, which will show up via the call
                    // graph rooted at the other external entries we pick).
                    auto L = F->getLinkage();
                    const bool external =
                        (L == llvm::GlobalValue::ExternalLinkage) ||
                        (L == llvm::GlobalValue::WeakAnyLinkage) ||
                        (L == llvm::GlobalValue::WeakODRLinkage) ||
                        (L == llvm::GlobalValue::ExternalWeakLinkage) ||
                        (L == llvm::GlobalValue::CommonLinkage) ||
                        (L == llvm::GlobalValue::AppendingLinkage);
                    if (!external) continue;
                    std::string funcName = F->getName().str();
                    if (entrySet.insert(funcName).second) {
                        entryPoints.push_back(funcName);
                        ++nPromoted;
                    }
                }
                std::cout << "[Auto-Entry Fallback] Promoted "
                          << nPromoted
                          << " externally-linkable function(s) as pseudo-"
                             "entries (single-file TU fallback)."
                          << std::endl;
            }
        }
        
        if (entryPoints.empty()) {
            std::cerr << "Error: Could not find any entry points in: " 
                      << bitcodeFilePath << std::endl;
            TH = std::make_unique<psr::DIBasedTypeHierarchy>(*DB);
            ICFG = std::make_unique<psr::LLVMBasedICFG>(DB.get(), psr::CallGraphAnalysisType::CHA, std::vector<std::string>{});
            PTA = std::make_unique<psr::LLVMAliasSet>(DB.get());
            return;
        }
        
        std::cout << "[Auto-Entry] Total: " << entryPoints.size() << " kernel entry point(s):" << std::endl;
        for (size_t i = 0; i < std::min(entryPoints.size(), size_t(20)); i++) {
            std::cout << "  - " << entryPoints[i] << std::endl;
        }
        if (entryPoints.size() > 20) {
            std::cout << "  ... and " << (entryPoints.size() - 20) << " more" << std::endl;
        }
    }

    std::cout << "[Phasar] Building DIBasedTypeHierarchy..." << std::endl;
    std::cout.flush();
    TH = std::make_unique<psr::DIBasedTypeHierarchy>(*DB);
    std::cout << "[Phasar] DIBasedTypeHierarchy done." << std::endl;
    
    std::cout << "[Phasar] Building LLVMBasedICFG (CHA)..." << std::endl;
    std::cout.flush();
    ICFG = std::make_unique<psr::LLVMBasedICFG>(DB.get(), psr::CallGraphAnalysisType::CHA, entryPoints);
    std::cout << "[Phasar] LLVMBasedICFG done." << std::endl;
    
    std::cout << "[Phasar] Building LLVMAliasSet..." << std::endl;
    std::cout.flush();
    PTA = std::make_unique<psr::LLVMAliasSet>(DB.get());
    std::cout << "[Phasar] LLVMAliasSet done." << std::endl;
    
    // Store discovered entry points for later use
    discoveredEntryPoints_ = entryPoints;

    std::cout << "[Phasar] Building location maps for all functions..." << std::endl;
    std::cout.flush();
    size_t funcCount = 0;
    size_t totalFuncs = 0;
    for (const llvm::Function *F : DB->getAllFunctions()) {
        if (!F->isDeclaration()) totalFuncs++;
    }
    
    for (const llvm::Function *F : DB->getAllFunctions()) {
        if (F->isDeclaration()) continue;
        funcCount++;
        if (funcCount % 1000 == 0 || funcCount == totalFuncs) {
            std::cout << "[Phasar] Processed " << funcCount << "/" << totalFuncs << " functions..." << std::endl;
            std::cout.flush();
        }
        for (const auto &BB : *F) {
            for (const auto &I : BB) {
                if (const auto &Loc = I.getDebugLoc()) {

                    std::string File = PathUtils::extractBaseFileName(Loc->getFilename().str());
                    unsigned Line = Loc.getLine();

                    if (!File.empty() && Line > 0) {
                        NodeLoc NLoc(File, Line, nullptr);
                        if (const auto *CI = llvm::dyn_cast<llvm::CallInst>(&I)) {
                            LocToCallInstsMap[NLoc].push_back(CI);
                        } else if (const auto *LI = llvm::dyn_cast<llvm::LoadInst>(&I)) {
                            LocToLoadInstsMap[NLoc].push_back(LI);
                        } else if (const auto *SI = llvm::dyn_cast<llvm::StoreInst>(&I)) {
                            LocToStoreInstsMap[NLoc].push_back(SI);
                        }
                    }
                }
            }
        }
    }
    std::cout << "[Phasar] Location maps built. Total functions: " << funcCount << std::endl;
}

std::vector<const llvm::CallInst *> PhasarPointerAnalysis::getCallInstsByLoc(const NodeLoc &Loc) const {
    auto It = LocToCallInstsMap.find(Loc);
    return (It != LocToCallInstsMap.end()) ? It->second : std::vector<const llvm::CallInst *>();
}

std::vector<const llvm::LoadInst *> PhasarPointerAnalysis::getLoadInstsByLoc(const NodeLoc &Loc) const {
    auto It = LocToLoadInstsMap.find(Loc);
    return (It != LocToLoadInstsMap.end()) ? It->second : std::vector<const llvm::LoadInst *>();
}

std::vector<const llvm::StoreInst *> PhasarPointerAnalysis::getStoreInstsByLoc(const NodeLoc &Loc) const {
    auto It = LocToStoreInstsMap.find(Loc);
    return (It != LocToStoreInstsMap.end()) ? It->second : std::vector<const llvm::StoreInst *>();
}

// Destructor needs to be defined in the .cpp file where the types are complete
PhasarPointerAnalysis::~PhasarPointerAnalysis() = default;

bool PhasarPointerAnalysis::areAliases(const llvm::Value *V1, const llvm::Value *V2) {
    if (!V1 || !V2) {
        return false;
    }
    // Use the alias() method from LLVMAliasSet, which returns an AliasResult enum
    return PTA->alias(V1, V2) != psr::AliasResult::NoAlias;
}

std::set<const llvm::Value *> PhasarPointerAnalysis::getPointsToSet(const llvm::Value *Ptr) {
    // IMPORTANT: The LLVMAliasSet analysis does not provide points-to sets, only alias results.
    // This function is kept to satisfy the interface, but it will not return useful data with this backend.
    // For true points-to sets, a different Phasar analysis would be needed.
    return {};
}

const llvm::Value* PhasarPointerAnalysis::getValueByName(const std::string &funcName, const std::string &varName) {
    const llvm::Function* F = DB->getFunction(funcName);
    if (!F) {
        return nullptr;
    }
    for (const auto &BB : *F) {
        for (const auto &I : BB) {
            if (I.hasName() && I.getName() == varName) {
                return &I;
            }
        }
    }
    for (const auto &Arg : F->args()) {
        if (Arg.hasName() && Arg.getName() == varName) {
            return &Arg;
        }
    }
    return nullptr;
}

std::vector<EntryPointInfo> PhasarPointerAnalysis::getPotentialEntryPoints() {
    std::vector<EntryPointInfo> potentialEntries;
    return potentialEntries;
}

EntryPointInfo PhasarPointerAnalysis::getMainFunction() const {
    EntryPointInfo mainInfo;
    
    // First try 'main'
    if (const llvm::Function *MainFunc = DB->getFunctionDefinition("main")) {
        mainInfo.functionName = MainFunc->getName().str();

        if (auto *SP = MainFunc->getSubprogram()) {
            mainInfo.fileName = SP->getFilename().str();
            mainInfo.lineNumber = SP->getLine();
        } else {
            mainInfo.fileName = "N/A";
            mainInfo.lineNumber = 0;
        }

        std::cout << "DEBUG: Found entry point -> " << mainInfo.toString() << std::endl;
        return mainInfo;
    }
    
    // No 'main' - try to return the first discovered kernel entry point
    if (!discoveredEntryPoints_.empty()) {
        const std::string& firstEntry = discoveredEntryPoints_[0];
        if (const llvm::Function *EntryFunc = DB->getFunctionDefinition(firstEntry)) {
            mainInfo.functionName = EntryFunc->getName().str();

            if (auto *SP = EntryFunc->getSubprogram()) {
                mainInfo.fileName = SP->getFilename().str();
                mainInfo.lineNumber = SP->getLine();
            } else {
                mainInfo.fileName = "N/A";
                mainInfo.lineNumber = 0;
            }

            std::cout << "DEBUG: Using kernel entry point -> " << mainInfo.toString() << std::endl;
            return mainInfo;
        }
    }
    
    std::cerr << "Warning: Could not find a 'main' function definition in the provided bitcode." << std::endl;
    return {};
}

std::vector<EntryPointInfo> PhasarPointerAnalysis::getAllEntryPointInfos() const {
    std::vector<EntryPointInfo> allEntries;
    
    for (const std::string& entryName : discoveredEntryPoints_) {
        if (const llvm::Function *EntryFunc = DB->getFunctionDefinition(entryName)) {
            EntryPointInfo info;
            info.functionName = EntryFunc->getName().str();
            
            if (auto *SP = EntryFunc->getSubprogram()) {
                info.fileName = SP->getFilename().str();
                info.lineNumber = SP->getLine();
            } else {
                info.fileName = "N/A";
                info.lineNumber = 0;
            }
            
            allEntries.push_back(info);
        }
    }
    
    std::cout << "getAllEntryPointInfos: Returning " << allEntries.size() << " entry points for kernel module analysis" << std::endl;
    return allEntries;
}

std::vector<const llvm::Function *> PhasarPointerAnalysis::getAllLLVMFunctions() const {
    std::vector<const llvm::Function *> AllFunctions;
    if (DB) {
        // LLVMProjectIRDB 通过基类继承了 getAllFunctions() 方法
        for (const llvm::Function *F : DB->getAllFunctions()) {
            AllFunctions.push_back(F);
        }
    }
    return AllFunctions;
}

std::vector<const llvm::Function*> PhasarPointerAnalysis::getCalleesOfCallAt(const llvm::Instruction* callInst) const {
    if (!ICFG || !callInst) {
        return {};
    }
    // 内部实现细节：调用ICFG的具体方法
    auto calleeRange = ICFG->getCalleesOfCallAt(callInst);
    // 将Phasar返回的范围（range）转换为一个标准的vector
    return {calleeRange.begin(), calleeRange.end()};
}

std::vector<const llvm::GlobalVariable*> PhasarPointerAnalysis::getAllGlobalVariables() const {
    std::vector<const llvm::GlobalVariable*> globalVars;
    if (DB) {
        for (const auto& Global : DB->getModule()->globals()) {
            globalVars.push_back(&Global);
        }
    }
    return globalVars;
}

std::vector<std::string> PhasarPointerAnalysis::discoverCallbackEntryPoints() const {
    std::vector<std::string> callbacks;
    if (!DB) return callbacks;

    const llvm::Module* M = DB->getModule();
    if (!M) return callbacks;

    std::set<std::string> seen;

    for (const auto& GV : M->globals()) {
        if (!GV.hasInitializer()) continue;
        if (!GV.hasName()) continue;

        llvm::StringRef gvName = GV.getName();

        // Skip LLVM internal globals and kernel metadata tables
        if (gvName.startswith("llvm.") ||
            gvName.startswith("__ksymtab") ||
            gvName.startswith("__kstrtab") ||
            gvName.startswith("__param") ||
            gvName.startswith("__mod_") ||
            gvName.startswith("____versions") ||
            gvName.startswith("__initcall") ||
            gvName.startswith("__exitcall"))
            continue;

        const llvm::Constant* Init = GV.getInitializer();

        std::vector<const llvm::Function*> funcs;
        std::set<const llvm::Constant*> visited;
        collectFunctionPointersFromConstant(Init, funcs, visited);

        if (funcs.size() >= 2) {
            std::cout << "[Callback Discovery] Global '" << gvName.str()
                      << "' contains " << funcs.size() << " function pointer(s):" << std::endl;

            for (const llvm::Function* F : funcs) {
                std::string name = F->getName().str();
                if (seen.insert(name).second) {
                    callbacks.push_back(name);
                    std::cout << "  - " << name << std::endl;
                }
            }
        }
    }

    return callbacks;
}

// Recursively collect non-local memory access pointers from a function and its callees
static void collectMemoryPointers(const llvm::Function* F, int depth,
                                  std::set<const llvm::Function*>& visited,
                                  std::set<const llvm::Value*>& writePtrs,
                                  std::set<const llvm::Value*>& readPtrs) {
    if (!F || F->isDeclaration() || depth < 0 || !visited.insert(F).second)
        return;

    for (const auto& BB : *F) {
        for (const auto& I : BB) {
            if (auto* SI = llvm::dyn_cast<llvm::StoreInst>(&I)) {
                auto* base = SI->getPointerOperand()->stripPointerCasts();
                if (!llvm::isa<llvm::AllocaInst>(base))
                    writePtrs.insert(SI->getPointerOperand());
            } else if (auto* LI = llvm::dyn_cast<llvm::LoadInst>(&I)) {
                auto* base = LI->getPointerOperand()->stripPointerCasts();
                if (!llvm::isa<llvm::AllocaInst>(base))
                    readPtrs.insert(LI->getPointerOperand());
            } else if (auto* CI = llvm::dyn_cast<llvm::CallInst>(&I)) {
                if (auto* callee = CI->getCalledFunction())
                    collectMemoryPointers(callee, depth - 1, visited,
                                          writePtrs, readPtrs);
            }
        }
    }
}

void PhasarPointerAnalysis::computeEntryPointConflicts() {
    if (!DB || discoveredEntryPoints_.size() < 2) return;
    const llvm::Module* M = DB->getModule();
    if (!M) return;

    struct FuncFootprint {
        std::set<const llvm::Value*> writePtrs;
        std::set<const llvm::Value*> readPtrs;
    };

    std::map<std::string, FuncFootprint> footprints;

    std::cout << "[Conflict Analysis] Computing memory footprints for "
              << discoveredEntryPoints_.size() << " entry points..." << std::endl;

    for (const auto& name : discoveredEntryPoints_) {
        const llvm::Function* F = M->getFunction(name);
        if (!F || F->isDeclaration()) continue;
        FuncFootprint fp;
        std::set<const llvm::Function*> visited;
        collectMemoryPointers(F, 2, visited, fp.writePtrs, fp.readPtrs);
        footprints[name] = std::move(fp);
    }

    size_t totalPairs = discoveredEntryPoints_.size() *
                        (discoveredEntryPoints_.size() - 1) / 2;

    for (size_t i = 0; i < discoveredEntryPoints_.size(); ++i) {
        for (size_t j = i + 1; j < discoveredEntryPoints_.size(); ++j) {
            const auto& n1 = discoveredEntryPoints_[i];
            const auto& n2 = discoveredEntryPoints_[j];
            auto it1 = footprints.find(n1);
            auto it2 = footprints.find(n2);
            if (it1 == footprints.end() || it2 == footprints.end()) continue;

            bool conflict = false;

            // Check writes of F1 against all accesses of F2
            for (const auto* wp : it1->second.writePtrs) {
                if (conflict) break;
                for (const auto* rp : it2->second.writePtrs) {
                    if (areAliases(wp, rp)) { conflict = true; break; }
                }
                if (conflict) break;
                for (const auto* rp : it2->second.readPtrs) {
                    if (areAliases(wp, rp)) { conflict = true; break; }
                }
            }
            // Check writes of F2 against reads of F1
            if (!conflict) {
                for (const auto* wp : it2->second.writePtrs) {
                    if (conflict) break;
                    for (const auto* rp : it1->second.readPtrs) {
                        if (areAliases(wp, rp)) { conflict = true; break; }
                    }
                }
            }

            if (conflict) {
                auto key = (n1 < n2) ? std::make_pair(n1, n2)
                                     : std::make_pair(n2, n1);
                conflictingPairs_.insert(key);
            }
        }
    }

    std::cout << "[Conflict Analysis] " << conflictingPairs_.size()
              << " conflicting pairs out of " << totalPairs
              << " total (" << std::fixed << std::setprecision(1)
              << (100.0 * conflictingPairs_.size() / std::max(totalPairs, size_t(1)))
              << "% kept)" << std::endl;
}

bool PhasarPointerAnalysis::areEntryPointsConflicting(
        const std::string& f1, const std::string& f2) const {
    if (conflictingPairs_.empty()) return true; // no analysis done → conservative
    auto key = (f1 < f2) ? std::make_pair(f1, f2) : std::make_pair(f2, f1);
    return conflictingPairs_.count(key) > 0;
}

// Return the raw underlying Module owned by the Phasar IRDB, used by
// field-level helpers (needs DataLayout etc.).
const llvm::Module* PhasarPointerAnalysis::getModule() const {
    return DB ? DB->getModule() : nullptr;
}

// -- Field-level conflict query ----------------------------------------------

namespace {

// Walk F (and up to `depth` levels of callees) and bucket each non-local
// pointer by SharedFieldKey. Two parallel maps separate writes from reads
// so the caller can enforce "at least one side writes" for conflict.
static void collectFieldFootprint(const llvm::Function* F, int depth,
                                  const llvm::Module& M,
                                  std::set<const llvm::Function*>& visited,
                                  std::set<query::SharedFieldKey>& writes,
                                  std::set<query::SharedFieldKey>& reads) {
    if (!F || F->isDeclaration() || depth < 0 || !visited.insert(F).second)
        return;
    for (const auto& BB : *F) {
        for (const auto& I : BB) {
            if (auto* SI = llvm::dyn_cast<llvm::StoreInst>(&I)) {
                if (auto key = query::SharedFieldKey::fromValue(
                        SI->getPointerOperand(), M)) {
                    writes.insert(*key);
                }
            } else if (auto* LI = llvm::dyn_cast<llvm::LoadInst>(&I)) {
                if (auto key = query::SharedFieldKey::fromValue(
                        LI->getPointerOperand(), M)) {
                    reads.insert(*key);
                }
            } else if (auto* CI = llvm::dyn_cast<llvm::CallInst>(&I)) {
                if (auto* callee = CI->getCalledFunction())
                    collectFieldFootprint(callee, depth - 1, M, visited,
                                          writes, reads);
            }
        }
    }
}

} // namespace

std::vector<query::SharedFieldKey>
PhasarPointerAnalysis::getFieldsConflictingBetween(
        const std::string& e1, const std::string& e2) const {
    std::vector<query::SharedFieldKey> out;
    if (!DB) return out;
    const llvm::Module* M = DB->getModule();
    if (!M) return out;
    const llvm::Function* F1 = M->getFunction(e1);
    const llvm::Function* F2 = M->getFunction(e2);
    if (!F1 || !F2 || F1->isDeclaration() || F2->isDeclaration()) return out;

    std::set<query::SharedFieldKey> w1, r1, w2, r2;
    {
        std::set<const llvm::Function*> visited;
        collectFieldFootprint(F1, 2, *M, visited, w1, r1);
    }
    {
        std::set<const llvm::Function*> visited;
        collectFieldFootprint(F2, 2, *M, visited, w2, r2);
    }

    // Conflict = same field touched by both, with at least one side writing.
    auto add = [&](const query::SharedFieldKey& k) {
        if (std::find(out.begin(), out.end(), k) == out.end())
            out.push_back(k);
    };
    for (const auto& k : w1) {
        if (w2.count(k) || r2.count(k)) add(k);
    }
    for (const auto& k : w2) {
        if (r1.count(k)) add(k);
    }
    return out;
}

bool PhasarPointerAnalysis::doEntriesHaveSharedData(
        const std::string& e1, const std::string& e2) const {
    return !getFieldsConflictingBetween(e1, e2).empty();
}

bool PhasarPointerAnalysis::isGuardedByStaticInitializer(const llvm::StoreInst* storeInst) const {
    if (!storeInst) {
        return false;
    }

    // 1. 获取Store指令所在的BasicBlock
    const llvm::BasicBlock* currentBlock = storeInst->getParent();
    if (!currentBlock) {
        return false;
    }

    // 2. 获取Store指令的目标指针（即被写入的静态变量）
    const llvm::Value* storedPtr = storeInst->getPointerOperand();
    if (!storedPtr->hasName() || !storedPtr->getName().startswith("_ZZ")) {
        return false; // 我们只关心对静态局部变量的写操作
    }

    // 3. 构建对应的守卫变量名
    std::string varName = storedPtr->getName().str();
    std::string guardName = "_ZGV" + varName.substr(1); // 将 _ZZ 替换为 _ZGV

    // 4. 回溯查找控制流前驱，寻找守卫检查
    // 为了避免在复杂的CFG中无限循环，我们只回溯有限的步数（通常守卫检查就在前一个或前几个块中）
    std::queue<const llvm::BasicBlock*> worklist;
    std::set<const llvm::BasicBlock*> visited;
    worklist.push(currentBlock);
    visited.insert(currentBlock);
    
    int search_depth = 0;
    const int max_depth = 5; // 限制回溯深度

    while (!worklist.empty() && search_depth < max_depth) {
        const llvm::BasicBlock* block = worklist.front();
        worklist.pop();

        // 获取当前块的终止指令，通常是BranchInst
        const llvm::Instruction* terminator = block->getTerminator();
        if (const auto* branch = llvm::dyn_cast<llvm::BranchInst>(terminator)) {
            if (branch->isConditional()) {
                // 如果是条件分支，检查其条件
                if (const auto* cmp = llvm::dyn_cast<llvm::CmpInst>(branch->getCondition())) {
                    if (const auto* load = llvm::dyn_cast<llvm::LoadInst>(cmp->getOperand(0))) {
                        const llvm::Value* loadedVar = load->getPointerOperand();
                        if (loadedVar->hasName() && loadedVar->getName() == guardName) {
                            // 找到了！这个Store被对应的守卫变量检查所控制
                            return true;
                        }
                    }
                }
            }
        }
        
        // 如果当前块不是，则继续向上回溯
        if (block != &block->getParent()->getEntryBlock()) {
             for (const llvm::BasicBlock *Pred : llvm::predecessors(block)) {
                if (visited.find(Pred) == visited.end()) {
                    worklist.push(Pred);
                    visited.insert(Pred);
                }
            }
        }
        search_depth++;
    }

    return false;
}