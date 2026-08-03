// src/LLMUtil/AgentManager.cpp

#include "LLMUtil/AgentManager.h"
#include <iostream>
#include <vector>
#include <map>
#include "CCPG/CCPGNode.h"
#include "CPG/Node.h"
#include "LLMUtil/ConcurrencyContract.h"
#include "CCPG/ThreadCreationTree.h"
#include "CCPG/ManualEntryConfig.h"
#include "LLMUtil/ThreadPair.h"
#include "LLMUtil/DetectorAgent.h"
#include "LLMUtil/InterleavingAnalysisAgent.h"
#include "LLMUtil/ObjectTriageAgent.h"
#include "LLMUtil/SharedToolKit.h"
#include "Query/VulnerabilitySurfaceGenerator.h"
#include "CCPG/HBGraph.h"
#include <set>
#include <string>
#include <algorithm>
#include <utility>
#include <unordered_map>
#include <sstream>
#include <cstdlib>
#include <climits>
#include <cctype>
#include <fstream>
#include <atomic>
#include <future>
#include <mutex>
#include <thread>
#include "nlohmann/json.hpp"

namespace llm_client {

// ===========================================================================
// Static-composition pipeline (gated by LACE_STATIC_COMPOSE).
//
// Realises the "per-thread contract -> deterministic interleaving -> agent
// calibration" design: Phase A derives each thread's requirement/guarantee
// contract once; Phase B mechanically applies the requirement-discharge rule over
// the surface's conflicting accesses (surface = recall floor, contract =
// precision/discharge);
// Phase C only calibrates the surviving candidates. Helpers below implement the
// deterministic Phase B composition. The folded per-cluster path is unchanged
// when the env is unset.
// ===========================================================================
namespace {

using OrderClause = LLM::ConcurrencyContract::OrderClause;

std::string relUpper(std::string s) {
    for (char& c : s) c = static_cast<char>(std::toupper((unsigned char)c));
    return s;
}

bool allowFuzzyContractBinding() {
    if (const char* e = std::getenv("LACE_ALLOW_FUZZY_CONTRACT_BINDING"))
        return e[0] && e[0] != '0';
    return false;
}

// Evaluation/observability switch: when LACE_EVAL_VERBOSE=1, Phase A dumps each
// per-thread contract's clauses (resource + assume/guarantee relations), and
// Phase B logs every deterministic discharge decision. Off by default so normal
// runs stay quiet; used for the deep per-stage evaluation.
bool evalVerbose() {
    if (const char* e = std::getenv("LACE_EVAL_VERBOSE")) return e[0] && e[0] != '0';
    return false;
}

bool trustHardNonLockDischarge() {
    if (const char* e = std::getenv("LACE_TRUST_HARD_NONLOCK_DISCHARGE"))
        return e[0] && e[0] != '0';
    return false;
}

// Ablation switches: each removes exactly one pipeline component.
bool skipPhaseCEnabled() {
    if (const char* e = std::getenv("LACE_SKIP_PHASE_C"))
        return e[0] && e[0] != '0';
    return false;
}

bool noDeterministicDischargeEnabled() {
    if (const char* e = std::getenv("LACE_NO_DETERMINISTIC_DISCHARGE"))
        return e[0] && e[0] != '0';
    return false;
}

// Paper-faithful node-anchored composition (L2). When set (together with
// LACE_STATIC_COMPOSE), Phase A emits node-anchored contracts and Phase B runs
// the requirement-driven L2 checker instead of the surface-driven composeVerdict.
bool contractL2Enabled() {
    if (const char* e = std::getenv("LACE_CONTRACT_L2"))
        return e[0] && e[0] != '0';
    return false;
}

// Which static edges the L2 checker accepts as cross-thread synchronization.
// By default only the structural ones (lockset, fork/join): the protocol-level
// edges the static model seeds from API names -- RCU grace periods, completion
// signal/wait, registration-to-callback -- are exactly the mechanisms the
// contract is supposed to supply as Order/Wait guarantees, so letting them
// discharge a requirement on their own would bypass the contract. Set
// LACE_L2_SYNC_ALL=1 to admit them again (ablation).
HBGraph::SyncPolicy l2SyncPolicy() {
    if (const char* e = std::getenv("LACE_L2_SYNC_ALL"))
        if (e[0] && e[0] != '0') return HBGraph::SyncPolicy::AllMechanisms;
    return HBGraph::SyncPolicy::StructuralOnly;
}

// thread tid's clause for surface object index oi. Two-level match:
//   1) oi listed in the clause's objectIds (lock-region group or single anchor);
//   2) the compat single objectId == oi;
// A legacy fuzzy resource-name fallback exists only behind an env flag. Phase B
// must prefer an anchored miss over binding a requirement to the wrong resource.
const OrderClause* clauseForObject(const LLM::ConcurrencyContract& c, int oi,
                                   const query::SharedObject& O) {
    for (const auto& cl : c.clauses) {
        for (int id : cl.objectIds) if (id == oi) return &cl;
        if (cl.objectId == oi) return &cl;
    }
    if (allowFuzzyContractBinding() && O.name.size() > 2) {
        for (const auto& cl : c.clauses) {
            if (cl.objectId >= 0 || !cl.objectIds.empty() || cl.resource.empty()) continue;
            if (cl.resource.find(O.name) != std::string::npos ||
                O.name.find(cl.resource) != std::string::npos)
                return &cl;
        }
    }
    return nullptr;
}

// Whether clause `cl`'s local requirements (`assume`) are anchored to surface
// object `oi` -- i.e. the requirement is *about this pointer/resource*.
//
// A clause may COVER several surface objects through `objectIds` (a lock-region
// merge: one EXCLUDE guarantee spans multiple fields protected by the same
// lock, and `clauseForObject` returns it for any member so the guarantee can
// discharge across the group). But `assume` names ONE concrete resource -- the
// clause's primary anchor (`objectId`). Raising a resource-specific requirement
// such as STABLE_DURING(live(A)) as a hazard on a *sibling* object B that merely
// shares A's lock would bind the requirement to the wrong pointer and invent a
// bug on B. So the guarantee side keeps spanning the merge group, while the
// assume side (hazard generation) is honoured only on the clause's own object.
//
// This is the "each event must bind the pointer it actually reasons about"
// invariant applied to the requirement side of composition.
bool assumeAnchoredToObject(const OrderClause* cl, int oi) {
    if (!cl) return false;
    if (cl->objectId == oi) return true;             // explicit primary anchor
    // Compat: a single-object clause that used only `objectIds` (no separate
    // primary) is unambiguously anchored to that one object.
    if (cl->objectId < 0 && cl->objectIds.size() == 1 && cl->objectIds.front() == oi)
        return true;
    return false;
}

bool hasAssumeRel(const OrderClause* cl, const std::string& rel) {
    if (!cl) return false;
    const std::string want = relUpper(rel);
    for (const auto& a : cl->assume) if (relUpper(a.relation) == want) return true;
    return false;
}
bool hasAnyAssume(const OrderClause* cl) { return cl && !cl->assume.empty(); }
bool establishesOrder(const OrderClause* cl) {
    if (!cl) return false;
    for (const auto& g : cl->guarantee) {
        std::string r = relUpper(g.relation);
        if (r == "ORDER" || r == "EXCLUDE" || r == "LINEARIZE" || r == "WAIT" ||
            r == "SERIALIZE" || r == "COUNTS")
            return true;
    }
    return false;
}

// Mechanism evidence class for a stated guarantee. HARD means the clause names a
// concrete synchronization mechanism (lock, RCU grace, barrier, join/quiesce,
// refcount RMW); SOFT means it relies on a weaker state/flag/protocol statement.
// HARD is still only evidence unless the checker can prove it covers the current
// requirement/hazard pair.
enum class MechClass { None, Soft, Hard };

MechClass mechanismClass(const LLM::ConcurrencyContract::SyncProv& g) {
    std::string d = g.relation + " " + g.detail;
    for (char& c : d) c = static_cast<char>(std::tolower((unsigned char)c));
    auto has = [&](const char* kw) { return d.find(kw) != std::string::npos; };

    const std::string rel = relUpper(g.relation);
    if (rel == "COUNTS") return MechClass::Hard;  // legacy refcount RMW discipline
    if (rel == "EXCLUDE" || rel == "SERIALIZE") {
        // A lock / interrupt-context guarantee is hard; a guarantee that leans on a
        // bare published flag/state bit is soft (needs the step-3 verifier).
        if (has("flag") || has("state bit") || has("bool"))
            return MechClass::Soft;
        return MechClass::Hard;
    }
    if (rel == "ORDER" || rel == "WAIT") {
        // Hard ordering primitives: each one is a concrete cited mechanism, so a
        // stated guarantee here is well-grounded enough to discharge.
        if (has("rcu") || has("synchronize") || has("call_rcu") || has("kfree_rcu") ||
            has("barrier") || has("smp_") || has("store_release") || has("load_acquire") ||
            has("acquire") || has("release") || has("flush_work") || has("flush_workqueue") ||
            has("cancel_work") || has("cancel_delayed_work") || has("kthread_stop") ||
            has("wait_for_completion") || has("del_timer_sync") || has("timer_delete_sync") ||
            has("napi_disable") || has("drain") || has("join") || has("quiesce") ||
            has("grace") || has("active_callbacks") || has("callbacks_empty") ||
            has("readers_empty") || has("active_holders") || has("holders==0") ||
            has("cmpxchg") || has("xchg") || has("refcount") || has("kref"))
            return MechClass::Hard;
        // Published flag / bare program order / unknown ordering: be conservative.
        return MechClass::Soft;
    }
    if (rel == "LINEARIZE") {
        // A bare linearization point is often only one atom of a larger macro. Trust it
        // for deterministic discharge only when it names a concrete atomic/refcount
        // object or a close/admission transition that the contract also grounds.
        if (has("refcount") || has("kref") || has("counts") || has("cmpxchg") ||
            has("xchg") || has("atomic") || has("admission") || has("close"))
            return MechClass::Hard;
        return MechClass::Soft;
    }
    return MechClass::None;
}

// A clause names a HARD *non-lock* order (RCU/refcount/barrier/join/RMW).
// These protections structurally cannot appear as a shared lock, so Phase C
// needs to see them. By default Phase B treats them as review evidence, not as
// an automatic discharge, because endpoint alignment still has to be checked
// against the concrete requirement/hazard pair.
bool establishesHardNonLockOrder(const OrderClause* cl) {
    if (!cl) return false;
    for (const auto& g : cl->guarantee) {
        std::string r = relUpper(g.relation);
        if (r == "COUNTS") return true;
        if ((r == "ORDER" || r == "WAIT" || r == "LINEARIZE") &&
            mechanismClass(g) == MechClass::Hard) return true;
    }
    return false;
}

struct AccKind { bool read = false, write = false, free = false; };
AccKind threadAccessKind(const query::SharedObject& O, int tid) {
    AccKind k;
    for (const auto& a : O.accesses) {
        if (a.thread_id != tid) continue;
        if (a.access_type == "Write")      k.write = true;
        else if (a.access_type == "Free")  k.free  = true;
        else                               k.read  = true;
    }
    return k;
}

bool keepLockedNullHazardsEnabled() {
    if (const char* e = std::getenv("LACE_KEEP_LOCKED_NULL_HAZARDS"))
        return e[0] && e[0] != '0';
    return false;
}

bool nullStateHazard(const query::SharedObject& O) {
    bool nullWrite = false;
    bool readLikeUse = false;
    for (const auto& a : O.accesses) {
        std::string code = a.code_snippet;
        std::string lower = code;
        for (char& c : lower) c = static_cast<char>(std::tolower((unsigned char)c));
        if (a.access_type == "Write" &&
            (lower.find("= null") != std::string::npos ||
             lower.find("= nullptr") != std::string::npos)) {
            nullWrite = true;
        }
        if (a.access_type == "Read" &&
            (code.find("->") != std::string::npos ||
             lower.find("deref") != std::string::npos)) {
            readLikeUse = true;
        }
    }
    return nullWrite && readLikeUse;
}

// Surface-side serialization: every access of BOTH threads to O is lock-protected
// and the two lock sets intersect. Conservative (any unlocked access -> false).
bool surfaceSharedLock(const query::SharedObject& O, int ta, int tb) {
    std::set<std::string> la, lb;
    bool aAny = false, bAny = false, aAll = true, bAll = true;
    for (const auto& a : O.accesses) {
        if (a.thread_id == ta) {
            aAny = true;
            if (a.is_lock_protected && !a.protecting_lock.empty()) la.insert(a.protecting_lock);
            else aAll = false;
        }
        if (a.thread_id == tb) {
            bAny = true;
            if (a.is_lock_protected && !a.protecting_lock.empty()) lb.insert(a.protecting_lock);
            else bAll = false;
        }
    }
    if (!aAny || !bAny || !aAll || !bAll) return false;
    for (const auto& l : la) if (lb.count(l)) return true;
    return false;
}

// Strongest hazard a requirer-clause realises against a violator's access kind.
// Returns {tier, label}; empty tier means "no assume hazard" (raw conflict).
std::pair<std::string, std::string> hazardTier(const OrderClause* req, const AccKind& viol) {
    if (!req) return {"", ""};
    const bool stable = hasAssumeRel(req, "STABLE_DURING") || hasAssumeRel(req, "COUNT_GUARDED");
    const bool isolated = hasAssumeRel(req, "REGION_ISOLATED") || hasAssumeRel(req, "ATOMIC");
    const bool ordered = hasAssumeRel(req, "ORDER") || hasAssumeRel(req, "PREC");
    const bool mediated = hasAssumeRel(req, "CONFLICT_MEDIATED");

    if (stable && viol.free)
        return {"high", "STABLE_DURING violated by retire/free -> lifetime"};
    if (isolated && (viol.write || viol.free))
        return {"high", "REGION_ISOLATED violated by conflicting event -> atomicity"};
    if (mediated && (viol.write || viol.free))
        return {"medium", "CONFLICT_MEDIATED lacks covering synchronization"};
    if (ordered && viol.free)
        return {"high", "ORDER violated by retire/free -> lifetime/order"};
    if (ordered && viol.write)
        return {"medium", "ORDER violated by concurrent write"};
    if (hasAnyAssume(req))
        return {"medium", "local requirement met by conflicting access"};
    return {"", ""};
}

// Collapse whitespace/newlines and truncate -- keeps the verdict compact so a
// calibration session can read the concrete content instead of re-reading source.
std::string oneLine(const std::string& in, size_t maxLen = 160) {
    std::string s;
    s.reserve(in.size());
    bool prevSpace = false;
    for (char c : in) {
        if (c == '\n' || c == '\r' || c == '\t' || c == ' ') {
            if (!prevSpace) { s += ' '; prevSpace = true; }
        } else { s += c; prevSpace = false; }
    }
    while (!s.empty() && s.back() == ' ') s.pop_back();
    if (s.size() > maxLen) s = s.substr(0, maxLen) + " ...";
    return s;
}

// The thread's access that best matches the violating kind (Free > Write > Read),
// for citing the concrete site in the verdict (location + code).
const query::ThreadAccess* representativeAccess(const query::SharedObject& O, int tid,
                                                const AccKind& vk) {
    const query::ThreadAccess* best = nullptr;
    for (const auto& a : O.accesses) {
        if (a.thread_id != tid) continue;
        bool match = (vk.free && a.access_type == "Free") ||
                     (vk.write && a.access_type == "Write") ||
                     (!vk.free && !vk.write && a.access_type == "Read");
        if (match) return &a;
        if (!best) best = &a;
    }
    return best;
}

// The violator access to CITE in a candidate. The naive choice (most-severe
// access type) is misleading when the severe access is itself serialized: e.g.
// a publish `obj = x` done UNDER a lock vs a concurrent UNPROTECTED reader of
// the same pointer. Citing the locked write makes the candidate read as "two
// serialized writes" and the reviewer (correctly) rejects it, even though the
// real race front is the unprotected access that escapes all serialization
// (the classic unlocked guard-read / torn-scalar read). So prefer the violator
// thread's UNPROTECTED conflicting access (mutation first, then read); only if
// every violator access is lock-protected do we fall back to the severe site.
const query::ThreadAccess* escapingViolatorAccess(const query::SharedObject& O, int tid,
                                                  const AccKind& vk) {
    const query::ThreadAccess* unprotMut = nullptr;
    const query::ThreadAccess* unprotRead = nullptr;
    for (const auto& a : O.accesses) {
        if (a.thread_id != tid) continue;
        const bool unprotected = !a.is_lock_protected || a.protecting_lock.empty();
        if (!unprotected) continue;
        if (a.access_type == "Write" || a.access_type == "Free") {
            if (!unprotMut) unprotMut = &a;
        } else if (!unprotRead) {
            unprotRead = &a;
        }
    }
    if (unprotMut) return unprotMut;
    if (unprotRead) return unprotRead;
    return representativeAccess(O, tid, vk);
}

// Render an access as "Type in fn @ loc | code: <snippet>".
std::string fmtAccess(const query::ThreadAccess* a) {
    if (!a) return "(site not located)";
    const std::string& fn = a->containing_function.empty() ? a->function_name
                                                           : a->containing_function;
    std::string out = a->access_type + " in " + (fn.empty() ? "?" : fn) + " @ " + a->location;
    if (!a->code_snippet.empty()) out += " | code: " + oneLine(a->code_snippet, 140);
    return out;
}

// Append the requirer clause's assume relations (the inferred order intent +
// provenance) so the calibration session sees WHAT order is required, not just
// that "a thread requires something".
void appendAssume(std::stringstream& vs, const OrderClause* req) {
    if (!req || req->assume.empty()) return;
    for (const auto& a : req->assume) {
        vs << "        requires: " << a.relation;
        if (!a.detail.empty()) vs << " " << a.detail;
        if (!a.provenance.empty()) vs << "  [prov: " << oneLine(a.provenance, 120) << "]";
        vs << "\n";
    }
}

const char* mechClassName(MechClass c) {
    switch (c) {
        case MechClass::Hard: return "hard";
        case MechClass::Soft: return "soft";
        default: return "none";
    }
}

// Append the guarantee(s) the requirer/violator DID state (so the session can
// see what B weighed and why it judged the order uncovered).
void appendGuarantee(std::stringstream& vs, const OrderClause* a, const OrderClause* b) {
    bool any = false;
    for (const OrderClause* c : {a, b}) {
        if (!c) continue;
        for (const auto& g : c->guarantee) {
            vs << "        stated guarantee: " << g.relation;
            if (!g.detail.empty()) vs << " " << g.detail;
            vs << " [mechanism=" << mechClassName(mechanismClass(g))
               << "] (B did not deterministically discharge this candidate)\n";
            any = true;
        }
    }
    if (!any) vs << "        no guarantee stated by either thread for this resource.\n";
}

std::string phaseBHazardClass(const std::string& label) {
    std::string s = label;
    for (char& c : s) c = static_cast<char>(std::tolower((unsigned char)c));
    if (s.find("use_after_free") != std::string::npos ||
        s.find("free") != std::string::npos ||
        s.find("lifetime") != std::string::npos)
        return "lifetime";
    if (s.find("refcount") != std::string::npos ||
        s.find("count_guarded") != std::string::npos ||
        s.find("double") != std::string::npos)
        return "refcount";
    if (s.find("prec") != std::string::npos ||
        s.find("order") != std::string::npos)
        return "ordering";
    if (s.find("atomic") != std::string::npos ||
        s.find("data_race") != std::string::npos ||
        s.find("raw conflict") != std::string::npos)
        return "atomicity";
    return "other";
}

const query::ThreadAccess* mutatingAnchor(const query::SharedObject& O,
                                          int reqT, const AccKind& reqK,
                                          int violT, const AccKind& violK) {
    auto mutationFor = [&](int tid, const AccKind& k) -> const query::ThreadAccess* {
        AccKind want;
        want.free = k.free;
        want.write = !k.free && k.write;
        if (!want.free && !want.write) return nullptr;
        return representativeAccess(O, tid, want);
    };
    if (const auto* a = mutationFor(violT, violK)) return a;
    if (const auto* a = mutationFor(reqT, reqK)) return a;
    return representativeAccess(O, violT, violK);
}

std::string anchorKey(const query::ThreadAccess* a) {
    if (!a) return "none";
    if (a->node_id >= 0) return "n" + std::to_string(a->node_id);
    const std::string& fn = a->containing_function.empty() ? a->function_name
                                                           : a->containing_function;
    return fn + "@" + a->location + "|" + a->access_type;
}

int tierRank(const std::string& t) {
    if (t == "high") return 2;
    if (t == "medium") return 1;
    return 0;
}

int envInt(const char* name, int fallback) {
    if (const char* e = std::getenv(name)) {
        if (e[0]) {
            char* end = nullptr;
            long v = std::strtol(e, &end, 10);
            if (end && *end == '\0' && v >= 0 && v <= 1000) return static_cast<int>(v);
        }
    }
    return fallback;
}

int contractParallelism() {
    return std::max(1, envInt("LACE_CONTRACT_PARALLELISM", 4));
}

struct FlowPrior {
    bool enabled = false;
    std::set<std::string> functions;
    std::set<std::string> files;
};

std::string basenameOf(std::string p) {
    size_t pos = p.find_last_of("/\\");
    return pos == std::string::npos ? p : p.substr(pos + 1);
}

void addFlowFunction(FlowPrior& fp, const nlohmann::json& v) {
    if (v.is_string() && !v.get<std::string>().empty())
        fp.functions.insert(v.get<std::string>());
}

void addFlowFile(FlowPrior& fp, const nlohmann::json& v) {
    if (!v.is_string()) return;
    std::string s = v.get<std::string>();
    if (s.empty()) return;
    fp.files.insert(s);
    fp.files.insert(basenameOf(s));
}

FlowPrior loadFlowPrior() {
    FlowPrior fp;
    // Oracle annotations must not affect normal detection. Keep this strictly
    // opt-in for ablation/debug runs.
    bool enabled = false;
    if (const char* e = std::getenv("LACE_ENABLE_FLOW_PRIOR"))
        enabled = e[0] && e[0] != '0';
    if (!enabled) return fp;

    std::string path = "flow_annotation.json";
    if (const char* p = std::getenv("LACE_FLOW_PRIOR_PATH"))
        if (p[0]) path = p;
    std::ifstream in(path);
    if (!in.is_open()) return fp;
    nlohmann::json j = nlohmann::json::parse(in, nullptr, false);
    if (j.is_discarded() || !j.is_object()) return fp;

    auto addThread = [&](const nlohmann::json& th) {
        if (!th.is_object()) return;
        if (th.contains("entry") && th["entry"].is_object()) {
            addFlowFunction(fp, th["entry"].value("function", ""));
            addFlowFile(fp, th["entry"].value("file", ""));
        }
        if (th.contains("bug_site") && th["bug_site"].is_object()) {
            addFlowFunction(fp, th["bug_site"].value("function", ""));
            addFlowFile(fp, th["bug_site"].value("file", ""));
        }
        if (th.contains("call_chain") && th["call_chain"].is_array()) {
            for (const auto& c : th["call_chain"]) {
                if (!c.is_object()) continue;
                addFlowFunction(fp, c.value("function", ""));
                addFlowFile(fp, c.value("file", ""));
            }
        }
    };
    if (j.contains("true_interleaving") && j["true_interleaving"].is_object()) {
        for (auto& [k, v] : j["true_interleaving"].items())
            if (k.rfind("thread_", 0) == 0) addThread(v);
    }
    if (j.contains("coverage") && j["coverage"].is_object()) {
        const auto& cov = j["coverage"];
        for (const char* key : {"analysis_required_files", "compiled_files", "affected_files_from_patch"}) {
            if (cov.contains(key) && cov[key].is_array())
                for (const auto& f : cov[key]) addFlowFile(fp, f);
        }
    }
    if (!fp.functions.empty() || !fp.files.empty()) fp.enabled = true;
    return fp;
}

int flowPriorScore(const query::ThreadAccess& a, const FlowPrior& fp) {
    if (!fp.enabled) return 0;
    int s = 0;
    const std::string& fn = a.containing_function.empty() ? a.function_name
                                                          : a.containing_function;
    if (!fn.empty() && fp.functions.count(fn)) s += 500;
    if (!a.function_name.empty() && fp.functions.count(a.function_name)) s += 250;
    for (const auto& f : fp.files) {
        if (!f.empty() && a.location.find(f) != std::string::npos) {
            s += 80;
            break;
        }
    }
    return s;
}

int flowPriorScore(const query::SharedObject* o, const FlowPrior& fp) {
    if (!o || !fp.enabled) return 0;
    int best = 0;
    for (const auto& a : o->accesses)
        best = std::max(best, flowPriorScore(a, fp));
    return best;
}

struct PhaseBCandidate {
    int objectId = -1;
    const query::SharedObject* object = nullptr;
    std::string tier;
    std::string label;
    int reqT = -1;
    int violT = -1;
    AccKind reqKind;
    AccKind violKind;
    const OrderClause* reqClause = nullptr;
    const OrderClause* violClause = nullptr;
    const query::ThreadAccess* violAccess = nullptr;
    const query::ThreadAccess* anchor = nullptr;
    int merged = 1;
    std::vector<std::string> alternates;
};

std::string resourceFamily(const query::SharedObject* o);

// Phase B: deterministic requirement-discharge composition over one session's objects.
// Produces the human-readable verdict block injected into Phase C and the tier
// tallies. `kept` is the number of candidates that survive (drives whether the
// session is calibrated at all). Discharged = serialized pairs dropped (surface
// common-lock AND a contract guarantee — conservative AND
// so a hallucinated guarantee alone never drops a real bug). Benign torn-scalar
// raw conflicts (no stated requirement, scalar-torn, no free/list/self-race) are
// suppressed unless keepLow.
std::string composeVerdict(
    const std::vector<const query::SharedObject*>& objs,
    const std::set<int>& threadSet,
    const std::map<int, LLM::ConcurrencyContract>& contractsByTid,
    const std::map<const query::SharedObject*, int>& objIndex,
    ThreadCreationTree* tct,
    const std::unordered_map<int, Thread*>& threadById,
    bool keepLow,
    int& hi, int& med, int& low, int& dis, int& kept,
    std::vector<PhaseBCandidate>* outSelected = nullptr) {
    hi = med = low = dis = kept = 0;
    (void)threadSet;
    std::map<std::string, PhaseBCandidate> byAnchor;
    std::vector<PhaseBCandidate> raw;

    for (const query::SharedObject* O : objs) {
        if (!O) continue;
        auto oiIt = objIndex.find(O);
        if (oiIt == objIndex.end()) continue;
        int oi = oiIt->second;
        std::vector<int> tids(O->accessing_thread_ids.begin(), O->accessing_thread_ids.end());
        // Candidate thread pairs: distinct cross-thread pairs from the object's
        // accessing threads. A self-race is NOT a special case here: the surface
        // materialises a second concurrent instance of a reentrant root as a
        // sibling synthetic thread id (rootId + 1000000) and duplicates the
        // root's accesses under it, so the ordinary (rootId, synthId) cross-pair
        // below already models "the handler racing itself". That synthId has no
        // Thread object, so the mayHappenInParallel gate skips it (correct: two
        // instances of the same handler on different CPUs are parallel by
        // construction -- the surface's is_self_race IS that MHP evidence).
        std::vector<std::pair<int, int>> pairs;
        for (size_t i = 0; i < tids.size(); ++i)
            for (size_t j = i + 1; j < tids.size(); ++j)
                pairs.emplace_back(tids[i], tids[j]);
        for (const auto& pr : pairs) {
                int t1 = pr.first, t2 = pr.second;
                // A self-race sibling (synthId = base + 1000000) shares the base
                // root's real Thread for MHP purposes; only re-gate genuine
                // distinct roots that both have Thread objects.
                const bool selfRace = O->is_self_race &&
                    (t1 % 1000000 == t2 % 1000000);
                if (!selfRace) {
                    auto T1 = threadById.find(t1), T2 = threadById.find(t2);
                    if (T1 != threadById.end() && T2 != threadById.end() && T1->second && T2->second &&
                        !tct->mayHappenInParallel(T1->second, T2->second))
                        continue;
                }
                AccKind k1 = threadAccessKind(*O, t1), k2 = threadAccessKind(*O, t2);
                bool t1touch = k1.read || k1.write || k1.free;
                bool t2touch = k2.read || k2.write || k2.free;
                bool conflict = t1touch && t2touch && (k1.write || k1.free || k2.write || k2.free);
                if (!conflict) continue;

                const OrderClause* clA = contractsByTid.count(t1)
                    ? clauseForObject(contractsByTid.at(t1), oi, *O) : nullptr;
                const OrderClause* clB = contractsByTid.count(t2)
                    ? clauseForObject(contractsByTid.at(t2), oi, *O) : nullptr;

                // Deterministic discharge (skipped under LACE_NO_DETERMINISTIC_DISCHARGE):
                const bool lifetimeHazard = k1.free || k2.free;
                const bool semanticNullHazard =
                    keepLockedNullHazardsEnabled() && nullStateHazard(*O);
                if (!noDeterministicDischargeEnabled()) {
                    bool lockDischarge = !lifetimeHazard && !semanticNullHazard &&
                                         surfaceSharedLock(*O, t1, t2) &&
                                         (establishesOrder(clA) || establishesOrder(clB));
                    bool hardDischarge = establishesHardNonLockOrder(clA) ||
                                         establishesHardNonLockOrder(clB);
                    const bool keepUnverifiedHard = hardDischarge &&
                        !trustHardNonLockDischarge() && !lockDischarge;
                    if (lockDischarge || (hardDischarge && !keepUnverifiedHard)) {
                        ++dis;
                        if (evalVerbose()) {
                            std::cout << "    [dischargeB] obj#" << oi << " '"
                                      << (O->name.empty() ? "<anon>" : O->name) << "' t" << t1
                                      << (selfRace ? "(self)" : ("/t" + std::to_string(t2)))
                                      << " via " << (lockDischarge ? "lock-exclusion" : "hard-nonlock")
                                      << " lifetimeHazard=" << (lifetimeHazard ? 1 : 0) << std::endl;
                        }
                        continue;
                    }
                }

                // Pointer/resource binding: discharge above used clA/clB as-is
                // (a merged EXCLUDE guarantee legitimately spans the whole
                // lock-region group), but the resource-specific `assume` that
                // generates a hazard must be honoured only on the object it is
                // anchored to -- otherwise a lifetime/atomicity requirement about
                // resource A mis-fires as a bug on sibling field B (same lock).
                const OrderClause* reqA = assumeAnchoredToObject(clA, oi) ? clA : nullptr;
                const OrderClause* reqB = assumeAnchoredToObject(clB, oi) ? clB : nullptr;
                auto hA = hazardTier(reqA, k2);  // t1 requires, t2 violates
                auto hB = hazardTier(reqB, k1);  // t2 requires, t1 violates
                if (semanticNullHazard) {
                    if (reqA && hasAssumeRel(reqA, "STABLE_DURING") && k2.write) {
                        hA = {"high", "STABLE_DURING violated by NULL-state invalidation"};
                    }
                    if (reqB && hasAssumeRel(reqB, "STABLE_DURING") && k1.write) {
                        hB = {"high", "STABLE_DURING violated by NULL-state invalidation"};
                    }
                }
                std::string tier, label;
                int reqT = t1, violT = t2;
                if (!hA.first.empty() && tierRank(hA.first) >= tierRank(hB.first)) {
                    tier = hA.first; label = hA.second; reqT = t1; violT = t2;
                } else if (!hB.first.empty()) {
                    tier = hB.first; label = hB.second; reqT = t2; violT = t1;
                }
                if (tier.empty()) {
                    bool suppressible = O->has_scalar_torn_access && !O->has_free_operation &&
                                        !O->has_list_mutation && !O->is_self_race;
                    // In manual entry mode the surface is already tightly scoped to
                    // the analyst-declared concurrent contexts, so we do NOT hard-drop
                    // benign torn-scalars here -- we demote them to low tier and let
                    // the Phase C sink-gate decide (recall-first). The narrow drop is
                    // kept only for the broad auto-discovery surface.
                    if (suppressible && !keepLow && !manualentry::enabled()) continue;
                    tier = "low";
                    label = "raw conflict (no stated order requirement)";
                }

                const AccKind& vk = (violT == t1) ? k1 : k2;
                const AccKind& rk = (reqT == t1) ? k1 : k2;
                const OrderClause* reqClause  = (reqT == t1) ? clA : clB;
                const OrderClause* violClause = (reqT == t1) ? clB : clA;
                PhaseBCandidate c;
                c.objectId = oi;
                c.object = O;
                c.tier = tier;
                c.label = label;
                c.reqT = reqT;
                c.violT = violT;
                c.reqKind = rk;
                c.violKind = vk;
                c.reqClause = reqClause;
                c.violClause = violClause;
                c.violAccess = escapingViolatorAccess(*O, violT, vk);
                c.anchor = mutatingAnchor(*O, reqT, rk, violT, vk);
                raw.push_back(std::move(c));
        }
    }

    // Pre-C dedup: many candidates are the same root mutation observed from
    // multiple readers/call paths. Collapse them before asking C to review.
    for (auto& c : raw) {
        std::string key = std::to_string(c.objectId) + "|" + phaseBHazardClass(c.label) +
                          "|" + anchorKey(c.anchor);
        auto it = byAnchor.find(key);
        if (it == byAnchor.end()) {
            byAnchor.emplace(key, std::move(c));
            continue;
        }
        PhaseBCandidate& rep = it->second;
        ++rep.merged;
        if (tierRank(c.tier) > tierRank(rep.tier)) {
            std::string oldRep = "thread " + std::to_string(rep.reqT) + " vs thread " +
                                 std::to_string(rep.violT) + ": " + fmtAccess(rep.violAccess);
            c.merged = rep.merged;
            c.alternates = std::move(rep.alternates);
            c.alternates.push_back(oneLine(oldRep, 180));
            rep = std::move(c);
        } else {
            std::string alt = "thread " + std::to_string(c.reqT) + " vs thread " +
                              std::to_string(c.violT) + ": " + fmtAccess(c.violAccess);
            rep.alternates.push_back(oneLine(alt, 180));
        }
    }

    std::vector<PhaseBCandidate> candidates;
    candidates.reserve(byAnchor.size());
    for (auto& [_, c] : byAnchor) candidates.push_back(std::move(c));
    std::stable_sort(candidates.begin(), candidates.end(), [](const auto& a, const auto& b) {
        if (tierRank(a.tier) != tierRank(b.tier)) return tierRank(a.tier) > tierRank(b.tier);
        int ar = a.object ? a.object->risk_score : 0;
        int br = b.object ? b.object->risk_score : 0;
        if (ar != br) return ar > br;
        return a.merged > b.merged;
    });

    // CONCURRENCY_CONTRACT_SPEC.md §7/§8: do NOT truncate covered candidates to a
    // tiny top-N. The byAnchor dedup above already collapses same-anchor variants,
    // so the remaining candidates are distinct (resource, hazard, anchor) classes.
    // Reviewing them all does NOT add dialogues -- they go into ONE batched session
    // prompt. A generous overall cap (maxTotal) only bounds prompt size; chunking
    // for very large sessions is handled later. The per-tier caps stay env-tunable
    // for ablation but default high so high/medium are no longer dropped.
    const int maxHigh = envInt("LACE_B2C_MAX_HIGH", 10000);
    const int maxMed = envInt("LACE_B2C_MAX_MED", 10000);
    const int maxLow = keepLow ? envInt("LACE_B2C_MAX_LOW", 10000) : 0;
    const int maxTotal = envInt("LACE_B2C_MAX_TOTAL", 60);
    std::vector<PhaseBCandidate> selected;
    int sh = 0, sm = 0, sl = 0;
    for (auto& c : candidates) {
        if (static_cast<int>(selected.size()) >= maxTotal) break;
        if (c.tier == "high") {
            if (sh >= maxHigh) continue;
            ++sh;
        } else if (c.tier == "medium") {
            if (sm >= maxMed) continue;
            ++sm;
        } else {
            if (sl >= maxLow) continue;
            ++sl;
        }
        selected.push_back(std::move(c));
    }

    // Hand the structured survivors back so the orchestrator can regroup them
    // across sessions for batched bounded verification (spec §8). The pointers
    // (object/clauses/accesses) reference the surface and contracts, which outlive
    // the whole Phase B/C phase.
    if (outSelected) *outSelected = selected;

    std::stringstream vs;
    if (raw.size() != selected.size()) {
        vs << "  Phase-B pre-C reduction: raw_pairs=" << raw.size()
           << ", anchor_groups=" << candidates.size()
           << ", selected_for_review=" << selected.size()
           << " (caps total=" << maxTotal << ", high=" << maxHigh
           << ", medium=" << maxMed << ", low=" << maxLow << ").\n";
    }

    int cid = 1;
    std::set<int> renderedObjects;
    for (const auto& c : selected) {
        if (!c.object) continue;
        if (renderedObjects.insert(c.objectId).second) {
            vs << "  object [obj#" << c.objectId << "] "
               << (c.object->name.empty() ? "<anon>" : c.object->name)
               << (c.object->type.empty() ? "" : ("  (type: " + c.object->type + ")")) << ":\n";
        }
        if (c.tier == "high") ++hi; else if (c.tier == "medium") ++med; else ++low;
        ++kept;
        vs << "    - [C" << cid++ << "][" << c.tier << "] " << c.label;
        if (c.merged > 1) vs << "  (represents " << c.merged << " same-anchor variants)";
        vs << "\n";
        vs << "      violated_clause = " << phaseBHazardClass(c.label)
           << "; resource_family = " << resourceFamily(c.object) << "\n";
        if (c.reqClause && !c.reqClause->assume.empty()) {
            vs << "      requirer = thread " << c.reqT << ":\n";
            appendAssume(vs, c.reqClause);
        } else {
            vs << "      requirer = thread " << c.reqT
               << " (raw conflict; no stated order requirement)\n";
        }
        vs << "      violator = thread " << c.violT << ": "
           << fmtAccess(c.violAccess) << "\n";
        if (c.anchor && c.anchor != c.violAccess)
            vs << "      mutating anchor = " << fmtAccess(c.anchor) << "\n";
        appendGuarantee(vs, c.reqClause, c.violClause);
        if (!c.alternates.empty()) {
            vs << "      same-anchor alternate observations:";
            size_t limit = std::min<size_t>(c.alternates.size(), 4);
            for (size_t i = 0; i < limit; ++i) vs << "\n        * " << c.alternates[i];
            if (c.alternates.size() > limit) vs << "\n        * ...";
            vs << "\n";
        }
    }
    return vs.str();
}

// Resource family for batched verification grouping (spec §2/§8). Derived from the
// canonical surface NAME, not the LLVM `type` (which is a useless "ptr" for nearly
// every kernel object). Groups all fields of the same struct together, each global
// symbol / alloc-site as its own family:
//   field:struct.<TYPE>.<field>@<off>  -> "struct.<TYPE>"
//   global:<symbol>                    -> "global:<symbol>"
//   obj:<alloc-site>                   -> "obj:<alloc-site>"
// Keeps the grouping subsystem-independent (no CVE-specific names).
std::string resourceFamily(const query::SharedObject* o) {
    if (!o) return "?";
    const std::string& n = o->name;
    size_t sp = n.find("struct.");
    if (sp != std::string::npos) {
        size_t typeStart = sp + 7;                      // after "struct."
        size_t end = n.find('.', typeStart);            // dot before the field name
        if (end == std::string::npos) end = n.find('@', typeStart);
        if (end == std::string::npos) end = n.size();
        return "struct." + n.substr(typeStart, end - typeStart);
    }
    if (n.rfind("global:", 0) == 0) {
        std::string g = n.substr(7);
        size_t at = g.find('@');
        return "global:" + (at == std::string::npos ? g : g.substr(0, at));
    }
    if (n.rfind("obj:", 0) == 0) return n;
    size_t arrow = n.find("->");
    if (arrow != std::string::npos) return n.substr(0, arrow);
    size_t dot = n.find('.');
    if (dot != std::string::npos) return n.substr(0, dot);
    if (!o->type.empty() && o->type != "ptr") return o->type;
    return n.empty() ? "?" : n;
}

// Render a verdict block for an arbitrary set of Phase-B candidates (spec §8
// batched review). Same per-candidate format as composeVerdict's own rendering,
// but over candidates regrouped across sessions by (resource_family, clause).
std::string renderBatchVerdict(const std::vector<PhaseBCandidate>& sel) {
    std::stringstream vs;
    int cid = 1;
    std::set<int> renderedObjects;
    for (const auto& c : sel) {
        if (!c.object) continue;
        if (renderedObjects.insert(c.objectId).second) {
            vs << "  object [obj#" << c.objectId << "] "
               << (c.object->name.empty() ? "<anon>" : c.object->name)
               << (c.object->type.empty() ? "" : ("  (type: " + c.object->type + ")")) << ":\n";
        }
        vs << "    - [C" << cid++ << "][" << c.tier << "] " << c.label;
        if (c.merged > 1) vs << "  (represents " << c.merged << " same-anchor variants)";
        vs << "\n";
        vs << "      violated_clause = " << phaseBHazardClass(c.label)
           << "; resource_family = " << resourceFamily(c.object) << "\n";
        if (c.reqClause && !c.reqClause->assume.empty()) {
            vs << "      requirer = thread " << c.reqT << ":\n";
            appendAssume(vs, c.reqClause);
        } else {
            vs << "      requirer = thread " << c.reqT
               << " (raw conflict; no stated order requirement)\n";
        }
        vs << "      violator = thread " << c.violT << ": "
           << fmtAccess(c.violAccess) << "\n";
        if (c.anchor && c.anchor != c.violAccess)
            vs << "      mutating anchor = " << fmtAccess(c.anchor) << "\n";
        appendGuarantee(vs, c.reqClause, c.violClause);
        if (!c.alternates.empty()) {
            vs << "      same-anchor alternate observations:";
            size_t limit = std::min<size_t>(c.alternates.size(), 4);
            for (size_t i = 0; i < limit; ++i) vs << "\n        * " << c.alternates[i];
            if (c.alternates.size() > limit) vs << "\n        * ...";
            vs << "\n";
        }
    }
    return vs.str();
}

// ---- Ablation: Phase-B candidates -> Hypotheses (skip Phase C) ------------
// Converts PhaseBCandidate objects directly to query::Hypothesis so Phase C
// (LLM calibration) can be skipped entirely. Each candidate becomes an
// unverified hypothesis with nodes derived from the surface access node_ids.
query::Hypothesis phaseBToHypothesis(const PhaseBCandidate& c, int seqId) {
    query::Hypothesis h;
    h.id = "phaseB_" + std::to_string(seqId);
    h.severity = c.tier;
    std::string cls = phaseBHazardClass(c.label);
    if (cls == "UAF" || cls == "lifetime")
        h.bug_category = "use_after_free";
    else if (cls == "refcount" || cls == "double_free")
        h.bug_category = "double_free";
    else if (cls == "null_deref")
        h.bug_category = "null_dereference";
    else
        h.bug_category = "data_race";
    std::ostringstream desc;
    desc << "[Phase-B direct] " << c.label;
    if (c.object)
        desc << " on " << (c.object->name.empty() ? "<anon>" : c.object->name);
    desc << " (thread " << c.reqT << " vs thread " << c.violT << ")";
    if (c.merged > 1)
        desc << " [" << c.merged << " same-anchor variants]";
    h.description = desc.str();
    if (c.violAccess && c.violAccess->node_id >= 0)
        h.nodes["violator"] = c.violAccess->node_id;
    if (c.anchor && c.anchor->node_id >= 0 && c.anchor != c.violAccess)
        h.nodes["anchor"] = c.anchor->node_id;
    return h;
}

// ===========================================================================
// L2 (paper-faithful) requirement-driven checker.
//
// Realises paper Sections 3.4-3.5 directly: build an ordering-evidence graph
// E_ord from the node-anchored Order/Wait guarantees composed across the
// thread-set PLUS the static happens-before graph, derive a protection map from
// the Exclude/AtomicOp guarantees and the surface locksets, then evaluate each
// requirement's discharge condition as graph queries. A requirement that cannot
// be discharged becomes a violation candidate. Candidates are NOT generated from
// bare surface conflicts here -- only from undischarged requirements -- which is
// the key difference from the surface-driven composeVerdict() above.
// ===========================================================================
namespace l2 {

using NodeReq = LLM::ConcurrencyContract::NodeReq;
using NodeGuar = LLM::ConcurrencyContract::NodeGuar;

// Ordering evidence composed across all contracts in a thread-set.
struct OrderingEvidence {
    CCPG* ccpg = nullptr;
    HBGraph* hb = nullptr;
    std::vector<std::pair<int,int>> guarEdges;          // Order(a->b) and Wait(c->v_w)
    std::map<std::string, std::vector<int>> exclExclusive; // token -> nodes (exclusive mode)
    std::map<std::string, std::vector<int>> exclShared;    // token -> nodes (shared mode)
    std::map<std::string, std::vector<int>> atomicTok;     // token -> nodes (AtomicOp)
    int maxDepth = 64;

