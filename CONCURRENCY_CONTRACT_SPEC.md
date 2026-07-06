# Concurrency Contract Specification

This document records the current ThreadContract design used by the
`lace-architecture` branch. It supersedes the older `prec/atomic/count_guarded`
contract vocabulary while keeping the same implementation pipeline:

```text
static evidence packet
  -> per-thread ThreadContract
  -> deterministic requirement/guarantee composition
  -> LLM calibration of residual candidates
```

The old experimental outputs are intentionally left in `experimental_result/`
and `kernel_experiment/`. The pre-redesign implementation is also preserved by
the Git tag:

```text
pre-contract-redesign-20260706
```

## 1. Design Goal

Static analysis provides concrete facts: shared objects, access sites, lock
annotations, possible frees, list mutations, thread contexts, and call chains.
It often misses the operation-level synchronization semantics that make those
facts safe or unsafe.

Lace uses the LLM only to recover this missing semantic layer. The recovered
semantics are stored in ThreadContracts so that the checker can compose them
deterministically.

The key split is:

```text
Requirement:
  what a local statement or region needs from environment threads to be safe.

Guarantee:
  what synchronization contributes to the execution history.
```

## 2. ThreadContract Shape

A ThreadContract belongs to one thread context.

```text
ThreadContract:
  thread_id
  entry_function
  role / summary
  clauses[]
  ordering[]

Clause:
  resource
  object_id / object_ids
  sites
  assume[]       # local requirements
  guarantee[]    # Level-0 synchronization effects or macros over them
  no_order_needed / no_order_reason
```

The C++ implementation keeps the field names `assume` and `guarantee` for
compatibility with the existing pipeline. Their content now follows the
vocabulary below.

## 3. Requirement Vocabulary

Requirements are not bug symptoms. They are anchored obligations of local code.

Allowed `assume[].relation` values:

```text
ORDER(A, B)
  A must occur before B.

CONFLICT_MEDIATED(A, B)
  A and B may conflict; correctness requires ordering, non-overlap, or a valid
  linearized protocol between them.

REGION_ISOLATED(region, hazards)
  no listed environment hazard may enter region while region executes.

STABLE_DURING(region, predicate_or_resource)
  a predicate or resource must remain valid throughout region.

PROGRESS_ENABLED(wait, enabler)
  optional extension: wait must have a matching enabling event or condition.
```

Examples:

```text
STABLE_DURING(pattern_show_use_region, live(trigger_data))
ORDER(init(work), queue_work(work))
REGION_ISOLATED(check_use_region, invalidators)
CONFLICT_MEDIATED(list_traversal, list_del)
```

The requirement side may refer to concrete statements or regions. It does not
need to normalize every local operation into `USE(R)` or `READ(R)`.

## 4. Guarantee Vocabulary

Guarantees are derived from what synchronization can do to executions, not from
API names.

Allowed `guarantee[].relation` values:

```text
ORDER(a, b)
  event a happens before event b.

EXCLUDE(token, region, mode)
  regions protected by the same token and incompatible modes cannot overlap.

LINEARIZE(object, operation)
  operation takes effect at one abstract point in the history of object.

WAIT(wait_event, condition_or_enabler)
  execution cannot pass wait_event until the condition or enabling event holds.
```

The theoretical anchors are standard synchronization semantics:

```text
happens-before / memory model        -> ORDER
mutual exclusion                     -> EXCLUDE
atomic objects / linearizability     -> LINEARIZE
condition synchronization / waiting  -> WAIT
```

## 5. High-Level Macros

Source-level synchronization mechanisms should be described as macros over the
Level-0 atoms.

```text
lock:
  LINEARIZE(lock_state, acquire)
  EXCLUDE(lock, critical_region, exclusive)
  LINEARIZE(lock_state, release)

handoff / completion:
  WAIT(wait_return, signal_observed)
  ORDER(signal_side_effects, wait_return)

RCU grace period:
  LINEARIZE(rcu_domain, reader_enter)
  LINEARIZE(rcu_domain, reader_exit)
  WAIT(grace_period_return, preexisting_readers_empty)
  ORDER(preexisting_reader_exit, grace_period_return)

reference / capability:
  LINEARIZE(ref_object, acquire)
  LINEARIZE(ref_object, release)
  WAIT(retire_or_free, active_holders == 0)

callback close-and-drain:
  LINEARIZE(domain, admission := closed)
  WAIT(api_return, active_callbacks_empty)
  ORDER(callback_exit, api_return)

validation/retry:
  LINEARIZE(witness, version_transition)
  successful-path validation condition
```

A plain state check is evidence for a requirement. It is not a guarantee unless
it is backed by `ORDER`, `EXCLUDE`, `LINEARIZE`, or `WAIT`.

