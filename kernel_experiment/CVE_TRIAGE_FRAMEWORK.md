# Lace CVE Triage Framework

Use this before launching the detector + evaluator on any new MISS CVE.
Filling out the form costs ~5 minutes and saves 25+ minutes of LLM
tokens + evaluator time when a CVE is fundamentally out of static-
analysis reach (Cat C of `UNRECALLABLE_2026-05-18.md`).

---

## 1. Five-minute decision form

For each candidate CVE, answer these in order. Stop at the first `→ OUT OF REACH`.

### Step T1 — Patch shape

Read `ground_truth.json["patch"]` and the `description` field. Classify the *fix shape*:

| Patch shape | Examples | Lace mappable? |
|---|---|---|
| **Add `READ_ONCE / WRITE_ONCE` annotation** | u8/u16/u32 cross-thread RW → annotate | YES — Pattern #3 (scalar tearing) |
| **Add `spin_lock / mutex_lock` around an access** | unprotected write later locked | YES — Template 1 (conflicts + concurrent) |
| **Switch `list_for_each_entry` → `_rcu` + `rcu_read_lock`** | non-RCU iteration over RCU-mutated list | YES — Pattern #2 |
| **Reorder free vs. unregister/close path** | move `device_remove_groups()` before `deactivate()` | YES — Template 2 (use vs free) |
| **Add `synchronize_rcu / wait_for_completion / kthread_stop`** | join cleanup-vs-callback | YES — Template 2 with `hb(use, free, expected=false)` |
| **Add reference-count increment before publication** | `refcount_inc()` before `list_add()` | YES — Pattern #7 |
| **Move publication (`skb_get`, `list_add`) AFTER mutation** | publish-then-mutate ordering bug | BOUNDARY — needs `unsafe_atomic_block` template |
| **Reorder rb_tree iteration vs. concurrent rb_erase** | rb_next after lock-drop races rb_erase | BOUNDARY — surface has rb_node fields but no rb_mutation_race flag |
| **Add new logic / state-machine branch (NO sync change)** | check `stale_event_ptr`, recompute index | **OUT OF REACH** — Cat C (CVE-2025-37882) |
| **Change semantic interpretation of hardware/event payload** | reinterpret COMP_RING_UNDERRUN to suppress matching | **OUT OF REACH** — Cat C |
| **Patch is a kbuild / configuration fix, not C code** | Kconfig dependency tightening | **OUT OF REACH** |

### Step T2 — Required components in the surface

Identify what the patched-race hypothesis needs to look like, then check if all components are already in the surface (use any prior LLM_dump or run with `LACE_EARLY_EXIT_AFTER_SURFACE=1`):

| Need | How to grep | If absent → blocker |
|---|---|---|
| Patched shared field/object | `python` over `vulnerability_surface.json` looking for `name` | B1.shared_field_extraction |
| All patched access sites (writer / reader / freer / user) | `accesses[].location` matches patch hunks | B1 (subgraph) |
| Both threads of the race | `accesses[].function_name` distinct + cross-thread | A2.thread_set_coverage |
| Risk score ranks the object in top-25 | `risk_score` >= ~120 typical for top-25 | B3.risk_scoring |
| Right flags fire (`has_list_mutation`, `has_scalar_torn_access`, …) | `obj.has_*` booleans | B3 (sub-issue) |

### Step T3 — LLM tractability of selection

Even when surface is perfect, the LLM may pick the wrong (writer, reader) pair. Estimate the LLM-selection difficulty:

- **EASY** — the patched object has 1-2 cross-thread function pairs total. LLM can't go wrong.
- **MEDIUM** — 3-6 cross-thread function pairs. LLM needs `suggested_hypotheses` guidance.
- **HARD** — >6 function pairs OR readers/writers are deeply nested behind macros. LLM needs Pattern-specific prompt augmentation.

### Step T4 — Verdict

Combine T1 + T2 + T3:

- **GO** (within reach): T1 = YES; T2 = all required surface signals present (or only B3 risk-boost gap); T3 ∈ {EASY, MEDIUM with existing suggested_hypotheses path}.
- **TRY** (boundary): T1 = BOUNDARY OR T2 has B1 gap that we can fix with a surface-side enhancement OR T3 = HARD needing a new prompt pattern.
- **PARK** (out of reach): T1 = OUT OF REACH OR T2 has a CPG-coverage gap (see `UNRECALLABLE_2026-05-18.md` Cat A/B) OR the patch is non-code.

When verdict = PARK, write the CVE into `UNRECALLABLE_2026-05-18.md` with a one-paragraph rationale and skip detector run.

---

## 2. Triage of currently-remaining 7-set (post-Fix #7 baseline)