    bool nodeOk(int id) const { return ccpg && ccpg->getNodeByID(id) != nullptr; }

    // Control-flow may-reach (no cross-thread sync required). Paper's approximation
    // for source-side closure / target-side prefix; imprecise guards remain a
    // source of error, as the methodology states.
    bool cfReach(int from, int to) const {
        if (from == to) return true;
        CCPGNode* a = ccpg->getNodeByID(from);
        CCPGNode* b = ccpg->getNodeByID(to);
        if (!a || !b) return false;
        return hb->hbReachable(a, b, maxDepth, /*requireSyncEdge=*/false);
    }
    // Static must-order that crosses at least one genuine synchronization edge.
    // Restricted to structural synchronization (locks, fork/join); RCU,
    // completion and registration orderings must come from a recovered
    // guarantee instead (see l2SyncPolicy).
    bool staticSyncOrder(int from, int to) const {
        CCPGNode* a = ccpg->getNodeByID(from);
        CCPGNode* b = ccpg->getNodeByID(to);
        if (!a || !b) return false;
        return hb->hbReachable(a, b, maxDepth, /*requireSyncEdge=*/true,
                               l2SyncPolicy());
    }

    // Must-order witness a (prec) b: a static synchronized chain, OR a chain that
    // uses >=1 composed guarantee edge (source closure -> guarantee -> target
    // prefix, possibly chained). Pure control-flow reachability alone is NOT a
    // witness (it never returns true without a sync edge or a guarantee hop).
    bool ordReach(int from, int to) const {
        if (from == to) return true;
        if (staticSyncOrder(from, to)) return true;
        if (guarEdges.empty()) return false;
        std::set<int> visited;
        std::vector<int> work;
        for (const auto& e : guarEdges)
            if (cfReach(from, e.first) && visited.insert(e.second).second)
                work.push_back(e.second);
        while (!work.empty()) {
            int x = work.back(); work.pop_back();
            if (cfReach(x, to)) return true;
            for (const auto& e : guarEdges)
                if ((x == e.first || cfReach(x, e.first)) &&
                    visited.insert(e.second).second)
                    work.push_back(e.second);
        }
        return false;
    }

