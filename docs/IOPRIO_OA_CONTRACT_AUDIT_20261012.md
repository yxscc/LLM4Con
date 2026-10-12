# Manual Req/Gua audit: ioprio and OA TC6

Reviewed original 48-turn ledgers from commit `d90a229`, their full tool
trajectories, pinned source/dependency excerpts, historical fixes and offline E
replay with checker `2d1b6a7`. Final Agent review was disabled. Historical fixes
were used by the human-side audit only; no new model request used GT or fixes.
No original ledger or active uncapped experiment was modified.

Evaluation clarification: approximate matching of a genuine concurrency defect
counts as recall here; exact root-cause/type identity is not required. Removing
Gua from an already reported requirement should retain the candidate. The
removal probes below are expected monotonic behavior, not evidence of faulty
Gua reasoning. Assess actual bindings, applicable synchronization endpoints and
rule premises instead.

## ioprio: the lifetime Req is specific and supported

Original `req:1` is `MustPrecede`:

- Use: `get_task_ioprio`, `p->io_context->ioprio`, ioprio.c:154,
  compiler node 6273.
- Reclamation alternatives: `put_io_context` blk-ioc.c:154, node 6478;
  `ioc_release_fn` blk-ioc.c:119, node 7144.
- Actor A is a caller querying another task's priority. Actor B is that
  target task exiting; the trajectory queried `kernel/exit.c:792–793`.
- Req scope explicitly states the same io_context lifetime generation. The
  last-reference and immediate/asynchronous-free premises are retained.

Source supports an actual bad execution: A loads the io_context pointer while
evaluating line 154; B clears the target task's slot, drops its final reference
and frees the object; A then dereferences its previously loaded pointer. The
reader takes neither task_lock nor an io_context reference. Caller-side RCU
keeps task lookup safe; this io_context free is not deferred by that RCU region.
The original protocol record explicitly recognizes this distinction.

The historical fix adds task_lock/task_unlock around the slot test and field
read. If the reader acquires the lock first, it finishes before exit can clear
the slot/drop the reference; if exit clears first, the reader sees NULL and
skips the dereference. The free itself is outside task_lock. Thus the real
protection is lock-mediated slot access combined with a branch/lifetime rule,
not simply locking both the read and the free.

Gua audit:

- `gua:1` correctly covers task-locked slot operations in create/get/exit and
  explicitly excludes the unlocked get_task_ioprio reader.
- `gua:2` correctly records refcount atomics and explicitly says the reader
  without a reference does not participate. Atomic counter operations alone
  are not a complete structured lifetime-protection proof.
- `gua:0` describes RCU retirement of io_cq, a different object, and must not
  protect io_context reclamation.
- Other Gua concern association lists or request completion, not this UAF.

Verdict: Req extraction identifies the genuine root relation, not merely a
coincidentally matching file. However, E rejects every Gua for this Req and
returns only missing synchronization chains (U→each free). It does not establish
last-reference reachability, alias identity or a feasible schedule independently.
Removing all Gua produces the identical candidate and failed operand pairs.
This is a supported Req plus an unproved E candidate, not a completed protocol
verification. The current MustPrecede checker only uses Order/Wait chains; the
slot-lock/NULL-branch argument is not represented by its existing Exclude rule.
This is an implementation observation, not a patched-kernel execution result.

## OA TC6: genuine conflict, underspecified requirement

Original `req:1` is `MustBeMediated` over the shared `waiting_tx_skb` slot:

- Producer accesses at 1202–1216 include store 1213, compiler node 6972.
- Consumer load 1007 and clear 1008 are compiler nodes 9092 and 9098.
- Additional operands are slot observations at 1064–1065 and 1098–1099.
- All six operands bind. The Agent queries LAN865x ndo_start_xmit and probe/
  remove source and identifies the separate per-instance SPI kthread.

The actual bad transaction is:

1. Slot is NULL; SPI thread loads NULL into ongoing_tx_skb at 1007.
2. Producer stores a new skb at 1213.
3. SPI thread clears waiting_tx_skb at 1008, losing the newly accepted skb.

The correct invariant is that the consumer's load-and-clear constitutes one
ownership-transfer transaction with respect to the producer's slot assignment.
The historical fix holds the same new tx_skb_lock continuously across 1007–1008
and around producer assignment. Network-side TX serialization cannot exclude
the separate SPI worker.

