# Precision fix strategy — v22 → v24

v22 numbers from `evaluation_report_agent_v22.json`:

```
total bug reports : 588   (≈6 per CVE)
TP                :  21   precision 3.57%
FP                : 567   FP-rate    96.43%
recall            : 15/100 CVE
```

The blowup is uniform: **25+ CVEs each carry 8-11 FPs**, regardless
of whether the CVE itself is HIT or MISS. The detector behaves as if
it has a quota — it ALWAYS emits 5-10 hypotheses, and most of them
don't match the patched object.

---

## 1. FP decomposition (verified from `fp_root_causes`)

| Category | Component | Count | % | Phase |
|---|---|---|---|---|
| `hypothesis_too_aggressive` | DetectorAgent_prompt | **337** | **60%** | LLM (P2) |
| `lockset_wrong` | LSAnalysis | **79** | **14%** | Static (P3) |
| `hbg_missing_edge` | HBGraph | **74** | **13%** | Static (P3) |
| `constraint_misverified` | HypothesisVerifier+AliasChecker | 28 | 5% | Static (P3) |
| `hypothesis_too_aggressive` | VerificationAgent_prompt | 18 | 3% | LLM (P4) |
| `thread_local_var` | HypothesisVerifier+AliasChecker | 12 | 2% | Static (P3) |
| `thread_id_misassigned` | thread_extraction+TCT | 7 | 1% | Static |
| `other` | mix | 10 | 2% | mix |

Three real root causes account for **86%** of all FPs:
1. LLM scattering hypotheses on sibling/cousin fields (60%)
2. LSAnalysis missing same-lock equivalence (14%)
3. HBGraph missing kernel-specific HB edges (13%)

---

## 2. Root cause #1: LLM hypothesis scatter (337 FPs)

### Concrete pattern (from evidence text)

**CVE-2013-1792** — patch fixes `user_struct->uid_keyring` race in
`install_user_keyrings()`. The LLM proposed:
- idx 2: `cred->session_keyring` in `install_session_keyring_to_cred()` → FP
- idx 3: same again, kept by verification LLM → FP
- idx 4: `cred->process_keyring` in `install_process_keyring_to_cred()` → FP
- idx 6: `cred->thread_keyring` in `key_fsuid_changed` → FP

Every proposal is a *plausible-looking sibling keyring field*. None
of them touches the actual patched object (`user_struct->uid_keyring`).

**CVE-2016-9806** — patch protects `nlk->cb.skb` and `nlk->cb.module`
in `netlink_dump`. The LLM proposed:
- idx 1: `nlk->groups` in `netlink_release/netlink_getname` → FP
- idx 2: same field again, different reader → FP
- idx 3: `nlk->cb_running` in `netlink_sock_destruct` → FP
- … and 5 more on adjacent `nlk->*` fields

**CVE-2016-7911** — patch fixes UAF race on `task->io_context`. The
LLM proposed `io_context->ioprio` data_race — wrong race type
entirely (RW vs UAF) even though sibling field.

### What's actually happening

v23 Fix #8.1 added a **mandatory enumeration rule** to the prompt:

> "before you may call `finish_detection`, you MUST call
> `get_object_details` on EVERY top-25 surface object whose flag list
> contains BOTH `[CROSS_THREAD_RW]` AND at least one of …"

This was added to fix CVE-2024-27404 (LLM skipped `remote_id` after
exploring `local_id`). But the side effect is exactly the scatter:
the LLM now feels obliged to **propose hypotheses on every flagged
object**, including all sibling fields adjacent to the real target.

### Quantitative confirmation

If the LLM proposed only 2 hypotheses per CVE (instead of 6), FPs
would drop to ~200 even with NO other change, simply because
detector output volume drops 3x. The TP count would be marginally
affected because top-2 hypotheses tend to be the highest-scored
ones (where the actual patched object usually is when present in
the surface).

---

## 3. Root cause #2: LSAnalysis same-lock miss (79 FPs)

Direct quotes from evidence text:

> **CVE-2017-6346**: reader at af_packet.c:1662 and writer at :1728
> both dominated by `mutex_lock(&fanout_mutex)`. Verifier did not
> apply same-lock exclusion.

> **CVE-2022-48830**: both accesses inside
> `spin_lock(&isotp_notifier_lock)` critical sections. Same.

> **CVE-2022-48931**: both protected by `configfs_dirent_lock`.
> Same.