    // Both nodes covered by the same exclusion token under incompatible modes
    // (at least one exclusive) -> mutual exclusion mediates the pair.
    bool commonExclusion(int a, int b) const {
        for (const auto& [tok, nodes] : exclExclusive) {
            bool aIn = std::find(nodes.begin(), nodes.end(), a) != nodes.end();
            bool bInEx = std::find(nodes.begin(), nodes.end(), b) != nodes.end();
            bool bInSh = false;
            auto sh = exclShared.find(tok);
            if (sh != exclShared.end())
                bInSh = std::find(sh->second.begin(), sh->second.end(), b) != sh->second.end();
            bool bIn = bInEx || bInSh;
            // symmetric: a may be in shared, b in exclusive
            bool aInSh = false;
            if (sh != exclShared.end())
                aInSh = std::find(sh->second.begin(), sh->second.end(), a) != sh->second.end();
            if ((aIn && bIn) || (aInSh && bInEx)) return true;
        }
        return false;
    }
    // Both nodes are atomic accesses to the same token -> compatible atomic protocol.
    bool atomicCompatible(int a, int b) const {
        for (const auto& [tok, nodes] : atomicTok) {
            bool aIn = std::find(nodes.begin(), nodes.end(), a) != nodes.end();
            bool bIn = std::find(nodes.begin(), nodes.end(), b) != nodes.end();
            if (aIn && bIn) return true;
        }
        return false;
    }
};

// Compose ordering + protection evidence from every contract in the thread-set.
OrderingEvidence buildEvidence(const std::set<int>& threadSet,
                               const std::map<int, LLM::ConcurrencyContract>& contractsByTid,
                               CCPG* ccpg, HBGraph* hb) {
    OrderingEvidence ev;
    ev.ccpg = ccpg;
    ev.hb = hb;
    for (int tid : threadSet) {
        auto it = contractsByTid.find(tid);
        if (it == contractsByTid.end()) continue;
        for (const NodeGuar& g : it->second.nodeGuars) {
            if (g.form == "Order" || g.form == "Wait") {
                for (int a : g.a)
                    for (int b : g.b)
                        if (ev.nodeOk(a) && ev.nodeOk(b)) ev.guarEdges.push_back({a, b});
            } else if (g.form == "Exclude") {
                bool shared = (g.mode == "shared");
                auto& dst = shared ? ev.exclShared[g.token] : ev.exclExclusive[g.token];
                for (int a : g.a) if (ev.nodeOk(a)) dst.push_back(a);
            } else if (g.form == "AtomicOp") {
                for (int a : g.a) if (ev.nodeOk(a)) ev.atomicTok[g.token].push_back(a);
            }
        }
    }
    return ev;
}

// One undischarged requirement.
struct L2Candidate {
    int objectId = -1;
    const query::SharedObject* object = nullptr;
    std::string form;
    std::string reason;
    int reqTid = -1;
    int aNode = -1;
    int bNode = -1;
    bool lifetime = false;   // a free is on the failing side -> UAF/lifetime shape
};

// Access type ("Read"/"Write"/"Free") of the surface access at node_id on object O.
std::string accTypeOfNode(const query::SharedObject* O, int node) {
    if (!O) return "";
    for (const auto& a : O->accesses)
        if (a.node_id == node) return a.access_type;
    return "";
}
bool sameLockCovers(const query::SharedObject* O, int a, int b) {
    if (!O) return false;
    const query::ThreadAccess* pa = nullptr;
    const query::ThreadAccess* pb = nullptr;
    for (const auto& acc : O->accesses) {
        if (acc.node_id == a) pa = &acc;
        if (acc.node_id == b) pb = &acc;
    }
    if (!pa || !pb) return false;
    return pa->is_lock_protected && pb->is_lock_protected &&
           !pa->protecting_lock.empty() && pa->protecting_lock == pb->protecting_lock;
}

// Evaluate every requirement in the thread-set's contracts; collect the
// undischarged ones as candidates.
std::vector<L2Candidate> checkRequirements(
    const std::vector<const query::SharedObject*>& objs,
    const std::set<int>& threadSet,
    const std::map<int, LLM::ConcurrencyContract>& contractsByTid,
    const std::map<const query::SharedObject*, int>& objIndex,
    CCPG* ccpg, HBGraph* hb) {

    OrderingEvidence ev = buildEvidence(threadSet, contractsByTid, ccpg, hb);

    // objectId -> SharedObject* for this session (so a requirement's objectId can
    // recover its surface accesses for lock/conflict facts).
    std::map<int, const query::SharedObject*> objById;
    for (const query::SharedObject* O : objs) {
        auto it = objIndex.find(O);
        if (it != objIndex.end()) objById[it->second] = O;
    }

    std::vector<L2Candidate> out;
    for (int tid : threadSet) {
        auto cit = contractsByTid.find(tid);
        if (cit == contractsByTid.end()) continue;
        for (const NodeReq& r : cit->second.nodeReqs) {
            const query::SharedObject* O =
                (r.objectId >= 0 && objById.count(r.objectId)) ? objById.at(r.objectId) : nullptr;

            if (r.form == "MustPrecede") {
                // Discharged iff every a precedes every b via a must-order witness.
                for (int a : r.a) {
                    for (int b : r.b) {
                        if (ev.ordReach(a, b)) continue;
                        L2Candidate c;
                        c.objectId = r.objectId; c.object = O; c.form = r.form;
                        c.reqTid = tid; c.aNode = a; c.bNode = b;
                        c.lifetime = (accTypeOfNode(O, b) == "Free");
                        c.reason = "MustPrecede undischarged: no must-order witness from "
                                   "node " + std::to_string(a) + " to node " + std::to_string(b) +
                                   (r.note.empty() ? "" : ("  [" + r.note + "]"));
                        out.push_back(std::move(c));
                        goto nextReq;  // one candidate per requirement is enough
                    }
                }
            } else if (r.form == "MustBeMediated") {
                for (int a : r.a) {
                    for (int b : r.b) {
                        bool mediated = ev.ordReach(a, b) || ev.ordReach(b, a) ||
                                        ev.commonExclusion(a, b) || ev.atomicCompatible(a, b) ||
                                        sameLockCovers(O, a, b);
                        if (mediated) continue;
                        L2Candidate c;
                        c.objectId = r.objectId; c.object = O; c.form = r.form;
                        c.reqTid = tid; c.aNode = a; c.bNode = b;
                        c.lifetime = (accTypeOfNode(O, a) == "Free" || accTypeOfNode(O, b) == "Free");
                        c.reason = "MustBeMediated undischarged: conflicting nodes " +
                                   std::to_string(a) + " and " + std::to_string(b) +
                                   " share no ordering, exclusion, or atomic protocol" +
                                   (r.note.empty() ? "" : ("  [" + r.note + "]"));
                        out.push_back(std::move(c));
                        goto nextReq;
                    }
                }
            } else if (r.form == "MustBeAtomic") {
                // Conflicting accesses = other threads' writes/frees on the object.
                std::vector<int> conflicts;
                if (O) {
                    for (const auto& acc : O->accesses) {
                        if (threadSet.count(acc.thread_id) == 0) continue;
                        if (acc.thread_id == tid) continue;
                        if ((acc.access_type == "Write" || acc.access_type == "Free") &&
                            acc.node_id >= 0)
                            conflicts.push_back(acc.node_id);
                    }
                }
                if (r.a.empty()) goto nextReq;
                int first = r.a.front(), last = r.a.back();
                for (int cnode : conflicts) {
                    // Discharged for cnode iff it is fully ordered outside the region
                    // or excluded/atomically coordinated with it.
                    bool outside = ev.ordReach(cnode, first) || ev.ordReach(last, cnode);
                    bool excluded = false;
                    for (int s : r.a)
                        if (ev.commonExclusion(s, cnode) || ev.atomicCompatible(s, cnode) ||
                            sameLockCovers(O, s, cnode)) { excluded = true; break; }
                    if (outside || excluded) continue;
                    L2Candidate c;
                    c.objectId = r.objectId; c.object = O; c.form = r.form;
                    c.reqTid = tid; c.aNode = first; c.bNode = cnode;
                    c.lifetime = (accTypeOfNode(O, cnode) == "Free");
                    c.reason = "MustBeAtomic undischarged: conflicting node " +
                               std::to_string(cnode) + " can interleave the atomic region" +
                               (r.note.empty() ? "" : ("  [" + r.note + "]"));
                    out.push_back(std::move(c));
                    goto nextReq;
                }
            }
            nextReq:;
        }
    }
    return out;
}

query::Hypothesis toHypothesis(const L2Candidate& c, int seqId) {
    query::Hypothesis h;
    h.id = "l2_" + std::to_string(seqId);
    if (c.form == "MustPrecede")
        h.bug_category = c.lifetime ? "use_after_free" : "order_violation";
    else if (c.form == "MustBeMediated")
        h.bug_category = c.lifetime ? "use_after_free" : "data_race";
    else  // MustBeAtomic
        h.bug_category = c.lifetime ? "use_after_free" : "atomicity_violation";
    h.severity = c.lifetime ? "high" : "medium";
    std::ostringstream d;
    d << "[L2] " << c.reason;
    if (c.object) d << " on " << (c.object->name.empty() ? "<anon>" : c.object->name);
    d << " (thread " << c.reqTid << ")";
    h.description = d.str();
    if (c.aNode >= 0) h.nodes["a"] = c.aNode;
    if (c.bNode >= 0) h.nodes["b"] = c.bNode;
    return h;
}

// ---- Strict evidence-bounded Phase C filter (paper's calibration step) ----
// Reviews the checker's candidates and keeps only those it does NOT reject. It
// cannot add candidates: the output is always a subset of the input, so recall
// is bounded by Phase B and calibration only affects precision. Fail-open: an
// un-judged candidate stays kept.
class Calibrator : public Conversation {
public:
    Calibrator(std::shared_ptr<LLMClient> client, CCPG* ccpg)
        : Conversation(client, sysPrompt(), 40), ccpg_(ccpg) {}