(Note: SYZBOT-3536db46dfa58c57 already HIT; CVE-2025-37882 already PARK'd.)

| CVE | T1 shape | T2 surface | T3 selection | Verdict | Notes |
|---|---|---|---|---|---|
| **CVE-2024-27404** | Add `WRITE_ONCE` to 3 writers of `subflow->remote_id` + `READ_ONCE` to 1 reader | field present (`mptcp_subflow_context.remote_id@146`), 7 accesses incl. all 4 patch sites, all 3 writers + reader; **was rank 98 (risk 66) pre-Fix #7, now rank 36 (risk 146)** | MEDIUM — 4 threads, simple plain scalar | **GO** | Fix #7 raises rank to 36, top-25 still slightly tight; if LLM doesn't pick, raise +80 cap further or add Pattern-#3 explicit teaching |
| **CVE-2024-43830** | Reorder `device_remove_groups()` before `deactivate()` to close UAF on `trigger_data` | `field:struct.led_classdev.trigger_data@536` at **rank 12 risk 79**, in top-25 already | EASY — single shared field, clear deactivate / sysfs handler pair | **GO** | Highest-confidence GO of the set; LLM should propose UAF Template 2 with `free=deactivate()` and `use=sysfs show/store(trigger_data)` |
| **CVE-2024-56555** | Drop `proc->inner_lock` during `binder_add_freeze_work` iteration races `binder_deferred_release` rb_erase/move-to-dead-nodes; fix is to use `_safe` iteration form | Surface has `global:binder_dead_nodes` (rank 3 risk 252), `binder_node.rb_node@104`, `rb_node.__rb_parent_color`; but **no `proc->nodes` field nor a dedicated rb_mutation_race flag** | HARD — rb-tree iteration is not a list_for_each / list_helper pattern; verifier won't synthesise rb_erase as Write on rb_node sibling fields | **TRY** | Boundary — need: (a) extend surface generator to flag `rb_erase`/`rb_link_node` as Write on the rb-tree root field, (b) new Pattern #9 "rb-tree iteration vs rb_erase across lock-drop window" in prompt. Estimated 1-2 day implementation. |
| **CVE-2025-38165** | Move `skb_get(skb)` AFTER `skb_linearize()` to avoid publishing a still-being-mutated skb in sk_psock backlog → recvmsg path | **No prior dump** — needs first detector run to assess surface | UNKNOWN until detector run | **TRY** | Probable BOUNDARY (publish-then-mutate ordering bug). Run detector once with `LACE_EARLY_EXIT_AFTER_SURFACE=1`, then re-triage. |
| **CVE-2025-38250** | `vhci_release()` → `hci_unregister_dev` → `kfree(vhci_data)` races concurrent `ioctl(/dev/vhci)` on the same fd; fix adds RCU synchronization in `hci_unregister_dev` after unlinking from `hci_dev_list` | **No prior dump** — needs first detector run to assess surface | UNKNOWN until detector run | **GO** | Looks like canonical Template 2 UAF; surface should have `hci_dev_list` (probably global) + `hdev->driver_data`. Confirm with one detector run. |

### Recommended order (highest expected ROI first)

1. **CVE-2024-43830** (verdict GO, easy template, top-25 already): expect HIT with current prompt or one prompt nudge.
2. **CVE-2024-27404** (verdict GO, currently running): expect HIT if rank-36 is good enough; if not, one more risk-boost.
3. **CVE-2025-38250** (verdict GO, UAF template): blind run, expect HIT if hci_dev_list surfaces.
4. **CVE-2024-56555** (verdict TRY, rb-tree extension): defer — needs backend change.
5. **CVE-2025-38165** (verdict TRY, publish-then-mutate): defer — needs `unsafe_atomic_block` template or new pattern.

---

## 3. Triage-to-action mapping

| If primary blocker is … | Then fix component is … | Cost |
|---|---|---|
| **Patch shape OUT OF REACH** | (none — PARK + document) | n/a |
| **B1 / A2 surface gap (field missing, callback missing)** | VulnerabilitySurfaceGenerator OR CPG regeneration (Cat A/B in UNRECALLABLE doc) | medium / high |
| **B3 risk-scoring rank too low** | VulnerabilitySurfaceGenerator risk-boost (e.g. Fix #7) | low |
| **C1 / C4 LLM picks wrong object / wrong access site** | DetectorAgent suggested_hypotheses generation OR system prompt pattern | low |
| **D2 conflicts FAIL (write side unrecognised)** | HypothesisVerifier eval_conflicts fallback (e.g. Fix #6) | low |
| **D4 same_lock FAIL (alias breakdown)** | HypothesisVerifier eval_same_lock fallback (e.g. Fix #3c) | low |
| **E1 VerificationAgent drops correct hypothesis as FP** | VerificationAgent prompt (Anti-pattern A/B/C in DetectorAgent.cpp) | low |

---

## 4. Lessons captured so far (v23 sprint)

1. **Run with `LACE_EARLY_EXIT_AFTER_SURFACE=1` first** — 30 s vs. 25 min. Use the resulting `vulnerability_surface.json` to do T2 without LLM cost.
2. **Cleanup-path races are usually benign** — they get serialised by the outer state machine. Pattern-prompt anti-pattern A/B already nudges the LLM away from them; don't be surprised if surface is full of them.
3. **Static aliasing of `list_del(&entry->member)` vs. `list_for_each_entry(head, ...)` is undecidable** — that's why Fix #3b/#3c/#6 relax `same_location` / `same_lock` / `conflicts` to surface-level co-location.
4. **Semantic event-state TOCTOU races are out of static reach** — Cat C. Don't iterate.
