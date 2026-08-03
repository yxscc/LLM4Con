#include "LLMUtil/ContractGeneratorAgent.h"
#include "LLMUtil/SharedToolKit.h"
#include "CCPG/ThreadCreationTree.h"
#include "CCPG/CCPG.h"
#include "CCPG/CCPGNode.h"
#include "CCPG/ManualEntryConfig.h"
#include "CPG/Node.h"
#include "PhasarUtil/LLVMAnalyzer.h"
#include "llvm/IR/Value.h"
#include <cctype>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <unordered_set>

namespace llm_client {

namespace {
// An object is HIGH-RISK (memory-safety / data-race shaped) iff the static surface
// flagged it as carrying an unprotected cross-thread mutation, a free, a list
// mutation, or a self-race. For such an object the contract MUST take a position
// (a real assume/guarantee, or an explicit no_order_needed) — silently omitting it
// is the recall hole that makes Phase B unable to compose a candidate. Benign
// scalars (pure reads / counters / config) are NOT high-risk and stay selective.
bool isHighRiskObject(const query::SharedObject* o) {
    if (!o) return false;
    return o->has_free_operation || o->has_list_mutation || o->is_self_race ||
           o->has_unprotected_write;
}

// Coverage is enabled in analyst-scoped (manual entry) recall-first runs by default,
// and can be forced on/off for ablation via LACE_CONTRACT_COVERAGE=1/0. The broad
// auto-discovery surface (hundreds of objects) keeps the legacy selective behavior
// unless explicitly enabled, to bound contract-generation cost.
bool coverageEnabled() {
    if (const char* e = std::getenv("LACE_CONTRACT_COVERAGE"))
        return e[0] && e[0] != '0';
    return manualentry::enabled();
}

// Paper-faithful node-anchored contract mode. Off by default: the legacy free-text
// clause path is unchanged unless LACE_CONTRACT_L2 is set (together with
// LACE_STATIC_COMPOSE, which gates the composition pipeline).
bool contractL2Enabled() {
    if (const char* e = std::getenv("LACE_CONTRACT_L2"))
        return e[0] && e[0] != '0';
    return false;
}

const char* riskTag(const query::SharedObject* o) {
    if (!o) return "high-risk";
    if (o->has_free_operation) return "free/UAF";
    if (o->has_list_mutation) return "list mutation";
    if (o->is_self_race) return "self-race";
    if (o->has_unprotected_write) return "unprotected cross-thread write";
    return "cross-thread read/write";
}

std::string lowerCopy(std::string s) {
    for (char& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return s;
}

std::string upperCopy(std::string s) {
    for (char& c : s) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    return s;
}

std::string canonicalRequirementRelation(const std::string& rel) {
    std::string r = lowerCopy(rel);
    if (r == "prec" || r == "order") return "ORDER";
    if (r == "atomic") return "REGION_ISOLATED";
    if (r == "count_guarded" || r == "stable_during") return "STABLE_DURING";
    if (r == "conflict_mediated") return "CONFLICT_MEDIATED";
    if (r == "region_isolated") return "REGION_ISOLATED";
    if (r == "progress_enabled") return "PROGRESS_ENABLED";
    return upperCopy(rel);
}

std::string canonicalGuaranteeRelation(const std::string& rel) {
    std::string r = lowerCopy(rel);
    if (r == "order") return "ORDER";
    if (r == "serialize" || r == "exclude") return "EXCLUDE";
    if (r == "counts" || r == "linearize") return "LINEARIZE";
    if (r == "wait") return "WAIT";
    return upperCopy(rel);
}

bool containsAny(const std::string& s, const std::vector<std::string>& needles) {
    for (const auto& n : needles)
        if (s.find(n) != std::string::npos) return true;
    return false;
}

// Deterministic guard for no_order_needed: the model may batch a high-risk object
// as benign only when this thread's own access snippets do NOT show an obvious
// consequential use. This is intentionally syntactic and conservative: it prevents
// branch predicates or selection/removal tests from being placed in the benign
// bucket. It is not the only way to discover consequences -- the prompt still asks
// the model to follow callees for indirect sinks -- but obvious local sinks should
// never be waived away by no_order_needed.
bool hasObviousLocalConsequence(const query::SharedObject* o, int tid, std::string* evidence) {
    if (!o) return false;
    for (const auto& a : o->accesses) {
        if (a.thread_id != tid) continue;
        std::string c = lowerCopy(a.code_snippet);
        bool branchLike = containsAny(c, {"if (", "if(", "while (", "while(", "for (", "for(",
                                          "&&", "||", "!=", "==", "<=", ">=", " ? "});
        // Less-than/greater-than are noisy in C++ templates/angles, so keep them as
        // secondary signals only when the line also references the object as a value.
        bool indexOrSize = containsAny(c, {"[", "alloc", "skb_put", "memcpy", "memmove",
                                           "copy_", "_copy", "len", "size", "bound"});
        bool lifetimeOrLinks = containsAny(c, {"kfree", "free(", "refcount", "kref",
                                               "list_", "hlist_", "rb_", "xarray", "xa_"});
        if (branchLike || indexOrSize || lifetimeOrLinks) {
            if (evidence) {
                *evidence = a.code_snippet;
                if (!a.location.empty()) *evidence += " @ " + a.location;
            }
            return true;
        }
    }
    return false;
}
}  // namespace

ContractGeneratorAgent::ContractGeneratorAgent(CCPG* ccpg, std::shared_ptr<LLMClient> client)
    : Conversation(client, build_system_prompt(), 25), ccpg_(ccpg) {
    l2Mode_ = contractL2Enabled();
}

std::string ContractGeneratorAgent::build_system_prompt() {
    if (contractL2Enabled()) {
        return R"CONTRACT(
You are an expert in concurrent Linux-kernel C code. For ONE thread (given its entry
function and creation site), build that thread's ThreadContract, anchoring every
operand to CONCRETE CCPG NODE IDS.

**CRITICAL: DO NOT REPLY WITH CHAT TEXT. ONLY USE THE PROVIDED TOOLS.**

A ThreadContract has NO notion of a final bug. It records two things about THIS
thread's operations on shared objects:

  1. REQUIREMENTS -- structural obligations this thread needs from its concurrent
     environment for its own operations to be safe. Use ONLY these three forms
     (each operand is a LIST OF NODE IDS naming concrete operations):

       * MustPrecede(a_nodes, b_nodes)
           Every operation in a_nodes must complete before any operation in
           b_nodes begins. Example: a read/use of an object must precede its free.

       * MustBeAtomic(a_nodes)
           The ordered operation sequence in a_nodes must execute as one
           indivisible unit; no conflicting operation from another thread may
           interleave between its steps. (b_nodes is empty.)

       * MustBeMediated(a_nodes, b_nodes)
           a_nodes and b_nodes are conflicting operations on the same object
           (at least one a write). They must be coordinated by ordering, mutual
           exclusion, or a compatible atomic-access protocol. If nothing mediates
           them, that is an unmediated conflict (a data race).

  2. GUARANTEES -- synchronization effects THIS thread's code actually provides.
     Use ONLY these four forms (operands are NODE IDS):

       * Order(a_nodes, b_nodes)
           a happens-before b (a determinate handoff), e.g. the node after a
           completion wait, a matched release->acquire, thread join, or a
           close-and-drain API return. Emit ONLY when the code truly forces it.

       * Exclude(a_nodes, token, mode)
           the a_nodes run under mutual exclusion named by `token` (e.g. a lock
           pointer) in `mode` (exclusive/shared); regions under the same token in
           incompatible modes cannot overlap.

       * AtomicOp(a_nodes, token)
           the a_nodes are a single atomic access to `token` (hardware atomics,
           cmpxchg, atomic_* helpers).

       * Wait(a_nodes, b_nodes)
           execution cannot reach b_nodes (the continuation after a blocking call)
           until the enabling event a_nodes has happened.

KEEP GUARANTEES HONEST: emit a guarantee ONLY for synchronization the code really
provides for THIS resource. A lock guarding a different field, a wait on another
domain, or a bare state check is NOT a guarantee. Do not invent an Order edge to
make a requirement look discharged -- if the ordering is not established in the
code, simply omit it; the checker will then surface the requirement as a candidate.

HOW TO NAME NODES: the evidence packet lists each access as `[node=N]`. Use those
ids as operands. To find more operations inside a function, call
`get_function_ops(function_id)` -- it returns each operation's node_id, code, and
location. Always pass real node ids that appear in the evidence or in a
get_function_ops result; never invent an id.

**Workflow (few tool calls):**
1. (Optional) confirm_role_and_summary -- a one-line characterization.
2. For each shared object this thread meaningfully operates on, call
   add_requirement with the matching form and node operands, and add_guarantee for
   any synchronization the code provides. Attach object_id and a short note.
3. For a HIGH-RISK object you reviewed and found carries no obligation for THIS
   thread, call declare_no_obligation(object_id, reason) instead of dropping it.
4. Call finalize_contract.
)CONTRACT";
    }
    return R"CONTRACT(
You are an expert in concurrent Linux-kernel C code. For ONE thread (given its entry
function and creation site), build that thread's ThreadContract.

**CRITICAL: DO NOT REPLY WITH CHAT TEXT. ONLY USE THE PROVIDED TOOLS.**

The contract has NO notion of a final bug. It records:
  1. local requirements: what THIS thread's statements/regions need from the
     environment to be safe; and
  2. guarantee candidates: synchronization effects this thread actually provides.

A requirement is NOT a bug symptom such as UAF/data-race/deadlock. It is an
anchored execution obligation over a concrete statement or region. Use only this
obligation vocabulary in assume[]:

  * ORDER(A, B)
      A must happen before B. Example: ORDER(init(obj), publish(obj)).

  * CONFLICT_MEDIATED(A, B)
      A and B may conflict; correctness requires ordering, non-overlap, or a
      valid linearized synchronization protocol between them.

  * REGION_ISOLATED(region, hazards)
      No listed environment hazard may enter the region while it is executing.

  * STABLE_DURING(region, predicate_or_resource)
      A resource or protocol predicate must remain valid throughout the region.
      Example: STABLE_DURING(pattern_show_use_region, live(trigger_data)).

  * PROGRESS_ENABLED(wait, enabler)
      Optional: a wait must have a matching enabling event or condition.

A guarantee is NOT an API family. It is what synchronization contributes to the
execution history. Use only these Level-0 atoms in guarantee[]:

  * ORDER(a, b)
      a happens before b, e.g. ORDER(callback_exit, device_remove_groups_return).

  * EXCLUDE(token, region, mode)
      regions under the same token and incompatible modes cannot overlap, e.g.
      EXCLUDE(sk->sk_lock, sendmsg_critical_region, exclusive).

  * LINEARIZE(object, operation)
      operation takes effect at one abstract point in the synchronization object's
      history, e.g. LINEARIZE(refcount, acquire) or LINEARIZE(sysfs_domain, close).

  * WAIT(wait_event, condition_or_enabler)
      execution cannot pass wait_event until the condition/enabler holds, e.g.
      WAIT(synchronize_rcu_return, preexisting_readers_empty).

High-level APIs are macros over these atoms. For example:
  - mutex/spinlock: LINEARIZE(lock state) + EXCLUDE(locked region)
  - handoff/completion: WAIT(wait_return, signal) + ORDER(signal_side_effects, wait_return)
  - RCU grace period: LINEARIZE(reader enter/exit) + WAIT(grace period, readers empty)
    + ORDER(reader_exit, grace_period_return)
  - refcount/capability: LINEARIZE(acquire/release) + WAIT(retire, active_holders==0)
  - callback close-and-drain: LINEARIZE(domain admission := closed) +
    WAIT(api_return, active_callbacks_empty) + ORDER(callback_exit, api_return)
  - validation/retry: LINEARIZE(version transitions) + successful-path validation.

KEEP GUARANTEES HONEST: only list effects the code actually provides and only when
they cover the resource/protocol in this clause. A lock that guards a different
field, a wait on another domain, or a state check without synchronization does not
discharge this resource's requirement. Static candidate locks are evidence, not proof.

Requirement proposal is bounded by the static evidence packet. Emit a clause when
THIS thread has a source-anchored safety obligation or provides a synchronization
effect for the resource/protocol. Good requirement evidence includes:
  * a region that uses a resource whose lifetime may be changed elsewhere;
  * a check-use or validate-use interval whose predicate can be invalidated elsewhere;
  * a value that flows into a branch, selection, index, size, pointer, list/tree link,
    publication, or lifetime/free decision;
  * a publish-before-use or init-before-publish relationship;
  * a wait whose matching enabler is part of the protocol.

For an ordinary read-only scalar / statistics counter / config value that drives no
safety decision, you may skip it unless it is marked HIGH-RISK; for HIGH-RISK objects
you must explicitly account for it.

COVERAGE MANDATE: some listed objects are marked HIGH-RISK because static analysis saw
an unsynchronized write, free, list mutation, or self-race. For each HIGH-RISK object:
  (a) if it has a source-anchored safety obligation, emit a real assume and any real
      guarantee atoms/macros;
  (b) if it is reviewed and has no safety obligation for THIS thread, put all such ids
      into ONE no_order_needed=true clause with object_ids=[...].

Follow values into callees when needed. Attach provenance to every assume and
guarantee. Do not invent requirements for values that drive nothing, and do not
bucket as benign a value that reaches a branch/index/pointer/lifetime/list decision.
LOCK-REGION MERGING: when several listed fields are protected by the same lock/region,
emit one clause with object_ids=[...] and one EXCLUDE(...) guarantee.

**Workflow (one message, few clauses):**
1. Call confirm_role_and_summary (a one-line, informational characterization).
2. Call report_clause ONLY for the resources that meet the selectivity test above
   (assume and/or guarantee + provenance). Set object_id (single) or object_ids (a
   lock-protected group). It is correct to emit FEW clauses -- skip the rest.
3. (Optional) Call report_ordering for any thread-level cross-resource order
   (e.g. "writes_before_publish: init obj->data before publishing obj").
4. Call finalize_contract.
)CONTRACT";
}