    std::vector<char> review(const std::vector<L2Candidate>& cands) {
        keep_.assign(cands.size(), 1);   // fail-open default: keep
        n_ = static_cast<int>(cands.size());
        if (n_ == 0) return keep_;
        reset();
        set_token_budget(20000);
        set_max_turns(2 * n_ + 8);
        pin_next_user_message();
        send_message(renderBatch(cands));
        return keep_;
    }

private:
    static std::string sysPrompt() {
        return R"CAL(
You calibrate concurrency-defect CANDIDATES produced by a deterministic checker.
Each candidate is a requirement the checker could NOT discharge (no ordering,
mutual exclusion, or atomic protocol was found between two operations).

Your ONLY job is to decide, for each candidate, whether it is a genuine,
reportable concurrency defect or a false positive. You CANNOT introduce new
defects; you only keep or reject the given candidates.

Reject a candidate ONLY with concrete evidence that it is not a real defect, e.g.:
  * the two operations cannot actually run concurrently -- one is one-time
    init/setup/activation that happens-before any user/sysfs access, or a
    parent step that completes before the child context starts;
  * they are ordered by construction, or genuinely covered by the same lock;
  * the field is a benign statistic/log value that drives no safety decision.
Keep a candidate when a real unordered/unmediated conflict or use-before-free is
plausible. When in doubt, KEEP (recall matters more than precision here).

**CRITICAL: use ONLY the tools. Call judge(candidate_id, verdict, reason) once per
candidate, then finish_review. Inspect code with the read tools if needed.**
)CAL";
    }

    std::string nodeStr(int id) const {
        CCPGNode* n = (ccpg_ && id >= 0) ? ccpg_->getNodeByID(id) : nullptr;
        if (!n || !n->getCPGNode()) return "node " + std::to_string(id) + " <unknown>";
        return "node " + std::to_string(id) + ": " + oneLine(n->getCPGNode()->getCode(), 120) +
               "  @ " + n->getNodeLoc().toString();
    }

    std::string renderBatch(const std::vector<L2Candidate>& cands) {
        std::stringstream ss;
        ss << "Calibrate the following " << n_ << " candidate(s). For EACH, call "
              "judge(candidate_id, verdict, reason) with verdict \"keep\" or \"reject\", "
              "then finish_review.\n\n";
        for (int i = 0; i < n_; ++i) {
            const L2Candidate& c = cands[i];
            ss << "[" << i << "] " << c.form << (c.lifetime ? " (lifetime/UAF)" : "")
               << (c.object ? ("  on " + (c.object->name.empty() ? std::string("<anon>")
                                                                 : c.object->name)) : "")
               << "\n";
            ss << "     a = " << nodeStr(c.aNode) << "\n";
            if (c.bNode >= 0) ss << "     b = " << nodeStr(c.bNode) << "\n";
            ss << "     checker: " << oneLine(c.reason, 240) << "\n";
            ss << "     requirer thread = " << c.reqTid << "\n\n";
        }
        return ss.str();
    }

