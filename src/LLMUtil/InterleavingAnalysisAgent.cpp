#include "LLMUtil/InterleavingAnalysisAgent.h"
#include "LLMUtil/SharedToolKit.h"
#include "Query/SourceRefResolver.h"
#include "CCPG/CCPGNode.h"
#include "CPG/Node.h"

#include <algorithm>
#include <iostream>
#include <sstream>

namespace llm_client {

namespace {

// Compact, prompt-friendly rendering of one thread's contract: the §4.6 order/sync
// content (assume/guarantee per resource) is what drives the mismatch reasoning;
// the legacy descriptive fields are rendered only as a fallback.
std::string contractToText(const LLM::ConcurrencyContract& c) {
    std::stringstream ss;
    ss << "  - thread " << c.threadId << " role=" << (c.role.empty() ? "?" : c.role) << "\n";
    if (!c.summary.empty()) ss << "    summary: " << c.summary << "\n";

    for (const auto& cl : c.clauses) {
        ss << "    resource " << (cl.resource.empty() ? "?" : cl.resource) << ":\n";
        ss << "      requires(assume):";
        if (cl.assume.empty()) ss << " (none)";
        else for (const auto& r : cl.assume) ss << " " << r.detail;
        ss << "\n      establishes(guarantee):";
        if (cl.guarantee.empty()) ss << " (none)";
        else for (const auto& g : cl.guarantee) ss << " " << g.detail;
        ss << "\n";
    }
    if (!c.ordering.empty()) {
        ss << "    thread-order:";
        for (const auto& o : c.ordering) ss << " {" << o << "}";
        ss << "\n";
    }

    // Fallback: legacy descriptive contract (no order/sync clauses present).
    if (c.clauses.empty()) {
        if (!c.sharedVariables.empty()) {
            ss << "    shared vars:";
            for (const auto& v : c.sharedVariables) {
                ss << " " << v.variableName << "(" << v.accessType;
                if (!v.protectingPrimitives.empty()) {
                    ss << ",lock=";
                    for (size_t i = 0; i < v.protectingPrimitives.size(); ++i)
                        ss << (i ? "/" : "") << v.protectingPrimitives[i];
                }
                ss << ")";
            }
            ss << "\n";
        }
        if (!c.synchronization.primitives.empty()) {
            ss << "    sync:";
            for (const auto& p : c.synchronization.primitives) ss << " " << p.identifier;
            ss << "\n";
        }
    }
    return ss.str();
}

std::string riskFlags(const query::SharedObject& o) {
    std::stringstream ss;
    if (o.has_unprotected_write)        ss << " unprotected_write";
    if (o.has_free_operation)           ss << " free";
    if (o.has_cross_thread_rw)          ss << " cross_thread_rw";
    if (o.has_inconsistent_locking)     ss << " inconsistent_locking";
    if (o.has_scalar_torn_access)       ss << " scalar_torn";
    if (o.has_list_mutation)            ss << " list_mutation";
    if (o.is_self_race)                 ss << " self_race";
    std::string s = ss.str();
    return s.empty() ? " (none)" : s;
}

} // namespace

InterleavingAnalysisAgent::InterleavingAnalysisAgent(
    std::shared_ptr<LLMClient> client, CCPG* ccpg, ThreadCreationTree* tct,
    query::HypothesisVerifier* verifier)
    : Conversation(client, "", /*max_history=*/30),
      ccpg_(ccpg), tct_(tct), verifier_(verifier) {
    set_system_prompt(build_system_prompt());
}

std::string InterleavingAnalysisAgent::build_system_prompt() {
    return R"(You are an expert in concurrent Linux-kernel C code. You are given a pair of
concurrent threads (or one self-racing thread) and the shared object(s) they both touch,
plus each thread's "concurrency contract": for those objects, the ORDER it requires
(assume) and the order it establishes via synchronization (guarantee). Your job is to find
whether the threads can INTERLEAVE so that a required order is broken, and to propose
grounded hypotheses. A single bug may span MORE THAN ONE object (e.g. a free of object A
and a dangling use reached via object B) -- reason across all the listed objects together.

THE SINGLE MISMATCH (how a bug emerges -- no pattern catalog):
A real bug exists when, for some required order R0:
  (1) one thread REQUIRES R0 on this object (an `assume`: prec / atomic / count_guarded);
  (2) another thread has an event that VIOLATES R0 (e.g. a free before the use; a
      conflicting write inside the atomic region; a use before init); AND
  (3) NO thread's `guarantee` establishes R0 (no serialize/order/counts covers it --
      or the lock that looks relevant guards a different field / is dropped first).
Then confirm the interleaving is FEASIBLE by grounding (concurrent AND not-hb AND
not-lock-protected AND conflicts). If (3) fails -- some guarantee really does establish
R0 -- it is benign; do not report.

The bug category is just a LABEL for which R0 broke (it EMERGES, you do not pick a
template): prec(use,free)->use_after_free, prec(init,use)->uninitialized_read/publish,
atomic(region)->data_race/atomicity_violation, count_guarded->refcount/double_free.
`bug_category` is FREE-FORM; do not force a fixed template.

Use the contracts to spot the candidate R0 and the missing guarantee; if no contracts
are provided, infer the required order directly from the source and accesses.

You may read source freely:
- `get_function_by_name(name)`  -> full function body (PREFERRED lens).
- `read_thread_entry(thread_id)` -> a thread's entry function source.
- `get_callees` / `get_callers` / `get_function_ops` -> navigation / node-level fallback.

GROUNDING: when you have a concrete racing interleaving, call `propose_race_hypothesis`.
You name the racing SITES (give each a `role`) and the SITES you locate either by:
  - `access_index`: the [A#] index from the object's listed accesses (PREFERRED — these
    are already grounded), or
  - `node_id`: an explicit CCPG node id (e.g. from get_function_ops), or
  - `file`/`line`/`symbol`/`snippet`: a source reference we will resolve.
Then give `constraints` over those roles using this static predicate vocabulary
(node refs are the role names you defined, or integer node ids):
  - concurrent {a,b}            : a and b may run concurrently (MHP)
  - conflicts {a,b}             : a,b touch the same location and >=1 writes
  - same_location {a,b}         : a,b access the same memory location
  - op_kind {node,kind}         : kind in READ|WRITE|RMW|CALL
  - not_lock_protected {node}   : node is not inside a protecting lock region
  - same_lock {node1,node2}     : both held under the same lock (use to REFUTE)
  - reachable {from,to}         : control-flow reachable
  - hb {a,b,expected}           : a happens-before b (expected=false asserts NO hb)
  - in_thread {node,thread}     : node executes in thread id
A typical data race: roles writer+reader, constraints concurrent{writer,reader} +
conflicts{writer,reader} + not_lock_protected{writer}. A UAF: roles free+use,
concurrent{free,use} + same_location{free,use} + hb{free,use,expected:false}.
The verifier grounds your proposal and returns per-predicate feedback; if it does not
hold, gather more evidence or refine — do not restate the same failing proposal.

VALIDITY (reject benign / HB-ordered races BEFORE proposing):
  - callback enabled only AFTER registration (request_irq/register_*/queue_work/
    wake_up_process AFTER the writes) -> writes happen-before callback, NOT racing.
  - construction-before-publication (build in local, publish via store/rcu_assign_pointer/
    WRITE_ONCE at the end) -> construction stores are not racing.
  - module_init/_probe writes complete-before any syscall/ioctl/fileops invocation.
  - freeze/quiesce/synchronize_rcu/synchronize_irq before publish/free drains readers.
  - kthread_stop/cancel_work_sync/flush_work before free waits for the user to exit.
  - refcount-protected lookup (kref_/refcount_ get before use, put before free) -> not UAF.

BENIGN (do not report): a pure statistics/diagnostic counter (packet/byte counters,
*_stats, *_dropped) whose racy value never flows into a pointer/index/size/branch/lifetime
decision is a benign lost-update -- skip it.

When you have proposed every real bug (or there is none) for these threads, call
`finish_analysis`. Propose only bugs you are confident are malignant; precision matters.)";
}