## 6. Requirement Generation

Requirement generation is a constrained proposal mechanism.

Static analysis first prepares a per-thread evidence packet:

```text
shared resources / protocol objects touched by the thread
this thread's concrete access sites
other concurrent contexts touching the same objects
candidate hazards: free, write, publish, invalidation, list/tree mutation
candidate locks and synchronization calls
guards, path conditions, call chains, and source snippets
```

The LLM proposes requirements only within this bounded packet. Each requirement
must have:

```text
subject: concrete statement or region
resource/protocol: resource, predicate, callback domain, or object
obligation: one of the requirement vocabulary entries
hazards: candidate environment actions
evidence: source lines or call-chain provenance
```

High-risk objects are not silently dropped. If the thread reviewed a high-risk
object and found no local safety obligation, it emits a `no_order_needed` clause
with a reason. This makes missed requirements auditable in later experiments.

Open point for experiments: requirement recall is still the most important risk.
We should measure whether missed detections come from missed requirements,
missed guarantee semantics, or static grounding failures.

## 7. Deterministic Composition

The checker composes contracts over shared resources and related thread
contexts. Conceptually:

```text
CHECK(contracts, surface):
  collect anchored requirements
  collect candidate hazards
  lower guarantee macros into ORDER/EXCLUDE/LINEARIZE/WAIT atoms
  materialize checker relations:
    HB, NO_OVERLAP, LIN, WAIT_GRAPH, MAY_CONC, ALIAS

  for each requirement r:
    bind r to matching hazards through resource aliases and surface accesses
    if guarantees discharge r:
      suppress this pair
    else if surface facts still allow a violating interleaving:
      emit a residual candidate for calibration
```

Discharge rules:

```text
ORDER(A, B):
  discharged by HB(A, B)

CONFLICT_MEDIATED(A, B):
  discharged by HB either way, NO_OVERLAP, or a valid LINEARIZE protocol

REGION_ISOLATED(region, hazards):
  discharged when every matching hazard is ordered outside or excluded from region

STABLE_DURING(region, P):
  discharged when every invalidation of P is ordered after region, excluded from
  region, or mediated by a valid ref/RCU/drain/validation protocol

PROGRESS_ENABLED(wait, enabler):
  optional; discharged when the enabler can satisfy the wait and no unresolved
  wait cycle remains
```

The current C++ Phase B is a conservative approximation of this design. It
recognizes the new relation names and still accepts legacy relation names from
older prompts by mapping them into the new vocabulary. Surface-proven common
locks can discharge a pair deterministically. Non-lock hard mechanisms such as
RCU, refcount, barriers, joins, and drains are kept as strong evidence for Phase
C by default, because the current checker does not yet prove that their endpoints
cover the concrete requirement/hazard pair. `LACE_TRUST_HARD_NONLOCK_DISCHARGE=1`
enables the older speed-oriented behavior for ablation runs.

## 8. Running Example

For the LED/sysfs lifetime bug:

```text
Sysfs thread requirement:
  STABLE_DURING(pattern_show_use_region, live(trigger_data))

Deactivation thread hazard:
  retire(trigger_data)

Close-and-drain guarantee:
  LINEARIZE(sysfs_domain, admission := closed)
  WAIT(device_remove_groups_return, active_callbacks_empty)
  ORDER(callback_exit, device_remove_groups_return)
```

Buggy order:

```text
retire(trigger_data)
device_remove_groups()
```

The drain happens after retirement, so the checker cannot derive:

```text
pattern_show_use_region finishes -> retire(trigger_data)
```

The residual interleaving remains:

```text
retire(trigger_data)
enter/use sysfs callback
device_remove_groups() drains later
```

Fixed order:

```text
device_remove_groups()
retire(trigger_data)
```

Now the close-and-drain macro discharges the stability requirement.

## 9. Compatibility Notes

The implementation accepts several legacy outputs and maps them to the current
vocabulary:

```text
prec/order        -> ORDER
atomic            -> REGION_ISOLATED
count_guarded     -> STABLE_DURING
serialize         -> EXCLUDE
counts            -> LINEARIZE
```

This compatibility is only for robustness during transition. New prompts should
emit the current vocabulary.

## 10. Scope

The claim is not that the vocabulary covers every concurrency bug or every
synchronization API. The claim is:

```text
Lace handles safety obligations whose relevant synchronization semantics can be
represented as ordering, exclusion, linearized abstract-state changes, and
waiting until a condition or enabler holds.
```

Weak-memory-only bugs, liveness bugs such as deadlock, and arbitrary functional
semantic bugs are outside the current main scope unless represented by the
relations above.