    std::vector<Tool> get_available_tools() const override {
        auto tools = SharedToolKit::get_shared_tools();
        tools.push_back({"judge", "Keep or reject ONE candidate by its [i] id.", {
            {"candidate_id", "integer", "The [i] index of the candidate.", true},
            {"verdict", "string", "\"keep\" or \"reject\".", true},
            {"reason", "string", "One concise justification.", true}
        }});
        tools.push_back({"finish_review", "Call after judging all candidates.", {}});
        return tools;
    }

    std::string execute_tool(const std::string& name, const nlohmann::json& args) override {
        auto shared = SharedToolKit::handle_shared_tool(name, args, ccpg_);
        if (shared) return *shared;
        if (name == "judge") {
            if (!args.contains("candidate_id") || !args["candidate_id"].is_number_integer())
                return R"({"error":"judge needs integer candidate_id."})";
            int id = args["candidate_id"].get<int>();
            if (id < 0 || id >= n_)
                return R"({"error":"candidate_id out of range."})";
            std::string v = args.value("verdict", std::string());
            keep_[id] = (v == "reject") ? 0 : 1;
            nlohmann::json r;
            r["status"] = std::string("recorded ") + (keep_[id] ? "keep" : "reject") +
                          " for candidate " + std::to_string(id);
            return r.dump();
        }
        if (name == "finish_review") return "finish";
        nlohmann::json e; e["error"] = "unknown tool " + name; return e.dump();
    }

    std::string parseResult(const std::vector<ChatMessage>&) override { return "done"; }

    CCPG* ccpg_ = nullptr;
    std::vector<char> keep_;
    int n_ = 0;
};

}  // namespace l2

// ---- Hypothesis dedup (deterministic post-pass; recall-safe) --------------
// The same root cause is often confirmed in several sessions at different
// use-sites (e.g. ONE teardown write raced by sendmsg / recvmsg / poll loads),
// inflating the count with near-duplicates. We collapse hypotheses that share a
// canonical key into one representative and fold the other observed sites into
// its description. REPORTING-only: the represented bug (incl. any GT) is kept,
// so recall is unchanged. The key uses only GENERIC structure -- the racing
// surface object, a normalized hazard class, and the mutating write/free node --
// nothing tuned to any particular CVE.

// Generic hazard class from the model's free-text bug_category. Order matters:
// the specific lifetime/refcount signals win over the generic "atomicity" prefix
// the model attaches to almost everything.
std::string hazardClass(std::string s) {
    for (char& c : s) c = static_cast<char>(std::tolower((unsigned char)c));
    auto has = [&](const char* k) { return s.find(k) != std::string::npos; };
    if (has("refcount") || has("double_free") || has("double free") || has("reference count"))
        return "refcount";
    if (has("uaf") || has("use-after") || has("use after") || has("null") || has("lifetime") ||
        has("teardown") || has("unbind") || has("release") || has("dangling") || has("stale") ||
        has("free"))
        return "lifetime";
    if (has("publish") || has("uninitialized") || has("ordering") || has("order ") ||
        has("init-before") || has("before "))
        return "ordering";
    if (has("atomic") || has("data_race") || has("data race") || has("race") || has("torn") ||
        has("rmw") || has("lost") || has("inconsist"))
        return "atomicity";
    return "other";
}

enum class DedupLevel { Exact, Anchor, ObjectClass };

// Canonical merge key. nodeToObj/nodeToKind map a grounded CCPG node id to its
// surface object index / access kind.
std::string canonicalKey(const query::Hypothesis& h, DedupLevel level,
                         const std::unordered_map<int, int>& nodeToObj,
                         const std::unordered_map<int, std::string>& nodeToKind) {
    const std::string cls = hazardClass(h.bug_category);
    std::vector<int> nodeIds;
    int anchor = -1, anchorRank = -1;        // Free(2) > Write(1) is the mutation
    int primaryObj = INT_MAX;
    bool anyObj = false;
    for (const auto& [role, nid] : h.nodes) {
        nodeIds.push_back(nid);
        auto ko = nodeToObj.find(nid);
        if (ko != nodeToObj.end()) { anyObj = true; primaryObj = std::min(primaryObj, ko->second); }
        auto kk = nodeToKind.find(nid);
        if (kk != nodeToKind.end()) {
            int r = (kk->second == "Free") ? 2 : (kk->second == "Write" ? 1 : -1);
            if (r > anchorRank) { anchorRank = r; anchor = nid; }
        }
    }
    std::sort(nodeIds.begin(), nodeIds.end());
    if (anchor >= 0) {
        auto it = nodeToObj.find(anchor);
        if (it != nodeToObj.end()) primaryObj = it->second;   // anchor's object is most precise
    }
    if (level == DedupLevel::ObjectClass && anyObj)
        return cls + "|obj" + std::to_string(primaryObj);
    if (level == DedupLevel::Anchor && anchor >= 0 && anyObj)
        return cls + "|obj" + std::to_string(primaryObj) + "|a" + std::to_string(anchor);
    // Exact, or fallback when no surface object/anchor resolves (never over-merge).
    std::string nset;
    for (int n : nodeIds) nset += std::to_string(n) + ",";
    return cls + "|nset:" + nset;
}

int severityRank(const std::string& s) {
    if (s == "high" || s == "critical") return 2;
    if (s == "medium") return 1;
    return 0;
}

std::vector<query::Hypothesis> dedupHypotheses(
        std::vector<query::Hypothesis> in,
        const query::VulnerabilitySurface& surface,
        DedupLevel level) {
    if (in.size() < 2) return in;
    std::unordered_map<int, int> nodeToObj;
    std::unordered_map<int, std::string> nodeToKind, nodeToLoc;
    for (size_t oi = 0; oi < surface.shared_objects.size(); ++oi) {
        for (const auto& a : surface.shared_objects[oi].accesses) {
            if (a.node_id < 0) continue;
            nodeToObj.emplace(a.node_id, static_cast<int>(oi));
            nodeToKind.emplace(a.node_id, a.access_type);
            const std::string& fn = a.containing_function.empty() ? a.function_name
                                                                 : a.containing_function;
            if (!nodeToLoc.count(a.node_id)) nodeToLoc[a.node_id] = fn + " @ " + a.location;
        }
    }

    std::map<std::string, std::vector<size_t>> groups;
    for (size_t i = 0; i < in.size(); ++i)
        groups[canonicalKey(in[i], level, nodeToObj, nodeToKind)].push_back(i);
    if (groups.size() == in.size()) return in;   // nothing to merge

    std::vector<query::Hypothesis> out;
    out.reserve(groups.size());
    for (auto& [key, members] : groups) {
        (void)key;
        size_t best = members.front();
        for (size_t idx : members) {
            const auto& a = in[idx];
            const auto& b = in[best];
            int ra = severityRank(a.severity), rb = severityRank(b.severity);
            if (ra != rb) { if (ra > rb) best = idx; continue; }
            if (a.nodes.size() > b.nodes.size()) best = idx;
        }
        query::Hypothesis rep = std::move(in[best]);
        if (members.size() > 1) {
            std::set<std::string> sites;
            for (size_t idx : members) {
                if (idx == best) continue;
                for (const auto& [role, nid] : in[idx].nodes) {
                    auto it = nodeToLoc.find(nid);
                    if (it != nodeToLoc.end()) sites.insert(it->second);
                }
            }
            std::string note = " [dedup: merged " + std::to_string(members.size() - 1) +
                               " near-duplicate finding(s) of the same root cause";
            if (!sites.empty()) {
                note += "; also observed at: ";
                size_t k = 0;
                for (const auto& s : sites) {
                    if (k++) note += "; ";
                    note += s;
                    if (k >= 6) { note += "; ..."; break; }
                }
            }
            note += "]";
            rep.description += note;
        }
        out.push_back(std::move(rep));
    }
    return out;
}

DedupLevel dedupLevelFromEnv() {
    if (const char* e = std::getenv("LACE_DEDUP_LEVEL")) {
        std::string s = e;
        if (s == "exact") return DedupLevel::Exact;
        if (s == "object-class" || s == "object_class") return DedupLevel::ObjectClass;
    }
    return DedupLevel::Anchor;   // default: collapse same-mutation use-sites, keep distinct mutations
}

bool dedupEnabled() {
    if (const char* e = std::getenv("LACE_NO_DEDUP")) return !(e[0] && e[0] != '0');
    return true;
}

bool surfaceBudgetEnabled() {
    if (const char* e = std::getenv("LACE_DISABLE_SURFACE_BUDGET"))
        return !(e[0] && e[0] != '0');
    return true;
}

bool forceKeepObject(const query::SharedObject& o) {
    return o.has_free_operation || o.has_list_mutation || o.is_self_race;
}

int objectPriorityScore(const query::SharedObject* o, const FlowPrior& flowPrior);

void applyCostFirstSurfaceBudget(const query::VulnerabilitySurface& surface,
                                 std::set<int>& objKeep,
                                 const FlowPrior& flowPrior) {
    if (!surfaceBudgetEnabled() || objKeep.size() <= 80) return;

    // CONCURRENCY_CONTRACT_SPEC.md §7: the surface IS the recall floor, so do NOT
    // risk-rank-truncate it. Cost is bounded downstream by deterministic contract
    // discharge (benign conflicts dropped without LLM), the per-thread contract
    // budget (the legitimate O(#threads) denominator), and the per-session candidate
    // cap. The only remaining object truncation is a soundness/throughput safety
    // valve for a PATHOLOGICALLY huge surface where Phase-B pair enumeration would
    // not finish; even there, lifecycle carriers (free/list/self-race) are kept.
    const bool pathological = surface.shared_objects.size() >= 1200 ||
                              surface.conflicting_pair_count >= 50000;
    if (!pathological) return;  // keep every conflicting object: recall floor intact

    int defaultGlobal = 300;
    int defaultPerSet = 6;
    const int globalCap = envInt("LACE_SURFACE_GLOBAL_CAP", defaultGlobal);
    const int perThreadSetCap = envInt("LACE_SURFACE_PER_THREADSET_CAP", defaultPerSet);

    std::set<int> reduced;
    std::map<std::set<int>, int> perSet;
    int forced = 0, budgeted = 0;

    std::vector<int> order;
    order.reserve(objKeep.size());
    for (int id : objKeep) order.push_back(id);
    std::stable_sort(order.begin(), order.end(), [&](int a, int b) {
        return objectPriorityScore(&surface.shared_objects[a], flowPrior) >
               objectPriorityScore(&surface.shared_objects[b], flowPrior);
    });

    // Strong lifecycle carriers stay even if the budget is full; the recall loss
    // is concentrated on lower-risk repeated scalars/config/counter fields. Flow
    // prior is only a soft reorder, never a hard keep/drop rule.
    for (int id : order) {
        if (!objKeep.count(id)) continue;
        const auto& o = surface.shared_objects[id];
        if (forceKeepObject(o)) {
            reduced.insert(id);
            ++forced;
            continue;
        }
        if (budgeted >= globalCap) continue;
        auto ts = o.accessing_thread_ids;
        if (ts.empty()) ts.insert(-1);
        int& used = perSet[ts];
        if (used >= perThreadSetCap) continue;
        reduced.insert(id);
        ++used;
        ++budgeted;
    }

    if (reduced.size() < objKeep.size()) {
        std::cout << "  [surface-budget] " << objKeep.size() << " -> "
                  << reduced.size() << " objects (forced=" << forced
                  << ", budgeted=" << budgeted
                  << ", global_cap=" << globalCap
                  << ", per_threadset_cap=" << perThreadSetCap << ")"
                  << std::endl;
        objKeep.swap(reduced);
    }
}

int objectPriorityScore(const query::SharedObject* o, const FlowPrior& flowPrior) {
    if (!o) return 0;
    int s = o->risk_score;
    if (o->has_free_operation) s += 600;
    if (o->has_list_mutation) s += 300;
    if (o->is_self_race) s += 250;
    if (o->has_unprotected_write) s += 120;
    if (o->has_cross_thread_rw) s += 80;
    if (o->has_missing_atomic_annotation) s += 40;
    s += flowPriorScore(o, flowPrior);
    return s;
}

bool hugeSurface(const query::VulnerabilitySurface& surface) {
    return surface.conflicting_pair_count >= 1500 ||
           surface.total_thread_count >= 120 ||
           surface.shared_objects.size() >= 800;
}

bool largeSurface(const query::VulnerabilitySurface& surface) {
    return hugeSurface(surface) ||
           surface.conflicting_pair_count >= 500 ||
           surface.total_thread_count >= 60 ||
           surface.shared_objects.size() >= 300;
}

bool sessionBudgetEnabled() {
    if (const char* e = std::getenv("LACE_DISABLE_SESSION_BUDGET"))
        return !(e[0] && e[0] != '0');
    return true;
}

bool calibrateMedOnlyEnabled() {
    if (const char* e = std::getenv("LACE_CALIBRATE_MED_ONLY"))
        return e[0] && e[0] != '0';
    // Analyst-scoped (manual entry) mode is recall-first and its surface is small,
    // so a medium-only session is worth one verification dialogue rather than a
    // silent skip.
    return manualentry::enabled();
}

int sessionFlowPriorScore(const std::vector<const query::SharedObject*>& objs,
                          const FlowPrior& flowPrior) {
    int s = 0;
    for (const auto* o : objs) s += flowPriorScore(o, flowPrior);
    return s;
}

using ClusterMap = std::map<std::set<int>, std::vector<const query::SharedObject*>>;
using SessionList = std::vector<std::pair<std::set<int>, std::vector<const query::SharedObject*>>>;

SessionList buildBudgetedSessions(const ClusterMap& clusters,
                                  const query::VulnerabilitySurface& surface,
                                  size_t maxObjsPerSession,
                                  const FlowPrior& flowPrior) {
    struct Chunk {
        std::set<int> threads;
        std::vector<const query::SharedObject*> objects;
        int score = 0;
    };
    std::vector<Chunk> chunks;
    for (const auto& [ts, objs] : clusters) {
        std::vector<const query::SharedObject*> sorted = objs;
        std::stable_sort(sorted.begin(), sorted.end(), [&](const auto* a, const auto* b) {
            return objectPriorityScore(a, flowPrior) > objectPriorityScore(b, flowPrior);
        });
        for (size_t i = 0; i < sorted.size(); i += maxObjsPerSession) {
            Chunk ch;
            ch.threads = ts;
            ch.objects.assign(sorted.begin() + i,
                              sorted.begin() + std::min(sorted.size(), i + maxObjsPerSession));
            for (const auto* o : ch.objects) ch.score += objectPriorityScore(o, flowPrior);
            // Prefer concrete small/frontier thread-sets over giant fan-in groups
            // when scores tie; they are cheaper and usually more diagnostic.
            ch.score -= static_cast<int>(ts.size()) * 3;
            chunks.push_back(std::move(ch));
        }
    }
    std::stable_sort(chunks.begin(), chunks.end(), [](const Chunk& a, const Chunk& b) {
        if (a.score != b.score) return a.score > b.score;
        if (a.objects.size() != b.objects.size()) return a.objects.size() > b.objects.size();
        return a.threads.size() < b.threads.size();
    });

    // Spec §7: modestly raised to use the budget freed by discharge (sessions whose
    // conflicts are all benign now skip calibration). NOT raised aggressively: each
    // surviving session is still one calibration dialogue, so on cases WITHOUT hard
    // mechanisms (no discharge) a high ceiling multiplies Phase-C cost. The real lever
    // for high coverage is batched bounded verification (spec §8); until that lands,
    // keep this near the original cap. Env-tunable for ablation.
    int defaultCap = hugeSurface(surface) ? 35 : (largeSurface(surface) ? 50 : 70);
    const int cap = envInt("LACE_SESSION_CAP", defaultCap);

    SessionList sessions;
    sessions.reserve(chunks.size());
    size_t limit = sessionBudgetEnabled() ? std::min<size_t>(chunks.size(), cap) : chunks.size();
    for (size_t i = 0; i < limit; ++i)
        sessions.emplace_back(std::move(chunks[i].threads), std::move(chunks[i].objects));

    if (sessions.size() < chunks.size()) {
        std::set<int> tids;
        size_t objs = 0;
        for (const auto& [ts, os] : sessions) {
            tids.insert(ts.begin(), ts.end());
            objs += os.size();
        }
        std::cout << "  [session-budget] " << chunks.size() << " -> " << sessions.size()
                  << " sessions (objects=" << objs << ", threads_in_play=" << tids.size()
                  << ", cap=" << cap << ")" << std::endl;
    }
    return sessions;
}

std::set<int> budgetContractThreads(
        const SessionList& sessions,
        const query::VulnerabilitySurface& surface,
        ThreadCreationTree* tct,
        const std::unordered_map<int, Thread*>& threadById,
        const FlowPrior& flowPrior) {
    std::map<int, int> score;
    int surfacePairs = 0, mhpPairs = 0, conflictPairs = 0, selfRaceThreads = 0;
    bool fullMode = false;
    if (const char* e = std::getenv("LACE_PHASE_A_MODE")) {
        std::string s = e;
        fullMode = (s == "full");
    }

    for (const auto& [ts, objs] : sessions) {
        for (const auto* o : objs) {
            if (!o) continue;
            int os = objectPriorityScore(o, flowPrior);
            if (fullMode) {
                for (int tid : ts) score[tid] += os;
                continue;
            }

            std::vector<int> tids;
            for (int tid : o->accessing_thread_ids)
                if (ts.count(tid)) tids.push_back(tid);

            if (o->is_self_race && tids.size() == 1) {
                AccKind k = threadAccessKind(*o, tids.front());
                if (k.read || k.write || k.free) {
                    score[tids.front()] += os + 350;
                    ++selfRaceThreads;
                }
            }

            for (size_t i = 0; i < tids.size(); ++i) {
                for (size_t j = i + 1; j < tids.size(); ++j) {
                    int t1 = tids[i], t2 = tids[j];
                    ++surfacePairs;
                    auto T1 = threadById.find(t1), T2 = threadById.find(t2);
                    if (T1 != threadById.end() && T2 != threadById.end() &&
                        T1->second && T2->second && tct &&
                        !tct->mayHappenInParallel(T1->second, T2->second))
                        continue;
                    ++mhpPairs;

                    AccKind k1 = threadAccessKind(*o, t1), k2 = threadAccessKind(*o, t2);
                    bool t1touch = k1.read || k1.write || k1.free;
                    bool t2touch = k2.read || k2.write || k2.free;
                    bool conflict = t1touch && t2touch &&
                                    (k1.write || k1.free || k2.write || k2.free);
                    if (!conflict) continue;
                    ++conflictPairs;

                    int bonus = os;
                    if (k1.free || k2.free || o->has_free_operation) bonus += 700;
                    else if (o->has_list_mutation) bonus += 350;
                    else if (k1.write || k2.write) bonus += 180;

                    score[t1] += bonus + (k1.free ? 300 : (k1.write ? 120 : 30));
                    score[t2] += bonus + (k2.free ? 300 : (k2.write ? 120 : 30));
                }
            }
        }
    }
    std::vector<std::pair<int, int>> ranked(score.begin(), score.end());
    std::stable_sort(ranked.begin(), ranked.end(), [](const auto& a, const auto& b) {
        if (a.second != b.second) return a.second > b.second;
        return a.first < b.first;
    });

    // Spec §7: per-thread contracts are the legitimate O(#threads) cost denominator
    // (Phase A is parallel and cheaper than per-pair calibration), so budget here by
    // thread count rather than by truncating the object surface. Raised modestly;
    // objects whose threads have no contract fall through to a low-tier raw conflict.
    int defaultCap = hugeSurface(surface) ? 30 : (largeSurface(surface) ? 45 : 90);
    const int cap = envInt("LACE_CONTRACT_THREAD_CAP", defaultCap);
    std::set<int> out;
    for (size_t i = 0; i < ranked.size() && static_cast<int>(i) < cap; ++i)
        out.insert(ranked[i].first);
    if (!fullMode) {
        std::cout << "  [phaseA-b0] surface_pairs=" << surfacePairs
                  << ", mhp_pairs=" << mhpPairs
                  << ", conflict_pairs=" << conflictPairs
                  << ", self_race_threads=" << selfRaceThreads
                  << ", candidate_threads=" << ranked.size()
                  << std::endl;
    }
    if (out.size() < ranked.size()) {
        std::cout << "  [contract-thread-budget] " << ranked.size() << " -> "
                  << out.size() << " threads (cap=" << cap << ")" << std::endl;
    }
    return out;
}

} // namespace