std::vector<Tool> InterleavingAnalysisAgent::get_available_tools() const {
    auto tools = SharedToolKit::get_shared_tools();

    tools.push_back({"read_thread_entry",
        "Get a thread's entry function name and source body.", {
        {"thread_id", "number", "The thread id to inspect.", true}
    }});

    nlohmann::json sites_schema = {
        {"type", "array"},
        {"description", "The racing access sites. Give each a semantic role."},
        {"items", {
            {"type", "object"},
            {"properties", {
                {"role", {{"type", "string"}, {"description", "semantic role, e.g. writer/reader/free/use/guard/publish"}}},
                {"access_index", {{"type", "integer"}, {"description", "index into the listed accesses [A#] (preferred)"}}},
                {"node_id", {{"type", "integer"}, {"description", "explicit CCPG node id"}}},
                {"file", {{"type", "string"}}},
                {"line", {{"type", "integer"}}},
                {"symbol", {{"type", "string"}, {"description", "enclosing function name (for source-ref grounding)"}}},
                {"snippet", {{"type", "string"}, {"description", "a substring of the racing line"}}}
            }},
            {"required", {"role"}}
        }}
    };
    nlohmann::json constraints_schema = {
        {"type", "array"},
        {"description", "Static predicates over the site roles that must all hold."},
        {"items", {
            {"type", "object"},
            {"properties", {
                {"predicate", {{"type", "string"}, {"description", "concurrent|conflicts|same_location|op_kind|not_lock_protected|same_lock|reachable|hb|in_thread"}}},
                {"args", {{"type", "object"}, {"description", "predicate args; node refs are role names or node ids"}}}
            }},
            {"required", {"predicate"}}
        }}
    };

    std::vector<Parameter> propose_params;
    propose_params.emplace_back("description", "string", "Natural-language description of the harmful interleaving.", true);
    propose_params.emplace_back("consequence", "string",
        "Optional: brief note on the harmful outcome if useful for the report.", false);
    propose_params.emplace_back("bug_category", "string", "Free-form bug category (emergent, not a template).", true);
    propose_params.emplace_back("severity", "string", "low|medium|high (optional).", false);
    propose_params.emplace_back("racing_sites", std::move(sites_schema), true);
    propose_params.emplace_back("constraints", std::move(constraints_schema), false);
    tools.push_back({"propose_race_hypothesis",
        "Propose one grounded race hypothesis; it is verified against the static model "
        "and recorded only if all constraints hold.", std::move(propose_params)});

    tools.push_back({"finish_analysis", "Call when no more real bugs remain for these threads.", {}});
    return tools;
}

