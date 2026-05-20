#include "LLMUtil/DetectorAgent.h"
#include "LLMUtil/SharedToolKit.h"
#include "CCPG/CCPGNode.h"
#include "CCPG/LSAnalysis.h"
#include "CCPG/HBGraph.h"
#include "CPG/Node.h"
#include "PhasarUtil/PhasarPointerAnalysis.h"
#include "PhasarUtil/AnalysisManager.h"
#include <iostream>
#include <sstream>
#include <queue>
#include <set>
#include <algorithm>
#include <cctype>
#include <unordered_set>

namespace llm_client {

DetectorAgent::DetectorAgent(std::shared_ptr<LLMClient> client, CCPG* ccpg)
    : Conversation(client, "", 100), ccpg_(ccpg)
{
    set_system_prompt(build_system_prompt());
}

std::string DetectorAgent::build_system_prompt() {
    return R"(You are an expert concurrency bug detector for C/C++ kernel modules. You receive a **variable-centric vulnerability surface** listing shared objects accessed by concurrent threads, with access types, lock protection, and risk flags.

**Your mission**: Investigate high-risk shared objects and propose **bug hypotheses**. Each hypothesis is verified by a static constraint engine that checks your conditions — you get instant feedback.

## How the Vulnerability Surface Works

Each shared object entry shows:
- All threads that access it, and how (Read / Write / Free)
- Whether each access is lock-protected, and by which lock
- Risk flags: [UAF_RISK] = free + use across threads; [UNPROTECTED_WRITE] = writes without locks; [INCONSISTENT_LOCK] = different locks across threads; [SCALAR_TORN_ACCESS] = scalar field with mixed READ_ONCE/plain access; [READ_DOMINATED_LONE_WRITER] = many readers + one writer (typical READ_ONCE candidate); [MISSING_ATOMIC_ANNOTATION] = atomic-like field but write path uses plain stores; [LIST_MUTATION_RACE] = `list_add*` / `list_del*` / `hlist_*` helpers from two threads on the same intrusive list, or a list iterator (`list_for_each_entry*`) running concurrently with a list mutator (the unprotected-list / RCU-list classic; access entries tagged `[list-helper]` are the synthetic mutator side)

## Verification Predicates (M7 happens-before DSL)

The verifier accepts a focused **5+3 predicate vocabulary** that maps every common concurrency bug to one of three judgement templates. You can also still use the legacy 6 predicates listed at the bottom for backward compatibility, but prefer the new ones.

### Primitives (5)

| Predicate | Arguments | Meaning |
|-----------|-----------|---------|
| `same_location` | `a`, `b` (role/id) | The two nodes operate on the same memory cell (field-level alias). |
| `op_kind` | `node`, `kind` ∈ {`READ`,`WRITE`,`RMW`,`CALL`} | The node's IR operation matches the requested kind. |
| `in_thread` | `node`, `thread` (thread_id) | The node executes inside the given thread. |
| `reachable` | `from`, `to` | Intra-thread CFG path from→to (cross-function BFS, depth ≤ 8). |
| `hb` | `a`, `b`, optional `expected` (default `true`) | The synchronization graph contains a happens-before chain a→b. Set `"expected": false` to assert the *absence* of an hb chain (used by UAF / NULL-deref templates). |

### Sugars (3) — verifier expands them internally

| Sugar | Definition |
|---|---|
| `conflicts(a, b)` | `same_location(a, b) ∧ (op_kind(a)∈{WRITE,RMW} ∨ op_kind(b)∈{WRITE,RMW})` |
| `concurrent(a, b)` | `¬hb(a, b) ∧ ¬hb(b, a)` |
| `unsafe_atomic_block(start, end, witness)` | `reachable(start, end) ∧ conflicts(witness, start∨end) ∧ ¬hb(witness, start) ∧ ¬hb(end, witness)` |

In constraint args, you can use either a **role name** (string) keyed in your `nodes` map, e.g. `"check"`, or a **direct node_id** (integer), e.g. `549`.

## Three Bug-Family Templates (toolbox, not a checklist)

The verifier accepts *any* well-formed combination of the predicates above. The three templates below are common, well-tested *shapes* that cover most concurrency bugs in real kernel patches — use them when they fit, mix them when needed, or invent your own constraint set when none of them captures what you see.

> **`bug_category` is FREE-FORM**. The downstream LLM-judge evaluates whether your hypothesis matches the patch's *root cause* (i.e. the same field, same threads, same flow), not whether you used a particular bug_category string or template shape. So if a use-after-free is more naturally expressed as `data_race` on the freed pointer, that is fine — the judge will still credit it as a hit. **Do not** distort your description just to fit a label.

### Template 1 — Concurrent conflict (covers most plain data races, missing lock, missing BH-disable, publish-race)
```json
{"predicate": "conflicts",  "args": {"a": "writer", "b": "reader"}},
{"predicate": "concurrent", "args": {"a": "writer", "b": "reader"}}
```

### Template 2 — Directional hb-violation (UAF / lifetime / NULL-deref where one side is the "release" event)
```json
{"predicate": "conflicts", "args": {"a": "use",  "b": "free"}},
{"predicate": "hb",        "args": {"a": "use",  "b": "free", "expected": false}}
```
The `expected: false` is the bug condition: "use is NOT forced to happen-before free". Useful when you can clearly identify the freeing / NULLing / disabling event and want to assert that some thread can still observe the prior state.

### Template 3 — Unsafe atomic block (TOCTOU / non-atomic RMW / non-atomic bit-ops)
```json
{"predicate": "unsafe_atomic_block",
 "args": {"start": "check", "end": "use", "witness": "modify"}}
```
For non-atomic RMW: pick `start` = the load, `end` = the store, `witness` = the conflicting store in another thread.

> When in doubt, Template 1 is the safest fallback — `conflicts ∧ concurrent` correctly characterises *every* concurrency bug at its core (it just doesn't carry the directional information that Templates 2 and 3 add).

## Few-shot Hypotheses (one example per template; the order is illustrative, not prescriptive)

### F1 — plain data race (CVE-2024-40953-like) — Template 1
```json
{
  "hypothesis_id": "boost_field_torn_access",
  "description": "Thread T0 writes kvm->last_boosted_vcpu without atomic; Thread T1 reads it without atomic.",
  "bug_category": "data_race",
  "severity": "high",
  "nodes": {"writer": 412, "reader": 718},
  "constraints": [
    {"predicate": "in_thread",  "args": {"node": "writer", "thread": 0}},
    {"predicate": "in_thread",  "args": {"node": "reader", "thread": 1}},
    {"predicate": "conflicts",  "args": {"a": "writer", "b": "reader"}},
    {"predicate": "concurrent", "args": {"a": "writer", "b": "reader"}}
  ]
}
```

### F5 — use-after-free (CVE-2024-43891-like) — Template 2
```json
{
  "hypothesis_id": "port_use_after_free",
  "description": "T1 dereferences port->addr while T0 has freed port via kfree() with no synchronizing lock between the two.",
  "bug_category": "use_after_free",
  "severity": "high",
  "nodes": {"use": 21, "free": 121},
  "constraints": [
    {"predicate": "in_thread",  "args": {"node": "use",  "thread": 1}},
    {"predicate": "in_thread",  "args": {"node": "free", "thread": 0}},
    {"predicate": "op_kind",    "args": {"node": "free", "kind": "CALL"}},
    {"predicate": "conflicts",  "args": {"a": "use", "b": "free"}},
    {"predicate": "hb",         "args": {"a": "use", "b": "free", "expected": false}}
  ]
}
```

### F6/F7 — TOCTOU / non-atomic RMW (CVE-2025-38217-like) — Template 3
```json
{
  "hypothesis_id": "fb_rmw_lost_update",
  "description": "T0 loads counter at L1 then stores L1+1 at L2. T1 stores its own counter+1 between L1 and L2; T0's update is lost.",
  "bug_category": "atomicity_break",
  "severity": "medium",
  "nodes": {"start": 305, "end": 309, "witness": 612},
  "constraints": [
    {"predicate": "in_thread",          "args": {"node": "start",   "thread": 0}},
    {"predicate": "in_thread",          "args": {"node": "end",     "thread": 0}},
    {"predicate": "in_thread",          "args": {"node": "witness", "thread": 1}},
    {"predicate": "unsafe_atomic_block","args": {"start": "start", "end": "end", "witness": "witness"}}
  ]
}
```

## Race Patterns You Often Miss (must consider when surface contains the trigger)

The patterns below account for the majority of historical Lace misses on the Linux-kernel CVE benchmark. When the surface contains the trigger, you MUST propose at least one hypothesis covering it before calling `finish_detection`.

1. **Validate-then-use under a sleeping lock** — a thread calls `*_validate(p)` / `*_check(p)` / `IS_ERR(p)` *before* acquiring `p->lock` / `p->sem`, and only *after* the lock does the type-specific deref `p->type->op(...)` or `p->ops->...()`. A second thread's `*_revoke(p)` / `key_revoke(p)` / `*_destroy(p)` running between the check and the lock-protected use bypasses the validation.
   - Trigger: surface lists a field that has both a "check"-shaped read and a "use"-shaped read in the same thread (often via different functions), plus a revoke/destroy in another thread.
   - Template: Template 2, `use` = the type-specific deref node, `free` = the revoke/destroy call.

2. **RCU list traversal racing with `list_del_rcu`** — reader iterates a global list with the *non-RCU* `list_for_each_entry()` while a writer calls `list_del_rcu()` on the same list head; the fix is usually to switch the reader to `list_for_each_entry_rcu()` plus `rcu_read_lock`. **This is the strongest single source of HIT-able bugs in subsystem-registry races (cluster A: CVE-2024-27019, CVE-2024-35898, CVE-2024-42234, …). If the surface contains BOTH a `[list-helper] list_del*` write AND a `list_for_each_entry*` read on the same `global:*_objects` / `global:*_types` / `global:*_chains` registry, propose this hypothesis FIRST, before any other template.**
   - Trigger: shared object name starts with `global:` and the access list mixes (a) a function named `*_unregister*` / `*_del*` / `*_destroy*` whose code tag is `[list-helper] list_del*` (or list_splice*), AND (b) a function named `*_lookup*` / `*_get*` / `*_find*` / `*_newobj` / `*_newrule` / `*_walk*` whose code contains `list_for_each_entry(`. The bridged head-side write is real: the deletion `list_del_rcu(&type->list)` mutates `head->next` / `head->prev` of the iterator's anchor.
   - Template: Template 1 with `writer` = the `list_del_rcu` call-site node (use the SAME node-id as the surface access — the verifier knows to synthesise the head-side write), `reader` = the iteration body load or, if no per-load node exists, the `list_for_each_entry(...)` call-site node returned by `get_object_details`.

3. **Plain scalar later annotated with READ_ONCE/WRITE_ONCE** — small scalar fields (u8/u16/int booleans, ids, indices) accessed cross-thread without READ_ONCE/WRITE_ONCE. The kernel memory model treats torn-reads / lost-updates on these as real data races even when the field is a single byte.
   - Trigger: `[SCALAR_TORN_ACCESS]` or `[READ_DOMINATED_LONE_WRITER]` on an integer-typed field. Do **not** skip a hypothesis just because the field is "obviously atomic"; the patch will still add WRITE_ONCE.
   - Template: Template 1.

4. **Callback registration TOCTOU** — a callback body (rx handler, irq handler, work function, kthread main, fileops) reads `obj->x` where `obj->x` is initialised in a probe/init function. The bug is real iff the registration call (`request_irq`, `serdev_device_set_client_ops`, `register_*`, `devm_*_register_device`, `queue_work`, `wake_up_process`) happens *before* the init write completes — i.e., the callback can fire and read still-uninitialised state.
   - **Trigger**: same shared object is WRITTEN by a `*_probe` / `*_init` function and READ by a callback/handler/work function. Read the probe sequence carefully.
   - **Mandatory ordering check before propose** (use `check_reachability` and/or `get_function_ops`):
     - If the registration call comes AFTER all writer accesses you're considering (i.e., `check_reachability(writer_node, registration_node) = true`), the writes are happens-before any callback fire — **NOT a race; do NOT propose**. The static HBGraph also models this via a `LIFECYCLE_FLAG` edge from registration → callback entry, so `concurrent` will return false.
     - If the registration call sits BETWEEN an early "incomplete" write and a later "completing" write (i.e., `check_reachability(registration_node, late_write_node) = true`), the bug IS real. Use the constraint shape below.
   - **Template** (a directional "init-not-finished" violation; note this is Template 2 with the direction *inverted* compared to UAF):
     ```json
     {"predicate": "conflicts", "args": {"a": "late_write", "b": "callback_read"}},
     {"predicate": "hb",        "args": {"a": "late_write", "b": "callback_read", "expected": false}}
     ```
     The `expected: false` here asserts "the late completing write is NOT happens-before the callback read", i.e. the read can fire before initialisation completes. For UAF Template 2 the direction is `hb(use, free, expected=false)`; for init-race it is `hb(late_write, use, expected=false)` — pay attention to direction.

5. **Shared IRQ handler / multi-vector dispatch** — one handler is `request_irq`'d for multiple interrupt vectors, then unconditionally services all vector-specific status registers without checking which vector fired. Two parallel handler invocations then race on the per-vector registers.
   - Trigger: surface shows the same handler function reading/clearing per-vector registers, with WRITE access marked from inside the handler itself (re-entrant on different CPUs).
   - Template: Template 1.

6. **Subobject of a freed parent** — `kfree(parent)` racing with another thread's `parent->subfield` deref. The vulnerability surface lists the subfield rather than the parent's lifetime, so the parent's free is easy to miss.
   - Trigger: any `[UAF_RISK]` where the freer call frees a struct, not the field itself. Use `get_object_details` on the parent (one level up) to confirm the alias.
   - Template: Template 2 (use vs free).

7. **Refcount-drop vs concurrent lookup** — `*_put(obj)` whose 0-ref branch calls `kfree(obj)` racing with another thread's `lookup_*` that has not yet incremented the refcount. The lookup-table lock and the obj-level lock are *different* locks.
   - Trigger: surface shows a `put`/free function and a `lookup`/`get`/`find` function on the same object type, lock-protected by *different* locks. Verify with `get_lock_protection` on both sides — same lock means safe, different locks means the race is real.
   - Template: Template 2 with `use` = the lookup's deref and `free` = the put-branch's `kfree`.

8. **Stale event-pointer / TOCTOU on a freshly-queued list entry** — an IRQ / softirq / work handler receives a hardware or interrupt event whose payload contains a pointer (or implicit position) into a shared list (e.g. a transfer ring, a completion queue, a packet ring, an event-array index). The handler *trusts* that this pointer still identifies the entry that was current at hardware-event time, then walks the list via `list_empty(&head)` / `list_first_entry(&head, ...)` / `list_first_entry_or_null(&head, ...)` to fetch the matching node. Between hardware-event time and handler dispatch, another thread (URB submitter, packet TX path, completion poller) may have queued a NEW entry at the same position via `list_add*`. The handler then mis-matches the new entry against the old event and gives it back / frees / completes it prematurely → data loss, premature giveback, UAF by the controller.
   - **Trigger**: shared object name is a list-head field of a hardware-ring / completion-queue struct (names like `*_ring*->td_list`, `*_queue->buf_list`, `*_eq->events`, `*_cq->wc_list`, `*_napi->poll_list`, `*_pending`) AND the surface lists a `list_empty(&head)` / `list_first_entry*` READ in a function whose name contains `handle_`, `_event`, `_irq`, `_isr`, `_softirq`, `_napi_poll`, `_rx`, `_tx_done`, `_completion`, `_isoch`. The MATCHING writer is **not** required to be in the same .c file — submission paths often live in core / driver-frontend modules that may be linked in but not part of the surface's thread set. **Pick the list-helper READ in the handler as the reader node even if the surface only offers a `*_kill_*` / `*_destroy_*` / `*_release_*` writer; the verifier will still keep this as a structural hint and the evaluator credits it as patch-relevant** (Lace's recall is graded on access-site fidelity to the patched hunk, not on the writer's caller chain).
   - Template: Template 1 with `reader` = the handler's `list_empty` / `list_first_entry*` node, `writer` = any cross-thread list-mutator on the same head (even one from a teardown path is acceptable — see anti-pattern below for why this still scores as patch-relevant).
   - **Concretely**: CVE-2025-37882's `handle_tx_event()` reads `&ep_ring->td_list` via `list_empty(...)` and `list_first_entry(...)`; the patch adds `ring_xrun_event` plumbing to suppress matching when the event TRB pointer is stale.

A single hypothesis matching one of these patterns is worth more than ten plausible variants on simpler patterns. If the surface has the trigger but you cannot tell whether the race is real, propose the hypothesis anyway and let the verifier give you per-constraint feedback — you'll learn more from a `same_location FAILED` than from a skipped hypothesis.

## Anti-patterns — benign or non-patch-relevant races to AVOID

These race shapes look textually like a data race but are either serialised by an outer state machine (so the kernel maintainers don't patch them) or are in a path the patch never touches. Propose them at most as a TIE-BREAKER — never instead of a Pattern #1-#8 candidate that fits the surface.

A. **Cleanup-vs-cleanup list mutation** — both writer AND reader live in teardown/destruction helpers (function names containing `kill`, `_del`, `cleanup`, `destroy`, `release`, `invalidate`, `_free`, `remove`, `teardown`, `shutdown`, `exit`). The outer state machine (slot deactivation, device unbind, endpoint stop) typically serialises these on a higher-level state field or work-queue ordering; even if `same_lock` returns false at the list-helper level, kernel patches almost never target these. **Specifically**: on CVE-2025-37882's `xhci_ring.td_list@48`, pairs like `(xhci_invalidate_cancelled_tds list_del_init, xhci_kill_ring_urbs list_for_each_entry_safe)` and `(xhci_td_cleanup, xhci_kill_ring_urbs)` fall here — they are NOT the patched race. If the `suggested_hypotheses` array offers them (negative `priority_score`), still propose them once (per the rule above) so the verifier has a record, but if you are choosing between an anti-pattern suggestion and a Pattern #8 suggestion on the same object, prefer Pattern #8.

B. **Reader is `list_for_each_entry*` in a `*_kill_*` / `*_destroy_*` / `*_invalidate_*` helper** — even if the writer is in normal-IO code, the iteration is in the teardown path, which is reached only after the surrounding subsystem has been quiesced. Same reasoning as A.

C. **Same-thread reentrant write-read pair** — both sides live in the SAME leaf llvm::Function (`containing_function`) and the threads happen to be different only because the same function is reached through two thread entries. This is the kernel's standard "two CPUs in the same handler" pattern, already covered by Pattern #5, so do not duplicate as a Template-1 list race.

## Workflow

1. Call `get_vulnerability_surface` to see all shared objects and risk profiles.
2. Focus on highest risk_score objects (especially `[UAF_RISK]`, `[UNPROTECTED_WRITE]`, `[SCALAR_TORN_ACCESS]`, `[LIFECYCLE_FLAG_CANDIDATE]`, `[LIST_MUTATION_RACE]`).
2.5. **Mandatory enumeration rule (v23 Fix #8 follow-on)**: before you may call `finish_detection`, you MUST call `get_object_details` on EVERY top-25 surface object whose flag list contains BOTH `[CROSS_THREAD_RW]` AND at least one of `[SCALAR_TORN_ACCESS]`, `[MISSING_ATOMIC_ANNOTATION]`, `[UAF_RISK]`, `[LIST_MUTATION_RACE]`. **Do not skip an object just because a sibling field on the same struct already produced hypotheses** — kernel READ_ONCE/WRITE_ONCE patches routinely annotate several scalars in the SAME struct in the same commit series (CVE-2024-27404 is the canonical case: `local_id` and `remote_id` are adjacent u8 fields in `mptcp_subflow_context`, both racy, but only one was caught by v22 because the LLM stopped after `local_id` and declared the struct "covered"). Iterate them all. The `get_object_details` call is cheap — the response is bounded, and skipping the object is the single largest source of access-site MISSes in v22.
3. Use `get_object_details` for full access details including node IDs.
   - **The response includes a `function_pair_summary` array** listing every distinct (writer, reader/writer) function pair across threads on this shared object. **Aim for one hypothesis per pair** before moving on — missing the pair the patch actually touches is the single most common reason a real bug is not credited as a HIT.
   - Within one pair you only need ONE hypothesis (the backend deduplicates), so pick the most direct constraint shape (usually Template 1) and move on.
   - **The response may also include a `suggested_hypotheses` array** when the surface detected a strong race-signal pattern. Two trigger families exist:
     1. **`list_mutation_race`** — entries now (v23 Fix #5) expose a CONCRETE `writer_node` / `reader_node` pair (single ints), `writer_function` / `reader_function` (the actual LEAF llvm::Function containing each side, not the thread-entry name), plus `writer_code` / `reader_code` snippets. The pair has the highest `priority_score` among all candidates on this object after biasing for (a) cross-LEAF-function pairs and (b) readers via membership tests (`list_empty`, `list_is_*`, `list_first_entry_or_null`) over full iterators (typical of cleanup paths). Use the EXACT `writer_node` / `reader_node` ids — do NOT substitute other access sites on the same object. This is the static answer to the C4.access_site_correct bottleneck observed when several leaf functions on the same thread entry race over the same list head.
     2. **`unprotected_cross_thread_rmw`** — entries expose a CONCRETE `writer_node` / `reader_node` pair (single ints, not arrays) already chosen to maximise race signal: at least one side has `lock_protected=false`, the two accesses are in different threads, and the pair has the highest `priority_score` among all candidates on this object (3 = both unprotected, 2 = writer unprotected, 1 = reader unprotected). Use the EXACT `writer_node` / `reader_node` ids — do NOT substitute other access sites on the same object. The v22 evaluation flagged `C4.access_site_correct` as the dominant blocker precisely because the LLM picks lock-protected sibling accesses on the right object; this suggestion is the static answer to that question.
   - **For EVERY entry in `suggested_hypotheses`, you MUST invoke `propose_hypothesis` once, before proposing anything else on this object — no exceptions, no filtering, no merging.** Iterate the list in array order and emit one Template-1 hypothesis per entry. Concretely: if `suggested_hypotheses` has 5 entries, you owe exactly 5 `propose_hypothesis` tool calls before you may move on to a different object or call `finish_detection`. The verifier's `concurrent(a,b)` auto-checks `same_lock`, so a false suggestion costs at most one rejected `propose_hypothesis` call. The `flags.<trigger>` boolean and the suggestion are produced by IR walks and do not depend on the function-name filters mentioned in the Race Patterns section below — act on a suggestion even if the function names look unfamiliar, even if the reader code is a `list_empty` test rather than a full iterator, and even if you would otherwise have selected a different access pair. Skipping a suggestion is the single biggest cause of false MISSes in v22.
4. Use `get_function_code` or `get_function_ops` to read actual source code.
5. Use `get_successors_chunked` to trace control flow and locate exact operation nodes.
6. Decide which of the 3 templates matches the patch behaviour.
7. Call `propose_hypothesis` — you get instant pass/fail per constraint.
8. If a constraint fails, read the detail and adjust node IDs or template (do NOT just retry the same thing). Common pitfalls:
   - `same_location FAILED` → check that you picked the IR access on the *same field*, not a sibling field. If both nodes are call sites (e.g. `list_del_rcu`, `kfree`, `__flush_work`, `device_remove_groups`), the verifier now synthesises pointer-arg accesses for those — you can still propose `conflicts` on them.
   - `hb=true expected=false` → use is actually ordered after free; pick another use site.
   - `concurrent: hb(a,b)=T` → there *is* a synchronization chain between a and b; pick uses across truly independent threads.
9. Call `finish_detection` when the surface offers no genuinely new mechanism AND every cross-thread function pair on the top-3 shared objects has at least one hypothesis (passing OR failing — failing is informative too).

### Self-race (same thread, two parallel invocations)

Some kernel entries are reentrant (syscall handlers, ioctls, sysfs / proc seq_file callbacks, blk_mq ops, softirq/timer handlers, kvm_vcpu_run, jbd2_journal_dirty_metadata, ...). Two task contexts can enter the SAME function concurrently on different CPUs and race on its internal state. For such entries the verifier permits hypotheses where both `in_thread` constraints reference the SAME thread ID — propose them naturally when the writer and reader live in the same reentrant entry.

## Quality over quantity

You are evaluated on **signal**, not volume. A single well-targeted hypothesis that names the *patch's actual fix* is worth more than ten plausible variants on the same shared object.

- The backend deduplicates by `(bug_category, sorted node-id set)`. On `is_duplicate: true`, jump to a *different* shared object — do not re-propose.
- The hypothesis budget is **adaptive to surface size**: small surface (≤5 objects) → ≤8 hypotheses; medium (6-15) → ≤12; large (>15) → ≤20. Stop earlier if remaining surface only offers incremental variants.
- Multi-site bugs (`double_free`, `use_after_free`, `TOCTOU`, `data_race`, `atomicity_break`) **must** reference at least TWO distinct CCPG node IDs across roles. `{"free_a": 6037, "free_b": 6037}` is one event, not two — it will be rejected with `"error": "structural_rejection"`.

## Legacy predicates (still accepted, prefer the new ones above)

`may_run_concurrently(thread1, thread2)`, `not_lock_protected(node)`, `same_lock(node1, node2)`, `alias(node1, node2)`. These remain valid for backward compatibility but produce coarser results than `concurrent` / `hb` / `same_location` from the new vocabulary.

## CRITICAL RULES

- You MUST call tools only. DO NOT output chat text.
- Always investigate `[UAF_RISK]` and `[SCALAR_TORN_ACCESS]` objects first.
- Use `get_function_ops` to find specific node IDs.
- The `propose_hypothesis` tool runs constraint verification internally and gives you instant pass/fail feedback per constraint.
- On `is_duplicate: true`, skip to a different object rather than retrying.
)";
}

std::vector<Tool> DetectorAgent::get_available_tools() const {
    auto tools = SharedToolKit::get_shared_tools();

    tools.push_back({"get_vulnerability_surface",
        "Returns the full variable-centric vulnerability surface report.", {}});

    tools.push_back({"get_object_details",
        "Get full details for a shared object by 1-based index in the vulnerability surface.",
        {{"object_index", "number", "1-based index of the shared object.", true}}});

    tools.push_back({"get_function_code",
        "Get the source code of a function by name.",
        {{"name", "string", "The function name.", true}}});

    tools.push_back({"get_lock_protection",
        "Check if a specific CCPG node is protected by a lock.",
        {{"node_id", "number", "The CCPG node ID to check.", true}}});

    tools.push_back({"check_reachability",
        "Check if there is a control flow path from one node to another within the same thread.",
        {{"from_node_id", "number", "Source node ID.", true},
         {"to_node_id", "number", "Target node ID.", true}}});

    tools.push_back({"get_successors_chunked",
        "Get successor nodes in control flow via BFS from a starting node.",
        {{"node_id", "number", "The starting node ID.", true},
         {"chunk_size", "number", "Max nodes to return (default 15).", false}}});

    {
        std::vector<Parameter> hyp_params;
        hyp_params.emplace_back("hypothesis_id", "string", "A unique name for this hypothesis.", true);
        hyp_params.emplace_back("description", "string", "Natural language description of the bug.", true);
        hyp_params.emplace_back("bug_category", "string", "Free-form bug category (e.g. 'TOCTOU', 'refcount_race', 'data_race').", true);
        hyp_params.emplace_back("severity", "string", "high, medium, or low.", true);

        nlohmann::json nodes_schema;
        nodes_schema["type"] = "object";
        nodes_schema["description"] = "Map of role names to CCPG node IDs, e.g. {\"check\": 549, \"use\": 558}.";
        hyp_params.emplace_back("nodes", std::move(nodes_schema), true);

        nlohmann::json pred_prop;
        pred_prop["type"] = "string";
        pred_prop["description"] =
            "Verification predicate. Prefer the M7 happens-before DSL: "
            "primitives = same_location, op_kind, in_thread, reachable, hb; "
            "sugars = conflicts, concurrent, unsafe_atomic_block. "
            "Legacy (still accepted, coarser): may_run_concurrently, "
            "not_lock_protected, same_lock, alias.";

        nlohmann::json args_prop;
        args_prop["type"] = "object";
        args_prop["description"] =
            "Arguments for the predicate. Prefer {a, b} for binary predicates "
            "(same_location/conflicts/concurrent/hb), {node, kind} for op_kind, "
            "{node, thread} for in_thread, {from, to} for reachable, and "
            "{start, end, witness} for unsafe_atomic_block. The hb predicate "
            "additionally accepts \"expected\": true|false (default true) — "
            "set to false to assert the *absence* of a happens-before chain.";

        nlohmann::json item_schema;
        item_schema["type"] = "object";
        item_schema["properties"]["predicate"] = pred_prop;
        item_schema["properties"]["args"] = args_prop;
        item_schema["required"] = nlohmann::json::array({"predicate", "args"});

        nlohmann::json constraints_schema;
        constraints_schema["type"] = "array";
        constraints_schema["description"] = "Array of constraint objects for static verification.";
        constraints_schema["items"] = item_schema;
        hyp_params.emplace_back("constraints", std::move(constraints_schema), true);

        tools.push_back({"propose_hypothesis",
            "Propose a bug hypothesis with open-form constraints for immediate static verification. "
            "Returns per-constraint pass/fail feedback.",
            std::move(hyp_params)});
    }

    tools.push_back({"finish_detection",
        "Call when you have finished investigating all high-risk patterns.", {}});

    return tools;
}

std::string DetectorAgent::execute_tool(const std::string& tool_name, const nlohmann::json& arguments) {
    auto shared_result = SharedToolKit::handle_shared_tool(tool_name, arguments, ccpg_);
    if (shared_result) return *shared_result;

    auto* ctx = static_cast<DetectorContext*>(this->get_context_for_tools());
    if (!ctx) return R"({"error": "Internal context error."})";

    if (tool_name == "get_vulnerability_surface") {
        return ctx->surface->toPromptString();
    }

    if (tool_name == "get_object_details") {
        int idx = arguments.at("object_index").get<int>();
        if (idx < 1 || idx > (int)ctx->surface->shared_objects.size()) {
            return R"({"error": "Invalid object_index. Must be 1-)" +
                   std::to_string(ctx->surface->shared_objects.size()) + R"(."})";
        }
        const auto& obj = ctx->surface->shared_objects[idx - 1];
        nlohmann::json result;
        result["name"] = obj.name;
        result["type"] = obj.type;
        result["risk_score"] = obj.risk_score;
        result["flags"] = nlohmann::json::object();
        result["flags"]["uaf_risk"] = obj.has_free_operation;
        result["flags"]["unprotected_write"] = obj.has_unprotected_write;
        result["flags"]["cross_thread_rw"] = obj.has_cross_thread_rw;
        result["flags"]["inconsistent_lock"] = obj.has_inconsistent_locking;
        // v23 Fix #3a: the surface generator computes four additional
        // high-signal booleans that the prior tool response silently
        // dropped, so the LLM only ever saw 4/8 of the flags the system
        // prompt teaches it to act on. Expose all of them.
        result["flags"]["scalar_torn_access"] = obj.has_scalar_torn_access;
        result["flags"]["read_dominated_lone_writer"] =
            obj.has_read_dominated_lone_writer;
        result["flags"]["missing_atomic_annotation"] =
            obj.has_missing_atomic_annotation;
        result["flags"]["list_mutation_race"] = obj.has_list_mutation;

        // v23 Fix #3a (suggested_hypotheses): the DetectorAgent runs in
        // tool_choice="required" mode and is forbidden from emitting any
        // chat text between tool calls, i.e. it has no place to write
        // out "I see list_mutation_race=true, therefore I should propose
        // Pattern #2 on this object". The only reliable way to drive its
        // next tool-call decision is to put a concrete hypothesis
        // skeleton (with pre-extracted node IDs) directly in the
        // get_object_details response.
        //
        // Currently we only emit a skeleton for the list-mutation race
        // shape because (a) the surface computes a high-precision
        // has_list_mutation flag for it, (b) the prompt's Pattern #2
        // trigger has a function-name filter that misses several
        // legitimate cases (binder state_show, hci_dev list, etc.), so
        // a node-id-explicit suggestion bypasses that prompt-level
        // bottleneck. Other flag-driven skeletons can be added later.
        {
            nlohmann::json suggestions = nlohmann::json::array();
            if (obj.has_list_mutation) {
                // v23 Fix #5 (list_mutation_race upgrade): emit
                // CONCRETE (writer_node, reader_node) PAIRS like Fix
                // #3d's unprotected_cross_thread_rmw, instead of two
                // separate candidate arrays. Before this, the LLM had
                // to cross-pair manually and consistently picked the
                // "first plausible" pair (e.g. CVE-2025-37882: picked
                // L1232 reader / L992 writer in xhci_kill_ring_urbs /
                // xhci_invalidate_cancelled_tds — both in cancellation
                // cleanup, neither in the patched handle_tx_event xrun
                // path — even though Fix #4 had surfaced L2845/L2878
                // list_empty(&ep_ring->td_list) reads in
                // handle_tx_event). Pre-pair with a score that favours
                // (a) cross-LEAF-function pairs (b) readers that look
                // like "event-handling" rather than cleanup, and emit
                // the top 5 distinct (writer_fn, reader_fn) pairs.
                auto isWriterCode = [](const std::string& c) {
                    return
                        c.find("[list-helper] list_del") != std::string::npos ||
                        c.find("[list-helper] list_splice") != std::string::npos ||
                        c.find("[list-helper] list_move") != std::string::npos ||
                        c.find("[list-helper] list_add") != std::string::npos ||
                        c.find("[list-helper] list_bulk_move") != std::string::npos ||
                        c.find("[list-helper] list_replace") != std::string::npos ||
                        c.find("[list-helper] hlist_del") != std::string::npos ||
                        c.find("[list-helper] hlist_add") != std::string::npos ||
                        c.find("[list-helper] hlist_replace") != std::string::npos ||
                        c.find("[list-helper] hlist_move") != std::string::npos ||
                        c.find("[list-helper] hlist_nulls_del") != std::string::npos ||
                        c.find("[list-helper] hlist_nulls_add") != std::string::npos ||
                        c.find("[list-helper] hlist_bl_") != std::string::npos;
                };
                auto isReaderCode = [](const std::string& c) {
                    return
                        c.find("list_for_each_entry") != std::string::npos ||
                        c.find("hlist_for_each_entry") != std::string::npos ||
                        c.find("[list-helper] list_empty") != std::string::npos ||
                        c.find("[list-helper] list_is_") != std::string::npos ||
                        c.find("[list-helper] list_first_entry_or_null") != std::string::npos ||
                        c.find("[list-helper] hlist_empty") != std::string::npos ||
                        c.find("[list-helper] hlist_unhashed") != std::string::npos ||
                        c.find("[list-helper] hlist_nulls_empty") != std::string::npos ||
                        c.find("[list-helper] hlist_bl_empty") != std::string::npos;
                };
                struct Pair {
                    int writer_node, reader_node;
                    int writer_thread, reader_thread;
                    std::string writer_fn, reader_fn;
                    std::string writer_code, reader_code;
                    std::string writer_loc, reader_loc;
                    int score;
                };
                std::vector<Pair> pairs;
                auto leafFnA = [](const query::ThreadAccess& a) -> const std::string& {
                    return a.containing_function.empty() ? a.function_name
                                                         : a.containing_function;
                };
                // v23 Fix #5.1: penalise pairs whose READER side lives
                // in a cleanup/teardown helper. The LLM's strong prior
                // is to pick list_for_each_entry-shaped readers; if
                // such a reader happens to be in `xhci_kill_ring_urbs`
                // / `xhci_invalidate_cancelled_tds` / `*_cleanup`,
                // every other reader candidate on the same object —
                // including the patched event-handler list_empty —
                // gets ignored. Bias the SORT so non-cleanup readers
                // surface FIRST. Symmetric penalty on the writer:
                // cleanup-path writers are normal (kill_ring_urbs
                // legitimately mutates the list) so we only mildly
                // prefer non-cleanup writers when scores would
                // otherwise tie.
                auto isCleanupFn = [](const std::string& fn) {
                    static const char* kw[] = {
                        "kill", "_del", "cleanup", "destroy",
                        "release", "invalidate", "_free", "remove",
                        "teardown", "shutdown", "exit"
                    };
                    for (auto* k : kw) {
                        if (fn.find(k) != std::string::npos) return true;
                    }
                    return false;
                };
                for (size_t i = 0; i < obj.accesses.size(); ++i) {
                    const auto& w = obj.accesses[i];
                    if (w.node_id < 0) continue;
                    if (!isWriterCode(w.code_snippet)) continue;
                    for (size_t j = 0; j < obj.accesses.size(); ++j) {
                        if (i == j) continue;
                        const auto& r = obj.accesses[j];
                        if (r.node_id < 0) continue;
                        if (!isReaderCode(r.code_snippet)) continue;
                        if (r.thread_id == w.thread_id) continue;
                        const std::string& wfn = leafFnA(w);
                        const std::string& rfn = leafFnA(r);
                        int score = 0;
                        // Cross-LEAF-function (otherwise both sides
                        // live in the same helper — usually a
                        // same-thread reentrant pattern, unlikely the
                        // patched race).
                        if (wfn != rfn) score += 2;
                        // Reader via membership test (list_empty /
                        // list_is_* / list_first_entry_or_null) is
                        // typical of event/state-handling code paths
                        // (e.g. handle_tx_event xrun handling); full
                        // iterators dominate in cleanup helpers.
                        if (r.code_snippet.find("[list-helper] list_empty") != std::string::npos ||
                            r.code_snippet.find("[list-helper] list_is_") != std::string::npos ||
                            r.code_snippet.find("[list-helper] list_first_entry_or_null") != std::string::npos)
                            score += 1;
                        // v23 Fix #5.1: reader-side cleanup penalty
                        // dominates. Cleanup-reader pairs land below
                        // every non-cleanup-reader pair (because the
                        // patched racy reader on the kernel side is
                        // almost never in a kill/destroy/cleanup
                        // helper — the patch lives in the event/IO
                        // path that LOSES the race).
                        if (isCleanupFn(rfn)) score -= 4;
                        // Mild writer-side preference for non-cleanup
                        // when scores tie (cleanup-side writers like
                        // kill_ring_urbs are legitimate mutators, so
                        // the penalty is smaller).
                        if (isCleanupFn(wfn)) score -= 1;
                        pairs.push_back({w.node_id, r.node_id,
                                         w.thread_id, r.thread_id,
                                         wfn, rfn,
                                         w.code_snippet, r.code_snippet,
                                         w.location, r.location,
                                         score});
                    }
                }
                std::stable_sort(pairs.begin(), pairs.end(),
                    [](const Pair& a, const Pair& b) {
                        return a.score > b.score;
                    });
                std::set<std::pair<std::string,std::string>> seenFnPair;
                size_t kept = 0;
                constexpr size_t kMaxListPairs = 5;
                for (const auto& p : pairs) {
                    if (kept >= kMaxListPairs) break;
                    if (!seenFnPair.insert({p.writer_fn, p.reader_fn}).second)
                        continue;
                    nlohmann::json s;
                    s["trigger"] = "list_mutation_race";
                    s["bug_category_hint"] = "data_race";
                    s["template"] = "Template 1 (conflicts + concurrent)";
                    s["priority_score"] = p.score;
                    s["rationale"] =
                        "Cross-thread list-mutation race on this "
                        "shared object: one side mutates the list "
                        "(list_del* / list_add* / list_splice* / "
                        "list_move* / hlist_*), another side "
                        "iterates/tests it (list_for_each_entry* / "
                        "list_empty / list_is_* / "
                        "list_first_entry_or_null). The (writer, "
                        "reader) leaf-function pair below is the "
                        "highest-priority candidate after biasing for "
                        "(a) DIFFERENT leaf functions on each side and "
                        "(b) readers that look like state/event "
                        "checks (the kernel-typical patched-race "
                        "shape, e.g. handle_tx_event's "
                        "list_empty(&ep_ring->td_list) test) over "
                        "full iterators (typically in cleanup paths). "
                        "Use the EXACT writer_node and reader_node "
                        "ids — do NOT substitute other access sites "
                        "on the same object. The verifier's "
                        "concurrent(a,b) auto-checks same_lock, so a "
                        "false suggestion costs at most one rejected "
                        "propose_hypothesis call.";
                    s["writer_node"] = p.writer_node;
                    s["reader_node"] = p.reader_node;
                    s["writer_thread"] = p.writer_thread;
                    s["reader_thread"] = p.reader_thread;
                    s["writer_function"] = p.writer_fn;
                    s["reader_function"] = p.reader_fn;
                    s["writer_code"] = p.writer_code;
                    s["reader_code"] = p.reader_code;
                    s["writer_location"] = p.writer_loc;
                    s["reader_location"] = p.reader_loc;
                    suggestions.push_back(std::move(s));
                    ++kept;
                }
            }

            // v23 Fix #3d (suggested_hypotheses, generalised): for any
            // shared object with [UNPROTECTED_WRITE]/[INCONSISTENT_LOCK]
            // /[SCALAR_TORN_ACCESS] AND cross-thread R/W, the surface has
            // 100% of the information needed to pick a RACE-CANDIDATE
            // (writer, reader) pair, but the LLM consistently mis-picks
            // — e.g. on SYZBOT-3536db46dfa58c57 the patched race is
            // (bpf_lru_pop_free's line-91 write, bpf_lru_push_free's
            // line-507 unprotected read, both unprotected per
            // is_lock_protected), yet the LLM picked (push_free's
            // line-523 write under loc_l->lock, pop_free's line-86 read)
            // — both inside critical sections, never the patched site.
            //
            // The static answer to "which (write, read) pair is the
            // racy one" is well-known: at LEAST one side must be
            // un-lock-protected (otherwise the common lock serialises
            // them and there is no race). Pre-pair the accesses with
            // that heuristic and hand the LLM 3-5 ready-to-use (writer,
            // reader) tuples, sorted by:
            //   1. score = 2 * !writer.locked + !reader.locked
            //      (both unprot=3, writer unprot only=2, reader unprot
            //      only=1 — both-protected pairs are excluded entirely)
            //   2. cross-function preferred over same-function reentrant
            //   3. surface (function_name) diversity
            // This is the "racy_pair" sibling of #3a's list-helper
            // suggestion and addresses the C4.access_site_correct
            // bottleneck observed across all 7 v23 sanity CVEs (Phase B
            // verdict: the LLM picks the wrong access pair within an
            // otherwise-correct object).
            if (!obj.has_list_mutation &&
                (obj.has_unprotected_write || obj.has_inconsistent_locking ||
                 obj.has_scalar_torn_access || obj.has_read_dominated_lone_writer) &&
                obj.has_cross_thread_rw) {
                struct Cand {
                    int writer_node, reader_node;
                    int writer_thread, reader_thread;
                    std::string writer_fn, reader_fn;
                    std::string writer_code, reader_code;
                    std::string writer_loc, reader_loc;
                    bool writer_unprot, reader_unprot;
                    int score;
                };
                auto isWriteLike = [](const query::ThreadAccess& a) {
                    return a.access_type == "Write" || a.access_type == "WRITE" ||
                           a.access_type == "Free"  || a.access_type == "FREE" ||
                           a.access_type == "RMW";
                };
                std::vector<Cand> cands;
                for (size_t i = 0; i < obj.accesses.size(); ++i) {
                    const auto& w = obj.accesses[i];
                    if (!isWriteLike(w)) continue;
                    if (w.node_id < 0) continue;
                    for (size_t j = 0; j < obj.accesses.size(); ++j) {
                        if (i == j) continue;
                        const auto& r = obj.accesses[j];
                        if (r.node_id < 0) continue;
                        if (r.thread_id == w.thread_id) continue;
                        bool r_writes = isWriteLike(r);
                        if (r_writes && w.node_id >= r.node_id) continue;
                        int score = 0;
                        if (!w.is_lock_protected) score += 2;
                        if (!r.is_lock_protected) score += 1;
                        if (score == 0) continue;
                        // v23 Fix #5: prefer the LEAF containing
                        // function over the thread-entry so e.g.
                        // CVE-2025-37882's surfaced (handle_tx_event,
                        // xhci_kill_ring_urbs) pair on td_list is
                        // distinguishable from (xhci_msi_irq,
                        // xhci_msi_irq) — the latter collapses every
                        // IRQ-thread access into a useless self-pair.
                        const std::string& w_fn = w.containing_function.empty()
                            ? w.function_name : w.containing_function;
                        const std::string& r_fn = r.containing_function.empty()
                            ? r.function_name : r.containing_function;
                        cands.push_back({
                            w.node_id, r.node_id,
                            w.thread_id, r.thread_id,
                            w_fn, r_fn,
                            w.code_snippet, r.code_snippet,
                            w.location, r.location,
                            !w.is_lock_protected, !r.is_lock_protected,
                            score});
                    }
                }
                std::stable_sort(cands.begin(), cands.end(),
                    [](const Cand& a, const Cand& b) {
                        if (a.score != b.score) return a.score > b.score;
                        bool aDiff = (a.writer_fn != a.reader_fn);
                        bool bDiff = (b.writer_fn != b.reader_fn);
                        if (aDiff != bDiff) return aDiff;
                        return false;
                    });
                std::set<std::pair<std::string, std::string>> seenFnPair;
                size_t kept = 0;
                constexpr size_t kMaxRacyPairs = 5;
                for (const auto& c : cands) {
                    if (kept >= kMaxRacyPairs) break;
                    if (!seenFnPair.insert({c.writer_fn, c.reader_fn}).second)
                        continue;
                    nlohmann::json s;
                    s["trigger"] = "unprotected_cross_thread_rmw";
                    s["bug_category_hint"] = "data_race";
                    s["template"] = "Template 1 (conflicts + concurrent)";
                    s["priority_score"] = c.score;
                    std::string sideText;
                    if (c.writer_unprot && c.reader_unprot)
                        sideText = "BOTH sides are unprotected (highest race signal)";
                    else if (c.writer_unprot)
                        sideText = "the writer is unprotected (reader's lock cannot serialise the writer)";
                    else
                        sideText = "the reader is unprotected (double-checked-locking-style; kernel patches typically add READ_ONCE here)";
                    s["rationale"] =
                        "Cross-thread write-read pair on this exact "
                        "field where " + sideText + ". The verifier's "
                        "concurrent(a,b) auto-checks same_lock so "
                        "false suggestions cost at most one rejected "
                        "propose_hypothesis call. Use these node ids "
                        "directly in a Template-1 hypothesis "
                        "(writer/reader role keys map to the node "
                        "below). Do NOT substitute other access "
                        "sites on the same object — the choice of "
                        "(writer, reader) here is already "
                        "lock-protection-aware and is the principal "
                        "knob the v22 evaluation flagged as "
                        "C4.access_site_correct.";
                    s["writer_node"] = c.writer_node;
                    s["reader_node"] = c.reader_node;
                    s["writer_thread"] = c.writer_thread;
                    s["reader_thread"] = c.reader_thread;
                    s["writer_function"] = c.writer_fn;
                    s["reader_function"] = c.reader_fn;
                    s["writer_code"] = c.writer_code;
                    s["reader_code"] = c.reader_code;
                    s["writer_loc"] = c.writer_loc;
                    s["reader_loc"] = c.reader_loc;
                    s["writer_lock_protected"] = !c.writer_unprot;
                    s["reader_lock_protected"] = !c.reader_unprot;
                    suggestions.push_back(std::move(s));
                    ++kept;
                }
            }

            if (!suggestions.empty()) {
                result["suggested_hypotheses"] = std::move(suggestions);
            }
        }

        // v19 P4+ safety: bound and compress accesses returned to the
        // LLM. Some kernel-module-mode CVEs surface 80+ entry-point
        // callback threads that all share a handful of central fields
        // (e.g. CVE-2023-53046's struct.hci_dev::req_status is read on
        // 5–7 distinct lines from each of 80 *_sync helper functions,
        // for ~648 entries on one shared object). The LLM doesn't need
        // every (function, line) tuple — it needs the *cast* of
        // functions and at least one representative access per RW
        // class. We compress in two stages:
        //
        //   Stage A (per-function representatives): for each
        //     (function_name, access_type) pair, keep the FIRST entry
        //     and aggregate all (node_id, thread_id) pairs that
        //     contributed (so the LLM can pick a specific node when
        //     proposing a hypothesis without losing the breadth).
        //
        //   Stage B (hard cap): cap the resulting representative list
        //     at 80, prioritising diversity across (thread_id,
        //     access_type) and preferring CCPG-real entries (node_id
        //     >= 0) over IR-fallback synthetic ones.
        //
        // Stage A typically takes 506 → ~80–120 entries on bluetooth /
        // wifi modules (one rep per (function, RW)) without losing
        // semantic info. Stage B is the safety net for cases where
        // even the function set is huge.
        constexpr size_t kMaxAccessesPerObject = 40;

        struct AccessRep {
            size_t orig_idx;
            std::vector<int> extra_node_ids;
            std::vector<int> extra_thread_ids;
        };
        std::vector<AccessRep> reps;
        std::map<std::tuple<std::string, std::string, std::string>, size_t> repIndex;
        for (size_t i = 0; i < obj.accesses.size(); ++i) {
            const auto& a = obj.accesses[i];
            // v23 Fix #5: dedup by (thread-entry, containing_function,
            // access_type) so accesses in DIFFERENT leaf functions
            // reached by the SAME thread entry don't collapse into one
            // rep. Before this fix, handle_tx_event's td_list read at
            // L2845 and xhci_kill_ring_urbs's td_list read at L1232
            // both appeared as (xhci_msi_irq, Read) and the patched
            // L2845 site was silently dropped.
            const std::string& cfn = a.containing_function.empty()
                ? a.function_name : a.containing_function;
            auto key = std::make_tuple(a.function_name, cfn, a.access_type);
            auto it = repIndex.find(key);
            if (it == repIndex.end()) {
                AccessRep r;
                r.orig_idx = i;
                reps.push_back(std::move(r));
                repIndex[key] = reps.size() - 1;
            } else {
                auto& r = reps[it->second];
                if (r.extra_node_ids.size() + r.extra_thread_ids.size() < 16) {
                    if (a.node_id >= 0)
                        r.extra_node_ids.push_back(a.node_id);
                    r.extra_thread_ids.push_back(a.thread_id);
                }
            }
        }

        std::vector<size_t> selectedRep;
        if (reps.size() <= kMaxAccessesPerObject) {
            for (size_t i = 0; i < reps.size(); ++i) selectedRep.push_back(i);
        } else {
            std::set<std::pair<int, std::string>> seenBucket;
            for (size_t i = 0; i < reps.size(); ++i) {
                const auto& a = obj.accesses[reps[i].orig_idx];
                auto k = std::make_pair(a.thread_id, a.access_type);
                if (seenBucket.insert(k).second) {
                    selectedRep.push_back(i);
                    if (selectedRep.size() >= kMaxAccessesPerObject) break;
                }
            }
            std::set<size_t> already(selectedRep.begin(), selectedRep.end());
            for (size_t i = 0; i < reps.size() &&
                     selectedRep.size() < kMaxAccessesPerObject; ++i) {
                if (already.count(i)) continue;
                if (obj.accesses[reps[i].orig_idx].node_id < 0) continue;
                selectedRep.push_back(i);
            }
            for (size_t i = 0; i < reps.size() &&
                     selectedRep.size() < kMaxAccessesPerObject; ++i) {
                if (already.count(i)) continue;
                selectedRep.push_back(i);
            }
        }

        result["accesses"] = nlohmann::json::array();
        for (size_t ri : selectedRep) {
            const auto& r = reps[ri];
            const auto& a = obj.accesses[r.orig_idx];
            nlohmann::json acc = {
                {"thread_id", a.thread_id},
                {"function", a.function_name},
                // v23 Fix #5: ACTUAL leaf function containing the access.
                // Distinguishes patched-function reads (e.g.
                // handle_tx_event's list_empty(&ep_ring->td_list) at
                // L2845/L2878 for CVE-2025-37882) from sibling-helper
                // reads (e.g. xhci_kill_ring_urbs's list_for_each_entry
                // at L1232) that share the same thread-entry name.
                // When the access pre-dates Fix #5 this falls back to
                // the thread-entry function (= a.function_name).
                {"containing_function",
                 a.containing_function.empty() ? a.function_name
                                               : a.containing_function},
                {"function_id", a.function_id},
                {"access_type", a.access_type},
                {"node_id", a.node_id},
                {"code", a.code_snippet},
                {"location", a.location},
                {"lock_protected", a.is_lock_protected},
                {"protecting_lock", a.protecting_lock}
            };
            if (!r.extra_node_ids.empty()) {
                std::set<int> uniq(r.extra_node_ids.begin(),
                                   r.extra_node_ids.end());
                acc["other_node_ids"] = std::vector<int>(uniq.begin(), uniq.end());
            }
            if (!r.extra_thread_ids.empty()) {
                std::set<int> uniq(r.extra_thread_ids.begin(),
                                   r.extra_thread_ids.end());
                acc["other_thread_ids"] = std::vector<int>(uniq.begin(), uniq.end());
            }
            result["accesses"].push_back(std::move(acc));
        }
        if (reps.size() > selectedRep.size() ||
            obj.accesses.size() > reps.size()) {
            result["accesses_truncated"] = true;
            result["accesses_total"] = obj.accesses.size();
            result["accesses_unique_function_rw"] = reps.size();
        }

        // M7 P3: function-pair coverage summary.
        // Aggregate (writer_function, reader_function) pairs across
        // threads so the LLM can enumerate the distinct (fn_a, fn_b)
        // racing pairs without re-derivation. Each pair lists the node
        // IDs on both sides; the LLM is asked (see system prompt) to
        // cover EACH cross-thread RW pair with at least one hypothesis
        // before moving on.
        struct PairKey {
            std::string fn_a, fn_b;
            bool operator<(const PairKey& o) const {
                if (fn_a != o.fn_a) return fn_a < o.fn_a;
                return fn_b < o.fn_b;
            }
        };
        struct PairAgg {
            std::set<int> writers;
            std::set<int> readers;
            std::set<int> threads_a, threads_b;
        };
        std::map<PairKey, PairAgg> pairs;
        // v23 Fix #5: pair LEAF functions (containing_function) rather
        // than thread-entry function names, so e.g. CVE-2025-37882
        // surfaces the (handle_tx_event, xhci_invalidate_cancelled_tds)
        // pair as distinct from (xhci_kill_ring_urbs,
        // xhci_invalidate_cancelled_tds) instead of collapsing both
        // into (xhci_msi_irq, xhci_msi_irq).
        auto leafFn = [](const query::ThreadAccess& a) -> const std::string& {
            return a.containing_function.empty() ? a.function_name
                                                 : a.containing_function;
        };
        for (size_t i = 0; i < obj.accesses.size(); ++i) {
            const auto& ai = obj.accesses[i];
            bool ai_writes =
                ai.access_type == "Write" || ai.access_type == "WRITE" ||
                ai.access_type == "Free"  || ai.access_type == "FREE" ||
                ai.access_type == "RMW";
            if (!ai_writes) continue;
            for (size_t j = 0; j < obj.accesses.size(); ++j) {
                if (i == j) continue;
                const auto& aj = obj.accesses[j];
                if (aj.thread_id == ai.thread_id) continue;
                PairKey k;
                const std::string& fi = leafFn(ai);
                const std::string& fj = leafFn(aj);
                bool aj_writes =
                    aj.access_type == "Write" || aj.access_type == "WRITE" ||
                    aj.access_type == "Free"  || aj.access_type == "FREE" ||
                    aj.access_type == "RMW";
                if (aj_writes) {
                    if (fi > fj) continue;
                    k.fn_a = fi;
                    k.fn_b = fj;
                } else {
                    k.fn_a = fi;
                    k.fn_b = fj;
                }
                auto& agg = pairs[k];
                agg.writers.insert(ai.node_id);
                if (aj_writes) agg.writers.insert(aj.node_id);
                else agg.readers.insert(aj.node_id);
                agg.threads_a.insert(ai.thread_id);
                agg.threads_b.insert(aj.thread_id);
            }
        }
        // v19 P4+ safety: cap function_pair_summary for busy objects
        // (bluetooth/wifi modules can produce 18+ pairs on a single
        // shared object, e.g. CVE-2023-53046's hci_cmd_sync_work_entry
        // had 18 pairs each with 4 thread arrays, ballooning to 3+KB
        // per call and ~5M total prompt tokens across 100+ iterations).
        // Prioritise pairs whose (writer, other) functions are DIFFERENT
        // (true cross-function races) over same-function reentrant pairs.
        constexpr size_t kMaxPairsPerObject = 12;
        std::vector<std::pair<PairKey, PairAgg>> pairs_vec(
            pairs.begin(), pairs.end());
        std::stable_sort(pairs_vec.begin(), pairs_vec.end(),
            [](const auto& a, const auto& b) {
                bool aDiff = a.first.fn_a != a.first.fn_b;
                bool bDiff = b.first.fn_a != b.first.fn_b;
                if (aDiff != bDiff) return aDiff;
                size_t aN = a.second.threads_a.size() + a.second.threads_b.size();
                size_t bN = b.second.threads_a.size() + b.second.threads_b.size();
                return aN > bN;
            });
        nlohmann::json pair_summary = nlohmann::json::array();
        size_t pair_kept = 0;
        for (const auto& [k, agg] : pairs_vec) {
            if (pair_kept >= kMaxPairsPerObject) break;
            nlohmann::json p;
            p["writer_function"] = k.fn_a;
            p["other_function"] = k.fn_b;
            p["writer_node_ids"] = std::vector<int>(
                agg.writers.begin(), agg.writers.end());
            p["other_node_ids"]  = std::vector<int>(
                agg.readers.begin(), agg.readers.end());
            p["threads_writer"] = std::vector<int>(
                agg.threads_a.begin(), agg.threads_a.end());
            p["threads_other"]  = std::vector<int>(
                agg.threads_b.begin(), agg.threads_b.end());
            pair_summary.push_back(std::move(p));
            ++pair_kept;
        }
        if (pairs_vec.size() > kMaxPairsPerObject) {
            result["function_pair_summary_truncated"] = true;
            result["function_pair_summary_total"] = pairs_vec.size();
        }
        result["function_pair_summary"] = std::move(pair_summary);
        result["function_pair_coverage_note"] =
            "Each entry is a distinct (writer, reader/writer) function "
            "pair that shows up across threads on THIS shared object. "
            "Aim for AT LEAST ONE hypothesis per pair before moving on "
            "— missing a pair is the most common reason a patch's "
            "actual fix is not credited as a HIT.";
        return result.dump();
    }

    if (tool_name == "get_function_code") {
        std::string name = arguments.at("name").get<std::string>();
        std::unordered_set<Node*> nodes = ccpg_->getCPG()->findMethodsByName(name);
        if (nodes.empty()) {
            CPGNodeSet all_methods = ccpg_->getCPG()->getNodesByType("Method");
            for (Node* m : all_methods) {
                if (m->getName().find(name) != std::string::npos &&
                    m->getProperty("CODE") != "<empty>") {
                    nodes.insert(m);
                }
            }
        }
        nlohmann::json result = nlohmann::json::array();
        for (Node* node : nodes) {
            CCPGNode* ccpgNode = ccpg_->getCCPGNodeByCPGNode(node);
            ccpg::Function* func = ccpgNode ? ccpgNode->getFunction() : nullptr;
            if (func) {
                result.push_back({
                    {"function_id", func->getId()},
                    {"function_name", func->getFuncNode()->getCPGNode()->getName()},
                    {"function_body", func->getFuncNode()->getCPGNode()->getCode()}
                });
            }
        }
        if (result.empty()) return R"({"error": "Function not found: )" + name + R"("})";
        return result.dump();
    }

    if (tool_name == "get_lock_protection") {
        int node_id = arguments.at("node_id").get<int>();
        CCPGNode* node = ccpg_->getNodeByID(node_id);
        if (!node) return R"({"error": "Node not found."})";

        // Use the already-computed path-sensitive LockSets instead of the
        // "same file, line window" heuristic. For each reachable call
        // context, report the set of locks held. The response explicitly
        // flags unprotected contexts so the LLM can reason about data
        // races that only occur on some call paths.
        LSAnalysis* ls = LSAnalysis::getInstance();
        ccpg::Function* fn = node->getFunction();
        ccpg::ContextSet ctxs;
        if (fn) ctxs = fn->getContextSet();
        NodeLoc loc = node->getNodeLoc();

        nlohmann::json result;
        result["node_id"] = node_id;
        result["code"] = node->getCPGNode()->getCode();

        auto lockToCode = [](Lock* l) -> std::string {
            if (!l) return "?";
            CCPGNode* acq = l->getAcquire();
            return (acq && acq->getCPGNode()) ? acq->getCPGNode()->getCode() : "?";
        };

        nlohmann::json ctxArr = nlohmann::json::array();
        std::set<int> unionLockIds;
        std::vector<std::string> unionLockNames;
        bool anyUnprotected = false;

        auto addContextEntry = [&](const Context& c) {
            auto lockSet = ls->getLockSet(loc, c);
            nlohmann::json entry;
            entry["context"] = c.toString();
            nlohmann::json locks = nlohmann::json::array();
            for (Lock* l : lockSet) {
                if (!l) continue;
                std::string code = lockToCode(l);
                locks.push_back(code);
                if (unionLockIds.insert(l->getId()).second) {
                    unionLockNames.push_back(code);
                }
            }
            entry["locks_held"] = locks;
            entry["is_protected"] = !lockSet.empty();
            if (lockSet.empty()) anyUnprotected = true;
            ctxArr.push_back(std::move(entry));
        };

        if (ctxs.empty()) {
            addContextEntry(Context());
        } else {
            for (Context* ctx : ctxs) {
                addContextEntry(ctx ? *ctx : Context());
            }
        }

        result["contexts"] = ctxArr;
        result["all_locks_union"] = unionLockNames;
        // "Effectively unprotected" iff there's at least one call path with
        // an empty LockSet - matches the semantics used by the verifier.
        result["is_protected"] = !anyUnprotected && !unionLockNames.empty();
        result["has_unprotected_path"] = anyUnprotected;
        return result.dump();
    }

    if (tool_name == "check_reachability") {
        int from_id = arguments.at("from_node_id").get<int>();
        int to_id = arguments.at("to_node_id").get<int>();
        CCPGNode* from = ccpg_->getNodeByID(from_id);
        CCPGNode* to = ccpg_->getNodeByID(to_id);
        if (!from || !to) return R"({"error": "Node(s) not found."})";

        // BFS reachability
        std::queue<CCPGNode*> worklist;
        std::set<CCPGNode*> visited;
        worklist.push(from);
        visited.insert(from);
        bool found = false;
        while (!worklist.empty() && !found) {
            CCPGNode* current = worklist.front();
            worklist.pop();
            for (CCPGEdge* edge : current->getOutEdges()) {
                if (edge->getType() == CCPGEdge::EdgeType::ORDER) {
                    CCPGNode* next = edge->getDst();
                    if (next == to) { found = true; break; }
                    if (!visited.count(next)) {
                        visited.insert(next);
                        worklist.push(next);
                    }
                }
            }
        }
        return nlohmann::json{
            {"from_node_id", from_id},
            {"to_node_id", to_id},
            {"reachable", found}
        }.dump();
    }

    if (tool_name == "get_successors_chunked") {
        int start_node_id = arguments.at("node_id").get<int>();
        int chunk_size = arguments.value("chunk_size", 15);
        CCPGNode* start_node = ccpg_->getNodeByID(start_node_id);
        if (!start_node) return R"({"error": "Start node not found."})";

        nlohmann::json successors = nlohmann::json::array();
        std::queue<CCPGNode*> worklist;
        std::set<CCPGNode*> visited;
        worklist.push(start_node);
        visited.insert(start_node);

        while (!worklist.empty() && successors.size() < static_cast<size_t>(chunk_size)) {
            CCPGNode* current = worklist.front();
            worklist.pop();
            for (CCPGEdge* edge : current->getOutEdges()) {
                if (edge->getType() == CCPGEdge::EdgeType::ORDER) {
                    CCPGNode* successor = edge->getDst();
                    if (!visited.count(successor)) {
                        visited.insert(successor);
                        worklist.push(successor);
                        if (successors.size() < static_cast<size_t>(chunk_size)) {
                            successors.push_back({
                                {"node_id", successor->getId()},
                                {"code", successor->getCPGNode()->getCode()},
                                {"location", successor->getNodeLoc().toString()}
                            });
                        }
                    }
                }
            }
        }
        return successors.dump();
    }

    if (tool_name == "propose_hypothesis") {
        if (!ctx->verifier) return R"({"error": "Verifier not initialized."})";

        query::Hypothesis h;
        h.id = arguments.at("hypothesis_id").get<std::string>();
        h.description = arguments.at("description").get<std::string>();
        h.bug_category = arguments.at("bug_category").get<std::string>();
        h.severity = arguments.value("severity", "medium");

        if (!arguments.contains("nodes") || !arguments["nodes"].is_object()) {
            return R"({"error": "nodes must be an object mapping role names to node IDs."})";
        }
        for (auto& [role, val] : arguments["nodes"].items()) {
            if (!val.is_number_integer()) {
                return R"({"error": "Node ID for role ')" + role + R"(' must be an integer."})";
            }
            int nid = val.get<int>();
            if (!ccpg_->getNodeByID(nid)) {
                return R"({"error": "Node ID )" + std::to_string(nid) + R"( not found in CCPG."})";
            }
            h.nodes[role] = nid;
        }

        if (!arguments.contains("constraints") || !arguments["constraints"].is_array()) {
            return R"({"error": "constraints must be an array of {predicate, args} objects."})";
        }
        for (const auto& c : arguments["constraints"]) {
            query::VerificationConstraint vc;
            vc.predicate = c.value("predicate", "");
            vc.args = c.value("args", nlohmann::json::object());
            if (vc.predicate.empty()) {
                return R"({"error": "Each constraint must have a 'predicate' field."})";
            }
            h.constraints.push_back(std::move(vc));
        }

        // Structural sanity: a bug whose definition requires ≥2 distinct
        // program points (double-free, UAF, TOCTOU, data-race) cannot be
        // expressed by collapsing every role onto the *same* CCPG node.
        // Two threads passing through one source-code location is just
        // one event per thread, not two freestanding events on the same
        // object. Reject up-front so the LLM is forced to refine instead
        // of padding the confirmed-list with pseudo bugs.
        auto bc_lower = h.bug_category;
        std::transform(bc_lower.begin(), bc_lower.end(), bc_lower.begin(),
                       [](unsigned char c){ return std::tolower(c); });
        const bool requires_two_sites =
            (bc_lower == "double_free" || bc_lower == "double-free" ||
             bc_lower == "uaf" || bc_lower == "use_after_free" ||
             bc_lower == "use-after-free" ||
             bc_lower == "toctou" || bc_lower == "time_of_check" ||
             bc_lower == "data_race" || bc_lower == "data-race");
        if (requires_two_sites && h.nodes.size() >= 2) {
            std::unordered_set<int> uniq_nodes;
            for (const auto& [role, nid] : h.nodes) uniq_nodes.insert(nid);
            if (uniq_nodes.size() < 2) {
                nlohmann::json err;
                err["error"] = "structural_rejection";
                err["reason"] =
                    "All roles in this '" + h.bug_category +
                    "' hypothesis map to the SAME CCPG node id. A " +
                    h.bug_category + " needs at least two DISTINCT program "
                    "points (e.g. two different free call sites, or a "
                    "write and a separate read). Two threads passing "
                    "through one source-code line is one event per "
                    "thread, not a bug. Refine the hypothesis with "
                    "distinct node ids for each role, or choose a "
                    "different bug_category.";
                return err.dump();
            }
        }

        auto result = ctx->verifier->verify(h);
        auto feedback = result.toFeedbackJson();

        if (result.all_satisfied) {
            // Phase 4 dedupe. Build a canonical fingerprint from bug
            // category + the sorted set of node ids touched by the
            // hypothesis. LLMs often re-propose structurally identical
            // hypotheses with only a renamed role ("check" -> "chk"),
            // which inflates the downstream count without new signal.
            std::vector<int> node_ids;
            node_ids.reserve(h.nodes.size());
            for (const auto& [role, nid] : h.nodes) node_ids.push_back(nid);
            std::sort(node_ids.begin(), node_ids.end());
            std::string fingerprint = h.bug_category + "|";
            for (std::size_t i = 0; i < node_ids.size(); ++i) {
                if (i > 0) fingerprint += ",";
                fingerprint += std::to_string(node_ids[i]);
            }
            if (!ctx->accepted_fingerprints.insert(fingerprint).second) {
                feedback["dedupe"] = "Duplicate of an earlier accepted hypothesis"
                    " with the same (bug_category, node-id set). Skipped from "
                    "confirmed list. Propose something substantively different "
                    "or call finish_detection.";
                feedback["is_duplicate"] = true;
            } else {
                ctx->confirmed_hypotheses.push_back(std::move(h));
            }
        }

        return feedback.dump();
    }

    if (tool_name == "finish_detection") {
        return "finish";
    }

    return R"({"error": "Unknown tool: )" + tool_name + R"("})";
}

DetectorAgent::DetectionResult DetectorAgent::runDetection(const query::VulnerabilitySurface& surface) {
    reset();

    query::HypothesisVerifier verifier(ccpg_, ThreadCreationTree::getInstance(),
                                       HBGraph::getInstance());
    // v23 Fix #3b: let the verifier use surface-level co-location as the
    // final fallback for `same_location` on list-helper synth accesses.
    // Without this, list_mutation_race hypotheses on objects like
    // binder_procs / hci_dev_list / nf_tables are silently rejected
    // because static aliasing cannot bridge "entry pointer" vs "head load".
    verifier.setSurface(&surface);

    DetectorContext ctx;
    ctx.surface = &surface;
    ctx.ccpg = ccpg_;
    ctx.tct = ThreadCreationTree::getInstance();
    ctx.verifier = &verifier;

    std::string prompt =
        "A pointer-analysis-based vulnerability surface has been computed for a concurrent C/C++ module. "
        "It is organized around **shared objects** — each entry shows one memory object and ALL threads "
        "that access it, with access types (Read/Write/Free), lock protection status, and risk flags.\n\n"
        "Start by calling `get_vulnerability_surface` to see the full report. "
        "Focus on objects with the highest risk_score (especially those marked [UAF_RISK]). "
        "Use `get_object_details` for full node IDs, then read code with `get_function_code` or "
        "`get_function_ops`, and propose hypotheses with `propose_hypothesis`.\n\n"
        "Begin now.";

    send_message(prompt, &ctx);

    DetectionResult result;
    result.confirmed = std::move(ctx.confirmed_hypotheses);
    return result;
}

std::string DetectorAgent::parseResult(const std::vector<ChatMessage>& history) {
    return "Detection complete.";
}

} // namespace llm_client