AgentManager::AgentManager(CCPG* cpg)
    : llmClient(LLMClient::get_instance()),
      entryFinder(cpg, llmClient),
      parallelAnalyzer(llmClient),
      contractGenerator(cpg, llmClient),
      ccpg(cpg) {
    if (!llmClient) {
        std::cerr << "Failed to initialize LLM Client. Please check URL and API key." << std::endl;
    }
}

std::vector<llm_client::ThreadPair> AgentManager::runAnalysisAgentMode() {
    if (!llmClient || !ccpg) {
        std::cerr << "LLM Client or CCPG not initialized. Aborting analysis.";
        return {};
    }

    std::cout << "\n--- Starting Agent-Mode Concurrency Analysis (Mechanism Rules) ---" << std::endl;

    ThreadCreationTree* tct = ThreadCreationTree::getInstance();

    // Phase 1: Generate vulnerability surface (pure static, no LLM)
    std::cout << "\n[Phase 1: Generating Vulnerability Surface (static)]" << std::endl;
    query::VulnerabilitySurfaceGenerator surfaceGen(ccpg, tct);
    auto surface = surfaceGen.generate();

    fs::path surface_path = TargetPath::getInstance()->getOutputDir() / "vulnerability_surface.json";
    std::ofstream surface_file(surface_path);
    if (surface_file.is_open()) {
        surface_file << surface.toJson().dump(2);
        surface_file.close();
        std::cout << "Vulnerability surface saved to: " << surface_path << std::endl;
    }

    size_t high_risk_count = 0;
    for (const auto& obj : surface.shared_objects) {
        if (obj.risk_score > 0) high_risk_count++;
    }

    std::cout << "\n  Threads: " << surface.total_thread_count
              << ", Conflicting pairs: " << surface.conflicting_pair_count
              << ", Shared objects: " << surface.shared_objects.size()
              << ", High-risk objects: " << high_risk_count
              << std::endl;

    if (surface.shared_objects.empty()) {
        std::cout << "No shared objects found. Skipping LLM analysis." << std::endl;
        return {};
    }

    // v23 P9a smoke aid: when LACE_EARLY_EXIT_AFTER_SURFACE is set, exit
    // right after Phase 1 so we can collect [P9a] stats and surface JSON
    // across many CVEs without spending any LLM tokens.
    if (const char* early = std::getenv("LACE_EARLY_EXIT_AFTER_SURFACE")) {
        if (early[0] != '\0' && early[0] != '0') {
            std::cout << "[LACE_EARLY_EXIT_AFTER_SURFACE] exiting after Phase 1"
                      << std::endl;
            return {};
        }
    }

    // Phase 2: DetectorAgent with mechanism-first rule instantiation.
    std::cout << "\n[Phase 2: DetectorAgent (mechanism rules, single LLM session)]" << std::endl;
    DetectorAgent detector(llmClient, ccpg);
    DetectorAgent::DetectionResult detectionResult;
    try {
        detectionResult = detector.runDetection(surface);
    } catch (const std::exception& e) {
        std::cerr << "\n[Phase 2 ERROR] LLM API call failed: " << e.what() << std::endl;
        std::cerr << "Vulnerability surface was saved successfully. Re-run when LLM API is available." << std::endl;
        return {};
    }

    confirmedHypotheses_ = std::move(detectionResult.confirmed);

    std::cout << "\n[Phase 2 Complete] " << confirmedHypotheses_.size()
              << " rules grounded by static verification." << std::endl;

    // Log confirmed hypotheses
    fs::path hyp_log_path = TargetPath::getInstance()->getOutputDir() / "confirmed_hypotheses.log";
    std::ofstream hyp_file(hyp_log_path);
    if (hyp_file.is_open()) {
        hyp_file << "========= Mechanism-Rule Detection Results =========\n\n";
        for (const auto& h : confirmedHypotheses_) {
            nlohmann::json hj = h.toJson();
            if (hj.contains("nodes") && hj["nodes"].is_object()) {
                nlohmann::json nodes_with_code;
                for (auto& [role, node_id] : hj["nodes"].items()) {
                    nlohmann::json node_info;
                    node_info["id"] = node_id;
                    CCPGNode* ccpg_node = ccpg->getNodeByID(node_id.get<int>());
                    if (ccpg_node && ccpg_node->getCPGNode()) {
                        node_info["code"] = ccpg_node->getCPGNode()->getCode();
                    } else {
                        node_info["code"] = "[Code not found]";
                    }
                    nodes_with_code[role] = node_info;
                }
                hj["nodes"] = nodes_with_code;
            }
            hyp_file << hj.dump(4) << "\n\n";
        }
        hyp_file.close();
        std::cout << "Hypotheses logged to: " << hyp_log_path << std::endl;
    }

    std::cout << "\n--- Agent-Mode Analysis Finished ---\n" << std::endl;

    // Return empty vector; caller should use getConfirmedHypotheses() instead
    return {};
}

