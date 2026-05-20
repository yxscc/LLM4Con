# B1 (shared_object_missed) fix strategy — v23+ next sprint

Derived from clustering all 32 `shared_object_missed` cases in
`evaluation_report_agent_v22.json`. Pre-sprint analysis written
2026-05-19 so we can pick the highest-ROI fix before the v23 full
rerun lands.

(v23 fixes already shipped — Fix #1..#8.1 — primarily address
`wrong_function_focus` (32 cases) and one `shared_object_missed`
case (CVE-2024-27404). The 32 B1 cases below are the remaining
addressable target.)

---

## 1. Clustering by technical root cause

### Cluster A — Nested / embedded sub-struct field extraction  (12 cases)

Most common shape: SharedFieldKey treats `parent->nested.field` (two
or three levels of GEP) as opaque, emits no canonical key, OR emits
only the **outer** offset and drops the inner. The patched field
ends up either missing or merged into a sibling.

| CVE | Patched field | Diagnostic from v22 evidence |
|---|---|---|
| **CVE-2022-49589** | `struct net->ipv4.sysctl_igmp_qrv` | "no candidate object corresponded to the patched sysctl field" — multi-level GEP `net.ipv4` not flattened |
| **CVE-2022-49634** | `ctl_table->data` *pointee* scalar | "modeled as `struct.ctl_table+8` but not the pointee" — indirect pointer not followed |
| **SYZBOT-01affb1491750534** | `pool_workqueue->stats[PWQ_STAT_COMPLETED]` | "indexed array field inside struct" — `gep %pwq, 0, fld, i64 N` not canonicalized |
| **CVE-2025-38037** | `vxlan_fdb->used` / `vxlan_fdb->updated` | "closest surfaced was vxlan_fdb+64, not per-field" — fields collapsed to outer offset |
| **SYZBOT-123b88b9ddea8e98** | `sock->sk_backlog.len` | "no identifiable sk_backlog.len entry" — embedded struct `sk_backlog` (struct member) not unfolded |
| **SYZBOT-1b830cb1f67689d4** | `sock->sk_receive_queue` (sk_buff_head) | "embedded queue/list heads" — same `struct sk_buff_head` embedded issue as CVE-2025-38250 |
| **SYZBOT-1c486d0b62032c82** | `current->fs->in_exec` | "per-task nested fields" — `task_struct.fs` is ptr, then `fs_struct.in_exec` |
| **CVE-2016-9806** | `netlink_sock.cb.module` / `cb.start` | "ranked 30, beyond sampling window" — *both* B1 (cb nested) + B3 (rank too low) |
| **CVE-2024-26861** | `noise_replay_counter` writes joined with `noise_keypair` reads | "writes through pointer helper not joined with reads through containing field" |
| **CVE-2025-21732** | `ib_umem_odp->private` reached via `to_ib_umem_odp(mr->umem)` | "alias through cast helper" — same as cluster B but for nested ptr |
| **SYZBOT-62955e4f963d38ab** | `tcp_sock->snd_nxt` via `tcp_sk(sk)` cast | "cast from sock to tcp_sock not aliased back" |
| **SYZBOT-5cce5938c6c2c518** | `inet_sk(sk)->daddr/sport/...` | "inet_sk() cast not preserved" |

**Shared root cause**: Lace's `SharedFieldKey` builds the key from a
*single* GEP instruction's offset. Real kernel code chains:
```c
sk = (struct sock *) ssk;       // implicit cast
tp = tcp_sk(sk);                // wraps cast
tp->snd_nxt = …;                // GEP at struct.tcp_sock offset N
```
Lace sees the GEP at `struct.tcp_sock+N` for the WRITE side, but the
READ side might be `inet_sk(sk)->...` which goes through a DIFFERENT
type cast and ends up at `struct.inet_sock+M` — even if M+inet_offset
== N, the keys mismatch and the two accesses don't aggregate.

### Cluster B — Helper-mediated publication / setter APIs  (6 cases)

The patched field is written ONLY through a setter helper that
abstracts the GEP+store. Surface generator doesn't inline.

| CVE | Helper | Field |
|---|---|---|
| **CVE-2024-35977** | `serdev_device_set_client_ops()` | `serdev_device->ops` |
| **CVE-2024-35986** | `power_supply_register()` / `psy_get_*` | `tusb1210->charger` (retained ref) |
| **CVE-2024-46704** | `INIT_WORK`/`set_work_data` macros | `work_struct->data` (bitmasked pointer) |
| **CVE-2024-53124** | `skb_clone_and_charge_r` → `sk_rmem_schedule` | `sock->sk_forward_alloc` |
| **CVE-2025-38078** | `snd_pcm_hw_free` → `do_hw_free` | `snd_pcm_runtime->dma_area` |
| **CVE-2025-38242** | folio swap-cache helpers | `folio->swap.val` etc. |

**Shared root cause**: surface generator sees only `call @helper(...)`
on these paths — no GEP, no Store to the patched field, so the
field never enters the surface as a Write.