int InterleavingAnalysisAgent::resolveRoleNode(const nlohmann::json& site,
                                               std::string& err) const {
    auto* ctx = static_cast<InterleavingContext*>(this->get_context_for_tools());
    // 1) access_index into the session's flat [A#] access list (already grounded).
    if (site.contains("access_index") && site["access_index"].is_number_integer()) {
        int idx = site["access_index"].get<int>();
        if (ctx && idx >= 0 && idx < static_cast<int>(ctx->accesses.size()) &&
            ctx->accesses[idx]) {
            int nid = ctx->accesses[idx]->node_id;
            if (nid >= 0 && ccpg_->getNodeByID(nid)) return nid;
            err = "access_index " + std::to_string(idx) + " has no usable node_id";
            return -1;
        }
        err = "access_index " + std::to_string(idx) + " out of range";
        return -1;
    }
    // 2) explicit node id.
    if (site.contains("node_id") && site["node_id"].is_number_integer()) {
        int nid = site["node_id"].get<int>();
        if (ccpg_->getNodeByID(nid)) return nid;
        err = "node_id " + std::to_string(nid) + " not found in CCPG";
        return -1;
    }
    // 3) source-ref (file/line/symbol/snippet) -> resolver.
    query::SourceRef ref;
    if (site.contains("file") && site["file"].is_string()) ref.file = site["file"].get<std::string>();
    if (site.contains("line") && site["line"].is_number_integer()) ref.line = site["line"].get<int>();
    if (site.contains("symbol") && site["symbol"].is_string()) ref.symbol = site["symbol"].get<std::string>();
    if (site.contains("snippet") && site["snippet"].is_string()) ref.snippet = site["snippet"].get<std::string>();
    int nid = query::resolveSourceRef(ccpg_, ref);
    if (nid >= 0) return nid;
    err = "could not ground site (provide access_index, node_id, or a clearer file/line/symbol/snippet)";
    return -1;
}