void AgentManager::runAnalysisContractMode(bool useContracts) {
    if (!llmClient || !ccpg) {
        std::cerr << "LLM Client or CCPG not initialized. Aborting analysis." << std::endl;
        return;
    }

    std::cout << "\n--- Starting Thread-Contract Concurrency Analysis (interleaving) --- "
              << "contracts=" << (useContracts ? "on" : "off") << std::endl;

    ThreadCreationTree* tct = ThreadCreationTree::getInstance();

    // Phase 1: vulnerability surface (pure static). Shared with the agent-mode
    // entry; here it scopes BOTH which threads get contracts and which objects
    // get an interleaving session, so the cost is O(shared objects), not O(T^2).
    std::cout << "\n[Phase 1: Generating Vulnerability Surface (static)]" << std::endl;
    query::VulnerabilitySurfaceGenerator surfaceGen(ccpg, tct);
    auto surface = surfaceGen.generate();

    fs::path surface_path = TargetPath::getInstance()->getOutputDir() / "vulnerability_surface.json";
    {
        std::ofstream surface_file(surface_path);
        if (surface_file.is_open()) surface_file << surface.toJson().dump(2);
    }

    std::cout << "\n  Threads: " << surface.total_thread_count
              << ", Conflicting pairs: " << surface.conflicting_pair_count
              << ", Shared objects: " << surface.shared_objects.size() << std::endl;

    if (surface.shared_objects.empty()) {
        std::cout << "No shared objects found. Skipping LLM analysis." << std::endl;
        confirmedHypotheses_.clear();
        return;
    }

    if (const char* early = std::getenv("LACE_EARLY_EXIT_AFTER_SURFACE")) {
        if (early[0] != '\0' && early[0] != '0') {
            std::cout << "[LACE_EARLY_EXIT_AFTER_SURFACE] exiting after Phase 1" << std::endl;
            return;
        }
    }

    // Grounding substrate shared by every per-object session.
    query::HypothesisVerifier verifier(ccpg, tct, HBGraph::getInstance());
    verifier.setSurface(&surface);
    FlowPrior flowPrior = loadFlowPrior();
    if (flowPrior.enabled) {
        std::cout << "  [flow-prior] functions=" << flowPrior.functions.size()
                  << ", files=" << flowPrior.files.size()
                  << " (oracle debug only; soft scoring)" << std::endl;
    }

    // Object triage (one cheap LLM pass): drop objects that cannot carry a real
    // concurrency bug (pure benign statistics counters, opaque unnamed objects)
    // BEFORE the expensive per-cluster sessions. This is the main cost lever (the
    // session count drives both Phase 3 and downstream verification) and doubles
    // as the "don't report benign" filter. Fail-open: keeps all on any failure,
    // and lifecycle carriers (free/list-mutation/self-race) are force-kept.
    // Disable with LACE_DISABLE_OBJECT_TRIAGE=1 (ablation).
    std::set<int> objKeep;
    bool triageOn = true;
    if (const char* d = std::getenv("LACE_DISABLE_OBJECT_TRIAGE"))
        if (d[0] != '\0' && d[0] != '0') triageOn = false;
    if (triageOn && surface.shared_objects.size() > 8) {
        std::cout << "\n[Phase 2.5: Object Triage] candidates="
                  << surface.shared_objects.size() << std::endl;
        ObjectTriageAgent triage(llmClient);
        objKeep = triage.selectObjects(surface);
        std::cout << "  -> keeping " << objKeep.size() << "/"
                  << surface.shared_objects.size() << " objects for interleaving"
                  << std::endl;
    } else {
        for (size_t i = 0; i < surface.shared_objects.size(); ++i)
            objKeep.insert(static_cast<int>(i));
    }

    // Cost-first deterministic surface budget. This is intentionally allowed to
    // lose a little recall: after the LLM/object triage, keep all strong lifecycle
    // carriers, then cap the repeated lower-risk objects globally and per
    // thread-set. It attacks the dataset-wide blow-up (hundreds/thousands of
    // surface objects in SYZBOT cases) before Phase A/B/C spend LLM turns on it.
    applyCostFirstSurfaceBudget(surface, objKeep, flowPrior);

    // Cluster kept objects by their exact accessing-thread-set. Objects touched by
    // the SAME set of threads are co-accessed in the same code paths (e.g. a ring's
    // head/tail/buffer, or a socket's several setsockopt fields), so analyzing them
    // one-at-a-time re-loads the same source and re-derives the same interleaving N
    // times. Each group becomes ONE Phase 3 session: sessions scale with the number
    // of distinct thread-sets (tens) rather than the number of objects (hundreds).
    // No object is dropped, so recall is unaffected. Very large clusters are split
    // into bounded chunks below.
    std::map<std::set<int>, std::vector<const query::SharedObject*>> clusters;
    size_t kept = 0;
    for (size_t oi = 0; oi < surface.shared_objects.size(); ++oi) {
        if (objKeep.find(static_cast<int>(oi)) == objKeep.end()) continue;
        const auto& obj = surface.shared_objects[oi];
        if (obj.accessing_thread_ids.size() < 2 && !obj.is_self_race) continue;
        clusters[obj.accessing_thread_ids].push_back(&obj);
        kept++;
    }

    // Contracts are derived INSIDE each Phase 3 cluster session (folded), not in a
    // separate per-thread pass: a standalone pass re-introduced the clause-emission
    // cost we removed, while a seeded pass captured no new callees. Instead, Phase 3
    // reuses source LAZILY across sessions: the first session that touches a thread
    // reads its callees, and the agent caches those reads so later overlapping
    // clusters preload them rather than re-reading (see InterleavingAnalysisAgent's
    // per-thread read cache). Theoretically the analysis is still per-thread
    // assume/guarantee contracts (analyzeCluster derives them inline); we just pay
    // for each callee's source once instead of once per overlapping cluster.
    if (useContracts) {
        std::cout << "\n[Phase 2: Contracts folded into per-cluster sessions "
                     "(scoped assume/guarantee derived inline; source reused across sessions)]"
                  << std::endl;
    } else {
        std::cout << "\n[Phase 2: SKIPPED — contract ablation off; agent reasons from source]"
                  << std::endl;
    }

    // Split oversized clusters so no single session carries too many fields, then
    // apply a cost-first session budget. Large SYZBOT cases can otherwise leave
    // 100+ distinct thread-set sessions even after object budgeting; keeping the
    // highest-risk chunks is the main large-case speed/recall trade-off.
    constexpr size_t kMaxObjsPerSession = 10;
    SessionList sessions = buildBudgetedSessions(clusters, surface, kMaxObjsPerSession, flowPrior);

    // ----- Gated: static-composition pipeline (Phase A -> B -> C) -----
    // Per-thread contracts, then deterministic requirement-discharge composition
    // over the surface conflicts, then agent calibration ONLY of surviving candidates.
    // Default off: the folded per-cluster path below is unchanged.
    bool staticCompose = false;
    if (const char* e = std::getenv("LACE_STATIC_COMPOSE")) staticCompose = (e[0] && e[0] != '0');
    bool keepLow = false;
    if (const char* e = std::getenv("LACE_COMPOSE_KEEP_LOW")) keepLow = (e[0] && e[0] != '0');

    // w/o LLM ablation: force the compose path even without contracts.
    bool skipPhaseA = false;
    if (const char* e = std::getenv("LACE_SKIP_PHASE_A"))
        if (e[0] && e[0] != '0') skipPhaseA = true;
    const bool enterCompose = staticCompose && (useContracts || skipPhaseA);

    if (enterCompose) {
        std::unordered_map<int, Thread*> threadById;
        for (Thread* t : tct->getThreads()) if (t) threadById[t->getId()] = t;
        std::map<const query::SharedObject*, int> objIndex;
        for (size_t i = 0; i < surface.shared_objects.size(); ++i)
            objIndex[&surface.shared_objects[i]] = static_cast<int>(i);

        std::set<int> tidsInPlay = budgetContractThreads(sessions, surface, tct, threadById, flowPrior);

        std::map<int, LLM::ConcurrencyContract> contractsByTid;
        if (skipPhaseA) {
            std::cout << "\n[Phase A: SKIPPED (ablation: no LLM contracts)]" << std::endl;
        } else {
        // Phase A: one seeded contract per thread (each thread's source read once).
        std::cout << "\n[Phase A: Per-Thread Contracts] threads=" << tidsInPlay.size() << std::endl;
        std::vector<int> tidsVec(tidsInPlay.begin(), tidsInPlay.end());
        const int workers = std::min<int>(contractParallelism(), static_cast<int>(tidsVec.size()));
        std::cout << "  [contract-parallel] workers=" << workers << std::endl;
        std::atomic<size_t> nextTid{0};
        std::mutex contractsMu, coutMu;
        std::vector<std::future<void>> futures;
        futures.reserve(workers);
        for (int w = 0; w < workers; ++w) {
            futures.push_back(std::async(std::launch::async, [&, w]() {
                ContractGeneratorAgent localGenerator(ccpg, llmClient);
                while (true) {
                    size_t idx = nextTid.fetch_add(1);
                    if (idx >= tidsVec.size()) break;
                    int tid = tidsVec[idx];
                    auto itT = threadById.find(tid);
                    if (itT == threadById.end() || !itT->second) continue;
                    // Cap objects per contract: a thread that touches dozens of objects makes
                    // the per-thread contract session explode (one clause/round-trip). Seed
                    // only the top-K by risk (surface is risk_score-sorted desc, so the first
                    // matches ARE the highest risk). Objects beyond K are NOT lost: Phase B's
                    // recall floor is the surface conflict, so they still become candidates
                    // (just without a stated assume -> low/medium tier).
                    constexpr size_t kMaxContractObjs = 20;
                    std::vector<const query::SharedObject*> touched;
                    std::vector<int> ids;
                    for (size_t oi = 0; oi < surface.shared_objects.size(); ++oi) {
                        if (!objKeep.count(static_cast<int>(oi))) continue;
                        const auto& o = surface.shared_objects[oi];
                        if (!o.accessing_thread_ids.count(tid)) continue;
                        if (o.accessing_thread_ids.size() < 2 && !o.is_self_race) continue;
                        touched.push_back(&o);
                        ids.push_back(static_cast<int>(oi));
                        if (touched.size() >= kMaxContractObjs) break;
                    }
                    if (touched.empty()) continue;
                    {
                        std::lock_guard<std::mutex> lock(coutMu);
                        std::cout << "  [contract " << (idx + 1) << "/" << tidsVec.size()
                                  << " w" << w << "] thread " << tid
                                  << " objects=" << touched.size() << std::endl;
                    }
                    auto c = localGenerator.generateContractForThread(itT->second, touched, ids);
                    if (c) {
                        std::lock_guard<std::mutex> lock(contractsMu);
                        contractsByTid.emplace(tid, std::move(*c));
                    }
                }
            }));
        }
        for (auto& f : futures) f.get();
        std::cout << "  [contract-parallel] generated " << contractsByTid.size()
                  << "/" << tidsVec.size() << " contracts" << std::endl;
        if (evalVerbose()) {
            for (const auto& [tid, c] : contractsByTid) {
                // L2 node-anchored dump.
                if (contractL2Enabled() && c.hasL2Content()) {
                    std::cout << "  [contractA-L2] thread " << tid << " role="
                              << (c.role.empty() ? "?" : c.role)
                              << " reqs=" << c.nodeReqs.size()
                              << " guars=" << c.nodeGuars.size() << std::endl;
                    for (const auto& r : c.nodeReqs) {
                        std::cout << "        req " << r.form << " a=[";
                        for (size_t i = 0; i < r.a.size(); ++i) std::cout << (i ? "," : "") << r.a[i];
                        std::cout << "] b=[";
                        for (size_t i = 0; i < r.b.size(); ++i) std::cout << (i ? "," : "") << r.b[i];
                        std::cout << "] obj=" << r.objectId
                                  << (r.note.empty() ? "" : ("  " + oneLine(r.note, 90))) << std::endl;
                    }
                    for (const auto& g : c.nodeGuars) {
                        std::cout << "        guar " << g.form << " a=[";
                        for (size_t i = 0; i < g.a.size(); ++i) std::cout << (i ? "," : "") << g.a[i];
                        std::cout << "] b=[";
                        for (size_t i = 0; i < g.b.size(); ++i) std::cout << (i ? "," : "") << g.b[i];
                        std::cout << "]";
                        if (!g.token.empty()) std::cout << " token=" << g.token;
                        if (!g.mode.empty()) std::cout << " mode=" << g.mode;
                        std::cout << (g.note.empty() ? "" : ("  " + oneLine(g.note, 90))) << std::endl;
                    }
                    continue;
                }
                std::cout << "  [contractA] thread " << tid << " role="
                          << (c.role.empty() ? "?" : c.role)
                          << " clauses=" << c.clauses.size() << std::endl;
                for (const auto& cl : c.clauses) {
                    std::cout << "      resource='" << cl.resource << "' obj=" << cl.objectId;
                    if (!cl.objectIds.empty()) {
                        std::cout << " objs=[";
                        for (size_t i = 0; i < cl.objectIds.size(); ++i)
                            std::cout << (i ? "," : "") << cl.objectIds[i];
                        std::cout << "]";
                    }
                    if (cl.noOrderNeeded)
                        std::cout << " NO_ORDER_NEEDED(" << oneLine(cl.noOrderReason, 80) << ")";
                    std::cout << std::endl;
                    for (const auto& a : cl.assume)
                        std::cout << "        assume " << a.relation << ": "
                                  << oneLine(a.detail, 110) << std::endl;
                    for (const auto& g : cl.guarantee)
                        std::cout << "        guarantee " << g.relation << ": "
                                  << oneLine(g.detail, 110) << std::endl;
                }
            }
        }
        if (contractsByTid.empty() && !tidsVec.empty()) {
            std::cout << "  [contract-parallel] no contracts generated; Phase B will use raw surface conflicts"
                      << std::endl;
        }
        } // end else (!skipPhaseA)

        // ----- L2 (paper-faithful) requirement-driven checker -----
        // Candidates come ONLY from undischarged node-anchored requirements (no
        // bare-surface-conflict floor). Phase C is not wired here yet: this path
        // emits the checker's candidates directly so the deterministic discharge
        // logic can be validated in isolation (e.g. buggy vs fixed CVE).
        if (contractL2Enabled()) {
            const bool skipPhaseC = skipPhaseCEnabled();
            std::cout << "\n[Phase B: L2 requirement-driven checker] sessions="
                      << sessions.size()
                      << (skipPhaseC ? "  (Phase C filter skipped, ablation)"
                                     : "  (+ Phase C strict calibration filter)")
                      << std::endl;
            HBGraph* hb = HBGraph::getInstance();
            l2::Calibrator calibrator(llmClient, ccpg);
            std::vector<query::Hypothesis> composedL2;
            int seq = 0;
            size_t sDone = 0;
            size_t totalCands = 0, totalKept = 0;
            for (auto& [ts, objs] : sessions) {
                ++sDone;
                auto cands = l2::checkRequirements(objs, ts, contractsByTid, objIndex, ccpg, hb);
                totalCands += cands.size();
                if (cands.empty()) continue;
                // Phase C: strict filter (subset of candidates; recall bounded by B).
                std::vector<char> keep;
                if (skipPhaseC) keep.assign(cands.size(), 1);
                else keep = calibrator.review(cands);
                size_t kept = 0;
                for (size_t i = 0; i < cands.size(); ++i) {
                    if (evalVerbose())
                        std::cout << "      [L2 cand " << (keep[i] ? "keep" : "REJECT")
                                  << "] " << cands[i].reason << std::endl;
                    if (!keep[i]) continue;
                    ++kept;
                    composedL2.push_back(l2::toHypothesis(cands[i], ++seq));
                }
                totalKept += kept;
                std::cout << "  [L2 session " << sDone << "/" << sessions.size()
                          << "] undischarged=" << cands.size() << " kept=" << kept
                          << (skipPhaseC ? "" : " (calibrated)") << std::endl;
            }
            std::cout << "  [L2] undischarged_total=" << totalCands
                      << " kept_after_calibration=" << totalKept << std::endl;
            if (dedupEnabled()) {
                size_t before = composedL2.size();
                composedL2 = dedupHypotheses(std::move(composedL2), surface, dedupLevelFromEnv());
                std::cout << "  [dedup] " << before << " -> " << composedL2.size()
                          << " hypotheses (merged near-duplicate root causes)" << std::endl;
            }
            confirmedHypotheses_ = std::move(composedL2);
            std::cout << "\n--- L2 Static-Composition Analysis Finished: "
                      << confirmedHypotheses_.size() << " hypotheses ("
                      << totalCands << " undischarged requirements over "
                      << sessions.size() << " sessions) ---\n" << std::endl;
            return;
        }

        // Phase B (deterministic) + Phase C (calibrate surviving candidates only).
        const bool skipPhaseC = skipPhaseCEnabled();
        const bool noDetDischarge = noDeterministicDischargeEnabled();
        std::string phaseBCLabel = "Phase B";
        if (!skipPhaseC) phaseBCLabel += "/C: Static Composition + Calibration";
        else phaseBCLabel += " only (Phase C skipped, ablation)";
        if (noDetDischarge) phaseBCLabel += " [no deterministic discharge]";
        std::cout << "\n[" << phaseBCLabel << "] sessions=" << sessions.size()
                  << std::endl;
        InterleavingAnalysisAgent calAgent(llmClient, ccpg, tct, &verifier);
        std::vector<query::Hypothesis> composed;
        size_t calibratedUnits = 0;

        // Batched bounded verification (spec §8) is OPT-IN for now. It bounds Phase-C
        // cost by regrouping survivors into #(family x clause) batches, but grouping
        // ACROSS thread-sets can make a calibration dialogue incoherent (threads that
        // do not actually interact + objects from different contexts), which dilutes
        // the interleaving reasoning and dropped confirmations in validation. Keep the
        // coherent per-thread-set path as default until batch grouping is refined to
        // preserve thread-set coherence. Enable with LACE_PHASE_C_BATCH=1.
        const char* batchEnv = std::getenv("LACE_PHASE_C_BATCH");
        const bool batchVerify = batchEnv && batchEnv[0] && batchEnv[0] != '0';

        if (!batchVerify) {
            // Legacy per-thread-set calibration (one dialogue per surviving session).
            size_t sDone = 0;
            int phaseBSeq = 0;
            for (auto& [ts, objs] : sessions) {
                ++sDone;
                int hi = 0, med = 0, low = 0, disc = 0, kc = 0;
                std::vector<PhaseBCandidate> sessionCands;
                std::string verdict = composeVerdict(objs, ts, contractsByTid, objIndex,
                                                     tct, threadById, keepLow,
                                                     hi, med, low, disc, kc,
                                                     skipPhaseC ? &sessionCands : nullptr);
                if (kc == 0) {
                    std::cout << "  [session " << sDone << "/" << sessions.size()
                              << "] no surviving candidate (discharged=" << disc << ") -- skip"
                              << std::endl;
                    continue;
                }
                if (skipPhaseC) {
                    for (const auto& cand : sessionCands)
                        composed.push_back(phaseBToHypothesis(cand, ++phaseBSeq));
                    std::cout << "  [session " << sDone << "/" << sessions.size()
                              << "] Phase-B direct: " << sessionCands.size()
                              << " candidates -> hypotheses (discharged=" << disc << ")"
                              << std::endl;
                    continue;
                }
                int flowScore = sessionFlowPriorScore(objs, flowPrior);
                if (hi == 0 && med > 0 && low == 0 && flowScore == 0 && !calibrateMedOnlyEnabled()) {
                    std::cout << "  [session " << sDone << "/" << sessions.size()
                              << "] med-only candidates=" << med
                              << " (flow_prior=0, discharged=" << disc
                              << ") -- skip C" << std::endl;
                    continue;
                }
                ++calibratedUnits;
                std::map<int, LLM::ConcurrencyContract> sub;
                for (int t : ts) {
                    auto it = contractsByTid.find(t);
                    if (it != contractsByTid.end()) sub.emplace(t, it->second);
                }
                std::cout << "  [session " << sDone << "/" << sessions.size()
                          << "] candidates hi=" << hi << " med=" << med << " low=" << low
                          << " (discharged=" << disc << ") -> calibrate" << std::endl;
                if (evalVerbose() && !verdict.empty())
                    std::cout << "  [composeB survivors]\n" << verdict << std::endl;
                auto hyps = calAgent.analyzeCluster(objs, ts, surface, true, &sub, &verdict);
                for (auto& h : hyps) composed.push_back(std::move(h));
            }
        } else {
            // Batched bounded verification (spec §8). Step 1: compose every
            // object-chunk deterministically, then MERGE chunks back by the exact
            // thread-set before Phase C. This preserves the interleaving context
            // (all candidates in one verification dialogue share the same threads)
            // while removing the artificial session inflation caused by splitting a
            // large thread-set into maxObjsPerSession-sized chunks.
            std::map<std::set<int>, std::vector<PhaseBCandidate>> byThreadSet;
            int totalDischarged = 0;
            int totalSurvivors = 0;
            for (auto& [ts, objs] : sessions) {
                int hi = 0, med = 0, low = 0, disc = 0, kc = 0;
                std::vector<PhaseBCandidate> sel;
                composeVerdict(objs, ts, contractsByTid, objIndex, tct, threadById,
                               keepLow, hi, med, low, disc, kc, &sel);
                totalDischarged += disc;
                totalSurvivors += static_cast<int>(sel.size());
                auto& bucket = byThreadSet[ts];
                for (auto& c : sel) bucket.push_back(c);
            }

            // Dedup inside each coherent thread-set by (object, clause, anchor).
            // We intentionally do NOT dedup across different thread-sets: the same
            // object/anchor can be a different interleaving witness under a different
            // concurrent context.
            int totalUnique = 0;
            std::vector<std::pair<std::set<int>, std::vector<PhaseBCandidate>>> ordered;
            for (auto& [ts, gcands] : byThreadSet) {
                std::set<std::string> seenKey;
                std::vector<PhaseBCandidate> uniq;
                for (auto& c : gcands) {
                    std::string k = std::to_string(c.objectId) + "|" +
                                    phaseBHazardClass(c.label) + "|" + anchorKey(c.anchor);
                    if (seenKey.insert(k).second) uniq.push_back(c);
                }
                if (uniq.empty()) continue;
                totalUnique += static_cast<int>(uniq.size());
                ordered.emplace_back(ts, std::move(uniq));
            }
            auto groupRank = [](const std::vector<PhaseBCandidate>& g) {
                int r = 0; for (auto& c : g) r = std::max(r, tierRank(c.tier)); return r;
            };
            std::stable_sort(ordered.begin(), ordered.end(),
                [&](const auto& a, const auto& b) {
                    int ra = groupRank(a.second), rb = groupRank(b.second);
                    if (ra != rb) return ra > rb;
                    if (a.second.size() != b.second.size()) return a.second.size() > b.second.size();
                    return a.first.size() < b.first.size();
                });

            if (skipPhaseC) {
                int phaseBSeq = 0;
                for (auto& [gts, gcands] : ordered)
                    for (auto& c : gcands)
                        composed.push_back(phaseBToHypothesis(c, ++phaseBSeq));
                std::cout << "  [phase-B batch, Phase C skipped] total=" << totalUnique
                          << " candidates -> hypotheses (discharged=" << totalDischarged << ")"
                          << std::endl;
            } else {
            const int maxBatches = envInt("LACE_PHASE_C_MAX_BATCHES", 60);
            const int batchSize = std::max(1, envInt("LACE_PHASE_C_BATCH_SIZE", 12));
            std::cout << "  [phase-C batch] survivors=" << totalSurvivors
                      << " unique=" << totalUnique
                      << " threadset_groups=" << ordered.size()
                      << " (discharged_total=" << totalDischarged
                      << ", batch_size=" << batchSize << ", max_batches=" << maxBatches
                      << ")" << std::endl;

            int batchNo = 0;
            bool capHit = false;
            for (auto& [gts, gcands] : ordered) {
                if (batchNo >= maxBatches) { capHit = true; break; }
                for (size_t off = 0; off < gcands.size();
                     off += static_cast<size_t>(batchSize)) {
                    if (batchNo >= maxBatches) { capHit = true; break; }
                    std::vector<PhaseBCandidate> chunk(
                        gcands.begin() + off,
                        gcands.begin() + std::min(gcands.size(),
                                                  off + static_cast<size_t>(batchSize)));
                    std::vector<const query::SharedObject*> gobjs;
                    std::set<const query::SharedObject*> seenObj;
                    int chi = 0, cmed = 0, clow = 0;
                    for (auto& c : chunk) {
                        if (c.object && seenObj.insert(c.object).second) gobjs.push_back(c.object);
                        if (c.tier == "high") ++chi;
                        else if (c.tier == "medium") ++cmed; else ++clow;
                    }
                    std::stringstream key;
                    key << "threads={";
                    { bool first = true; for (int t : gts) { key << (first ? "" : ",") << t; first = false; } }
                    key << "}";
                    int gFlow = sessionFlowPriorScore(gobjs, flowPrior);
                    if (chi == 0 && cmed > 0 && clow == 0 && gFlow == 0 &&
                        !calibrateMedOnlyEnabled()) {
                        std::cout << "  [batch] " << key.str() << " med-only=" << cmed
                                  << " -- skip C" << std::endl;
                        continue;
                    }
                    ++batchNo;
                    std::map<int, LLM::ConcurrencyContract> sub;
                    for (int t : gts) {
                        auto it = contractsByTid.find(t);
                        if (it != contractsByTid.end()) sub.emplace(t, it->second);
                    }
                    std::string verdict = renderBatchVerdict(chunk);
                    std::cout << "  [batch " << batchNo << "/<=" << maxBatches << "] " << key.str()
                              << " cand(hi=" << chi << ",med=" << cmed << ",low=" << clow
                              << ") objs=" << gobjs.size() << " threads=" << gts.size()
                              << " -> calibrate" << std::endl;
                    auto hyps = calAgent.analyzeCluster(gobjs, gts, surface, true, &sub, &verdict);
                    for (auto& h : hyps) composed.push_back(std::move(h));
                }
            }
            if (capHit)
                std::cout << "  [phase-C batch] max_batches=" << maxBatches
                          << " reached; remaining groups deferred" << std::endl;
            calibratedUnits = static_cast<size_t>(batchNo);
            } // end else (!skipPhaseC) for batch path
        }

        if (dedupEnabled()) {
            size_t before = composed.size();
            composed = dedupHypotheses(std::move(composed), surface, dedupLevelFromEnv());
            std::cout << "  [dedup] " << before << " -> " << composed.size()
                      << " hypotheses (merged near-duplicate root causes)" << std::endl;
        }
        confirmedHypotheses_ = std::move(composed);
        std::cout << "\n--- Static-Composition Analysis Finished: "
                  << confirmedHypotheses_.size() << " hypotheses ("
                  << (batchVerify ? "batched " : "")
                  << "calibrated " << calibratedUnits
                  << (batchVerify ? " batches" : " sessions") << " over "
                  << sessions.size() << " sessions) ---\n" << std::endl;
        return;
    }

    std::cout << "\n[Phase 3: Per-Cluster Interleaving Analysis] objects=" << kept
              << " thread-sets=" << clusters.size()
              << " sessions=" << sessions.size() << std::endl;
    InterleavingAnalysisAgent agent(llmClient, ccpg, tct, &verifier);
    std::vector<query::Hypothesis> all;
    size_t done = 0;
    for (auto& [ts, objs] : sessions) {
        done++;

        size_t naccesses = 0;
        for (const auto* o : objs) naccesses += o->accesses.size();
        std::cout << "  [cluster " << done << "/" << sessions.size() << "] threads={";
        { bool f = true; for (int t : ts) { std::cout << (f ? "" : ",") << t; f = false; } }
        std::cout << "} objects=" << objs.size()
                  << " accesses=" << naccesses << std::endl;

        auto hyps = agent.analyzeCluster(objs, ts, surface, useContracts);
        for (auto& h : hyps) all.push_back(std::move(h));
    }

    if (dedupEnabled()) {
        size_t before = all.size();
        all = dedupHypotheses(std::move(all), surface, dedupLevelFromEnv());
        std::cout << "  [dedup] " << before << " -> " << all.size()
                  << " hypotheses (merged near-duplicate root causes)" << std::endl;
    }
    confirmedHypotheses_ = std::move(all);
    std::cout << "\n--- Thread-Contract Analysis Finished: " << confirmedHypotheses_.size()
              << " hypotheses ---\n" << std::endl;
}

