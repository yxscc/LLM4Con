// src/PhasarUtil/PhasarPointerAnalysis.cpp

#include "PhasarUtil/PhasarPointerAnalysis.h"
#include "Util/PathUtils.h"

#include "phasar.h"
#include <algorithm>
#include <iomanip>
#include <unordered_map>
#include <cstring>
#include <cstdlib>

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
        name.find("compat_sys_") == 0 ||
        name.find("SyS_") == 0) {
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
//       The kernel emits addressable globals holding `ptr @<name>` for
//       exported symbols. Older builds use
//       `__UNIQUE_ID___addressable_<name><num>`; newer builds commonly use
//       `__addressable_<name><line>`. Presence of either form is an
//       unambiguous "this function is an exported kernel API surface".
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
    // Public API wrapper alias: an exported one-hop wrapper whose only
    // semantic callee is an internal implementation function. Some annotations
    // and bug reports name the internal implementation (e.g. __foo) rather than
    // the exported wrapper (foo); both represent the same externally-triggered
    // entry surface.
    SIG_PUBLIC_ALIAS  = 1 << 5,
    // External-linkage function that is not exported/registered in this sliced
    // bitcode, but has a field-level conflict with a strong entry. This covers
    // cross-TU public callbacks when the registration TU is absent from the
    // prepared case, without promoting all external functions.
    SIG_CROSS_TU_PUBLIC = 1 << 6,
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
static bool globalLooksLikeCallbackTable(const llvm::GlobalVariable& GV);

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
        // __kstrtab* are the export *name* string tables and ____versions is
        // modversion CRC data — neither references a function pointer.
        if (n.startswith("__kstrtab") || n.startswith("____versions")) continue;

        const llvm::Constant* Init = GV.getInitializer();

        // S2: EXPORT_SYMBOL — markers that store a pointer to the exported fn.
        //
        //  (a) Newer kernels/clang: `__UNIQUE_ID___addressable_<fn><num>` or
        //      `__addressable_<fn><line>` globals storing `ptr @fn`.
        //  (b) Older kernels (4.x): `__ksymtab_<name>` (and the gpl/section
        //      variants) emit a `struct kernel_symbol { ptrtoint(@fn), ptr
        //      @__kstrtab_<name> }` — a direct pointer to the exported
        //      function. PREL32-relative variants encode it as
        //      `trunc(sub(ptrtoint(@fn), ptrtoint(@self)))`; either way the
        //      function reference is reachable by walking the initializer.
        //
        // Both are the most trustworthy public-entry marker. (Earlier code
        // skipped __ksymtab outright, which silently dropped EVERY export on
        // older-kernel cases such as CVE-2016-9806's __netlink_dump_start.)
        if (n.startswith("__UNIQUE_ID___addressable_") ||
            n.startswith("__addressable_") ||
            n.startswith("__ksymtab")) {
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

        // S4: known callback / ops table globals. Do not promote arbitrary
        // struct globals that merely contain a function pointer; those are
        // often helper tables and create entry explosion.
        if (globalLooksLikeCallbackTable(GV)) {
            tagFunctionRefs(Init, SIG_OPS_MEMBER, sigMap);
        }
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

    // S7: thin exported-wrapper aliases. EXPORT_SYMBOL often exposes a small
    // wrapper while the meaningful body lives in an internal __foo function.
    // Treat the internal callee as an entry only when the wrapper has exactly
    // one real, defined callee; this avoids promoting arbitrary helper calls
    // from large exported APIs.
    auto isIgnoredWrapperCallee = [](llvm::StringRef name) {
        return name.startswith("llvm.") ||
               name.startswith("__asan") ||
               name.startswith("__kasan") ||
               name.startswith("__ubsan") ||
               name.startswith("__tsan") ||
               name.startswith("__msan");
    };
    auto namesLookLikeWrapperAlias = [](llvm::StringRef wrapper,
                                        llvm::StringRef impl) {
        std::string expected = "__" + wrapper.str();
        return impl == expected;
    };
    std::vector<const llvm::Function*> exportedFunctions;
    for (const auto& [F, sig] : sigMap) {
        if (F && !F->isDeclaration() && (sig & SIG_EXPORT_SYMBOL)) {
            exportedFunctions.push_back(F);
        }
    }
    for (const llvm::Function* F : exportedFunctions) {
        std::set<const llvm::Function*> callees;
        for (const llvm::BasicBlock& BB : *F) {
            for (const llvm::Instruction& I : BB) {
                const auto* CI = llvm::dyn_cast<llvm::CallBase>(&I);
                if (!CI) continue;
                const llvm::Function* callee = CI->getCalledFunction();
                if (!callee || callee->isDeclaration() || !callee->hasName())
                    continue;
                if (isIgnoredWrapperCallee(callee->getName())) continue;
                callees.insert(callee);
            }
        }
        if (callees.size() != 1) continue;
        const llvm::Function* impl = *callees.begin();
        if (!impl || impl == F || impl->isDeclaration()) continue;
        if (!impl->hasLocalLinkage()) continue;
        if (!namesLookLikeWrapperAlias(F->getName(), impl->getName())) continue;
        sigMap[impl] |= SIG_PUBLIC_ALIAS;
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
    if (s & SIG_PUBLIC_ALIAS)  add("public_alias");
    if (s & SIG_CROSS_TU_PUBLIC) add("cross_tu_public");
    return r.empty() ? "-" : r;
}

static bool isStrongThreadSignal(unsigned s) {
    return (s & (SIG_SECTION_INIT | SIG_SYSCALL |
                 SIG_OPS_MEMBER | SIG_INDIRECT_FORK)) != 0;
}

static bool isPureExportSignal(unsigned s) {
    return s == SIG_EXPORT_SYMBOL;
}

static bool isKnownCallbackTableTypeName(llvm::StringRef sname) {
    static const char* const kPatterns[] = {
        "struct.file_operations",
        "struct.proc_ops",
        "struct.seq_operations",
        "struct.kernfs_ops",
        "struct.attribute_group",
        "struct.device_attribute",
        "struct.driver_attribute",
        "struct.bus_attribute",
        "struct.class_attribute",
        "struct.kobj_attribute",
        "struct.bin_attribute",
        "struct.net_device_ops",
        "struct.ethtool_ops",
        "struct.net_proto_family",
        "struct.proto_ops",
        // Socket-layer protocol dispatch table (`struct proto`, e.g.
        // tcp_prot / sctp_prot / raw_prot). Holds the per-protocol
        // sendmsg/recvmsg/bind/connect/setsockopt/getsockopt/ioctl/
        // backlog_rcv callbacks, each invoked concurrently from
        // independent userspace sockets (and softirq for backlog_rcv) —
        // i.e. genuine parallel MHP roots. It is `global` (not `const`,
        // it carries mutable memory counters) and is NOT `_ops`-suffixed,
        // so neither the const heuristic nor the naming convention catch
        // it; it must be listed explicitly.
        "struct.proto",
        "struct.nfnl_callback",
        "struct.tty_operations",
        "struct.notifier_block",
        "struct.mmu_notifier_ops",
        "struct.pernet_operations",
        "struct.dev_pm_ops",
        "struct.platform_driver",
        "struct.bus_type",
        "struct.device_driver",
        "struct.nft_object_type",
        "struct.nft_expr_ops",
        "struct.nft_chain_type",
        "struct.iio_info",
        "struct.regmap_bus",
        // Async-execution callback containers. The function pointer they hold
        // (`.func` / `.function`) is launched by the kernel in a separate
        // context (kworker, softirq, timer/hrtimer IRQ) — i.e. it is a thread
        // LAUNCH mechanism (a "fork"), exactly the kind of independent
        // concurrent root the MHP model needs. These callbacks are stored in a
        // constant global initializer (DECLARE_WORK / DEFINE_TIMER / …) which
        // the address-taken pass intentionally skips, so they must be picked up
        // here or they are missed entirely.
        //
        // VM-area operations: `.fault`/`.page_mkwrite` run in the page-fault
        // context concurrently with `.close`/`.open` on munmap/fork — a genuine
        // MHP pair (e.g. perf_mmap_close vs the mmap fault path). Bounded
        // (~10 fn-ptrs per table).
        "struct.vm_operations_struct",
        // sysctl dispatch: ctl_table[].proc_handler is invoked on every
        // /proc/sys read/write from independent userspace tasks — concurrent
        // by construction. The table is an ARRAY of ctl_table (peeled above);
        // only the small set of generic .proc_handler functions are tagged
        // (.data points to variables, not functions, so it is never tagged).
        "struct.ctl_table",
        "struct.work_struct",
        "struct.delayed_work",
        "struct.rcu_work",
        "struct.tasklet_struct",
        "struct.tasklet_hrtimer",
        "struct.timer_list",
        "struct.hrtimer",
        "struct.kthread_work",
        "struct.kthread_delayed_work",
        "struct.irq_work",
    };
    for (const char* p : kPatterns) {
        llvm::StringRef pref(p);
        if (sname == pref) return true;
        if (sname.size() > pref.size() + 1 &&
            sname[pref.size()] == '.' &&
            sname.startswith(pref)) {
            return true;
        }
    }

    // General kernel ops-table naming convention: `struct.<subsys>_ops` /
    // `struct.<subsys>_operations`. These are the standard callback-container
    // types (file_operations, hwmon_ops, watchdog_ops, …). Matching on the
    // TYPE name (never on function names) keeps entry discovery structural and
    // avoids the explosion that "any struct holding a function pointer" would
    // cause: only function pointers that target *defined* functions inside
    // such a table are tagged, and the downstream tiering re-filters them.
    if (sname.startswith("struct.")) {
        // Strip LLVM's ".<n>" type-dedup suffix(es): struct.hwmon_ops.7 ->
        // struct.hwmon_ops.
        llvm::StringRef base = sname;
        for (;;) {
            size_t dot = base.rfind('.');
            if (dot == llvm::StringRef::npos || dot == 0) break;
            llvm::StringRef tail = base.substr(dot + 1);
            bool allDigits = !tail.empty();
            for (char c : tail) {
                if (c < '0' || c > '9') { allDigits = false; break; }
            }
            if (!allDigits) break;
            base = base.substr(0, dot);
        }
        if (base.endswith("_ops") || base.endswith("_operations")) {
            return true;
        }
        // Linux device-model driver containers: `struct.<bus>_driver` holds the
        // .probe/.remove/.shutdown callbacks (plus an embedded device_driver /
        // pm_ops). platform_driver / device_driver are already whitelisted
        // explicitly; generalize to the `_driver` convention so bus-specific
        // variants (serdev_device_driver, i2c_driver, spi_driver, …) are also
        // recognized — e.g. cros_ec_uart_probe in struct.serdev_device_driver,
        // which races its own serdev rx callback during port bring-up. probe is
        // init-phase, so the lifecycle filter still prevents spurious
        // probe-vs-remove pairs; only genuine probe-vs-callback MHP survives.
        if (base.endswith("_driver")) {
            return true;
        }
    }
    return false;
}

static const llvm::Type* peelArrayType(const llvm::Type* t) {
    while (t) {
        if (const auto* arr = llvm::dyn_cast<llvm::ArrayType>(t)) {
            t = arr->getElementType();
            continue;
        }
        break;
    }
    return t;
}

// Does this constant aggregate contain at least one pointer to a *defined*
// function? Used to confirm a name-based ops-table guess actually holds
// callbacks before we trust it.
static bool constantHoldsDefinedFunction(const llvm::Constant* C) {
    std::vector<const llvm::Function*> funcs;
    std::set<const llvm::Constant*> visited;
    collectFunctionPointersFromConstant(C, funcs, visited);
    return !funcs.empty();
}

static bool globalLooksLikeCallbackTable(const llvm::GlobalVariable& GV) {
    const llvm::Type* t = peelArrayType(GV.getValueType());
    const auto* st = llvm::dyn_cast_or_null<llvm::StructType>(t);
    if (st && st->hasName() && isKnownCallbackTableTypeName(st->getName()))
        return true;

    // Fallback for TYPE-MERGED bitcode. LLVM's module linker can dedup an
    // ops struct onto an unrelated identical-layout type — e.g. in a merged
    // .ll `struct mmu_notifier_ops` (the @mlx5_mn_ops table holding
    // mlx5_ib_invalidate_range) becomes `%struct.possible_net_t`, so the
    // type-name whitelist above silently fails. The kernel's `_ops` /
    // `_operations` GLOBAL naming convention is preserved through linking,
    // so use it as a backstop — but only for a `constant` struct global that
    // actually holds a defined function pointer, which is exactly the shape
    // of a real callback table and avoids matching mutable data globals.
    if (st && GV.isConstant() && GV.hasName() && GV.hasInitializer()) {
        llvm::StringRef gn = GV.getName();
        // Strip LLVM's ".<n>" dedup suffix from the global name.
        while (true) {
            size_t dot = gn.rfind('.');
            if (dot == llvm::StringRef::npos || dot == 0) break;
            llvm::StringRef tail = gn.substr(dot + 1);
            bool digits = !tail.empty();
            for (char c : tail) if (c < '0' || c > '9') { digits = false; break; }
            if (!digits) break;
            gn = gn.substr(0, dot);
        }
        if ((gn.endswith("_ops") || gn.endswith("_operations")) &&
            constantHoldsDefinedFunction(GV.getInitializer())) {
            return true;
        }
    }
    return false;
}

static std::unordered_map<const llvm::Function*,
                          std::vector<const llvm::Function*>>
buildDirectCallGraph(const llvm::Module* M) {
    std::unordered_map<const llvm::Function*,
                       std::vector<const llvm::Function*>> g;
    if (!M) return g;
    for (const llvm::Function& F : M->functions()) {
        if (F.isDeclaration()) continue;
        auto& outs = g[&F];
        for (const llvm::BasicBlock& BB : F) {
            for (const llvm::Instruction& I : BB) {
                const auto* CI = llvm::dyn_cast<llvm::CallInst>(&I);
                if (!CI) continue;
                const llvm::Function* callee = CI->getCalledFunction();
                if (!callee || callee->isDeclaration()) continue;
                outs.push_back(callee);
            }
        }
    }
    return g;
}

static bool reachesFunctionWithin(
        const llvm::Function* src,
        const llvm::Function* dst,
        const std::unordered_map<const llvm::Function*,
                                 std::vector<const llvm::Function*>>& cg,
        int maxDepth) {
    if (!src || !dst || src == dst || maxDepth <= 0) return false;
    std::queue<std::pair<const llvm::Function*, int>> q;
    std::set<const llvm::Function*> seen;
    q.push({src, 0});
    seen.insert(src);
    while (!q.empty()) {
        auto [cur, depth] = q.front();
        q.pop();
        if (depth >= maxDepth) continue;
        auto it = cg.find(cur);
        if (it == cg.end()) continue;
        for (const llvm::Function* next : it->second) {
            if (!next) continue;
            if (next == dst) return true;
            if (seen.insert(next).second) {
                q.push({next, depth + 1});
            }
        }
    }
    return false;
}

struct TierFootprint {
    std::set<query::SharedFieldKey> reads;
    std::set<query::SharedFieldKey> writes;
};

static void collectTierFootprint(
        const llvm::Function* F,
        const llvm::Module& M,
        const std::unordered_map<const llvm::Function*,
                                 std::vector<const llvm::Function*>>& cg,
        int depth,
        std::set<const llvm::Function*>& seen,
        TierFootprint& out) {
    if (!F || F->isDeclaration() || depth < 0 || !seen.insert(F).second)
        return;
    for (const llvm::BasicBlock& BB : *F) {
        for (const llvm::Instruction& I : BB) {
            if (const auto* SI = llvm::dyn_cast<llvm::StoreInst>(&I)) {
                if (auto key = query::SharedFieldKey::fromValue(
                        SI->getPointerOperand(), M)) {
                    out.writes.insert(*key);
                }
            } else if (const auto* LI = llvm::dyn_cast<llvm::LoadInst>(&I)) {
                if (auto key = query::SharedFieldKey::fromValue(
                        LI->getPointerOperand(), M)) {
                    out.reads.insert(*key);
                }
            }
        }
    }
    auto it = cg.find(F);
    if (it == cg.end()) return;
    for (const llvm::Function* callee : it->second) {
        collectTierFootprint(callee, M, cg, depth - 1, seen, out);
    }
}

static bool footprintsConflict(const TierFootprint& a,
                               const TierFootprint& b) {
    for (const auto& k : a.writes) {
        if (b.writes.count(k) || b.reads.count(k)) return true;
    }
    for (const auto& k : b.writes) {
        if (a.reads.count(k)) return true;
    }
    return false;
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
    if (!DB || !DB->getModule()) {
        std::cerr << "Error: failed to load LLVM module from: "
                  << bitcodeFilePath << std::endl;
        return;
    }

    // Try to find entry points: user-specified > 'main' > auto-detected kernel entries
    std::vector<std::string> entryPoints;
    
    // Priority 1: User-specified entry points (from config file)
    if (!userEntryPoints.empty()) {
        std::cout << "Using " << userEntryPoints.size() << " user-specified entry point(s):" << std::endl;
        for (const auto& ep : userEntryPoints) {
            // Verify the function exists in the bitcode
            if (DB && DB->getFunctionDefinition(ep)) {
                entryPoints.push_back(ep);
                entrySignalMasks_[ep] = SIG_SYSCALL | SIG_OPS_MEMBER;
                std::cout << "  - " << ep << " (found)" << std::endl;
            } else {
                std::cerr << "  - " << ep << " (NOT FOUND in bitcode, skipping)" << std::endl;
            }
        }
    }
    // Priority 2: Check for 'main'
    else if (DB && DB->getFunctionDefinition("main")) {
        entryPoints.push_back("main");
        entrySignalMasks_["main"] = SIG_SECTION_INIT;
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

            std::size_t nCrossTuPublicAdded = 0;
            if (M) {
                auto cg = buildDirectCallGraph(M);
                std::vector<const llvm::Function*> strongFuncs;
                for (const auto& [F, sig] : sigMap) {
                    if (F && !F->isDeclaration() && isStrongThreadSignal(sig)) {
                        strongFuncs.push_back(F);
                    }
                }

                std::unordered_map<const llvm::Function*, TierFootprint> strongFootprints;
                for (const llvm::Function* strong : strongFuncs) {
                    std::set<const llvm::Function*> seen;
                    TierFootprint fp;
                    collectTierFootprint(strong, *M, cg, /*depth=*/4, seen, fp);
                    strongFootprints.emplace(strong, std::move(fp));
                }

                auto isCompilerHelper = [](llvm::StringRef name) {
                    return name.startswith("llvm.") ||
                           name.startswith("__asan") ||
                           name.startswith("__kasan") ||
                           name.startswith("__ubsan") ||
                           name.startswith("__tsan") ||
                           name.startswith("__msan");
                };
                auto isExternallyLinkable = [](const llvm::Function* F) {
                    auto L = F->getLinkage();
                    return (L == llvm::GlobalValue::ExternalLinkage) ||
                           (L == llvm::GlobalValue::WeakAnyLinkage) ||
                           (L == llvm::GlobalValue::WeakODRLinkage) ||
                           (L == llvm::GlobalValue::ExternalWeakLinkage) ||
                           (L == llvm::GlobalValue::CommonLinkage) ||
                           (L == llvm::GlobalValue::AppendingLinkage);
                };
                auto sourceFileOf = [](const llvm::Function* F) {
                    if (!F) return std::string();
                    if (auto* SP = F->getSubprogram()) {
                        return SP->getFilename().str();
                    }
                    return std::string();
                };
                auto hasInModuleCaller = [](const llvm::Function* F) {
                    for (const llvm::User* U : F->users()) {
                        if (const auto* CB = llvm::dyn_cast<llvm::CallBase>(U)) {
                            if (CB->getCalledOperand() == F) return true;
                        }
                    }
                    return false;
                };

                for (const llvm::Function& F : M->functions()) {
                    if (F.isDeclaration()) continue;
                    if (sigMap[&F] != SIG_NONE) continue;
                    if (!isExternallyLinkable(&F)) continue;
                    if (!F.hasName() || isCompilerHelper(F.getName())) continue;
                    if (isDefinedInHeader(F)) continue;

                    bool reachableFromStrong = false;
                    for (const llvm::Function* strong : strongFuncs) {
                        if (reachesFunctionWithin(strong, &F, cg, /*maxDepth=*/4)) {
                            reachableFromStrong = true;
                            break;
                        }
                    }
                    if (reachableFromStrong) continue;

                    // An orphan (no caller inside the compiled slice) is
                    // necessarily invoked from an un-compiled TU, so it is a
                    // genuine cross-TU concurrent entry regardless of which
                    // file it is *defined* in. Only when the candidate has a
                    // local caller is file-locality a meaningful proxy for
                    // "sequential helper of the strong entry" — see below.
                    const bool candidateIsOrphan = !hasInModuleCaller(&F);

                    std::set<const llvm::Function*> seen;
                    TierFootprint fp;
                    collectTierFootprint(&F, *M, cg, /*depth=*/4, seen, fp);
                    const std::string candidateFile = sourceFileOf(&F);
                    bool conflicts = false;
                    for (const auto& [strong, strongFp] : strongFootprints) {
                        const std::string strongFile = sourceFileOf(strong);
                        // Same-file + locally-used ⇒ very likely a sequential
                        // helper sharing the entry's file-local state, not an
                        // independent concurrent context. Orphans bypass this
                        // (e.g. hci_cmd_sync_clear, defined in hci_sync.c next
                        // to the hci_cmd_sync_work consumer but only ever
                        // called from the un-compiled hci_unregister_dev path).
                        if (!candidateIsOrphan &&
                            !candidateFile.empty() && !strongFile.empty() &&
                            candidateFile == strongFile) {
                            continue;
                        }
                        if (footprintsConflict(fp, strongFp)) {
                            conflicts = true;
                            break;
                        }
                    }
                    if (!conflicts) continue;

                    sigMap[&F] |= SIG_CROSS_TU_PUBLIC;
                    ++nCrossTuPublicAdded;
                }
            }

            // Counters for reporting.
            int nSection = 0, nExport = 0, nSyscall = 0, nOps = 0, nIndirect = 0, nAlias = 0, nCrossTuPublic = 0;

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
                    entrySignalMasks_[name] = sig;
                    if (sig & SIG_SECTION_INIT)  nSection++;
                    if (sig & SIG_EXPORT_SYMBOL) nExport++;
                    if (sig & SIG_SYSCALL)       nSyscall++;
                    if (sig & SIG_OPS_MEMBER)    nOps++;
                    if (sig & SIG_INDIRECT_FORK) nIndirect++;
                    if (sig & SIG_PUBLIC_ALIAS)  nAlias++;
                    if (sig & SIG_CROSS_TU_PUBLIC) nCrossTuPublic++;
                }
            }

            std::cout << "[Auto-Entry Structural] "
                      << nSection << " init/exit section, "
                      << nExport  << " EXPORT_SYMBOL, "
                      << nSyscall << " syscall wrapper, "
                      << nOps     << " ops-table member, "
                      << nIndirect << " indirect-fork sink, "
                      << nAlias    << " public-wrapper alias, "
                      << nCrossTuPublic << " cross-TU public"
                      << " (" << entrySet.size() << " unique function(s))"
                      << std::endl;
            if (nCrossTuPublicAdded > 0) {
                std::cout << "[Auto-Entry Cross-TU] +"
                          << nCrossTuPublicAdded
                          << " external function(s) promoted by strong-entry "
                             "field conflict."
                          << std::endl;
            }

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
                    entrySignalMasks_[funcName] =
                        getKernelEntryPriority(funcName) == 1
                            ? SIG_SYSCALL
                            : SIG_SECTION_INIT;
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
                        entrySignalMasks_[funcName] = SIG_NONE;
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

    discoveredEntryPoints_ = entryPoints;

    // Entry tiering: all discovered entries remain ICFG seeds, but only
    // thread-root entries are later promoted to parallel ThreadCreationTree
    // roots. A pure EXPORT_SYMBOL is a public API surface, not necessarily an
    // independent concurrent root. If another discovered entry reaches it
    // inside this module, keep it as a reachable helper rather than a separate
    // LLM-facing thread.
    {
        const llvm::Module* M = DB ? DB->getModule() : nullptr;
        auto cg = buildDirectCallGraph(M);
        std::set<std::string> allEntryNames(entryPoints.begin(), entryPoints.end());
        std::unordered_map<std::string, TierFootprint> footprints;
        if (M) {
            for (const auto& name : entryPoints) {
                const llvm::Function* F = M->getFunction(name);
                if (!F || F->isDeclaration()) continue;
                std::set<const llvm::Function*> seen;
                TierFootprint fp;
                collectTierFootprint(F, *M, cg, /*depth=*/4, seen, fp);
                footprints.emplace(name, std::move(fp));
            }
        }
        std::set<std::string> kept;
        std::vector<std::string> strongNames;
        size_t strongKept = 0;
        size_t weakKept = 0;
        size_t weakPruned = 0;
        size_t weakPrunedReachable = 0;
        size_t weakPrunedNoStrongConflict = 0;

        bool hasStrong = false;
        for (const auto& name : entryPoints) {
            if (isStrongThreadSignal(entrySignalMasks_[name])) {
                hasStrong = true;
                strongNames.push_back(name);
            }
        }

        auto conflictsWithSet = [&](const std::string& name,
                                    const std::vector<std::string>& cand) {
            auto fpIt = footprints.find(name);
            if (fpIt == footprints.end()) return false;
            for (const auto& o : cand) {
                if (o == name) continue;
                auto oIt = footprints.find(o);
                if (oIt == footprints.end()) continue;
                if (footprintsConflict(fpIt->second, oIt->second)) return true;
            }
            return false;
        };
        auto reachableFromOtherEntry = [&](const std::string& name) {
            const llvm::Function* target = M ? M->getFunction(name) : nullptr;
            if (!target) return false;
            for (const auto& o : entryPoints) {
                if (o == name) continue;
                const llvm::Function* other = M ? M->getFunction(o) : nullptr;
                if (reachesFunctionWithin(other, target, cg, /*maxDepth=*/4))
                    return true;
            }
            return false;
        };

        // EXPERIMENT (gated): a STRONG entry whose only concurrency signal is
        // ops-table membership or init/exit-section placement is not necessarily an
        // independent concurrent root -- many such functions (ops callbacks reached
        // synchronously, exported helpers) are just callees of a real entry. We
        // currently keep ALL strong entries unconditionally; on whole-kernel IR that
        // over-counts threads massively. This measures, and (when the env is set)
        // prunes, strong entries that (a) carry ONLY ops/init strong signals (NOT a
        // syscall entry and NOT an async fork signal, which are genuine roots) and
        // (b) are reachable from another entry in the direct call graph.
        static const bool kExpReachStrong = [](){
            const char* e = std::getenv("LACE_EXP_REACH_PRUNE_STRONG");
            return e && e[0] && e[0] != '0';
        }();
        auto opsOrInitOnlyStrong = [&](unsigned sig) {
            if (!isStrongThreadSignal(sig)) return false;
            if (sig & (SIG_SYSCALL | SIG_INDIRECT_FORK)) return false;  // genuine roots
            return (sig & (SIG_OPS_MEMBER | SIG_SECTION_INIT)) != 0;
        };
        {
            size_t diagStrongReach = 0, diagPrunable = 0;
            for (const auto& name : entryPoints) {
                unsigned sig = entrySignalMasks_[name];
                if (!isStrongThreadSignal(sig)) continue;
                if (!reachableFromOtherEntry(name)) continue;
                ++diagStrongReach;
                if (opsOrInitOnlyStrong(sig)) ++diagPrunable;
            }
            std::cout << "[Auto-Entry Diag] strong+reachable=" << diagStrongReach
                      << " (ops/init-only prunable=" << diagPrunable << ")"
                      << (kExpReachStrong ? "  [EXP reach-prune ON]" : "")
                      << std::endl;
        }

        // Two-tier promotion of pure-export ("weak") entries.
        //
        //  Conservative tier (always on): a pure-export handler is kept as a
        //  thread root when it field-conflicts with a STRONG structural entry
        //  — the common "public API races a registered concurrent context"
        //  pattern — and is not just a helper reachable from another entry.
        //
        //  Broad tier (only when the module is concurrency-thin): additionally
        //  keep a pure-export handler that field-conflicts with ANY other
        //  entry. This recovers public-vs-public races where neither side has
        //  a strong structural signal (two exported handlers, two sysctl
        //  proc_handlers, two ops callbacks, …) — the dominant cause of
        //  zero-thread / missing-root collapses on self-contained modules.
        //
        //  We gate the broad tier on a thin conservative root set so we never
        //  inflate already entry-rich modules: promoting every data-touching
        //  public API in a big networking module explodes the O(threads^2)
        //  pairwise surface (observed: CVE-2024-26862 145→168 roots → timeout).
        //  A hard cap bounds the worst case of a thin-but-wide module. These
        //  are explosion-control valves, not recall criteria: recall is driven
        //  by the field-conflict relation, which mirrors may-happen-in-parallel.
        const size_t kThinRootBudget = 16;
        const size_t kBroadAddCap    = 48;

        struct WeakClass { bool reachable; bool cStrong; };
        std::unordered_map<std::string, WeakClass> weakClass;
        for (const auto& name : entryPoints) {
            unsigned sig = entrySignalMasks_[name];
            if (!(hasStrong && isPureExportSignal(sig))) continue;
            WeakClass wc;
            wc.reachable = reachableFromOtherEntry(name);
            wc.cStrong = wc.reachable ? false
                                      : conflictsWithSet(name, strongNames);
            weakClass[name] = wc;
        }

        // Conservative root count = strong entries + non-pure-export weak
        // entries (alias / cross-TU, always kept) + pure-export entries kept
        // by the conservative tier.
        size_t conservativeCount = 0;
        for (const auto& name : entryPoints) {
            unsigned sig = entrySignalMasks_[name];
            if (isStrongThreadSignal(sig)) { ++conservativeCount; continue; }
            if (!(hasStrong && isPureExportSignal(sig))) {
                ++conservativeCount;  // alias/cross-tu, or the no-strong module
                continue;
            }
            const WeakClass& wc = weakClass[name];
            if (!wc.reachable && wc.cStrong) ++conservativeCount;
        }
        bool broadMode = (conservativeCount < kThinRootBudget);
        size_t broadAdded = 0;

        for (const auto& name : entryPoints) {
            unsigned sig = entrySignalMasks_[name];
            bool strong = isStrongThreadSignal(sig);
            bool keep = true;
            if (hasStrong && isPureExportSignal(sig)) {
                const WeakClass& wc = weakClass[name];
                if (wc.reachable) {
                    keep = false; ++weakPrunedReachable;
                } else if (wc.cStrong) {
                    keep = true;  // conservative tier
                } else if (broadMode && broadAdded < kBroadAddCap &&
                           conflictsWithSet(name, entryPoints)) {
                    keep = true; ++broadAdded;  // broad tier (thin module)
                } else {
                    keep = false; ++weakPrunedNoStrongConflict;
                }
            } else if (kExpReachStrong && opsOrInitOnlyStrong(sig) &&
                       reachableFromOtherEntry(name)) {
                keep = false; ++weakPrunedReachable;  // experimental strong reach-prune
            }

            if (!keep) {
                ++weakPruned;
                continue;
            }
            if (kept.insert(name).second) {
                threadRootEntryPoints_.push_back(name);
                if (strong) ++strongKept;
                else ++weakKept;
            }
        }

        if (threadRootEntryPoints_.empty()) {
            threadRootEntryPoints_ = entryPoints;
            strongKept = 0;
            weakKept = threadRootEntryPoints_.size();
            weakPruned = 0;
        }

        std::cout << "[Auto-Entry Tiering] thread roots: "
                  << threadRootEntryPoints_.size() << "/" << entryPoints.size()
                  << " (strong=" << strongKept
                  << ", weak_kept=" << weakKept
                  << ", broad_mode=" << (broadMode ? "on" : "off")
                  << ", broad_added=" << broadAdded
                  << ", weak_pruned_reachable=" << weakPrunedReachable
                  << ", weak_pruned_no_strong_conflict="
                  << weakPrunedNoStrongConflict
                  << ", weak_pruned_total=" << weakPruned
                  << ")" << std::endl;
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
            auto sigIt = entrySignalMasks_.find(entryName);
            info.signalMask = sigIt == entrySignalMasks_.end() ? 0 : sigIt->second;
            info.signalSummary = signalsToString(info.signalMask);
            info.threadRoot =
                std::find(threadRootEntryPoints_.begin(),
                          threadRootEntryPoints_.end(),
                          entryName) != threadRootEntryPoints_.end();
            
            allEntries.push_back(info);
        }
    }
    
    std::cout << "getAllEntryPointInfos: Returning " << allEntries.size() << " entry points for kernel module analysis" << std::endl;
    return allEntries;
}

std::vector<EntryPointInfo> PhasarPointerAnalysis::getThreadRootEntryPointInfos() const {
    std::vector<EntryPointInfo> roots;

    for (const std::string& entryName : threadRootEntryPoints_) {
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
            auto sigIt = entrySignalMasks_.find(entryName);
            info.signalMask = sigIt == entrySignalMasks_.end() ? 0 : sigIt->second;
            info.signalSummary = signalsToString(info.signalMask);
            info.threadRoot = true;
            roots.push_back(info);
        }
    }

    std::cout << "getThreadRootEntryPointInfos: Returning "
              << roots.size()
              << " thread-root entry points for kernel module analysis"
              << std::endl;
    return roots;
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