std::string InterleavingAnalysisAgent::execute_tool(const std::string& tool_name,
                                                    const nlohmann::json& arguments) {
    // Read/navigation tools (shared toolkit + read_thread_entry) are subject to the
    // per-session exploration budget; reporting tools below are never capped.
    auto applyBudget = [&](const std::string& result) -> std::string {
        explore_calls_++;
        if (explore_calls_ > exploreHard_) return "finish";
        if (explore_calls_ > exploreSoft_) {
            nlohmann::json parsed = nlohmann::json::parse(result, nullptr, false);
            nlohmann::json out;
            out["result"] = parsed.is_discarded() ? nlohmann::json(result) : parsed;
            out["budget_notice"] =
                "Exploration budget nearly exhausted (" + std::to_string(explore_calls_) + "/" +
                std::to_string(exploreHard_) + " reads). STOP reading new functions. "
                "Propose any real bugs you have found with propose_race_hypothesis now, then "
                "call finish_analysis.";
            return out.dump();
        }
        return result;
    };

    // Cross-session cache: attribute every function the model reads by name to the
    // threads under analysis, so a later overlapping session can preload it.
    if (tool_name == "get_function_by_name" && arguments.contains("name") &&
        arguments["name"].is_string()) {
        const std::string nm = arguments["name"].get<std::string>();
        if (!nm.empty()) for (int t : curThreads_) threadReadFuncs_[t].insert(nm);
    }

    auto shared = SharedToolKit::handle_shared_tool(tool_name, arguments, ccpg_);
    if (shared) return applyBudget(*shared);

    auto* ctx = static_cast<InterleavingContext*>(this->get_context_for_tools());
    if (!ctx) return R"({"error":"Internal context error."})";
    ctx->total_tool_calls++;

    if (tool_name == "read_thread_entry") {
        if (!arguments.contains("thread_id") || !arguments["thread_id"].is_number_integer())
            return R"({"error":"missing integer thread_id"})";
        int tid = arguments["thread_id"].get<int>();
        for (Thread* t : tct_->getThreads()) {
            if (!t || t->getId() != tid) continue;
            ccpg::Function* mf = t->getThreadMainFunction();
            if (!mf || !mf->getFuncNode() || !mf->getFuncNode()->getCPGNode())
                return R"({"error":"thread has no resolved entry function"})";
            nlohmann::json out = {
                {"thread_id", tid},
                {"entry_function", mf->getFuncNode()->getCPGNode()->getName()},
                {"source", mf->getFuncNode()->getCPGNode()->getCode()}
            };
            return applyBudget(out.dump());
        }
        return R"({"error":"thread_id not found"})";
    }

    if (tool_name == "finish_analysis") {
        return "finish";
    }

    if (tool_name == "propose_race_hypothesis") {
        if (!verifier_) return R"({"error":"Verifier not initialized."})";
        if (!arguments.contains("racing_sites") || !arguments["racing_sites"].is_array() ||
            arguments["racing_sites"].size() < 1) {
            return R"({"error":"racing_sites must be a non-empty array."})";
        }

        query::Hypothesis h;
        h.description = arguments.value("description", std::string());
        std::string consequence = arguments.value("consequence", std::string());
        if (!consequence.empty()) {
            if (!h.description.empty()) h.description += "  ";
            h.description += "[consequence] " + consequence;
        }
        h.bug_category = arguments.value("bug_category", std::string("data_race"));
        h.severity = arguments.value("severity", std::string("medium"));
        h.id = "interleave_" + std::to_string(ctx->confirmed.size() + 1);

        // Bind roles -> node ids.
        std::vector<std::string> orderedRoles;
        for (const auto& site : arguments["racing_sites"]) {
            if (!site.contains("role") || !site["role"].is_string())
                return R"({"error":"each racing site needs a string 'role'."})";
            std::string role = site["role"].get<std::string>();
            std::string err;
            int nid = resolveRoleNode(site, err);
            if (nid < 0) {
                nlohmann::json e = {{"error", "could not ground role '" + role + "': " + err}};
                return e.dump();
            }
            h.nodes[role] = nid;
            orderedRoles.push_back(role);
        }

        // Parse constraints; auto-augment with a minimal grounding when the
        // model gave none, so every proposal is actually verified.
        if (arguments.contains("constraints") && arguments["constraints"].is_array()) {
            for (const auto& c : arguments["constraints"]) {
                if (!c.contains("predicate") || !c["predicate"].is_string()) continue;
                query::VerificationConstraint vc;
                vc.predicate = c["predicate"].get<std::string>();
                vc.args = c.contains("args") && c["args"].is_object() ? c["args"] : nlohmann::json::object();
                h.constraints.push_back(std::move(vc));
            }
        }
        if (h.constraints.empty() && orderedRoles.size() >= 2) {
            const std::string& a = orderedRoles[0];
            const std::string& b = orderedRoles[1];
            h.constraints.push_back({"concurrent", {{"a", a}, {"b", b}}});
            h.constraints.push_back({"conflicts", {{"a", a}, {"b", b}}});
        }
        if (h.constraints.empty()) {
            return R"({"error":"provide at least one constraint, or two racing sites for auto-grounding."})";
        }

        query::VerificationResult vr = verifier_->verify(h);
        nlohmann::json feedback;
        feedback["bug_category"] = h.bug_category;
        feedback["all_satisfied"] = vr.all_satisfied;
        feedback["grounding"] = vr.toFeedbackJson();

        if (vr.all_satisfied) {
            std::vector<int> nodeIds;
            for (const auto& [role, nid] : h.nodes) nodeIds.push_back(nid);
            std::sort(nodeIds.begin(), nodeIds.end());
            std::string fp = h.bug_category + "|";
            for (int nid : nodeIds) fp += std::to_string(nid) + ",";
            if (ctx->accepted_fingerprints.insert(fp).second) {
                ctx->confirmed.push_back(std::move(h));
                feedback["recorded"] = true;
            } else {
                feedback["is_duplicate"] = true;
            }
        } else {
            feedback["recorded"] = false;
            feedback["hint"] = "Some predicates failed. Inspect the grounding detail; "
                               "this interleaving may be benign or HB-ordered.";
        }
        return feedback.dump();
    }

    nlohmann::json err = {{"error", "Tool '" + tool_name + "' not implemented by this agent."}};
    return err.dump();
}

std::string InterleavingAnalysisAgent::parseResult(const std::vector<ChatMessage>&) {
    return "Interleaving analysis complete.";
}