std::vector<llm_client::ThreadPair> AgentManager::runAnalysis() {
    return runAnalysisLegacy();
}

std::vector<llm_client::ThreadPair> AgentManager::runAnalysisLegacy() {
    if (!llmClient || !ccpg) {
        std::cerr << "LLM Client or CCPG not initialized. Aborting analysis.";
        return {};
    }

    std::cout << "\n--- Starting Legacy LLM-based Concurrency Analysis ---" << std::endl;

    ThreadCreationTree* tct = ThreadCreationTree::getInstance();
    std::unordered_set<Thread*> threads = tct->getThreads();
    
    const auto candidateSharedObjects = tct->collectCandidateSharedObjects();

    std::cout << "\n[Phase 1: Generating Concurrency Contracts]" << std::endl;
    std::map<Thread*, LLM::ConcurrencyContract> contractMap;
    for(Thread* thread : threads) {
        int entryFuncId = thread->getThreadMainFunction() ? thread->getThreadMainFunction()->getId() : -1;
        if (entryFuncId != -1) {
            std::cout << "Analyzing thread " << thread->getId() << " (Entry Function ID: " << entryFuncId << ")" << std::endl;
            auto contractOpt = contractGenerator.generateContractForThread(thread);
            if (contractOpt) {
                std::cout << "  -> Successfully generated contract." << std::endl;
                contractMap.emplace(thread, std::move(contractOpt.value()));
            } else {
                std::cerr << "  -> Failed to generate contract." << std::endl;
            }
        } else {
            std::cerr << "Could not determine thread entry for fork site " << thread->getForkNode()->getId() << std::endl;
        }
    }

    std::cout << "\n[Phase 2: Analyzing Thread Pairs (LLM-based filtering)]" << std::endl;
    std::vector<ThreadPair> analysisResults;
    std::vector<Thread*> thread_vec(threads.begin(), threads.end());

    int total_pairs = thread_vec.size() * (thread_vec.size() - 1) / 2;
    int current_pair = 0;
    int skipped_by_llm = 0;
    
    int skipped_by_static = 0;
    for (size_t i = 0; i < thread_vec.size(); ++i) {
        for (size_t j = i + 1; j < thread_vec.size(); ++j) {
            Thread* thread1 = thread_vec[i];
            Thread* thread2 = thread_vec[j];

            if (!tct->mayHappenInParallel(thread1, thread2)) {
                skipped_by_static++;
                continue;
            }

            auto it1 = contractMap.find(thread1);
            auto it2 = contractMap.find(thread2);

            if (it1 != contractMap.end() && it2 != contractMap.end()) {
                current_pair++;
                std::cout << "Analyzing pair [" << current_pair << "/" << total_pairs << "]: "
                          << "Thread " << thread1->getId() << " and Thread " << thread2->getId() << std::endl;
                
                ThreadPair pair(thread1, it1->second, thread2, it2->second);
                parallelAnalyzer.analyze_parallelism(pair);
                
                std::cout << "  - Designed for Parallelism: " << (pair.analysis.designed_for_parallelism ? "Yes" : "No") << std::endl;
                std::cout << "    Reasoning: " << pair.analysis.design_reasoning << std::endl;
                std::cout << "  - Actually Concurrent: " << (pair.analysis.actually_concurrent ? "Yes" : "No") << std::endl;
                std::cout << "    Reasoning: " << pair.analysis.concurrency_reasoning << std::endl;

                if (!pair.analysis.actually_concurrent) {
                    skipped_by_llm++;
                    std::cout << "  -> Skipped by LLM (no concurrency risk identified)" << std::endl;
                    continue;
                }

                analysisResults.push_back(std::move(pair));
            }
        }
    }
    
    std::cout << "  -> Total pairs: " << total_pairs 
              << ", Skipped by static analysis: " << skipped_by_static
              << ", Skipped by LLM: " << skipped_by_llm 
              << ", Proceeding with: " << analysisResults.size() << std::endl;

    fs::path rules_log_path = TargetPath::getInstance()->getOutputDir() / "temporal_rules.log";
    std::ofstream rules_file(rules_log_path);
    if (rules_file.is_open()) {
        rules_file << "========= Generated Temporal Ordering Rules =========\n\n";
        int pair_count = 1;
        for (const auto& pair : analysisResults) {
            rules_file << "--- Pair " << pair_count++ << ": Thread " << pair.thread1->getId() 
                    << " vs Thread " << pair.thread2->getId() << " ---\n";
            if (pair.analysis.temporal_rules.empty()) {
                rules_file << "  No rules generated for this pair.\n\n";
            } else {
                for (const auto& rule_ptr : pair.analysis.temporal_rules) {
                    nlohmann::json rule_json = rule_ptr->to_json();
                    if (rule_json.contains("nodes") && rule_json["nodes"].is_object()) {
                        nlohmann::json nodes_with_code;
                        for (auto& [role, node_id] : rule_json["nodes"].items()) {
                            nlohmann::json node_info;
                            node_info["id"] = node_id;
                            CCPGNode* ccpg_node = ccpg->getNodeByID(node_id.get<int>());
                            if (ccpg_node && ccpg_node->getCPGNode()) {
                                node_info["code"] = ccpg_node->getCPGNode()->getCode();
                            } else {
                                node_info["code"] = "[Code not found for this node ID]";
                            }
                            nodes_with_code[role] = node_info;
                        }
                        rule_json["nodes"] = nodes_with_code;
                    }
                    rules_file << rule_json.dump(4) << "\n\n"; 
                }
            }
        }
        rules_file.close();
        std::cout << "Temporal rules have been logged to: " << rules_log_path << std::endl;
    }

    std::cout << "\n--- Legacy LLM-based Analysis Finished ---\n" << std::endl;
    return analysisResults;
}

} // namespace llm_client