Pattern: two `lock_func(&LOCK_VAR)` call sites with identical lock
argument are not being unified by `LSAnalysis::getLockSet`.

Likely cause: the lock pointer argument is a Phasar abstract location
that DIFFERS between the two call sites (one might be `&X` in
function A, another might be `&X` in function B, and Phasar's
context-sensitive abstract location splits them). `LSAnalysis` then
sees `{lock1}` vs `{lock2}` and concludes "different locks",
declares the accesses unprotected, and the race goes through.

---

## 4. Root cause #3: HBGraph missing edges (74 FPs)

Two distinct sub-patterns:

### 4a. File-lifetime ordering (CVE-2017-15265, 4+ FPs)

`file_operations.release` is the **last-file-reference** callback
in VFS. It is ordered after every `read`/`write`/`ioctl` on the
same file because the latter all hold a file ref. The HBGraph
currently treats them as concurrent peers.

### 4b. Custom kernel synchronization (CVE-2016-9806, 4+ FPs)

`netlink_lock_table()` / `netlink_unlock_table()` and
`netlink_table_grab()` / `netlink_table_ungrab()` are a custom
reader/writer scheme (the latter increments and waits for
`nl_table_users == 0`). HBGraph does not model this.

Similar custom schemes exist for:
- `cpus_read_lock`/`cpus_write_lock`
- `srcu_read_lock`/`synchronize_srcu`
- `percpu_down_read`/`percpu_down_write`
- `seqcount_begin`/`seqcount_retry`

---

## 5. Proposed fixes

### Corrected constraint: no patch leakage, no hard hypothesis cap

Do **not** use patch text as a detector-side filter. That would turn
the detector into a patch-validation oracle and does not represent a
real bug detector.

Also do **not** add a hard hypothesis cap. A quick check on the v22
TP positions shows that a cap would directly reduce recall:

```
cap=1:  7/21 TPs survive
cap=2: 10/21 TPs survive
cap=3: 13/21 TPs survive
cap=4: 16/21 TPs survive
cap=5: 17/21 TPs survive
```

Several real TPs appear late in the report list (`rank 6` and
`rank 9`), so output-budget control is the wrong precision lever.

### Fix #P1: LLM semantic triage during surface analysis

The main precision fix should happen when the LLM is reading the
surface and deciding which objects deserve hypotheses, not only at
the final verifier stage.

However, this must be **triage**, not another static verifier. The
division of labor should be:

| Layer | Role | Output |
|---|---|---|
| `VulnerabilitySurfaceGenerator` | high-recall candidate extraction and cheap signals | risk flags, access sites, locks, lifecycle hints |
| **LLM object triage** | semantic prioritization: is there a plausible vulnerability mechanism? | `REPORT_CANDIDATE` / `SKIP_WITH_REASON` |
| `HypothesisVerifier` | mechanically checkable constraints | same-location, conflict, same-lock, HB |
| optional LLM reachability pass | second opinion on cases static cannot decide | downgrade / keep with explanation |

This avoids duplicating the static verifier. The LLM should not try
to re-prove `same_location` or `same_lock` as a hard predicate. Its
job is to decide whether a surface object has a coherent bug story:
UAF, publish-before-init, stale pointer, list corruption, missing
atomic annotation, refcount/lifetime bug, etc.

Concretely, update the Fix #8.1 mandatory enumeration rule from:

> must call `get_object_details` on every high-risk top-25 object

to:

> must call `get_object_details` on every high-risk top-25 object
> and must produce an object-level decision:
> `REPORT_CANDIDATE` only if a concrete bug mechanism and consequence
> can be explained; otherwise `SKIP_WITH_REASON`.

Do not require the LLM to propose a hypothesis for every object it
inspects.

The LLM triage prompt should ask for these fields:

```
object:
mechanism: UAF | missing_lock | list_mutation | scalar_torn |
           publish_before_init | stale_pointer | none
positive_evidence:
negative_evidence:
why_not_benign_cleanup:
why_not_same_lifecycle_ordered:
why_not_atomic_or_rcu_protected:
consequence:
decision: REPORT_CANDIDATE | SKIP_WITH_REASON
```

### Why P1 overlaps with static verification but is still useful

There is some conceptual overlap with `HypothesisVerifier`, but the
scope is different:

- Static verification answers: "does this exact pair satisfy a
  formal predicate?"