std::vector<query::Hypothesis> InterleavingAnalysisAgent::analyzeObject(
    const query::SharedObject& object,
    const query::VulnerabilitySurface& surface,
    const std::map<int, LLM::ConcurrencyContract>& contracts) {

    reset();
    explore_calls_ = 0;
    set_system_prompt(build_system_prompt());

    InterleavingContext ctx;
    ctx.object = &object;
    ctx.surface = &surface;
    for (const auto& a : object.accesses) ctx.accesses.push_back(&a);

    std::stringstream ps;
    ps << "Analyze concurrent accesses to ONE shared object.\n\n";
    ps << "Shared object: " << (object.name.empty() ? "<anon>" : object.name);
    if (!object.type.empty()) ps << "  (type: " << object.type << ")";
    ps << "\nStatic risk flags:" << riskFlags(object) << "\n";
    ps << "Threads touching it: ";
    {
        bool first = true;
        for (int tid : object.accessing_thread_ids) { ps << (first ? "" : ", ") << tid; first = false; }
    }
    ps << "\n\nAccesses (reference these by access_index):\n";
    std::set<std::string> preloadNames;
    for (size_t i = 0; i < object.accesses.size(); ++i) {
        const auto& a = object.accesses[i];
        const std::string& fn = a.containing_function.empty() ? a.function_name
                                                              : a.containing_function;
        ps << "  [A" << i << "] thread " << a.thread_id
           << " " << a.access_type
           << " in " << fn
           << " @ " << a.location
           << (a.is_lock_protected ? (" [lock=" + a.protecting_lock + "]") : " [no lock]")
           << "\n        code: " << a.code_snippet << "\n";
        if (!fn.empty()) preloadNames.insert(fn);
    }

    if (!contracts.empty()) {
        ps << "\nPer-thread concurrency contracts:\n";
        for (const auto& [tid, c] : contracts) ps << contractToText(c);
    } else {
        ps << "\n(No contracts provided -- reason directly from source and accesses.)\n";
    }

    // #3 Preload the accessing functions' source so the model rarely needs read tools.
    std::string preloaded = preloadSource(preloadNames, /*charBudget=*/32000);
    if (!preloaded.empty()) {
        ps << "\nRelevant source (preloaded; use the read tools only for anything not shown):\n"
           << preloaded;
    }

    ps << "\nReason about harmful interleavings of these accesses, reject benign/HB-ordered "
          "ones, and call propose_race_hypothesis for each real bug. Call finish_analysis when done.";

    // P0 context hygiene: bound the window and pin this setup so it is never
    // pruned as tool observations accumulate.
    set_token_budget(16000);
    pin_next_user_message();

    try {
        send_message(ps.str(), &ctx);
    } catch (const std::exception& e) {
        std::cerr << "[InterleavingAnalysisAgent] object '" << object.name
                  << "' analysis error: " << e.what() << std::endl;
    }
    return std::move(ctx.confirmed);
}

