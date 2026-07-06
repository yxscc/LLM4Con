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

bool trustHardNonLockDischarge() {
    if (const char* e = std::getenv("LACE_TRUST_HARD_NONLOCK_DISCHARGE"))
        return e[0] && e[0] != '0';
    return false;
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
        for (size_t i = 0; i < tids.size(); ++i) {
            for (size_t j = i + 1; j < tids.size(); ++j) {
                int t1 = tids[i], t2 = tids[j];
                auto T1 = threadById.find(t1), T2 = threadById.find(t2);
                if (T1 != threadById.end() && T2 != threadById.end() && T1->second && T2->second &&
                    !tct->mayHappenInParallel(T1->second, T2->second))
                    continue;
                AccKind k1 = threadAccessKind(*O, t1), k2 = threadAccessKind(*O, t2);
                bool t1touch = k1.read || k1.write || k1.free;
                bool t2touch = k2.read || k2.write || k2.free;
                bool conflict = t1touch && t2touch && (k1.write || k1.free || k2.write || k2.free);
                if (!conflict) continue;

                const OrderClause* clA = contractsByTid.count(t1)
                    ? clauseForObject(contractsByTid.at(t1), oi, *O) : nullptr;
                const OrderClause* clB = contractsByTid.count(t2)
                    ? clauseForObject(contractsByTid.at(t2), oi, *O) : nullptr;

                // Deterministic discharge:
                //  (a) lock path: surface common-lock AND a stated guarantee -- the
                //      conservative AND so a vague EXCLUDE alone never drops a bug;
                //  (b) hard non-lock path: a stated RCU/refcount/barrier/join/RMW
                //      guarantee is only auto-trusted when explicitly enabled. The
                //      default sends it to Phase C because the checker does not yet
                //      prove endpoint alignment (e.g. drain-before-free vs drain-after-free).
                bool lockDischarge = surfaceSharedLock(*O, t1, t2) &&
                                     (establishesOrder(clA) || establishesOrder(clB));
                bool hardDischarge = establishesHardNonLockOrder(clA) ||
                                     establishesHardNonLockOrder(clB);
                // lockDischarge stays authoritative: it needs EVERY access under a
                // shared surface lock, a structural fact, not a bare assertion. Hard
                // non-lock guarantees need endpoint-aware composition, so keep them for
                // Phase C unless LACE_TRUST_HARD_NONLOCK_DISCHARGE=1 is set for an
                // ablation/speed run.
                const bool keepUnverifiedHard = hardDischarge &&
                    !trustHardNonLockDischarge() && !lockDischarge;
                if (lockDischarge || (hardDischarge && !keepUnverifiedHard)) {
                    ++dis;
                    continue;
                }

                auto hA = hazardTier(clA, k2);  // t1 requires, t2 violates
                auto hB = hazardTier(clB, k1);  // t2 requires, t1 violates
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

    if (staticCompose && useContracts) {
        std::unordered_map<int, Thread*> threadById;
        for (Thread* t : tct->getThreads()) if (t) threadById[t->getId()] = t;
        std::map<const query::SharedObject*, int> objIndex;
        for (size_t i = 0; i < surface.shared_objects.size(); ++i)
            objIndex[&surface.shared_objects[i]] = static_cast<int>(i);

        std::set<int> tidsInPlay = budgetContractThreads(sessions, surface, tct, threadById, flowPrior);

        // Phase A: one seeded contract per thread (each thread's source read once).
        std::cout << "\n[Phase A: Per-Thread Contracts] threads=" << tidsInPlay.size() << std::endl;
        std::map<int, LLM::ConcurrencyContract> contractsByTid;
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
        if (contractsByTid.empty() && !tidsVec.empty()) {
            std::cout << "  [contract-parallel] no contracts generated; Phase B will use raw surface conflicts"
                      << std::endl;
        }

        // Phase B (deterministic) + Phase C (calibrate surviving candidates only).
        std::cout << "\n[Phase B/C: Static Composition + Calibration] sessions=" << sessions.size()
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
            for (auto& [ts, objs] : sessions) {
                ++sDone;
                int hi = 0, med = 0, low = 0, disc = 0, kc = 0;
                std::string verdict = composeVerdict(objs, ts, contractsByTid, objIndex,
                                                     tct, threadById, keepLow,
                                                     hi, med, low, disc, kc);
                if (kc == 0) {
                    std::cout << "  [session " << sDone << "/" << sessions.size()
                              << "] no surviving candidate (discharged=" << disc << ") -- skip"
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
                // Step 3: one capped dialogue per candidate chunk inside the SAME
                // thread-set. Families/clauses remain visible in each candidate label
                // and object name, but no cross-thread-set mixing is allowed.
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