std::vector<Tool> ContractGeneratorAgent::get_l2_tools() const {
    auto tools = SharedToolKit::get_shared_tools();

    tools.push_back({"confirm_role_and_summary", "Confirms the role and summary of the thread (informational).", {
        {"role", "string", "A single, concise category (e.g., 'Worker', 'Reader', 'Reclaimer').", true},
        {"summary", "string", "A one-sentence description of the thread's function.", true}
    }});

    // Node-id array schema reused for operands.
    auto nodeArray = [](const std::string& desc) {
        return nlohmann::json{
            {"type", "array"},
            {"description", desc},
            {"items", {{"type", "integer"}}}
        };
    };

    {
        std::vector<Parameter> p;
        p.emplace_back("form", "string",
            "One of: MustPrecede | MustBeAtomic | MustBeMediated.", true);
        p.emplace_back("a_nodes", nodeArray(
            "CCPG node ids of operand a. MustPrecede: the operations that must finish first. "
            "MustBeAtomic: the ordered sequence that must be indivisible. "
            "MustBeMediated: one side of the conflicting pair."), true);
        p.emplace_back("b_nodes", nodeArray(
            "CCPG node ids of operand b. MustPrecede: the operations that must happen after a. "
            "MustBeMediated: the other side of the conflicting pair. Leave empty for MustBeAtomic."), false);
        p.emplace_back("object_id", "integer", "The [obj#N] surface index this requirement is about.", false);
        p.emplace_back("note", "string", "One concise sentence of provenance / rationale.", false);
        tools.push_back({"add_requirement",
            "Record ONE node-anchored requirement this thread needs from its environment "
            "(MustPrecede / MustBeAtomic / MustBeMediated). Operands are concrete CCPG node ids.",
            std::move(p)});
    }
    {
        std::vector<Parameter> p;
        p.emplace_back("form", "string",
            "One of: Order | Exclude | AtomicOp | Wait.", true);
        p.emplace_back("a_nodes", nodeArray(
            "CCPG node ids of operand a. Order: source. Exclude: the protected region nodes. "
            "AtomicOp: the atomic operation nodes. Wait: the enabling event."), true);
        p.emplace_back("b_nodes", nodeArray(
            "CCPG node ids of operand b. Order: target (a happens-before b). "
            "Wait: the continuation after the blocking call. Unused for Exclude/AtomicOp."), false);
        p.emplace_back("token", "string",
            "Exclude/AtomicOp only: the synchronization token (e.g. lock pointer 'sk->sk_lock', "
            "or an atomic variable). Regions/ops sharing a token are coordinated.", false);
        p.emplace_back("mode", "string", "Exclude only: 'exclusive' or 'shared'.", false);
        p.emplace_back("object_id", "integer", "The [obj#N] surface index this guarantee covers.", false);
        p.emplace_back("note", "string", "One concise sentence of provenance / rationale.", false);
        tools.push_back({"add_guarantee",
            "Record ONE node-anchored synchronization effect THIS thread's code truly provides "
            "(Order / Exclude / AtomicOp / Wait). Operands are concrete CCPG node ids. "
            "Do NOT emit an effect the code does not establish.",
            std::move(p)});
    }
    tools.push_back({"declare_no_obligation",
        "Explicitly declare that a reviewed HIGH-RISK object carries NO obligation for THIS thread "
        "(auditable no-op instead of silently omitting it).", {
        {"object_id", "integer", "The [obj#N] surface index reviewed and found benign for this thread.", true},
        {"reason", "string", "One concise sentence justifying why there is no obligation.", true}
    }});

    tools.push_back({"finalize_contract", "Final action to submit the complete contract.", {}});
    return tools;
}

std::vector<Tool> ContractGeneratorAgent::get_available_tools() const {
    if (l2Mode_) return get_l2_tools();

    auto tools = SharedToolKit::get_shared_tools();

    tools.push_back({"confirm_role_and_summary", "Confirms the role and summary of the thread (informational).", {
        {"role", "string", "A single, concise category (e.g., 'Worker', 'Reader', 'Reclaimer').", true},
        {"summary", "string", "A one-sentence description of the thread's function.", true}
    }});

    // One nested-object item schema reused for assume[] and guarantee[].
    auto reqItem = [](const std::string& relations, const std::string& examples) {
        return nlohmann::json{
            {"type", "object"},
            {"properties", {
                {"relation", {{"type", "string"}, {"description", "one of: " + relations}}},
                {"detail",   {{"type", "string"}, {"description", "the relation written out, e.g. " + examples}}},
                {"provenance", {{"type", "string"}, {"description", "source line / caller justifying it"}}}
            }},
            {"required", {"relation", "detail"}}
        };
    };
    nlohmann::json assume_schema = {
        {"type", "array"},
        {"description", "Local execution obligations this thread REQUIRES for its own correctness."},
        {"items", reqItem("ORDER | CONFLICT_MEDIATED | REGION_ISOLATED | STABLE_DURING | PROGRESS_ENABLED",
                          "\"STABLE_DURING(use_region, live(obj))\" | \"ORDER(init(obj), publish(obj))\" | \"REGION_ISOLATED(check_use_region, invalidators)\"")}
    };
    nlohmann::json guarantee_schema = {
        {"type", "array"},
        {"description", "Level-0 synchronization effects this thread actually provides (may be empty). "
                        "High-level APIs should be described as macros over ORDER/EXCLUDE/LINEARIZE/WAIT."},
        {"items", reqItem("ORDER | EXCLUDE | LINEARIZE | WAIT",
                          "\"ORDER(callback_exit, drain_return)\" | \"EXCLUDE(lock, region, exclusive)\" | \"LINEARIZE(refcount, acquire)\" | \"WAIT(drain_return, active_callbacks_empty)\"")}
    };
    nlohmann::json sites_schema = {
        {"type", "array"},
        {"description", "Provenance for the clause's operations, as \"function @ file:line\" strings."},
        {"items", {{"type", "string"}}}
    };

    nlohmann::json object_ids_schema = {
        {"type", "array"},
        {"description", "The [obj#N] indices this clause covers. Use MULTIPLE ids to merge "
                        "several fields under the SAME lock/region into one EXCLUDE clause."},
        {"items", {{"type", "integer"}}}
    };

    std::vector<Parameter> clause_params;
    clause_params.emplace_back("resource", "string", "The shared object/field this clause is about (e.g. 'key->payload', 'obj->data'). For a lock-region group, name the region.", true);
    clause_params.emplace_back("object_id", "integer", "The [obj#N] index of the listed shared object this clause is about (single-object clause).", false);
    clause_params.emplace_back("object_ids", std::move(object_ids_schema), false);
    clause_params.emplace_back("sites", std::move(sites_schema), false);
    clause_params.emplace_back("assume", std::move(assume_schema), false);
    clause_params.emplace_back("guarantee", std::move(guarantee_schema), false);
    clause_params.emplace_back("no_order_needed", "boolean",
        "Set true ONLY to explicitly declare that this resource carries NO order "
        "obligation for THIS thread: the racy value never flows into a branch/index/"
        "size/pointer/lifetime decision AND there is no use-before-free / "
        "init-before-publish. Use this for a HIGH-RISK object you reviewed and judged "
        "benign (instead of silently omitting it). When true, leave assume/guarantee "
        "empty and give the justification in no_order_reason.", false);
    clause_params.emplace_back("no_order_reason", "string",
        "Required when no_order_needed=true: one concise sentence justifying why this "
        "resource has no order obligation (e.g. 'plain diagnostic counter, value never "
        "drives control flow or lifetime').", false);
    tools.push_back({"report_clause",
        "Report THIS thread's local requirements and synchronization guarantees for ONE shared resource/protocol. "
        "Use assume: ORDER/CONFLICT_MEDIATED/REGION_ISOLATED/STABLE_DURING/PROGRESS_ENABLED; "
        "guarantee: ORDER/EXCLUDE/LINEARIZE/WAIT. For a HIGH-RISK object with no obligation, "
        "set no_order_needed=true + no_order_reason.",
        std::move(clause_params)});

    tools.push_back({"report_ordering", "Optional: a thread-level cross-resource ordering note.", {
        {"detail", "string", "e.g. 'writes_before_publish: init obj->data before publishing obj'.", true},
        {"provenance", "string", "source line / caller.", false}
    }});

    tools.push_back({"finalize_contract", "Final action to submit the complete contract.", {}});

    return tools;
}

