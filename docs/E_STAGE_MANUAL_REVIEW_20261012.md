# E-only development experiment: manual trajectory review

This is a manual review of source, historical fixes, submitted contracts,
tool trajectories and final E reports. Runtime diagnostics locate evidence;
they do not assign semantic correctness or GT recall. Fixes/GT were read only
after detection and were not supplied to the Agent. No final Agent review ran.

## Frozen run

- Implementation: `d90a229c041004da98c7d01e3e35f6bb06336fbd`.
- Model: `gpt-5.5-2026-04-24`; official SDK harness, Chat Completions.
- Source verified byte-for-byte at pinned Linux revisions; existing IR build
  provenance/configuration remains unverified.
- Seven development cases; four selected because legacy reports manually
  matched their GT. This is not an unbiased recall benchmark.
- Cap: 48 model turns per extraction. Four runs exhausted it. Nouveau and CAN
  completed with 46 and 47 model requests. A 48-turn ceiling is not justified
  as a general analysis budget by these observations.
- Remote raw output: `/mlx_devbox/users/mayunlong.39/playground/lace-e-stage-real-20261012`.
- Local raw mirror: `analysis_outputs/lace-e-diagnostics-20261012/real` in the
  containing research workspace. Original ledgers remain unchanged.

## Findings by case

| Case | Manual GT comparison | Main failing point in this run |
| --- | --- | --- |
| CVE-2024-26984, Nouveau | Native E `req:1` identifies first-map publication and observing nonzero map references without required visibility ordering. All nine operands bind. | E produces a corresponding candidate, but its ordinary order rule does not independently demonstrate the weak-memory failure. Other candidates suffer synchronization binding gaps. |
| CVE-2022-48830, CAN | Seven native E reports do not identify the GT length-snapshot allocation/copy race. | `req:4` mentions length/buffer consistency but represents unrelated `rx.idx` updates from FF/CF actors as one atomic sequence. E also accepts a truncated one-step sequence after binding loss. |
| CVE-2024-35986, PHY | Partial ledger does not represent external charger-provider retirement against cached-provider callback use. | Agent describes device references too broadly; preserving a wrapper device reference is not enough to establish provider callback/backing-data lifetime. Extraction also exhausts 48 turns before E. |
| CVE-2016-7911, ioprio | Partial `req:1` correctly links priority read to immediate/asynchronous io_context free. All four source operands bind. Unchanged-ledger offline replay produces an E candidate. | Native run exhausts 48 turns before E. Replay is diagnostic evidence, not a completed native detection. |
| CVE-2024-40953, KVM | Partial ledger omits the GT `last_boosted_vcpu` conflict between invocations of `kvm_vcpu_on_spin`. | Relevant read/store IR nodes exist (23237/23426); extraction focuses on other protocols and reaches the cap before all pattern reviews. No conclusion about eventual uncapped extraction yet. |
| CVE-2024-56788, OA TC6 | Partial `req:1` represents conflicting producer/consumer `waiting_tx_skb` accesses at the actual GT sites. All six operands bind. | Native run reaches the cap after contracts and all six pattern reviews, before validated finalization/E. Its mediated Req is less specific than the GT read-then-clear transaction. |
| CVE-2024-35898, netfilter | Native final report is an object-reset candidate, not the GT flowtable type-registry race. | Agent identifies expression/object/flowtable registries but submits only the expression registry Req, treating other registries as analogous. Relevant flowtable lookup/deletion IR exists. |

### Concrete evidence

**Nouveau:** `req:1` contains mapping writes at 174–176, callback pointer stores
at 250/252, reference publication at 253 and fast-path observations at 225–234.
Queried pinned refcount documentation does not promise the needed visibility
ordering. E's `no_synchronization_chain` candidate agrees with the target
protocol, but a missing sufficient order chain is not a proved memory-model
counterexample.

**CAN:** GT compares allocation using `so->rx.len` at 566 with length-dependent
copy at 570–571 while FF modifies length at 457/465. Submitted `req:4` instead
uses FF/CF `rx.idx` writes and several unrelated interference operands. The
compound-state pattern review mentions length, so checklist completion alone
does not establish correct contract extraction.

**PHY:** pinned `power_supply_get_property()` checks `use_cnt` at 1054 and calls
the provider at 1064. Unregister decrements the count with a warning at 1447
and proceeds to teardown. A persistent device reference can leave the count
positive; it does not establish lifetime of provider-owned callback state.
The partial ledger instead records local PHY/work teardown, its own supply
and lazy cached-reference initialization. Offline replay cannot invent the
missing external-provider Req. No corresponding typed Gua was submitted, so
this is not evidence that a particular Gua record is wrong.

**ioprio:** `req:1` includes the `p->io_context->ioprio` read at 154, frees at
`blk-ioc.c` 119/154, and `put_io_context_active` at 210. The source/trace
establish an unlocked reader against exit-side last-reference release. E
offline replay uses bound read 6273 and free nodes 6478/7144; applicability
premises remain explicit. The original run still has exit 2 and no native E.

**KVM:** IR read 23237 at 2589 and store 23426 at 2623 retain symbolic access
paths for `last_boosted_vcpu`. The submitted seven Req concern memslots, buses,
vCPU publication, VM list, dirty bitmap, worker stack and device ops table.
The GT fix is compiler access atomicity (`READ_ONCE`/`WRITE_ONCE`), which is not
identical to LLVM atomic load/store semantics. No blanket lock requirement or
formal support for that fix should be inferred from this run.

**OA TC6:** `req:1` binds producer read/write (1202–1216) and consumer load/clear
(1006–1008), plus slot observations at 1064–1065 and 1098–1099. Source protocol
records distinguish the control-buffer mutex from the unprotected data slot.
The wake/wait Gua is correctly described as admission/wakeup, not exclusion.
The original run never executes E, so report absence is not an E failure.

**netfilter:** `record_protocol` describes all global registries.
`add_requirement req:3` explicitly states analogous questions apply to object
and flowtable registries while recording only expression-registry operands.
The ownership pattern review repeats that assumption. The GT's
`nft_unregister_flowtable_type` deletion is actual IR call 43329 at 8156;
lookup has load 81077 (`nf_tables_flowtables`) and 81106 (`type.list.next`) at
8303. Thus the omission cannot be attributed to absent graph instructions.

## Generic changes motivated by this review

1. Default to no hard turn cap; explicit caps remain optional execution guards.
   Observe completed natural lengths before selecting a performance budget.
2. On turn exhaustion, run E over accepted partial contracts, preserve the
   nonzero exit/error and incomplete extraction, and skip final Agent review.
3. Reject atomicity candidates when sequence operands are missing, reduced to
   fewer than two steps, or their local order is not established. Preserve an
   unknown result instead of reporting a fabricated transaction.
4. Clarify one-activation transaction operands versus opposing interference,
   and require actual invariant-relevant operations rather than proxy fields.
5. Guide auditing to consider repeated invocations and each identified
   protocol instance; one analogous registry is not a representative substitute.

These changes do not solve macro binding, callback-completion representation,
interprocedural checking or weak-memory semantics. Evaluate their effects
through unchanged-ledger replay and diverse fresh E-only runs, separately.