std::vector<query::Hypothesis> InterleavingAnalysisAgent::analyzeCluster(
    const std::vector<const query::SharedObject*>& objects,
    const std::set<int>& threadSet,
    const query::VulnerabilitySurface& surface,
    bool useContractFraming,
    const std::map<int, LLM::ConcurrencyContract>* precomputedContracts,
    const std::string* staticVerdict) {

    reset();
    explore_calls_ = 0;
    set_system_prompt(build_system_prompt());

    const bool calibrate = (precomputedContracts != nullptr);

    // Calibration is a review of B's concrete candidates (contracts + verdict are
    // handed over), so it gets a tight read budget with a few points kept to resolve
    // a specific doubt; inline-derivation/ablation must build from source, so it
    // keeps the generous budget.
    exploreSoft_ = calibrate ? kCalibrateSoftBudget : kInlineSoftBudget;
    exploreHard_ = calibrate ? kCalibrateHardBudget : kInlineHardBudget;
    curThreads_ = threadSet;

    // Bound the absolute number of LLM round-trips per session. The read budget
    // (exploreHard_) already caps exploration, but reporting tools
    // (propose_race_hypothesis / finish_analysis) are intentionally uncapped, so
    // a model that keeps proposing without ever finishing can run a single
    // session away (observed: an amdgpu lock-ordering session burning millions
    // of tokens). This caps reads-budget + headroom for proposes/turns.
    // Env LACE_CALIBRATE_MAX_TURNS overrides (0 disables the cap entirely).
    // Healthy calibration sessions converge in ~5-8 turns; runaway sessions
    // (repeated propose_race_hypothesis without finishing) hit 30+. exploreHard_
    // reads + 8 leaves >2x headroom over healthy while cutting runaways hard.
    {
        int turnCap = exploreHard_ + 8;
        if (const char* e = std::getenv("LACE_CALIBRATE_MAX_TURNS"))
            turnCap = std::atoi(e);
        set_max_turns(turnCap);
    }

    InterleavingContext ctx;
    ctx.surface = &surface;
    if (objects.size() == 1) ctx.object = objects.front();  // keep single-object metadata

    std::stringstream ps;
    const bool many = objects.size() > 1;
    if (many) {
        ps << "Analyze a CLUSTER of " << objects.size()
           << " shared fields that are ALL touched by the same set of threads {";
        bool f = true;
        for (int t : threadSet) { ps << (f ? "" : ", ") << t; f = false; }
        ps << "}. They are co-accessed in the same code paths, so reason about them "
              "TOGETHER: one bug may span several fields (e.g. a producer index and the "
              "buffer it guards, or two reference pointers handed off between threads).\n\n";
    } else {
        ps << "Analyze concurrent accesses to ONE shared object.\n\n";
    }

    if (calibrate) {
        // CALIBRATION mode: contracts were derived per-thread in Phase A and the
        // deterministic single-mismatch composition already produced candidate
        // violations. The session's job is to VET them, not re-derive from scratch.
        ps << "Per-thread concurrency contracts (already derived in Phase A — assume = order\n"
              "REQUIRED for correctness; guarantee = order ESTABLISHED by synchronization):\n";
        for (int tid : threadSet) {
            auto it = precomputedContracts->find(tid);
            if (it != precomputedContracts->end()) ps << contractToText(it->second);
        }
        if (staticVerdict && !staticVerdict->empty()) {
            ps << "\nSTATIC COMPOSITION (single-mismatch) candidate violations for these\n"
                  "object(s). Each candidate already states the CONCRETE content: the requirer's\n"
                  "required order (relation + provenance), the violator's exact conflicting site\n"
                  "(function @ location + code), and what guarantee B weighed. This is your\n"
                  "primary evidence -- decide mostly FROM IT plus the contracts above; the source\n"
                  "is preloaded for context. Use the read tools ONLY to resolve a SPECIFIC doubt\n"
                  "(e.g. confirm a lock scope or a missing guard), not to re-derive the analysis.\n"
                  "CANDIDATE-ONLY REVIEW: evaluate ONLY the candidate IDs listed below (C1, C2,\n"
                  "...). Do NOT enumerate new thread pairs or add fresh bugs in calibration mode.\n"
                  "For each listed candidate, decide CONFIRM vs REJECT from the static composition\n"
                  "evidence and per-thread contracts above. CONFIRM when the assume violation is\n"
                  "real and no thread guarantee on THIS object establishes the required order R0.\n"
                  "REJECT if ANY holds: (a) a synchronization guarantee on THIS object truly covers\n"
                  "R0 (a lock/RCU/refcount guarding a DIFFERENT field does NOT count); (b) the two\n"
                  "flows provably cannot run concurrently -- NOTE a reentrant kernel entry\n"
                  "(syscall/ioctl/handler/work) executes on multiple CPUs at once, so two instances\n"
                  "of the SAME entry ARE concurrent unless an object-covering lock/HB serializes\n"
                  "them, and a '[self-race: concurrent instance]' access is a real second CPU, not a\n"
                  "duplicate; or (c) the candidate is clearly benign (pure stats/diagnostic counter\n"
                  "with no safety effect).\n"
                  "ANTI-OVER-REJECT (important): do NOT treat these as sufficient to REJECT a listed\n"
                  "candidate on their own — the static composition already surfaced it as a mismatch:\n"
                  "  - wait_for_completion / wait_for_completion_timeout / flush_work / cancel_work_sync\n"
                  "    on a *different* object or path than the one the candidate races;\n"
                  "  - a lock dropped before the racy access on THIS object (socket unlock before field\n"
                  "    use, release mutex then deref, etc.);\n"
                  "  - join/RCU/refcount on a parent/container while a nested field is still racy;\n"
                  "  - weak-memory / barrier annotation gaps (missing smp_wmb) when the candidate is a\n"
                  "    plain concurrent write vs read on the same field.\n"
                  "When the candidate states a concrete assume violation on THIS object and (a) does\n"
                  "not apply, default to CONFIRM unless you can cite a guarantee that directly\n"
                  "serializes the two listed sites on THIS object. CONFIRM by calling\n"
                  "propose_race_hypothesis (mention the candidate ID). The same-anchor alternate\n"
                  "observations are supporting context for the representative candidate, not separate\n"
                  "candidates:\n"
               << *staticVerdict << "\n";
        }
        ps << "\n";
    } else if (useContractFraming) {
        ps << "METHOD — per-thread concurrency contracts (derive inline, then check):\n"
              "For EACH thread above, from the preloaded source determine, FOR THESE "
              "OBJECT(S) ONLY (do not enumerate other state):\n"
              "  assume[] — the order this thread REQUIRES for its own correctness:\n"
              "    prec(a,b) (a happens-before b, e.g. init-before-read, use-before-free),\n"
              "    atomic([..]) (region/single access must not be interleaved by a conflicting write),\n"
              "    count_guarded(R,free) (free only after refcount hits zero).\n"
              "  guarantee[] — the order it actually ESTABLISHES via synchronization PRESENT in\n"
              "    the code: serialize(L,region) (lock/RCU/irq-disable), order(a<b via m)\n"
              "    (publish/flush/join/barrier/refcount-drop), counts(R). Leave empty if none\n"
              "    truly covers the object (a lock guarding a different field does NOT count).\n"
              "A BUG is exactly: one thread's assume order on an object is violated by another\n"
              "thread's (or a concurrent copy's) conflicting access, and NO thread's guarantee\n"
              "establishes that order. Reason briefly (you need not print full contracts), then\n"
              "call propose_race_hypothesis for each such violation.\n\n";
    } else {
        ps << "(Contract ablation OFF: reason about harmful interleavings directly from the "
              "source and accesses, without the assume/guarantee framing.)\n\n";
    }

    std::set<std::string> preloadNames;
    ps << "Shared field(s) and their accesses (reference accesses by access_index):\n";
    for (const query::SharedObject* op : objects) {
        if (!op) continue;
        ps << "=== object: " << (op->name.empty() ? "<anon>" : op->name);
        if (!op->type.empty()) ps << "  (type: " << op->type << ")";
        ps << "   flags:" << riskFlags(*op) << "\n";
        // Per-object lock grouping for the serialization view below. We use the
        // STATIC protecting-lock annotation only as a *candidate*; the prompt rule
        // lets the model override when a lock does not actually cover the object.
        std::map<std::string, std::vector<int>> byLock;
        std::vector<int> unlocked;
        for (const auto& a : op->accesses) {
            int gi = static_cast<int>(ctx.accesses.size());
            ctx.accesses.push_back(&a);
            const std::string& fn = a.containing_function.empty() ? a.function_name
                                                                  : a.containing_function;
            ps << "  [A" << gi << "] thread " << a.thread_id
               << " " << a.access_type
               << " in " << fn << " @ " << a.location
               << (a.is_lock_protected ? (" [lock=" + a.protecting_lock + "]") : " [no lock]")
               << "\n        code: " << a.code_snippet << "\n";
            if (!fn.empty()) preloadNames.insert(fn);
            if (a.is_lock_protected && !a.protecting_lock.empty())
                byLock[a.protecting_lock].push_back(gi);
            else
                unlocked.push_back(gi);
        }
        // Serialization view: which accesses are mutually serialized by a common
        // candidate lock vs. which are the race front. Only worth printing when the
        // object has enough accesses that pairwise reasoning is expensive (the
        // fan-in case). This focuses the model on the unlocked / cross-lock points
        // instead of re-deriving every handler's locking by reading all of them.
        if (op->accesses.size() > 3 && (byLock.size() + (unlocked.empty() ? 0 : 1)) > 1) {
            ps << "  -- lock/serialization view (static candidate locks) --\n";
            for (const auto& [lk, idxs] : byLock) {
                ps << "     under " << lk << ":";
                for (size_t i = 0; i < idxs.size(); ++i) ps << (i ? "," : " ") << "A" << idxs[i];
                ps << "   (mutually serialized IF this lock truly covers the object)\n";
            }
            if (!unlocked.empty()) {
                ps << "     NO LOCK (primary race front):";
                for (size_t i = 0; i < unlocked.size(); ++i) ps << (i ? "," : " ") << "A" << unlocked[i];
                ps << "\n";
            }
        }
    }

    std::string preloaded = preloadSource(preloadNames, /*charBudget=*/48000);
    if (!preloaded.empty()) {
        ps << "\nRelevant source (preloaded; use the read tools only for anything not shown):\n"
           << preloaded;
    }

    // Cross-session cache: preload functions the model already read while analysing
    // these same threads in earlier sessions, so it does not re-issue those reads.
    // Skip names already preloaded above; bound separately to avoid prompt bloat.
    std::set<std::string> cachedNames;
    for (int tid : threadSet) {
        auto it = threadReadFuncs_.find(tid);
        if (it == threadReadFuncs_.end()) continue;
        for (const std::string& nm : it->second)
            if (!preloadNames.count(nm)) cachedNames.insert(nm);
    }
    if (!cachedNames.empty()) {
        std::string cached = preloadSource(cachedNames, /*charBudget=*/24000);
        if (!cached.empty()) {
            ps << "\nPreviously-read source for these threads (cached from earlier sessions; "
                  "no need to re-read):\n" << cached;
        }
    }

    // Lockset focus: a race needs two conflicting accesses NOT serialized by a
    // common lock that covers the object. This prunes the fan-in (many syscall/fops
    // handlers all under the same socket lock are mutually serialized) WITHOUT
    // dropping recall: it is a focus/priority instruction the model can override,
    // and the genuinely racy access (the GT) is exactly the unlocked / cross-lock
    // one it points at.
    ps << "\nLOCKSET FOCUS: a real race needs two CONFLICTING accesses (>=1 write) that do "
          "NOT both hold a lock covering this object. Accesses grouped under the SAME lock "
          "above are serialized -- do NOT report a race between them, and you usually need "
          "not read those handlers closely -- UNLESS the source shows that lock does not "
          "actually protect this object. PRIORITIZE the NO-LOCK accesses and cross-lock "
          "pairs; that is where the bug, if any, lives.\n";

    ps << "\nReason about harmful interleavings across these field(s), reject benign/HB-ordered "
          "ones, and call propose_race_hypothesis for each real bug. Call finish_analysis when done.";

    set_token_budget(many ? 24000 : 16000);
    pin_next_user_message();

    try {
        send_message(ps.str(), &ctx);
    } catch (const std::exception& e) {
        std::cerr << "[InterleavingAnalysisAgent] cluster analysis error: " << e.what() << std::endl;
    }
    return std::move(ctx.confirmed);
}