std::optional<LLM::ConcurrencyContract> ContractGeneratorAgent::generateContractForThread(
    Thread* thread,
    const std::vector<const query::SharedObject*>& touchedObjects,
    const std::vector<int>& objectIds) {

    reset();
    explore_calls_ = 0;
    reportRounds_ = 0;
    seededObjectIds_.clear();
    seededObjectsById_.clear();
    for (size_t k = 0; k < objectIds.size(); ++k) {
        int id = objectIds[k];
        if (id >= 0) {
            seededObjectIds_.insert(id);
            if (k < touchedObjects.size() && touchedObjects[k])
                seededObjectsById_[id] = touchedObjects[k];
        }
    }
    // Context hygiene: bound the window and pin the task setup so a long
    // exploration does not blow up the prompt or strand the instructions.
    set_token_budget(16000);
    pin_next_user_message();

    if (!thread || !thread->getThreadMainFunction()) {
        return std::nullopt;
    }

    // SEEDED when the static surface already told us which objects this thread
    // touches: feed those + the thread's own accesses + the accessing functions'
    // source, and tighten the read budget. Otherwise fall back to blind
    // call-graph exploration (legacy path).
    const int tid = thread->getId();
    const bool seeded = !touchedObjects.empty();
    exploreSoft_ = seeded ? kSeededSoftBudget : kBlindSoftBudget;
    exploreHard_ = seeded ? kSeededHardBudget : kBlindHardBudget;

    auto contract = std::make_unique<LLM::ConcurrencyContract>(
        thread->getId(), 
        thread->getThreadMainFunction()->getId()
    );

    std::string fork_stmt = thread->getForkNode()->getCPGNode()->getCode();
    std::string entry_func_code = thread->getThreadMainFunction()->getFuncNode()->getCPGNode()->getCode();

    std::stringstream candidates_ss;
    std::set<std::string> preloadNames;
    const bool coverage = coverageEnabled();
    if (seeded) {
        // The objects this thread touches, with THIS thread's own accesses and a
        // compact environment summary for the same surface object.
        candidates_ss << "\n--- Shared objects THIS thread touches (from static analysis) ---\n"
                         "This is the surface's EXHAUSTIVE inventory -- you do NOT need to restate it.\n"
                         "Skip pure reads, statistics counters, and config scalars that drive no\n"
                         "decision. Set object_id to the [obj#N] number (or object_ids for several\n"
                         "fields under the SAME lock) so the resource is matched unambiguously. You\n"
                         "generally do NOT need to explore the call graph -- the accessing functions\n"
                         "and the main environment hazards are preloaded or summarized below.\n";
        if (coverage)
            candidates_ss << "Every object tagged [HIGH-RISK] below must be ACCOUNTED FOR. Emit a real\n"
                             "assume only for those with an anchored safety obligation (branch /\n"
                             "index / size / pointer-deref / use-before-free / list-or-tree / publish-use);\n"
                             "follow the\n"
                             "value into callees if needed. Put ALL the remaining (benign) HIGH-RISK ids\n"
                             "into a SINGLE no_order_needed=true clause (object_ids=[...]). Do NOT silently\n"
                             "drop a HIGH-RISK object, and do NOT invent an assume for a value that drives\n"
                             "nothing. IMPORTANT: fields named id/state/count/flag are NOT automatically\n"
                             "benign metadata -- if they are used to match/select/remove an object, gate a\n"
                             "shutdown/free, or enforce a limit, they need an anchored assume such as\n"
                             "STABLE_DURING(...) or REGION_ISOLATED(...).\n";
        for (size_t k = 0; k < touchedObjects.size(); ++k) {
            const query::SharedObject* o = touchedObjects[k];
            if (!o) continue;
            int oid = (k < objectIds.size()) ? objectIds[k] : -1;
            candidates_ss << "* ";
            if (oid >= 0) candidates_ss << "[obj#" << oid << "] ";
            candidates_ss << (o->name.empty() ? "<anon>" : o->name);
            if (!o->type.empty()) candidates_ss << "  (type: " << o->type << ")";
            if (coverage && isHighRiskObject(o))
                candidates_ss << "  [HIGH-RISK: " << riskTag(o) << "]";
            candidates_ss << "\n";
            for (const auto& a : o->accesses) {
                if (a.thread_id != tid) continue;  // only this thread's accesses
                const std::string& fn = a.containing_function.empty() ? a.function_name
                                                                      : a.containing_function;
                candidates_ss << "    - " << a.access_type << " in " << fn << " @ " << a.location;
                if (l2Mode_ && a.node_id >= 0) candidates_ss << " [node=" << a.node_id << "]";
                candidates_ss << (a.is_lock_protected ? (" [lock=" + a.protecting_lock + "]")
                                                      : " [no lock]")
                              << "\n          code: " << a.code_snippet << "\n";
                if (!fn.empty()) preloadNames.insert(fn);
            }
            std::vector<const query::ThreadAccess*> envHazards;
            std::vector<const query::ThreadAccess*> envReads;
            static const bool noCrossThreadCtx = []() {
                const char* e = std::getenv("LACE_NO_CROSS_THREAD_CONTEXT");
                return e && e[0] && e[0] != '0';
            }();
            if (!noCrossThreadCtx) {
            for (const auto& a : o->accesses) {
                const bool otherThread = a.thread_id != tid;
                const bool concurrentInstance = o->is_self_race && a.thread_id == tid;
                if (!otherThread && !concurrentInstance) continue;
                const bool mutating = a.access_type == "Write" || a.access_type == "Free" ||
                                      a.code_snippet.find("[list-helper]") != std::string::npos;
                if (mutating) envHazards.push_back(&a);
                else envReads.push_back(&a);
            }
            }
            if (!envHazards.empty() || !envReads.empty()) {
                candidates_ss << "    Environment accesses on the same surface object"
                              << " (use these to decide what THIS thread's code must tolerate):\n";
                auto renderEnv = [&](const std::vector<const query::ThreadAccess*>& v,
                                     size_t limit) {
                    size_t n = 0;
                    for (const auto* ap : v) {
                        if (!ap || n >= limit) break;
                        const std::string& fn = ap->containing_function.empty()
                            ? ap->function_name : ap->containing_function;
                        candidates_ss << "      - thread " << ap->thread_id << " "
                                      << ap->access_type << " in " << fn << " @ " << ap->location;
                        if (ap->node_id >= 0) candidates_ss << " [node=" << ap->node_id << "]";
                        candidates_ss << (ap->is_lock_protected
                                          ? (" [lock=" + ap->protecting_lock + "]")
                                          : " [no lock]")
                                      << "\n            code: " << ap->code_snippet << "\n";
                        if (!fn.empty() && (ap->access_type == "Write" ||
                                            ap->access_type == "Free"))
                            preloadNames.insert(fn);
                        ++n;
                    }
                    if (v.size() > limit)
                        candidates_ss << "      - ... " << (v.size() - limit)
                                      << " more similar access(es)\n";
                };
                renderEnv(envHazards, 8);
                renderEnv(envReads, 4);
            }
        }
        std::string preloaded = preloadSource(preloadNames, /*charBudget=*/28000);
        if (!preloaded.empty()) {
            candidates_ss << "\n--- Source of the accessing functions (preloaded; "
                             "use the read tools only for anything not shown) ---\n"
                          << preloaded;
        }
    } else {
        const std::set<const llvm::Value*>& candidateSharedObjects =
            ThreadCreationTree::getInstance()->collectCandidateSharedObjects();
        candidates_ss << "\n--- Candidate Shared Objects (from static analysis) ---\n";
        if (candidateSharedObjects.empty()) {
            candidates_ss << "None found.\n";
        } else {
            for (const auto* val : candidateSharedObjects) {
                if (val && val->hasName()) {
                    candidates_ss << "- " << LLVMAnalyzer::getInstance()->demangle_valueName(val->getName().str().c_str()) << "\n";
                }
            }
        }
    }

    std::string explore_hint = seeded
        ? "The accessing functions are preloaded above, so you have everything needed and\n"
          "should NOT read anything. Emit the contract in a SINGLE response: in one message\n"
          "issue confirm_role_and_summary, then a report_clause ONLY for the FEW resources\n"
          "that have a real local requirement or synchronization guarantee (merge same-lock\n"
          "fields into one clause via object_ids), then finalize_contract. Skip pure\n"
          "reads/counters/config scalars -- emitting few clauses is correct. Only if a clause\n"
          "genuinely depends on code not shown should you call get_function_by_name first.\n\n"
        : "Follow the thread's real work with `get_callees` / `get_function_by_name` before\n"
          "deciding the requirements — the access that matters is often in a callee.\n"
          "If the entry runs an opaque event loop (`event_base_loop`, `uv_run`, a worker\n"
          "dispatch), search for the handlers it invokes and reason about the shared state\n"
          "they touch; note in the summary that those callees are inferred.\n\n";

    // Node-anchored (L2) instructions replace the free-text clause guidance.
    std::string l2_hint =
        "Every access above is tagged [node=N]. Use those N as operands; call\n"
        "get_function_ops(function_id) to list more operations (each with its node_id) when\n"
        "you need a node that is not shown. NEVER invent a node id.\n\n";
    std::string l2_tail =
        "For each shared object this thread meaningfully operates on, decide its\n"
        "structural obligation and record it with add_requirement using CONCRETE node ids:\n"
        "  * MustPrecede(a_nodes, b_nodes): a use/read that must complete before a free or\n"
        "    reuse (a_nodes=the uses, b_nodes=the free/overwrite).\n"
        "  * MustBeAtomic(a_nodes): a check-then-use or multi-step update that must not be\n"
        "    interrupted (a_nodes=the ordered steps).\n"
        "  * MustBeMediated(a_nodes, b_nodes): two conflicting accesses on the same object\n"
        "    (>=1 write) that must be ordered/excluded/atomic (a_nodes and b_nodes=the two sides).\n"
        "Then use add_guarantee ONLY for synchronization THIS thread's code truly provides\n"
        "(Order / Exclude / AtomicOp / Wait), with node operands + token where applicable.\n"
        "If the code does NOT establish the ordering/exclusion a requirement needs, DO NOT\n"
        "invent a guarantee -- just omit it; the checker will surface the requirement.\n"
        "For a [HIGH-RISK] object you reviewed and found benign for THIS thread, call\n"
        "declare_no_obligation(object_id, reason) rather than dropping it. Then\n"
        "finalize_contract. Describe only this thread's obligations, not bug patterns.";

    std::string user_prompt = 
        "Build the concurrency contract for the following thread: its per-resource local\n"
        "requirements (assume) and synchronization effects (guarantee).\n"
        "Fork statement: " + fork_stmt + "\n"
        "Thread entry function body:\n```cpp\n" + entry_func_code + "\n```" +
        candidates_ss.str() + "\n\n" +
        (l2Mode_ ? l2_hint : explore_hint) +
        (l2Mode_ ? l2_tail :
        "For each resource you report: state the local requirement this thread needs\n"
        "(assume: ORDER / CONFLICT_MEDIATED / REGION_ISOLATED / STABLE_DURING /\n"
        "PROGRESS_ENABLED) and the Level-0 synchronization effects the code actually\n"
        "provides (guarantee: ORDER / EXCLUDE / LINEARIZE / WAIT). Leave guarantee empty\n"
        "if the code provides no synchronization that truly covers that resource. Emit a\n"
        "real assume ONLY for a source-anchored safety obligation: a lifetime-sensitive\n"
        "use region, check-use interval, branch/index/size/pointer/list/lifetime decision,\n"
        "publish-before-use relation, or wait/enabler protocol. Follow the value into\n"
        "callees to decide. Do not classify id/state/count/flag fields as benign merely\n"
        "because the name looks like metadata: if the value is used to match/select/remove\n"
        "an object, gate shutdown/free, or enforce a limit, it needs an anchored assume.\n"
        "Account for every [HIGH-RISK] object: the benign ones go together into a\n"
        "SINGLE no_order_needed=true clause (object_ids=[...]) — do not drop one, and do\n"
        "not fabricate an assume for a value that drives nothing.\n"
        "Attach provenance. Do NOT describe bug patterns; describe only this thread's "
        "obligations.");

    std::string response = send_message(user_prompt, contract.get());

    // --- Validation & Retry Logic ---
    int retries = 0;
    const int MAX_RETRIES = 3;
    bool valid = false;

    while (retries < MAX_RETRIES) {
        // A minimal valid contract must carry at least a role or real content. In
        // L2 mode the content is the node-anchored requirements/guarantees.
        if (!contract->role.empty() || contract->hasOrderContent() ||
            (l2Mode_ && contract->hasL2Content())) {
            valid = true;
            break;
        }

        std::cout << "  [ContractGenerator] Warning: LLM failed to use tools (empty contract). Retrying ("
                  << (retries + 1) << "/" << MAX_RETRIES << ")..." << std::endl;

        std::string retry_prompt = l2Mode_ ?
            std::string(
            "CRITICAL ERROR: You have NOT produced a contract via tools. "
            "Use the tools (no chat text):\n"
            "1. Call `confirm_role_and_summary`.\n"
            "2. Call `add_requirement` (MustPrecede/MustBeAtomic/MustBeMediated) with concrete\n"
            "   node ids, and `add_guarantee` for any real synchronization -- few is fine.\n"
            "3. Call `finalize_contract`.\n"
            "Perform these tool calls NOW based on your analysis.") :
            std::string(
            "CRITICAL ERROR: You have NOT produced a contract via tools. "
            "Use the tools (no chat text):\n"
            "1. Call `confirm_role_and_summary`.\n"
            "2. Call `report_clause` for the resources with a real local requirement or\n"
            "   synchronization guarantee (assume and/or guarantee) -- few is fine.\n"
            "3. Call `finalize_contract`.\n"
            "Perform these tool calls NOW based on your analysis.");

        response = send_message(retry_prompt, contract.get());
        retries++;
    }

    if (valid) {
        // Contract COMPLETENESS pass: a contract is only useful to static composition
        // if every dangerous object is addressed. Selective omission of a HIGH-RISK
        // object (unprotected write / free / list / self-race) is exactly what leaves
        // Phase B with no candidate for the real bug. Drive a few focused repair rounds
        // so each such object gets a position (real assume/guarantee or explicit
        // no_order_needed). Seeded + coverage-enabled (analyst-scoped) runs only.
        // The repair loop drives the legacy report_clause tool, so it is skipped in
        // node-anchored (L2) mode (which has its own declare_no_obligation path).
        if (!l2Mode_ && seeded && coverageEnabled())
            repairContractCoverage(*contract, tid, touchedObjects, objectIds);
        return std::move(*contract);
    }

    std::cerr << "  [ContractGenerator] Error: Failed to generate a valid contract after " << MAX_RETRIES << " retries." << std::endl;
    return std::nullopt;
}