### Cluster C — Atomic/refcount fields used by begin/end access APIs  (3 cases)

| CVE | Patched object | Pattern |
|---|---|---|
| **CVE-2024-42234** | folio refcount + `_deferred_list` + `large_rmappable` | begin/end access via `folio_undo_large_rmappable()` |
| **CVE-2024-45000** | `fscache_cookie->n_access` | `fscache_begin_cookie_access()` / `fscache_end_*` pair |
| **CVE-2024-53186** | `ksmbd_conn->r_count_q` waitqueue + `r_count` atomic | begin/end work-count |

**Shared root cause**: atomic_t / refcount_t are typed wrapper
structs; their `.counter` field load/store is the only GEP visible.
Surface generator emits `field:struct.atomic_t.counter` (useless —
millions of identical alias spans) or skips it.

### Cluster D — Cross-TU / build-step gaps  (4-5 cases)

Same root cause as CVE-2024-43830 / CVE-2025-38250 — patched function
or its referenced helpers live in a TU that wasn't linked into the
per-CVE `.ll`. Already documented in `UNRECALLABLE_2026-05-18.md` §7.

| CVE | Missing TU |
|---|---|
| CVE-2024-41081 | `ila_*` helpers + control fields outside ila_lwt |
| CVE-2024-43891 | `ftrace`/`trace_event_file` private-data resolution outside tracing core |
| CVE-2024-47715 | `mt76_dev->phys` indexed table — chip TU not linked |
| CVE-2025-38383 | `proc_create_single_data` + `seq_file->private` — outside fs/proc |
| CVE-2024-46704 | (overlaps Cluster B) — `workqueue.c` cross-TU |

### Cluster E — Below-top-25 ranking (B3, not B1)  (4 cases)

Despite being labelled `shared_object_missed`, in these cases the
field IS in the surface — just ranked below the Phase-2 sampling
window. Same root cause as the v23 Fix #7 / #8 work.

| CVE | Patched field | Rank |
|---|---|---|
| **CVE-2016-9806** | `netlink_sock.cb.module/start` | 30 |
| **CVE-2024-26861** | wireguard noise counter | 17 |
| **SYZBOT-373cf39d336f4370** | `sock->sk_err` | beyond top-25 |
| **SYZBOT-3872b8b1d5ece2a8** | `rcu_node->exp_tasks` | beyond top-35 |

### Cluster F — Direct globals / sysctl  (2 cases)

| CVE | Patched object |
|---|---|
| **CVE-2022-49589** | `net->ipv4.sysctl_*` (also in A) |
| **SYZBOT-565f500a8d3fb9b7** | `global:fib_info_cnt` (direct counter) |

---

## 2. ROI ranking

For each cluster, count `# cases × tractability × confidence-of-fix`.