- LLM triage answers: "should we even spend hypotheses on this
  object, given kernel semantics and likely benign explanations?"

Examples:

- If both sides are visibly under `mutex_lock(&fanout_mutex)`, static
  should eventually filter it with `same_lock`. But if LSAnalysis
  cannot unify the lock, the LLM can still mark the object as low
  confidence before hypothesis generation.
- If the pair is `file_operations.read` vs `file_operations.release`,
  HBGraph should eventually know VFS file-ref ordering. But until
  that is implemented, the LLM can treat release-path races as
  lifecycle-ordered unless there is evidence of use after final put.
- If a field has cross-thread RW but no dereference, no publication
  window, no list corruption, and no missing atomic pattern, the LLM
  should skip it even though static conflict predicates may pass.

This is intentionally a **soft semantic gate**, not a hard proof
system. It reduces the LLM's sibling-field scatter without killing
late-ranked real TPs.

### Deployment decision

Do not implement P1 immediately as a strict front-end filter while
recall is still moving. It could suppress newly recoverable cases if
the prompt is too conservative.

Record it as a staged precision plan:

1. **Now**: keep high-recall generation; log object-level triage
   decisions only (`REPORT_CANDIDATE` / `SKIP_WITH_REASON`) without
   dropping reports.
2. **After recall stabilizes**: compare triage decisions against
   TP/FP labels to measure whether it would have removed FPs without
   losing TPs.
3. **Then**: enable triage as a real pre-hypothesis filter for only
   high-confidence benign classes (same textual lock, release-only
   lifecycle, fully atomic/RCU access pair).
4. **Later**: add an optional LLM second reachability pass after
   static verification as a fallback for cases where static analysis
   is inconclusive.

**Expected FP reduction after activation**: potentially 150-250 of
the 337 `hypothesis_too_aggressive` cases, but this estimate must be
validated in logging-only mode first.

**Recall risk**: unknown until logging-only evaluation. Avoid enabling
hard filtering until we can show the 21 v22 TPs and the new v23 TPs
would survive.

### Fix #P2: LSAnalysis same-lock unification

**Two sub-fixes**:

(a) **Value-name-based unification**: when the LLM/Phasar can't
unify two lock pointers but they textually point to the same C
expression (`&fanout_mutex`, `&isotp_notifier_lock`), unify them
based on the **resolved-name string** rather than the abstract
location. Implement by augmenting each lock site with a "lock
expression name" (e.g. `&struct.packet_sock.fanout_mutex` if
field-derived, or the global symbol name if global) and unifying
on name equality.

(b) **Function-parameter lock pass-through**: when a lock is passed
into a function as a parameter, the call site's lock and the inner
use should unify. Today they're distinct Phasar abstract locations.
Track lock identity by C-level expression rather than SSA value.

**Expected FP reduction**: 50-70 of the 79 `lockset_wrong` cases.

**Recall risk**: same-lock unification REDUCES detected races. Some
v22 TPs might be on accesses we currently see as "different locks"
but are actually the same lock — those would become FN. Looking
through the 21 TPs, none of them depended on accidental
non-unification, so risk is low. But this needs validation.

**FP risk**: too-aggressive unification could mask real "different
lock instance" cases (e.g. per-CPU locks where the lock address
genuinely differs between threads). Mitigation: keep the
**per-instance** disambiguation when the lock is acquired with
`&per_cpu(...)`, `this_cpu_ptr(...)`, or via a callsite-specific
field on a different object.

### Fix #P3: HBGraph kernel-specific edges

(a) **File-lifetime edges**: for any `file_operations` struct, add
HB edges from EVERY non-release callback to the release callback on
the same struct. This is unconditional kernel semantics.

(b) **Custom sync primitive table**: hand-maintained list of
kernel-specific reader/writer schemes:
- `netlink_lock_table`/`netlink_table_grab`
- `cpus_read_lock`/`cpus_write_lock`
- `srcu_read_lock`/`synchronize_srcu`
- `percpu_down_read`/`percpu_down_write`
- `seqcount_begin`/`seqcount_retry`

Each entry tells HBGraph "calls to L synchronize with calls to W
in the way RCU/seqlock semantics dictate".

**Expected FP reduction**: 50-65 of the 74 `hbg_missing_edge`
cases.