std::string InterleavingAnalysisAgent::preloadSource(
    const std::set<std::string>& funcNames, std::size_t charBudget) const {
    std::stringstream ss;
    std::size_t used = 0;
    for (const auto& name : funcNames) {
        if (name.empty()) continue;
        std::unordered_set<Node*> nodes = ccpg_->getCPG()->findMethodsByName(name);
        for (Node* node : nodes) {
            CCPGNode* fn = ccpg_->getCCPGNodeByCPGNode(node);
            ccpg::Function* f = fn ? fn->getFunction() : nullptr;
            if (!f || !f->getFuncNode() || !f->getFuncNode()->getCPGNode()) continue;
            const std::string& code = f->getFuncNode()->getCPGNode()->getCode();
            if (code.empty() || code == "<empty>") continue;
            if (used + code.size() > charBudget) return ss.str();
            ss << "--- " << name << " ---\n" << code << "\n\n";
            used += code.size();
            break;  // one definition per name is enough
        }
    }
    return ss.str();
}

std::vector<query::Hypothesis> InterleavingAnalysisAgent::analyzeThread(
    int focusTid,
    const std::vector<const query::SharedObject*>& objects,
    const query::VulnerabilitySurface& surface,
    const std::map<int, LLM::ConcurrencyContract>& contracts) {

    reset();
    explore_calls_ = 0;
    set_system_prompt(build_system_prompt());

    InterleavingContext ctx;
    ctx.surface = &surface;

    std::stringstream ps;
    ps << "FOCUS THREAD: " << focusTid << ". Find bugs where an order that thread "
       << focusTid << " REQUIRES (its assume-clauses) is violated by a concurrent access "
       << "from any other thread (or by a concurrent copy of itself), over the shared "
       << "object(s) below.\n\n";

    // Put the focus thread's contract first, then the others as context.
    if (!contracts.empty()) {
        auto fc = contracts.find(focusTid);
        if (fc != contracts.end()) {
            ps << "Focus thread " << focusTid << " contract (orders it REQUIRES / establishes):\n";
            ps << contractToText(fc->second) << "\n";
        }
        bool wroteHeader = false;
        for (const auto& [tid, c] : contracts) {
            if (tid == focusTid) continue;
            if (!wroteHeader) { ps << "Concurrent threads' contracts (context):\n"; wroteHeader = true; }
            ps << contractToText(c);
        }
        ps << "\n";
    } else {
        ps << "(No contracts provided -- reason directly from source and accesses.)\n\n";
    }

    // List every object the focus thread touches, with ALL accesses (focus + others)
    // under a flat [A#] index, so the model sees who races with the focus thread.
    std::set<std::string> preloadNames;
    ps << "Shared object(s) touched by thread " << focusTid
       << " (with all concurrent accesses):\n";
    for (const query::SharedObject* op : objects) {
        if (!op) continue;
        ps << "=== object: " << (op->name.empty() ? "<anon>" : op->name);
        if (!op->type.empty()) ps << "  (type: " << op->type << ")";
        ps << "   flags:" << riskFlags(*op) << "\n";
        for (const auto& a : op->accesses) {
            int gi = static_cast<int>(ctx.accesses.size());
            ctx.accesses.push_back(&a);
            const std::string& fn = a.containing_function.empty() ? a.function_name
                                                                  : a.containing_function;
            ps << "  [A" << gi << "] thread " << a.thread_id
               << (a.thread_id == focusTid ? "*" : " ") << " " << a.access_type
               << " in " << fn << " @ " << a.location
               << (a.is_lock_protected ? (" [lock=" + a.protecting_lock + "]") : " [no lock]")
               << "\n        code: " << a.code_snippet << "\n";
            if (!fn.empty()) preloadNames.insert(fn);
        }
    }
    ps << "(* marks the focus thread's own accesses.)\n";

    // #3 Preload the racing functions' source so the model rarely needs read tools.
    std::string preloaded = preloadSource(preloadNames, /*charBudget=*/48000);
    if (!preloaded.empty()) {
        ps << "\nRelevant source (preloaded; use the read tools only for anything not shown):\n"
           << preloaded;
    }

    ps << "\nReason about harmful interleavings across these objects, reject benign/HB-ordered "
          "ones, and call propose_race_hypothesis for each real bug. Call finish_analysis when done.";

    set_token_budget(24000);
    pin_next_user_message();

    try {
        send_message(ps.str(), &ctx);
    } catch (const std::exception& e) {
        std::cerr << "[InterleavingAnalysisAgent] focus thread " << focusTid
                  << " analysis error: " << e.what() << std::endl;
    }
    return std::move(ctx.confirmed);
}

} // namespace llm_client