| Cluster | # cases | Cost | Risk | Expected HITs after fix |
|---|---|---|---|---|
| **A — nested/cast field extraction** | 12 | medium (~2 days) | medium | **6-9** (some overlap with E) |
| **B — helper-mediated setter APIs** | 6 | medium (~2 days) | medium | **3-5** |
| **E — below-top-25 ranking** | 4 | low (~½ day) | low | **2-4** (v23 Fix #7/#8 already partially covered) |
| **C — atomic/refcount wrappers** | 3 | medium-high (~3 days) | medium | **1-3** |
| **F — direct globals/sysctl** | 2 | low | low | **1-2** |
| **D — cross-TU build gap** | 4-5 | high (~1 week, kbuild rework) | low | **3-4** but blocks on build infra |

Cluster A is the top single-target: 12 cases, all share a single
underlying fix (multi-level GEP and cast-helper canonicalization
in `SharedFieldKey`). Even a partial fix (just the nested-GEP path
without cast aliasing) recovers 5-7 cases.

---

## 3. Recommended next sprint — Cluster A primary fix

### Fix #9 (proposed): multi-level GEP + cast-helper canonicalization in SharedFieldKey

**Scope**:

(a) **Multi-level GEP flattening**.
When IR has `getelementptr %T, ptr %base, i64 0, i32 i1, i32 i2, …`
or a *chained* sequence of GEPs reaching a leaf scalar, compute the
canonical key by walking the type chain accumulatively:
```
parent_struct_name + ".f1" + ".f2" + … + "@" + total_byte_offset
```
That gives keys like:
- `field:struct.net.ipv4.sysctl_igmp_qrv@(net_offset+ipv4_offset+q_offset)`
- `field:struct.sock.sk_backlog.len@(backlog_offset+len_offset)`
- `field:struct.task_struct.fs.in_exec@…`

For embedded **arrays inside structs**, also include the index:
- `field:struct.pool_workqueue.stats[PWQ_STAT_COMPLETED]@…` (use the
  constant index N if known)

(b) **Cast-helper alias hint**.
For each common kernel cast wrapper (`tcp_sk`, `inet_sk`, `ipv6_sk`,
`udp_sk`, `to_*`-family), record (parent_struct, sub_struct,
offset_in_parent). When we see a load through the cast wrapper,
emit a SharedFieldKey BOTH for `struct.tcp_sock.f@N` AND for
`struct.sock.f@(N + tcp_offset)` — same memory cell, two type
views. This aggregation lets `inet_sk(sk)->daddr` writes join up
with raw `sk->...` reads.

(c) **Pointee-field unfolding for stable pointers**.
For a struct field of pointer type that is initialised once and read
many times (e.g. `ctl_table->data`, `current->fs`, `mr->umem`), the
load from `field->ptr` reaches the pointee scalar via:
```
%p = load %T*, ptr %field_ptr
%v = load i32, ptr %p   ; underlying memory
```
When the pointer is `addr_taken=false` *and* the surrounding
function does not re-store `*field_ptr`, treat the pointee load as
an access to `*field_ptr`'s "current value's struct", and emit a
SharedFieldKey like `pointee:struct.ctl_table.data->scalar@0`. This
unblocks CVE-2022-49634, CVE-2025-21732, SYZBOT-1c486d0b62032c82.

**Expected HIT recovery**:
- After (a) alone: **CVE-2022-49589, SYZBOT-01affb1491750534,
  CVE-2025-38037, SYZBOT-123b88b9ddea8e98, SYZBOT-1b830cb1f67689d4,
  CVE-2016-9806** — 6 cases.
- After (a)+(b): also **SYZBOT-62955e4f963d38ab,
  SYZBOT-5cce5938c6c2c518, SYZBOT-6a2a295ae3340e8e,
  CVE-2024-26861** — +4 cases.
- After (a)+(b)+(c): also **CVE-2022-49634, CVE-2025-21732,
  SYZBOT-1c486d0b62032c82** — +3 cases.
- Total: **up to 13 HIT recovery from Cluster A**, with
  diminishing returns per sub-step.

Recommended implementation order: (a) → measure → (b) → measure → (c).
After step (a) we will already have meaningful gains and a clear
signal whether the per-CVE evaluator credits multi-level fields the
way we hope.

### Risks

- Multi-level GEP can blow up the SharedFieldKey universe (every
  nested scalar becomes a new object). Mitigation: only emit
  multi-level keys when the leaf scalar has cross-thread RW
  evidence; pure intra-thread reads of nested scalars stay
  collapsed under the parent.
- Cast-helper aliasing can over-aggregate: two unrelated `sk->`
  fields at the same offset in different sock subtypes can
  collide. Mitigation: only emit the alias pair when both views
  are seen with the SAME (function, basic block) load pattern.
- Pointee unfolding misses re-stores. Mitigation: be conservative
  and only emit pointee keys for fields that have exactly ONE
  store across the whole .ll (which catches the `init_once` pattern
  the kernel uses for ctl_table, fs_struct, mr->umem).

---

## 4. Second-priority: Cluster E (B3 ranking) quick wins

Already addressed by v23 Fix #7/#8/#8.1; just need to confirm at
scale that:
- CVE-2016-9806 jumps from rank 30 to top-25 after Fix #7's
  `<=3 → <=5` threads cap relaxation (it has 5+ threads — verify)
- SYZBOT-373cf39d336f4370 (sk_err) gets the `[SCALAR_TORN_ACCESS]`
  flag from Fix #8's plain-RW boost
- CVE-2024-26861 (rank 17) crosses into the top-15 once the LLM is
  forced to enumerate per the Fix #8.1 mandatory-iteration rule

No new code needed; just need the v23 full rerun to land.

---

## 5. Action plan when full rerun results arrive

1. **Diff v23 vs v22 by category**: for each Cluster A/B/C/D/E/F case,
   record (a) is field now in surface? (b) is field now in top-25?
   (c) did LLM propose? (d) verified as HIT?
2. **Re-cluster remaining MISSes** using the same framework. Cluster A
   should shrink visibly because some of its cases overlap with
   `wrong_function_focus` (now fixed at scale).
3. **Pick Cluster A (a)** as Fix #9 first deliverable; commit-test
   on 1-2 of the 6 stage-(a) cases before scaling.
4. **Skip Cluster D** (build-step) until we batch-collect 10+ cases.
5. **Defer Cluster C** (atomic/refcount) until A+B are landed —
   semantic overlap means several may auto-resolve.

---

## 6. Bird's-eye recap

- v22 baseline: **15 / 100 HIT (15.0%)**
- v23 sprint shipped (Fix #1..#8.1) targets `wrong_function_focus`
  (32 cases) and one B1 case (CVE-2024-27404 via Cluster A overlap).
  Expected v23 ceiling: **+8 to +15 HITs** depending on how much of
  the `wrong_function_focus` cluster the prompt-side fixes actually
  unlock at scale.
- Proposed v24 work: Fix #9 (Cluster A nested+cast canonicalization)
  + low-hanging Cluster B/E. Expected ceiling: **+10 to +15 HITs**
  on top of v23.
- Stretch target after both sprints: **~40 / 100 HIT (40%)**, with
  the ~50 unrecallable-set CVEs (Cat A/B/C/D) accounted for as
  "out of scope for current Lace shape".