**Recall risk**: low — adding HB edges only EXCLUDES races, and
none of the v22 TPs are races between callbacks of the same
`file_operations` (they're all in single-callback contexts).

**FP risk**: zero — the added edges encode correct kernel semantics.

### Fix #P4: Verifier alias-precision

The 28 `constraint_misverified` cases include:
- `old->user_ns` vs `new->user_ns` treated as same pointer
- `smp_store_release(&x)` vs `smp_load_acquire(&x)` treated as
  plain race (these are intentional, ordered accesses)

(a) Mark `smp_store_release` / `smp_load_acquire` / `READ_ONCE` /
`WRITE_ONCE` / `rcu_assign_pointer` / `rcu_dereference` as
**atomic-published** access classes. A race between two such
accesses on the same field is *not* a bug — it's the canonical
pattern.

(b) For `old->X` vs `new->X` where `new = prepare_creds(old)` (or
similar copy-on-write helper), the verifier should treat them as
**different memory cells until commit**. Add a small list of
"copy-on-write parent helpers" (`prepare_creds`, `copy_namespaces`,
`netdev_priv` returning a fresh slab, …).

**Expected FP reduction**: 18-22 of the 28.

**Recall risk**: zero on the 21 TPs (none are smp_*_release pairs).

### Fix #P5: Optional LLM reachability second pass

The verification LLM in Phase B currently approves 18 hypotheses
that should have been filtered. Do not use patch anchors. Instead,
make it a second semantic reachability pass for cases static
verification cannot decide confidently.

Inputs should include:
- the exact hypothesis pair
- static verifier results and failed/unknown predicates
- nearby source snippets
- lock/lifecycle/atomic/RCU hints from `get_object_details`

The pass should answer:
- can these two accesses reach the same runtime object?
- can they overlap in time under kernel lifecycle rules?
- is there a concrete vulnerability consequence?
- is there a benign synchronization/lifetime explanation?

For now this should be logging-only or downgrade-only, not a hard
drop, until recall impact is measured.

---

## 6. Net precision projection

| Fix | FPs cut | TPs preserved |
|---|---|---|
| **P1** (LLM semantic triage, after logging validation) | 150-250 | unknown until measured |
| **P2** (lockset unification) | 50-70 | 21 |
| **P3** (HBG kernel edges) | 50-65 | 21 |
| **P4** (verifier atomic + COW alias) | 18-22 | 21 |
| **P5** (LLM reachability second pass) | 12-16 | unknown until measured |
| **Total** | **280-423** | requires staged validation |

**Post-sprint v24 projection if P1/P5 validate safely**:
- TPs (with recall fixes): ~30-35
- FPs (after all above): ~135-235
- **Precision: 11-20%** (4-5x improvement over 3.57%)
- Reports/CVE: drops from 6 to ~2-3 by semantic filtering, not by cap

**Combined with recall plan** (v24 = ~30 HIT, ~50 with recall
sprint fully landed):
- Reach ~30% precision realistically; ~40-50% needs another
  round of static-analysis precision work.

---

## 7. Sequencing

**Week 1**:
1. Add P1 in logging-only mode: object-level triage decisions are
   emitted to logs, but no report is dropped.
2. P3 file-lifetime edges (40 lines in HBGraph; encode `release` HB
   to other fops).

**Week 2**:
3. P3 custom sync primitive table (incremental, add 5-7 entries).
4. P4 atomic-class recognition (50 lines in HypothesisVerifier).

**Week 3 (after recall sprint stable)**:
5. P2 lockset unification (the hardest; needs LSAnalysis rework).
6. Evaluate P1 logs against TP/FP labels; only then decide whether
   to enable high-confidence filters.
7. Add P5 LLM reachability second pass in logging-only mode.

---

## 8. Risk of recall regression

Aggregated across P1..P5, the risk to existing 21 TPs:
- P1: unknown until logging-only evaluation; do not hard-filter yet
- P2: low — possible some TPs depend on accidentally distinct
  lock IDs; needs spot check on the 3-4 TPs whose evidence
  mentions lock state
- P3: 0 — adding HB edges can only remove races; none of the 21
  TPs are between fops siblings or netlink_lock_table/etc.
- P4: 0 — smp_*_release pairs aren't among 21 TPs
- P5: unknown until logging-only evaluation; do not hard-drop yet

**Net recall risk**: acceptable only if P1/P5 remain logging-only
until measured. Static filters P2-P4 can proceed more safely because
they encode concrete kernel semantics rather than LLM judgment.