void ContractGeneratorAgent::repairContractCoverage(
    LLM::ConcurrencyContract& contract, int tid,
    const std::vector<const query::SharedObject*>& touchedObjects,
    const std::vector<int>& objectIds) {

    // Fresh read budget for the repair phase: the accessing sources were already
    // preloaded, so the model rarely needs to read, but resetting avoids a
    // budget-exhausted state from the main pass cutting the repair short.
    explore_calls_ = 0;

    for (int round = 0; round < kCoverageRepairRounds; ++round) {
        // HIGH-RISK objects this thread touches that the contract has NOT addressed
        // (no real assume/guarantee, no explicit no_order_needed).
        std::vector<size_t> missing;
        for (size_t k = 0; k < touchedObjects.size(); ++k) {
            const query::SharedObject* o = touchedObjects[k];
            int oid = (k < objectIds.size()) ? objectIds[k] : -1;
            if (!o || oid < 0) continue;
            if (!isHighRiskObject(o)) continue;
            if (contract.addressesObject(oid)) continue;
            missing.push_back(k);
        }
        if (missing.empty()) return;  // contract is complete over high-risk objects

        std::stringstream ss;
        ss << "CONTRACT COMPLETENESS CHECK: the following HIGH-RISK shared object(s) this "
              "thread touches are not yet accounted for. Decide whether each one has a "
              "source-anchored safety obligation for THIS thread.\n"
              "  * If YES (the value flows into a branch/comparison, an index/size/length, a "
              "pointer that is dereferenced, a use-before-free / init-before-publish / "
              "refcount-before-free, or a list/tree link): call report_clause for that "
              "[obj#N] with the matching assume (STABLE_DURING / REGION_ISOLATED / "
              "ORDER / CONFLICT_MEDIATED) (+ real ORDER/EXCLUDE/LINEARIZE/WAIT guarantees "
              "only if the code provides them). FOLLOW the value into callees (get_callees / "
              "get_function_by_name) if the use is not visible at the cited line.\n"
              "  * If NO (the value is only stored/copied/counted/logged and drives nothing): "
              "it is benign. Put ALL such ids together into a SINGLE report_clause with "
              "no_order_needed=true, object_ids=[...], and a one-line no_order_reason.\n"
              "Do NOT invent synchronization that is not in the code, and do NOT fabricate an "
              "assume for a value that drives nothing. After accounting for ALL of them, call "
              "finalize_contract.\n\n";
        for (size_t k : missing) {
            const query::SharedObject* o = touchedObjects[k];
            int oid = objectIds[k];
            ss << "* [obj#" << oid << "] " << (o->name.empty() ? "<anon>" : o->name)
               << "  <-- HIGH-RISK: " << riskTag(o);
            if (!o->type.empty()) ss << "  (type: " << o->type << ")";
            ss << "\n";
            for (const auto& a : o->accesses) {
                if (a.thread_id != tid) continue;
                const std::string& fn = a.containing_function.empty() ? a.function_name
                                                                      : a.containing_function;
                ss << "    - " << a.access_type << " in " << fn << " @ " << a.location
                   << (a.is_lock_protected ? (" [lock=" + a.protecting_lock + "]") : " [no lock]")
                   << "\n          code: " << a.code_snippet << "\n";
            }
        }

        pin_next_user_message();
        send_message(ss.str(), &contract);
    }
}

