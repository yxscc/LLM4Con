# Unrecallable Set — v23 P9a CPG/IR Coverage Gaps

Tracking doc for v22-MISS CVEs whose recall **cannot** be unblocked under
the current P9a (dynamic callback-registration scan) design, because the
required callback function is absent from the Joern CPG at the
`label="METHOD"` level. These are excluded from the v23 recall-improvement
scope and revisited only when the root cause (Joern parses the kernel
source with a default `#ifdef` configuration that differs from the LLVM
IR's `-DCONFIG_*` flags) is addressed at the CPG-generation layer.

Generated: 2026-05-18  (post Fix #1 = DWARF field labels, Fix #2 = P9a)

---

## 1. Methodology

For every v22 MISS, we ran the detector with `LACE_EARLY_EXIT_AFTER_SURFACE=1`
(no LLM cost) and recorded the `[P9a]` line:

```
[P9a] Dynamic callback discovery: scanned N stores, harvested K unique
callbacks, added X new entries (Y already known, Z not in CPG)
```

`Z>0` means: P9a found a store of `@fn` into a known callback-host struct
(`struct.notifier_block.notifier_call`, `struct.work_struct.func`,
`struct.timer_list.function`, `struct.hrtimer.function`,
`struct.delayed_work.work.func`, `struct.tasklet_struct.func`,
`struct.wait_queue_entry.func`, `struct.callback_head.func`,
`struct.irq_work.func`) inside the patched `.ll`, but neither
`cpg->findMethod(name)` (after demangle variants) nor
`cpg->findMethodByLLVMFunction(fn)` (file+line) could resolve `@fn` to a
CPG `METHOD` node.

The unrecallable set below was further verified with a multi-line aware
parse of `cpg_dot/<CVE>/export.dot`: for each callback we counted nodes
with `NAME="<fn>"` and split them by `label="METHOD"` (real / stub /
external) vs. `METHOD_REF` / `BINDING` / `TYPE`.

---

## 2. Cases (8 CVEs, 11 callback misses)

### 2.1 Category A — Callback completely absent from CPG (no node whatsoever)

The function definition was preprocessed out by Joern's frontend. Joern
saw none of the function's references either.

| CVE | Patched object | v22 metrics | Missing callback(s) | Likely `#ifdef` |
|---|---|---|---|---|
| **CVE-2024-35986** | `struct tusb1210->charger` | MISS · primary=`B1.shared_field_extraction` · bugs=10 · in_surface=`no` | `tusb1210_psy_notifier`, `tusb1210_chg_det_work` | `CONFIG_POWER_SUPPLY` in `drivers/phy/ti/phy-tusb1210.c` |
| **CVE-2024-46704** | `struct work_struct->data` | MISS · `B1.shared_field_extraction` · bugs=4 · in_surface=`no` | `work_for_cpu_fn` | `CONFIG_SMP` in `kernel/workqueue.c` |
| **CVE-2024-53136** | `struct inode metadata` / inode lock state read by `generic_fillattr()` | MISS · `A2.thread_set_coverage` · bugs=0 · in_surface=`no` | `synchronous_wake_function` | `CONFIG_SHMEM` block in `mm/shmem.c` (the function is in `kernel/sched/wait.c`, not preprocessed into the shmem TU view) |
| **SYZBOT-01affb1491750534** | `struct pool_workqueue->stats[PWQ_STAT_COMPLETED]` | MISS · `B1.shared_field_extraction` · bugs=8 · in_surface=`no` | `work_for_cpu_fn` | `CONFIG_SMP` (same as 46704) |
| **SYZBOT-52cb782c704e243e** | `struct fib6_nh->last_probe` | MISS · `C1.target_object_selection` · bugs=6 · in_surface=`unknown` | `rt6_probe_deferred` | `CONFIG_IPV6_ROUTER_PREF` in `net/ipv6/route.c` |

Notes:
- For **CVE-2024-35986** the patched field (`charger`) is itself inside
  the same `#ifdef CONFIG_POWER_SUPPLY` block as the missing callbacks.
  Joern parsed the struct **without** that block, hence the field is
  also unknown to the CPG-side view → even if we manually injected the
  threads, downstream surface generation would not recognise the field
  via name (only via offset). This case requires **CPG regeneration
  with `CONFIG_POWER_SUPPLY=y`** to make progress.
- **CVE-2024-46704 / SYZBOT-01affb1491750534**: `CONFIG_SMP` is the
  default in any realistic build, so this is purely a Joern frontend
  default-config gap. Easiest of the five to fix at the CPG-gen layer.
- **SYZBOT-52cb782c704e243e**: `rt6_probe_deferred` is the *direct*
  writer of `fib6_nh->last_probe`. Without it as a thread, the field
  never enters the surface as a cross-thread RW. Unblocking this CVE
  is highly likely to flip MISS→HIT.

### 2.2 Category B — Callback referenced but never defined as METHOD

Joern records `METHOD_REF` / `BINDING` / `TYPE` nodes for the callback
(it sees `INIT_WORK(&w, fn)` etc. and the function-pointer typedef), but
no `METHOD` node. The function body itself was skipped by the C
frontend.

| CVE | Patched object | v22 metrics | Missing callback | Notes |
|---|---|---|---|---|
| **CVE-2024-53160** | `struct kfree_rcu_cpu->monitor_work.timer.expires` | MISS · `C1.target_object_selection` · bugs=5 · in_surface=`yes_top_25` | `rcu_leak_callback` | Definition is `static void rcu_leak_callback(struct rcu_head *rhp) { }` — empty body. Joern's c2cpg appears to skip METHOD-node creation for trivial empty-body statics. **This callback is unrelated to the patch target**; even adding it as a thread would not flip recall. Already in_surface=top_25 → real fix is on the Phase-2 prompt side (C1). |
| **SYZBOT-4dfb96a94317a78f** | `global rcu_state.n_force_qs` | MISS · `A1.build_pipeline` · bugs=6 · in_surface=`unknown` | `rcu_leak_callback` | Same root cause as 53160. Patched object is on a different code path (`rcu_state.n_force_qs`), so the missing callback is **not relevant** to recall here either. Primary blocker is `A1.build_pipeline` — different problem entirely. |
| **SYZBOT-52e3dbded1f71729** | unknown | MISS · `C1.target_object_selection` · bugs=5 · in_surface=`unknown` | `kcm_done_work` | Definition `static void kcm_done_work(struct work_struct *w) { kcm_done(...); }` — single-statement. Joern same quirk as `rcu_leak_callback`. Patched object is `unknown` in v22, so we can't yet judge relevance. |

---

## 3. Summary table

| CVE | Missing callback | Category | Relevant to patch? | Blocks recall? |
|---|---|---|---|---|
| CVE-2024-35986 | `tusb1210_psy_notifier` / `tusb1210_chg_det_work` | A (`#ifdef CONFIG_POWER_SUPPLY`) | YES (direct race on `tusb->charger`) | YES |
| CVE-2024-46704 | `work_for_cpu_fn` | A (`#ifdef CONFIG_SMP`) | partial (one of many `work_struct` callers) | partial |
| CVE-2024-53136 | `synchronous_wake_function` | A (cross-TU, `CONFIG_SHMEM`) | unclear (primary is A2 anyway) | possibly |
| CVE-2024-53160 | `rcu_leak_callback` | B (empty-body skipped) | NO | no |
| SYZBOT-01affb1491750534 | `work_for_cpu_fn` | A (`#ifdef CONFIG_SMP`) | partial | partial |
| SYZBOT-4dfb96a94317a78f | `rcu_leak_callback` | B (empty-body skipped) | NO | no |
| SYZBOT-52cb782c704e243e | `rt6_probe_deferred` | A (`#ifdef CONFIG_IPV6_ROUTER_PREF`) | YES (direct writer of `last_probe`) | YES |
| SYZBOT-52e3dbded1f71729 | `kcm_done_work` | B (trivial-body skipped) | unclear | unclear |

**Recall ceiling lost to this gap**: 2 cases definitely unrecallable
without CPG-layer work (CVE-2024-35986, SYZBOT-52cb782c704e243e), +2-3
partial (CVE-2024-46704, SYZBOT-01affb1491750534, CVE-2024-53136).

---

## 4. Related (broader) coverage gaps

These are NOT in the 8-case set above but suffer the same family of
problems and are also outside the v23 P9a scope:

- **49 v22-MISS CVEs** have no callback-host stores in their `.ll` at
  all (P9a logs no harvest line). The callback function is defined in
  a different translation unit whose IR is not part of the patched
  `.ll`. Cross-TU coverage problem — orthogonal to CPG.
- **27 v22-MISS CVEs** have callbacks that P9a does harvest but they
  are **already** known to Phasar's entry-point detector, so P9a is a
  no-op. These are not blocked by the CPG gap; whatever else blocks
  their recall is unrelated to thread coverage.

---

## 5. Action items

### Do not attempt in v23 P9a scope

The 8 cases above are explicitly excluded from the v23 recall-improvement
scope. Any v23 metric reports should:
- (a) note that these 8 are pre-classified unrecallable at the
  CPG-coverage layer;
- (b) when computing "P9a contribution" deltas, do **not** count these
  as failures of P9a — count them as `cpg_coverage_gap` instead.

### Future work — possible unblocks, ranked by expected ROI

| Idea | Cases unblocked | Cost | Risk |
|---|---|---|---|
| **F1**: Regenerate CPG with kernel-default `=y` configs (`CONFIG_SMP`, `CONFIG_IPV6_ROUTER_PREF`, `CONFIG_POWER_SUPPLY`, `CONFIG_SHMEM`, …). The `joern-parse` invocation in `src/CPG/CPGGenerator.cpp` would receive `--define CONFIG_SMP=1 …`. | A-category (CVE-2024-35986, -46704, -53136, SYZBOT-01affb..., SYZBOT-52cb782c...) — up to 5 | medium (need a per-CVE config list, or a sensible default set) | medium (extra `#ifdef` paths may regress unrelated cases) |
| **F2**: Patch the c2cpg frontend (or post-process the CPG) to emit METHOD nodes for empty/trivial-body statics that already have METHOD_REF nodes. | B-category (CVE-2024-53160, SYZBOT-4dfb96a..., SYZBOT-52e3dbded...) — but most are NOT recall-blocking, so the realistic ceiling is 0–1 | medium (Scala patch to joern, or pure post-process) | low |
| **F3**: Synthesize a "virtual METHOD" CPG node directly from LLVM IR + DWARF when `findMethodByLLVMFunction` fails. Body comes from LLVM IR walks, never from CPG AST. Surface generator already does most of its work on LLVM IR, so this could work; HBGraph / lock-set may degrade. | All 8 + partially the 49 cross-TU set (if their IR is available in another `.ll`) | high (~500–1000 LoC, touches CPG + CCPG + LSAnalysis) | high |

### Recommended near-term direction (not part of this doc's scope, see PROMPT_v23_optimization)

The 17 v22-MISS cases in the C1 cluster where the patched field is
already in the surface top-25 (3 in top-5) are a much higher-ROI target
than the 8 cases here — those are Phase-2 prompt issues, not CPG/IR
coverage issues, and do not require any backend changes.

---

## 6. Category C — Semantic / event-state races (added 2026-05-19)

A second class of unrecallable CVEs has the patched function and
relevant access sites in the surface (sometimes even Top-1), but the
patch does **not** add list/lock synchronization — instead it changes
event-classification logic to detect stale hardware/IRQ-event payloads.
These races sit between hardware-event time and software-dispatch time
on a single thread of execution; from a static-analysis viewpoint,
there is no second thread doing a "Write" on the patched memory cell,
so no `data_race` hypothesis of any shape can match.

| CVE | Patched function | Patch shape | Surface coverage | Why unrecallable |
|---|---|---|---|---|
| **CVE-2025-37882** | `handle_tx_event()` (xhci-ring.c:2845) | Adds `ring_xrun_event` plumbing to suppress matching when `COMP_RING_UNDERRUN` / `COMP_RING_OVERRUN` event TRB pointer is stale relative to the ring enqueue position. No list-sync change, no lock added. | `field:struct.xhci_ring.td_list@48` at rank 2 (surface OK after Fix #4); `list_empty(...)` reader at line 2845 surfaced; v23 Fix #6 made the writer-side conflicts check pass. | Even when LLM correctly proposes `handle_tx_event` reader paired with any cross-thread list mutator, the evaluator (correctly) marks it FP because the patch doesn't touch the list-mutator side or add synchronization. The actual race is a TOCTOU between *hardware event-write time* and *software event-read time* on the same IRQ thread — there is no static second-writer to pair with. Evaluator's own `suggested_change`: "patch-aware/event-state templates for stale descriptor or event-pointer races" — i.e. requires a new hypothesis template that doesn't fit Lace's `(writer_node, reader_node) cross_thread` shape. |

Notes:
- The static-analysis pipeline did everything correctly for this case
  (surface, suggestion ranking, verifier acceptance under Fix #6). The
  KEEP'd `ring_td_list_kill_vs_tx_event_stale_event` hypothesis even
  names the right reader site. The evaluator's FP verdict is also
  correct on its own terms. The mismatch is at the **hypothesis-shape**
  level: Lace's universe is "two-thread data races with a writer", and
  this CVE's universe is "single-thread stale-event TOCTOU".
- **Fixes still landed in v23 are net-positive even on this CVE**: the
  surface now shows `handle_tx_event`'s `list_empty()` reader (Fix #4,
  #5, #6, prompt Pattern #8), and any future CVE that DOES have a
  matching submission-side list_add writer (e.g. on `cmd_list`,
  `cancelled_td_list`) will benefit. CVE-2025-37882 itself is parked.
- Likely future-work direction: introduce a `stale_event_payload`
  hypothesis template with constraints `(event_read, ring_advance,
  ¬hb(ring_advance, event_read))` — but this is a v24+ scope change.

### Updated recall ceiling (post v23 sub-fixes)

Adding CVE-2025-37882 to the unrecallable set: **9 CVEs** total now
classified as outside the v23 recall-improvement scope (8 CPG-coverage
gaps + 1 semantic event-state race).

---

## 7. Category D — Cross-TU coverage gap (added 2026-05-19)

The third class: patched object is in surface (B1 partial), but the
SIDE that the patch touches (the free site or the use site) lives in a
translation unit not linked into the per-CVE `.ll`. The surface for the
field only contains bookkeeping writes from the core module; the actual
race components are invisible to Lace.

| CVE | Patched object | What the surface has | What's missing | Why unrecallable |
|---|---|---|---|---|
| **CVE-2024-43830** | `struct led_classdev->trigger_data@536` (UAF in sysfs handler after `deactivate()` frees the trigger-specific data) | rank 13 / risk 79, but `accesses` is 20× `= NULL` bookkeeping writes from `led_trigger_register*` / `led_trigger_set_default` / `led_trigger_write` in `led-triggers.ll` — zero Reads, zero Frees | (a) the actual `kfree(trigger_data)` lives inside trigger-specific `*->deactivate()` implementations (e.g. `disk-activity-trig.c`, `usbport-trig.c`) that are not in the linked `.ll`; (b) sysfs `*_show` / `*_store` handlers that read `trigger_data` are registered via `device_add_groups()` whose callback host is also outside the `.ll` | LLM correctly proposes a sibling UAF on `led_classdev->trigger` (`led_cdev_trigger_set_null_vs_format_read`), but the evaluator (per v22 reason) explicitly grades sibling-field races as FP for this CVE. No `propose_hypothesis` shape over the surface graph can reach the patched access pair because both required nodes (free of `trigger_data`, read of `trigger_data` in sysfs) are absent from the IR Lace can see. |
| **CVE-2025-38250** | hdev SRCU lifetime: `hci_dev_reset → hci_dev_do_reset → hdev->flush() == vhci_flush → skb_queue_purge(&hdev->cmd_q)` races with `vhci_release → hci_unregister_dev → kfree(hdev)`; fix wraps the reset path in `srcu_read_lock(&hdev->srcu)` and adds `synchronize_srcu` to the unregister path | `hci_dev_reset` / `hci_unregister_dev` / `hci_release_dev` ARE in `hci_core.ll` and have many `field:struct.hci_dev.*` entries in top-25 (post Fix #8) | (a) `vhci_flush` (the actual Use of `cmd_q` from a different thread) lives in `drivers/bluetooth/hci_vhci.c` — not in `hci_core.ll`; (b) `hdev->cmd_q` is an embedded `struct sk_buff_head` and Lace's surface generator does not enumerate sub-field GEPs of embedded structs — there is NO `field:struct.hci_dev.cmd_q*` surface entry at all; (c) the dispatch through the `hdev->flush()` function pointer is opaque to interprocedural analysis from the `hci_core.ll` side | LLM proposes the closest reachable shape — sibling-field UAFs `sent_cmd_set_vs_release_free` and `fw_info_set_vs_release_free` (writer = `hci_send_cmd_sync` or `hci_set_fw_info`; reader = `hci_release_dev` reading the field before `kfree_*`). These are the same structural pattern (command-path races with cleanup) on a different field, so the evaluator (per v22 reason) grades them sibling-FP. Surfacing the patched `cmd_q` Use requires linking `hci_vhci.c` IR AND extending the surface generator to handle embedded-struct subfields. |

Notes:
- **This is a build-step problem, NOT a Lace-algorithm problem.** The
  fundamental fix is at the per-CVE `.ll` construction step (the
  `kernel_experiment/<CVE>/*.ll` is missing TUs that the patched race
  needs). The Lace surface generator / verifier / agent components all
  behave correctly given the IR they're handed; they simply cannot
  surface what is not present.
- **Recommended batch-fix strategy**: defer all Cat D CVEs and collect
  them. Once we have ~10 such cases we can implement ONE build-step
  upgrade (either (a) link a configurable allow-list of "related TUs"
  next to the patched file, e.g. `drivers/bluetooth/*.c` for hci_core
  CVEs, or (b) implement true cross-TU IR resolution via additional
  `.ll` files), and recover multiple CVEs in a single change.
- **Same root cause as the "49 cross-TU CVEs" in Section 4** — those 49
  were already counted, and this v23 sprint has now added 2 confirmed
  Cat D instances (CVE-2024-43830, CVE-2025-38250). The total Cat D
  set is therefore ~51 CVEs; expect more to be re-classified into Cat
  D as we triage future MISSes.
- Pre-flight T2 step in `CVE_TRIAGE_FRAMEWORK.md` should now also count
  "all surface accesses on patched field are bookkeeping `= NULL` /
  `= 0` writes with no read or free" as evidence of cross-TU gap, AND
  "patched function is reachable but the function-pointer dispatch
  (`hdev->flush()`) goes to a callee outside the .ll" as evidence of
  cross-TU + func-pointer gap.

### Updated recall ceiling (post v23 sub-fixes)

**11 CVEs** total now classified as outside the v23 recall-improvement
scope (8 CPG-coverage gaps + 1 semantic event-state race + 2 cross-TU
coverage gap [CVE-2024-43830, CVE-2025-38250]).
