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
    : Conversation(client, build_system_prompt(), 25), ccpg_(ccpg) {}

std::string ContractGeneratorAgent::build_system_prompt() {
    return R"CONTRACT(
You are an expert in concurrent Linux-kernel C code. For ONE thread (given its entry
function and creation site), build that thread's "concurrency contract": an
ORDER/SYNCHRONIZATION assume-guarantee description of how it touches shared state.

**CRITICAL: DO NOT REPLY WITH CHAT TEXT. ONLY USE THE PROVIDED TOOLS.**

A contract describes ONE thread's order obligations. It has NO notion of a "bug" and
is NOT a bug/defect pattern -- describe only what THIS thread requires and provides.
A contract is a SELECTIVE, high-signal summary, NOT a per-variable enumeration: the
shared objects listed to you are the surface's exhaustive access inventory and do not
need restating. Emit a clause for a resource ONLY when THIS thread either (1) has a
real order requirement for its OWN correctness, or (2) provides synchronization that
covers it. For each such resource you state two things using a CLOSED relation algebra:

  assume[] : the execution ORDER this thread REQUIRES for its OWN correctness
             (the inferred INTENT; it is usually NOT written literally in the source):
    * prec(a, b)             : event a must happen-before event b. Common instances:
                               prec(use, free) (use before free);
                               prec(init, expose) (fully initialize before the object/
                               pointer is published or made reachable).
    * atomic([a, b, ...])    : that region of events must not be interleaved by a
                               conflicting outside event (a single event means
                               "no concurrent conflicting write", the data-race case).
    * count_guarded(R, free) : R may be freed only after its refcount reaches zero.

  guarantee[] : the order this thread actually ESTABLISHES via SYNCHRONIZATION present
             in the buggy code (do NOT invent synchronization that is not there):
    * serialize(L, region)   : a lock/context (mutex, spinlock, RCU read-side,
                               irq/preempt disable) makes the region mutually exclusive.
                               Name the real lock as L (e.g. serialize(sk->sk_lock, region)).
    * order(a < b via m)     : a primitive m establishes a happens-before. Write m as ONE
                               canonical token below so composition can classify it.
                               HARD (hardware/compiler-backed memory ordering):
                                 - rcu       : synchronize_rcu / call_rcu / kfree_rcu /
                                               rcu read-side + rcu_assign_pointer
                                 - barrier   : smp_mb / smp_store_release / smp_load_acquire
                                 - join      : flush_work / cancel_work_sync / kthread_stop /
                                               wait_for_completion / del_timer_sync /
                                               synchronize_irq / napi_disable
                                 - refcount  : refcount/kref drop-then-free (atomic RMW)
                               SOFT (program order only, NOT memory-ordered -> will be
                               VERIFIED, never trusted blindly; label honestly):
                                 - published_flag : a plain bool/state bit (->valid/->dead/
                                                    ->closing) gates the access
                                 - program_order  : statements merely reordered, no barrier
                               e.g. order(publish < use via rcu); order(set < read via published_flag).
    * counts(R)              : refcount discipline maintains count_guarded(R).

RED LINE: use ONLY the six relations above. Do NOT enumerate "patterns" (no
"check-use-release", no "state-guard pattern"). Just describe THIS thread's order
requirements and the synchronization it provides. The relations are general and
subsystem-independent -- never tailor them to a specific bug.

KEEP PROTECTION HONEST: only list a guarantee that the code actually provides AND that
covers the resource in question. A lock that guards a different field, or is dropped
before the access, does NOT establish order for this resource -- omit it (leave
guarantee empty). Static "candidate locks" you see may not protect this object; judge
from the code what each one actually serializes.

Use get_callees / get_function_by_name to follow the thread's real work into callees
before deciding the requirements. Attach PROVENANCE (a source line / caller) to every
assume and guarantee.

SELECTIVITY vs COMPLETENESS: emit a clause when one of these holds for THIS thread:
  * a genuine order requirement: use-before-free / free-before-destroy /
    check-then-act / init-before-publish / refcount-before-free; or
  * the resource's racy value flows into a branch / index / size / pointer /
    lifetime decision (a NON-benign torn read -> assume atomic); or
  * this thread provides a real synchronization discipline over it
    (guarantee: serialize / order / counts).
For an ordinary read-only scalar / statistics counter / config value that drives no
decision, you may skip it.

COVERAGE MANDATE (completeness is what makes the contract sound): some listed objects
are marked **HIGH-RISK** (the static surface saw an unsynchronized write, a free, a
list mutation, or a self-race on them). You must ACCOUNT FOR every high-risk object --
never silently drop one -- but you decide its disposition by ONE test: does the racy
value have a CONSEQUENTIAL USE?

  CONSEQUENTIAL USE (=> emit a real assume/guarantee): the racy value flows into
    * control flow  -- a branch / comparison / condition / predicate; or
    * an index / a size / a length / a loop bound; or
    * a pointer that is dereferenced or stored for later dereference; or
    * a lifetime / free decision -- use-before-free, init-before-publish,
      refcount-before-free; or
    * a list / tree link that is mutated or traversed.
    FOLLOW THE VALUE: if this object's racy value is passed into a callee, returned to a
    caller, or copied to an alias, follow it (get_callees / get_function_by_name) to the
    point of use before deciding -- the consequence is often one or two calls away (e.g.
    a charge/RMW helper, a list helper, a deref in a worker). Do not judge "benign" just
    because the cited line itself only loads/stores it.

  NO CONSEQUENTIAL USE (=> benign): the value is only stored, copied, counted, or logged
    and never reaches any of the uses above.

DISPOSITION:
  (a) Consequential objects: emit report_clause with the matching assume (and any real
      guarantee). These are few -- only the values that actually drive a decision.
  (b) Benign high-risk objects: do NOT emit one clause each. Make ONE report_clause with
      no_order_needed=true and object_ids=[ALL benign high-risk ids], plus a one-line
      no_order_reason. This keeps the contract complete (every high-risk object is
      accounted for) without inventing order requirements.
COMMON TRAP: fields named id/state/count/flag often look like harmless metadata, but
they are consequential whenever they participate in equality/range predicates,
matching/selection/removal decisions, shutdown/free gates, or limit checks. Such uses
need an order requirement (often `assume atomic([read, branch])` or
`assume atomic([read, limit_check])`), not no_order_needed.
Be honest in BOTH directions: do not fabricate an atomic([...]) for a value that drives
nothing (that is a false positive), and do not bucket as benign a value that feeds a
branch/index/pointer/lifetime (that is a missed bug).
LOCK-REGION MERGING: when several listed fields are protected by the SAME lock/region,
emit ONE clause that lists all of them in object_ids and states a single serialize(...)
guarantee -- do NOT repeat the same guarantee field-by-field.

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

std::vector<Tool> ContractGeneratorAgent::get_available_tools() const {
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
        {"description", "Order this thread REQUIRES for its own correctness (the inferred intent)."},
        {"items", reqItem("prec | atomic | count_guarded",
                          "\"prec(use, free)\" | \"prec(init, expose)\" | \"atomic([check, use])\" | \"count_guarded(R, free)\"")}
    };
    nlohmann::json guarantee_schema = {
        {"type", "array"},
        {"description", "Order this thread ESTABLISHES via synchronization actually present in the code (may be empty). "
                        "For order(a < b via m), m MUST be one canonical token: HARD = rcu|barrier|join|refcount; "
                        "SOFT = published_flag|program_order (verified, not trusted)."},
        {"items", reqItem("serialize | order | counts",
                          "\"serialize(sk->sk_lock, region)\" | \"order(publish < use via rcu)\" | \"order(set < read via published_flag)\" | \"counts(R)\"")}
    };
    nlohmann::json sites_schema = {
        {"type", "array"},
        {"description", "Provenance for the clause's operations, as \"function @ file:line\" strings."},
        {"items", {{"type", "string"}}}
    };

    nlohmann::json object_ids_schema = {
        {"type", "array"},
        {"description", "The [obj#N] indices this clause covers. Use MULTIPLE ids to merge "
                        "several fields under the SAME lock/region into one serialize clause."},
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
        "Report THIS thread's order/sync obligations for ONE shared resource, using only the "
        "closed relation algebra (assume: prec/atomic/count_guarded; guarantee: serialize/order/counts). "
        "For a HIGH-RISK object with no obligation, set no_order_needed=true + no_order_reason.",
        std::move(clause_params)});

    tools.push_back({"report_ordering", "Optional: a thread-level cross-resource order requirement.", {
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
        // The objects this thread touches, with THIS thread's own accesses.
        candidates_ss << "\n--- Shared objects THIS thread touches (from static analysis) ---\n"
                         "This is the surface's EXHAUSTIVE inventory -- you do NOT need to restate it.\n"
                         "Skip pure reads, statistics counters, and config scalars that drive no\n"
                         "decision. Set object_id to the [obj#N] number (or object_ids for several\n"
                         "fields under the SAME lock) so the resource is matched unambiguously. You\n"
                         "generally do NOT need to explore the call graph -- the accessing functions\n"
                         "are preloaded below.\n";
        if (coverage)
            candidates_ss << "Every object tagged [HIGH-RISK] below must be ACCOUNTED FOR. Emit a real\n"
                             "assume only for those whose racy value has a CONSEQUENTIAL USE (branch /\n"
                             "index / size / pointer-deref / use-before-free / list-or-tree); follow the\n"
                             "value into callees if needed. Put ALL the remaining (benign) HIGH-RISK ids\n"
                             "into a SINGLE no_order_needed=true clause (object_ids=[...]). Do NOT silently\n"
                             "drop a HIGH-RISK object, and do NOT invent an assume for a value that drives\n"
                             "nothing. IMPORTANT: fields named id/state/count/flag are NOT automatically\n"
                             "benign metadata -- if they are used to match/select/remove an object, gate a\n"
                             "shutdown/free, or enforce a limit, they have CONSEQUENTIAL USE and need an\n"
                             "assume (e.g. atomic([read, branch])).\n";
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
                candidates_ss << "    - " << a.access_type << " in " << fn << " @ " << a.location
                              << (a.is_lock_protected ? (" [lock=" + a.protecting_lock + "]")
                                                      : " [no lock]")
                              << "\n          code: " << a.code_snippet << "\n";
                if (!fn.empty()) preloadNames.insert(fn);
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
          "that have a real order requirement or synchronization discipline (merge same-lock\n"
          "fields into one clause via object_ids), then finalize_contract. Skip pure\n"
          "reads/counters/config scalars -- emitting few clauses is correct. Only if a clause\n"
          "genuinely depends on code not shown should you call get_function_by_name first.\n\n"
        : "Follow the thread's real work with `get_callees` / `get_function_by_name` before\n"
          "deciding the requirements — the access that matters is often in a callee.\n"
          "If the entry runs an opaque event loop (`event_base_loop`, `uv_run`, a worker\n"
          "dispatch), search for the handlers it invokes and reason about the shared state\n"
          "they touch; note in the summary that those callees are inferred.\n\n";

    std::string user_prompt = 
        "Build the concurrency contract for the following thread: its per-resource ORDER\n"
        "requirements (assume) and the synchronization it ESTABLISHES (guarantee).\n"
        "Fork statement: " + fork_stmt + "\n"
        "Thread entry function body:\n```cpp\n" + entry_func_code + "\n```" +
        candidates_ss.str() + "\n\n" +
        explore_hint +
        "For each resource you report: state what order this thread NEEDS for its own\n"
        "correctness (assume: prec/atomic/count_guarded) and what order it actually\n"
        "ENFORCES here (guarantee: serialize/order/counts) — leave guarantee empty if the\n"
        "code provides no synchronization that truly covers that resource. Emit a real\n"
        "assume ONLY for a value with a CONSEQUENTIAL USE (branch / index / size /\n"
        "pointer-deref / use-before-free / list-or-tree); follow the value into callees to\n"
        "decide. Do not classify id/state/count/flag fields as benign merely because the\n"
        "name looks like metadata: if the value is used to match/select/remove an object,\n"
        "gate shutdown/free, or enforce a limit, it has CONSEQUENTIAL USE and needs an\n"
        "assume (typically atomic([read, branch]) or atomic([read, limit_check])).\n"
        "Account for every [HIGH-RISK] object: the benign ones go together into a\n"
        "SINGLE no_order_needed=true clause (object_ids=[...]) — do not drop one, and do\n"
        "not fabricate an assume for a value that drives nothing.\n"
        "Attach provenance. Do NOT describe bug patterns; describe only this thread's "
        "obligations.";

    std::string response = send_message(user_prompt, contract.get());

    // --- Validation & Retry Logic ---
    int retries = 0;
    const int MAX_RETRIES = 3;
    bool valid = false;

    while (retries < MAX_RETRIES) {
        // A minimal valid contract must carry at least a role or one order clause.
        if (!contract->role.empty() || contract->hasOrderContent()) {
            valid = true;
            break;
        }

        std::cout << "  [ContractGenerator] Warning: LLM failed to use tools (empty contract). Retrying ("
                  << (retries + 1) << "/" << MAX_RETRIES << ")..." << std::endl;

        std::string retry_prompt = 
            "CRITICAL ERROR: You have NOT produced a contract via tools. "
            "Use the tools (no chat text):\n"
            "1. Call `confirm_role_and_summary`.\n"
            "2. Call `report_clause` for the resources with a real order requirement or\n"
            "   synchronization discipline (assume and/or guarantee) -- few is fine.\n"
            "3. Call `finalize_contract`.\n"
            "Perform these tool calls NOW based on your analysis.";

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
        if (seeded && coverageEnabled())
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
              "thread touches are not yet accounted for. Decide each by ONE test -- does its "
              "racy value have a CONSEQUENTIAL USE?\n"
              "  * If YES (the value flows into a branch/comparison, an index/size/length, a "
              "pointer that is dereferenced, a use-before-free / init-before-publish / "
              "refcount-before-free, or a list/tree link): call report_clause for that "
              "[obj#N] with the matching assume (atomic([...]) / prec(use,free) / "
              "prec(init,expose) / count_guarded(R,free)) (+ a real guarantee only if the "
              "code provides one). FOLLOW the value into callees (get_callees / "
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
        return R"({"status": "Role/summary recorded. Now call report_clause ONLY for resources with a real order requirement or synchronization discipline (skip pure reads/counters/config scalars; merge same-lock fields via object_ids), then finalize_contract."})";
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

        if (arguments.contains("sites") && arguments["sites"].is_array())
            for (const auto& s : arguments["sites"])
                if (s.is_string()) clause.sites.push_back(s.get<std::string>());

        // WF4: keep only relations from the closed algebra; drop the rest with a note.
        static const std::set<std::string> kAssumeRel  = {"prec", "atomic", "count_guarded"};
        static const std::set<std::string> kGuardRel    = {"serialize", "order", "counts"};
        int dropped = 0;
        if (arguments.contains("assume") && arguments["assume"].is_array()) {
            for (const auto& it : arguments["assume"]) {
                if (!it.is_object()) continue;
                std::string rel = it.value("relation", std::string());
                if (!kAssumeRel.count(rel)) { ++dropped; continue; }
                clause.assume.push_back({rel, it.value("detail", std::string()),
                                         it.value("provenance", std::string())});
            }
        }
        if (arguments.contains("guarantee") && arguments["guarantee"].is_array()) {
            for (const auto& it : arguments["guarantee"]) {
                if (!it.is_object()) continue;
                std::string rel = it.value("relation", std::string());
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
                        "a real assume such as atomic([read, branch]) / atomic([read, "
                        "limit_check]) / prec(use,free), or split this object out of "
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
                " relation(s) outside the closed algebra (assume: prec/atomic/count_guarded; guarantee: serialize/order/counts).";

        // Selective progress feedback (survives token pruning -- it is the latest tool
        // result). Do NOT push the model to cover every object: a contract is a
        // high-signal summary, and unreported objects are caught by the static floor.
        // Just confirm and steer toward finalizing once the few real obligations are in.
        resp["clauses_so_far"] = static_cast<int>(contract->clauses.size());
        resp["status"] =
            "Clause recorded. For remaining [HIGH-RISK] objects, emit a real assume ONLY "
            "when the value has consequential use (branch/selection/removal, index/size, "
            "pointer/lifetime, list/tree, limit check). Batch the rest into ONE "
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