The submitted Req note mentions exactly-once consumption, but its executable
form checks pairwise mediation. It never encodes the dependent load/clear
sequence and producer interference as MustBeAtomic. Pairwise individual atomics
could satisfy that weaker form while still permitting the three-step loss above.
This follows from E's AtomicOp rule; no READ_ONCE/WRITE_ONCE patched-source run
was performed or claimed.

Gua audit:

- `gua:0` correctly describes kthread_stop before exit-time skb cleanup; it is
  not protection of runtime producer/consumer handoff.
- `gua:1` correctly describes the separate control-buffer mutex, not TX slot
  exclusion.
- `gua:2` explicitly records wake/wait as conditional scheduling/admission,
  not mutual exclusion. OR wait conditions, existing worker execution and lack
  of occurrence matching prevent an unconditional store-to-read order claim.
  Its uncertainty is preserved, so E cannot credit it as an established edge.

Verdict: the Agent identifies the correct actors/resource/sites, but structured
Req precision is insufficient to identify the atomicity root cause. E reports
eight unresolved pairs, without a transaction/interference witness. Removing
all Gua leaves exactly the same eight pairs and candidate.

## Manual checker probes (zero model calls)

Saved separately at remote
`/mlx_devbox/users/mayunlong.39/playground/lace-ioprio-oa-semantic-audit-20261012`,
with local mirror under `analysis_outputs/lace-e-diagnostics-20261012/semantic-audit`.
Derived ledgers are explicitly manual diagnostic inputs, not Agent detections.
Both original compiler graphs are unchanged.

| Probe | E result | What it establishes |
| --- | --- | --- |
| ioprio submitted Req/Gua | Candidate, 2 unresolved use/free pairs | Bound operations reach E. |
| ioprio same Req, all Gua removed | Same candidate/pairs | Current Gua did not establish or refute protection. |
| OA submitted Req/Gua | Candidate, 8 unresolved pairs | Relevant conflicting accesses reach E. |
| OA same Req, all Gua removed | Same candidate/pairs | Report alone does not demonstrate Gua understanding. |
| OA hand-written correct transaction: sequence [load 9092, clear 9098], interference [producer store 6972] | Unknown, no candidate; `sequence_order_not_established` | E cannot establish the valid within-iteration sequence. |

OA's sequence is inside a for-loop. E's global `before(a,b)` rejects any reverse
CFG reachability, including the next iteration's b→a path. That path does not
invalidate a→b order within one iteration. The checker currently conflates
iteration-sensitive sequencing with its stronger global ordering predicate.
It needs a rule for transaction occurrence/iteration before correct atomic
contracts can reliably detect this case. No such implementation change was
made during this audit or injected into the running experiment.

## Follow-up: an applicable Gua and its actual endpoints

OA's separate lifecycle `req:2` has a correct source-supported requirement:
SPI-thread accesses finish before exit frees residual skb pointers. All seven
operands bind. `gua:0` correctly cites kthread_stop, compiler node 7891, and
asserts that it waits for this SPI thread. In the actual source this call precedes
all frees, so its asserted use-before-free relations are true.

But the Gua's executable endpoints are already the uses and the final frees:
A=[9038,9040,9095,10360], B=[7895,7899,7903]. E uses these as virtual Order edges;
it does not mutate the original CCPG. The actual stop node is evidence only.
E checks evidence existence, then composes U→U and F→F around the declared
U→F edge. Its twelve established relations never check stop→free placement.

Four zero-model-call probes isolate that interface issue:

| Gua endpoint | CFG | Established relations | Unresolved relations |
| --- | --- | --- | --- |
| Original final-free endpoints | Actual original CFG | 12 | 0 |
| Original final-free endpoints | Synthetic CFG with stop after frees | 12 | 0 |
| Manually changed continuation endpoint to stop node 7891 | Actual original CFG | 12 | 0 |
| Manually changed continuation endpoint to stop node 7891 | Synthetic CFG with stop after frees | 0 | 12 |

Synthetic CFG probes move only [7890→7891→7892,7903→7904] to
[7890→7892,7903→7891→7904]. They are checker-input mutation tests, not recompiled
kernel variants or new bugs. Original source, graphs and Agent ledgers remain
unchanged. Derived artifacts retain labels distinguishing these probes.

This locates a concrete issue: current lifecycle Gua semantics and bindings are
correct for the actual code, but the endpoint choice embeds final program
placement into the Agent's assertion. Using the synchronization continuation
as the endpoint lets E independently check placement from CCPG. The endpoint
probe demonstrates that the existing prefix rule can then detect wrong order.
The loop-sensitive transaction issue remains separate.