std::string ContractGeneratorAgent::execute_tool(const std::string& tool_name, const nlohmann::json& arguments) {
    // Try shared tools first. These are read/navigation tools and are the only
    // ones subject to the exploration budget (reporting tools below are never
    // capped, so the model can always emit clauses and finalize).
    auto shared_result = SharedToolKit::handle_shared_tool(tool_name, arguments, ccpg_);
    if (shared_result) {
        explore_calls_++;
        if (explore_calls_ > exploreHard_) {
            // Deterministic stop: end the session with whatever was reported.
            return "finish";
        }
        if (explore_calls_ > exploreSoft_) {
            nlohmann::json parsed = nlohmann::json::parse(*shared_result, nullptr, false);
            nlohmann::json out;
            out["result"] = parsed.is_discarded() ? nlohmann::json(*shared_result) : parsed;
            out["budget_notice"] =
                "Exploration budget nearly exhausted (" + std::to_string(explore_calls_) + "/" +
                std::to_string(exploreHard_) + " reads). STOP reading new functions. Emit "
                "report_clause for each shared resource you have already identified, then call "
                "finalize_contract now.";
            return out.dump();
        }
        return *shared_result;
    }

    // Handle agent-specific tools
    auto* contract = static_cast<LLM::ConcurrencyContract*>(this->get_context_for_tools());
    if (!contract) {
        return R"({"error": "Internal context error: ConcurrencyContract not found."})";
    }

    // Paper-faithful node-anchored mode: route to the L2 tool handlers. Returns a
    // non-empty result when handled (including the "finish" sentinel); an empty
    // string means the tool is not an L2 tool and falls through to the error below.
    if (l2Mode_) {
        std::string r = execute_l2_tool(tool_name, arguments, contract);
        if (!r.empty()) return r;
        nlohmann::json err;
        err["error"] = "Tool '" + tool_name + "' is not available in node-anchored (L2) mode. "
                       "Use add_requirement / add_guarantee / declare_no_obligation / finalize_contract.";
        return err.dump();
    }

    // Bound the reporting loop: report_clause/confirm/report_ordering are not capped
    // by the read budget, so force the session to finish once the model has spent
    // more reporting rounds than the seeded objects could possibly need.
    if (tool_name == "confirm_role_and_summary" || tool_name == "report_clause" ||
        tool_name == "report_ordering") {
        if (++reportRounds_ > reportHardCap()) {
            return "finish";
        }
    }

    if (tool_name == "confirm_role_and_summary") {
        contract->setRole(arguments.at("role").get<std::string>());
        contract->setSummary(arguments.at("summary").get<std::string>());
        return R"({"status": "Role/summary recorded. Now call report_clause ONLY for resources with a real local requirement or synchronization guarantee (skip pure reads/counters/config scalars; merge same-lock fields via object_ids), then finalize_contract."})";
    }

    if (tool_name == "report_clause") {
        LLM::ConcurrencyContract::OrderClause clause;
        clause.resource = arguments.value("resource", std::string());
        if (clause.resource.empty())
            return R"({"error":"report_clause needs a non-empty 'resource'."})";
        // Anchor to surface object(s): accept a single object_id and/or an object_ids
        // array (lock-region grouping). Dedup; objectId stays = the first id (compat).
        {
            std::set<int> seen;
            auto add = [&](int v) { if (v >= 0 && seen.insert(v).second) clause.objectIds.push_back(v); };
            if (arguments.contains("object_id") && arguments["object_id"].is_number_integer())
                add(arguments["object_id"].get<int>());
            if (arguments.contains("object_ids") && arguments["object_ids"].is_array())
                for (const auto& v : arguments["object_ids"])
                    if (v.is_number_integer()) add(v.get<int>());
            if (!clause.objectIds.empty()) clause.objectId = clause.objectIds.front();
        }
        if (!seededObjectIds_.empty()) {
            if (clause.objectIds.empty()) {
                return R"({"error":"Seeded ThreadContract clauses must bind to the static surface with object_id or object_ids. Use the [obj#N] id shown in the evidence packet, or use report_ordering for a thread-level cross-resource note."})";
            }
            for (int oid : clause.objectIds) {
                if (!seededObjectIds_.count(oid)) {
                    nlohmann::json err;
                    err["error"] = "object_id " + std::to_string(oid) +
                        " was not listed in this thread's evidence packet. Use only the shown [obj#N] ids.";
                    return err.dump();
                }
            }
        }

        if (arguments.contains("sites") && arguments["sites"].is_array())
            for (const auto& s : arguments["sites"])
                if (s.is_string()) clause.sites.push_back(s.get<std::string>());

        // Keep only relations from the current closed contract vocabulary. Legacy
        // outputs are accepted and mapped into the new obligation/atom names.
        static const std::set<std::string> kAssumeRel = {
            "ORDER", "CONFLICT_MEDIATED", "REGION_ISOLATED",
            "STABLE_DURING", "PROGRESS_ENABLED"
        };
        static const std::set<std::string> kGuardRel = {
            "ORDER", "EXCLUDE", "LINEARIZE", "WAIT"
        };
        int dropped = 0;
        if (arguments.contains("assume") && arguments["assume"].is_array()) {
            for (const auto& it : arguments["assume"]) {
                if (!it.is_object()) continue;
                std::string rel = canonicalRequirementRelation(it.value("relation", std::string()));
                if (!kAssumeRel.count(rel)) { ++dropped; continue; }
                clause.assume.push_back({rel, it.value("detail", std::string()),
                                         it.value("provenance", std::string())});
            }
        }
        if (arguments.contains("guarantee") && arguments["guarantee"].is_array()) {
            for (const auto& it : arguments["guarantee"]) {
                if (!it.is_object()) continue;
                std::string rel = canonicalGuaranteeRelation(it.value("relation", std::string()));
                if (!kGuardRel.count(rel)) { ++dropped; continue; }
                clause.guarantee.push_back({rel, it.value("detail", std::string()),
                                            it.value("provenance", std::string())});
            }
        }
        // Explicit "no order obligation" declaration (contract completeness): record
        // it so a reviewed-benign high-risk object is auditable rather than silently
        // omitted. Only honored when no real order content was given.
        if (arguments.value("no_order_needed", false) &&
            clause.assume.empty() && clause.guarantee.empty()) {
            // Guard against the common false-benign mistake: ids/states/counters look
            // harmless by name, but if this thread's own access snippet already shows
            // the value in a branch/comparison, size/index, free/refcount, or list/tree
            // operation, it is consequential and must get a real assume instead, even
            // if the field name looks like metadata and the model tries to batch it into
            // the benign bucket.
            for (int oid : clause.objectIds) {
                auto it = seededObjectsById_.find(oid);
                if (it == seededObjectsById_.end()) continue;
                std::string evidence;
                if (hasObviousLocalConsequence(it->second, contract->threadId, &evidence)) {
                    nlohmann::json err;
                    err["error"] =
                        "no_order_needed rejected for [obj#" + std::to_string(oid) +
                        "]: this thread's access has an obvious consequential use. "
                        "Do not treat id/state/count/flag metadata as benign when it "
                        "drives a branch/comparison, selection/removal, size/index, "
                        "free/refcount, or list/tree operation. Emit report_clause with "
                        "a real assume such as REGION_ISOLATED(check_use_region, "
                        "invalidators) or STABLE_DURING(use_region, live(resource)), "
                        "or split this object out of "
                        "the benign object_ids list.";
                    if (!evidence.empty()) err["evidence"] = evidence;
                    return err.dump();
                }
            }
            clause.noOrderNeeded = true;
            clause.noOrderReason = arguments.value("no_order_reason", std::string());
        }

        contract->addClause(clause);
        nlohmann::json resp;
        if (dropped > 0)
            resp["warning"] = "Dropped " + std::to_string(dropped) +
                " relation(s) outside the closed vocabulary (assume: ORDER/CONFLICT_MEDIATED/REGION_ISOLATED/STABLE_DURING/PROGRESS_ENABLED; guarantee: ORDER/EXCLUDE/LINEARIZE/WAIT).";

        // Selective progress feedback (survives token pruning -- it is the latest tool
        // result). Do NOT push the model to cover every object: a contract is a
        // high-signal summary, and unreported objects are caught by the static floor.
        // Just confirm and steer toward finalizing once the few real obligations are in.
        resp["clauses_so_far"] = static_cast<int>(contract->clauses.size());
        resp["status"] =
            "Clause recorded. For remaining [HIGH-RISK] objects, emit a real assume ONLY "
            "when there is an anchored safety obligation (branch/selection/removal, index/size, "
            "pointer/lifetime, list/tree, limit check, publish/use, wait/enabler). Batch the rest into ONE "
            "no_order_needed=true clause with object_ids=[...]. id/state/count/flag fields "
            "are NOT benign if they drive a decision. Then finalize_contract.";
        return resp.dump();
    }

    if (tool_name == "report_ordering") {
        std::string detail = arguments.value("detail", std::string());
        if (detail.empty()) return R"({"error":"report_ordering needs a non-empty 'detail'."})";
        std::string prov = arguments.value("provenance", std::string());
        contract->addOrdering(prov.empty() ? detail : (detail + "  [" + prov + "]"));
        return R"({"status": "Thread-level ordering recorded."})";
    }

    if (tool_name == "finalize_contract") {
        return "finish"; // Signal completion
    }

    nlohmann::json error_resp;
    error_resp["error"] = "Tool '" + tool_name + "' not found or not implemented by this agent.";
    return error_resp.dump();
}

std::string ContractGeneratorAgent::execute_l2_tool(
    const std::string& tool_name, const nlohmann::json& arguments,
    LLM::ConcurrencyContract* contract) {

    // Bound the reporting loop the same way the legacy path does: node-anchored
    // reporting tools are not subject to the read budget, so cap total rounds.
    if (tool_name == "confirm_role_and_summary" || tool_name == "add_requirement" ||
        tool_name == "add_guarantee" || tool_name == "declare_no_obligation") {
        if (++reportRounds_ > reportHardCap()) return "finish";
    }

    // Parse a node-id array argument, validating each id against the CCPG. Collects
    // the invalid ones so the model gets an actionable error instead of a silent drop.
    auto parseNodes = [&](const char* key, std::vector<int>& out, std::vector<int>& invalid) {
        if (!arguments.contains(key) || !arguments[key].is_array()) return;
        std::set<int> seen;
        for (const auto& v : arguments[key]) {
            if (!v.is_number_integer()) continue;
            int id = v.get<int>();
            if (!seen.insert(id).second) continue;
            if (ccpg_ && ccpg_->getNodeByID(id)) out.push_back(id);
            else invalid.push_back(id);
        }
    };

    if (tool_name == "confirm_role_and_summary") {
        contract->setRole(arguments.value("role", std::string()));
        contract->setSummary(arguments.value("summary", std::string()));
        return R"({"status":"Role/summary recorded. Now call add_requirement / add_guarantee with concrete node ids, then finalize_contract."})";
    }

    if (tool_name == "add_requirement") {
        static const std::set<std::string> kForms = {"MustPrecede", "MustBeAtomic", "MustBeMediated"};
        std::string form = arguments.value("form", std::string());
        if (!kForms.count(form)) {
            return R"({"error":"add_requirement.form must be one of MustPrecede | MustBeAtomic | MustBeMediated."})";
        }
        LLM::ConcurrencyContract::NodeReq r;
        r.form = form;
        std::vector<int> invalid;
        parseNodes("a_nodes", r.a, invalid);
        parseNodes("b_nodes", r.b, invalid);
        if (r.a.empty()) {
            nlohmann::json err;
            err["error"] = "add_requirement needs a non-empty a_nodes of valid CCPG node ids.";
            if (!invalid.empty()) err["invalid_node_ids"] = invalid;
            return err.dump();
        }
        if (form != "MustBeAtomic" && r.b.empty()) {
            return R"({"error":"MustPrecede / MustBeMediated need a non-empty b_nodes (the second operand)."})";
        }
        r.objectId = arguments.contains("object_id") && arguments["object_id"].is_number_integer()
                         ? arguments["object_id"].get<int>() : -1;
        r.note = arguments.value("note", std::string());
        contract->addNodeReq(r);
        nlohmann::json resp;
        resp["status"] = "Requirement recorded (" + form + "). Continue with more requirements/guarantees, then finalize_contract.";
        resp["requirements_so_far"] = static_cast<int>(contract->nodeReqs.size());
        if (!invalid.empty()) resp["ignored_invalid_node_ids"] = invalid;
        return resp.dump();
    }

    if (tool_name == "add_guarantee") {
        static const std::set<std::string> kForms = {"Order", "Exclude", "AtomicOp", "Wait"};
        std::string form = arguments.value("form", std::string());
        if (!kForms.count(form)) {
            return R"({"error":"add_guarantee.form must be one of Order | Exclude | AtomicOp | Wait."})";
        }
        LLM::ConcurrencyContract::NodeGuar g;
        g.form = form;
        std::vector<int> invalid;
        parseNodes("a_nodes", g.a, invalid);
        parseNodes("b_nodes", g.b, invalid);
        if (g.a.empty()) {
            nlohmann::json err;
            err["error"] = "add_guarantee needs a non-empty a_nodes of valid CCPG node ids.";
            if (!invalid.empty()) err["invalid_node_ids"] = invalid;
            return err.dump();
        }
        if ((form == "Order" || form == "Wait") && g.b.empty()) {
            return R"({"error":"Order / Wait need a non-empty b_nodes (the target / continuation)."})";
        }
        if ((form == "Exclude" || form == "AtomicOp") && arguments.value("token", std::string()).empty()) {
            return R"({"error":"Exclude / AtomicOp need a non-empty 'token' (the lock/atomic that coordinates the region)."})";
        }
        g.token = arguments.value("token", std::string());
        g.mode = arguments.value("mode", std::string());
        g.objectId = arguments.contains("object_id") && arguments["object_id"].is_number_integer()
                         ? arguments["object_id"].get<int>() : -1;
        g.note = arguments.value("note", std::string());
        contract->addNodeGuar(g);
        nlohmann::json resp;
        resp["status"] = "Guarantee recorded (" + form + ").";
        resp["guarantees_so_far"] = static_cast<int>(contract->nodeGuars.size());
        if (!invalid.empty()) resp["ignored_invalid_node_ids"] = invalid;
        return resp.dump();
    }

    if (tool_name == "declare_no_obligation") {
        if (!arguments.contains("object_id") || !arguments["object_id"].is_number_integer())
            return R"({"error":"declare_no_obligation needs an integer object_id."})";
        contract->addNoObligation(arguments["object_id"].get<int>());
        return R"({"status":"No-obligation declaration recorded."})";
    }

    if (tool_name == "finalize_contract") {
        return "finish";
    }

    return std::string();  // not an L2 tool
}

std::string ContractGeneratorAgent::parseResult(const std::vector<llm_client::ChatMessage>& history) {
    // The contract is built progressively in the context object.
    // A simple confirmation is sufficient.
    return "Contract generated.";
}

std::string ContractGeneratorAgent::preloadSource(
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

} // namespace llm_client
