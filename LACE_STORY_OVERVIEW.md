# Lace Story Overview

This document records the current Lace story after the discussion on
requirements, guarantees, and reviewer-facing theory. It is meant to be a
working guide for later revisions of the introduction, motivation, and
methodology sections.

The core update is:

```text
Requirements are not bug symptoms.
They are anchored execution obligations of local code.

Guarantees are not API categories.
They are synchronization effects over execution histories.

High-level synchronization idioms are macros.
Their checker meaning must lower to a small set of Level-0 atoms.
```

## 1. The Intuitive Problem

Concurrency bugs happen because each thread may look correct by itself, while
the program fails when operations from different threads interleave in an
unexpected way.

Consider a shared object `obj`:

```c
// Thread A
use(obj);

// Thread B
free(obj);
```

This execution is safe:

```text
Thread A: use(obj)
Thread B: free(obj)
```

This execution is unsafe:

```text
Thread B: free(obj)
Thread A: use(obj)
```

The important condition is not simply that the program contains a use and a
free. The important condition is:

```text
The local use of obj is safe only if the environment does not retire obj before
or during that use.
```

This is a requirement. It belongs to the thread that performs the local use.
The thread is effectively saying:

```text
For my local statement to be safe, other threads must respect this execution
obligation.
```

Programs satisfy such obligations through synchronization. Different
synchronization mechanisms look very different in source code:

```text
mutex_lock()
synchronize_rcu()
kref_get()
device_remove_groups()
wait_for_completion()
atomic_cmpxchg()
read_seqcount_retry()
```

However, the checker should not reason about these as API names. It should
reason about what they do to the execution history.

## 2. Why Lace Splits the Work

Static analysis is good at collecting concrete facts:

```text
This statement reads a field.
This call may free an object.
This function is a callback.
This lock is held here.
This API is called after that API.
```

But static analysis often cannot recover the operation-level synchronization
semantics behind those facts:

```text
device_remove_groups() closes a sysfs callback domain and waits for active
callbacks.

deactivate() may retire trigger-specific callback data.

This state check is a lifecycle protocol predicate, not just an ordinary read.

This get/put pair is an ownership discipline, not just an atomic counter update.
```

Directly asking an LLM to inspect every possible thread interleaving is too
expensive and hard to verify. Lace therefore splits the work:

```text
Static analysis:
  collects program evidence and grounds checker queries.

LLM:
  recovers operation-level semantics from bounded evidence.

ThreadContract:
  stores those semantics as anchored obligations, hazards, and guarantees.

Checker:
  composes contracts deterministically and reports residual gaps.
```

The key idea is not "ThreadContract" by itself. The key idea is using the LLM to
recover missing synchronization semantics and encoding them in a form that a
deterministic checker can consume.

## 3. Running Example

The motivating bug is CVE-2024-43830 in the Linux LED subsystem. A sysfs
callback uses `trigger_data`, while a trigger-removal path frees it:

```c
// Thread A: Sysfs callback
ssize_t pattern_show(struct device *dev, char *buf)
{
    struct led *led_cdev = dev_get_drvdata(dev);
    struct data *data = led_cdev->trigger_data;
    return show_patterns(data, buf);
}

// Thread B: Trigger removal
void led_trigger_set(struct led *led_cdev)
{
    if (led_cdev->trigger) {
        led_cdev->trigger->deactivate(led_cdev);
        device_remove_groups(led_cdev->dev, led_cdev->trigger->groups);
    }
}

void pattern_trig_deactivate(struct led *led_cdev)
{
    kfree(led_cdev->trigger_data);
}
```

The semantic facts are:

```text
pattern_show() uses trigger_data.
deactivate() may retire trigger_data.
device_remove_groups() closes and drains the sysfs callback domain.
```

The buggy program order is:

```text
retire(trigger_data)
close_and_drain(sysfs_callbacks)
```

The close-and-drain operation is too late. The harmful interleaving is:

```text
Thread B: retire trigger_data
Thread A: enter sysfs callback
Thread A: use trigger_data
Thread B: close and drain sysfs callbacks
```

If the patch moves `device_remove_groups()` before `deactivate()`, the checker
can establish:

```text
sysfs callback use finishes
  -> close_and_drain returns
  -> retire(trigger_data)
```

The requirement is discharged, and the report disappears.

## 4. Overall Workflow

The current Lace workflow remains thread-centric:

```text
1. Build a static concurrency-aware program model.
2. Generate one ThreadContract for each thread context.
3. Compose ThreadContracts across related threads and resources.
4. Lower guarantees into checker-owned atoms.
5. Report requirements whose composed constraints still permit violating
   interleavings.
```

We do not introduce a separate global risk-slice pipeline. The evidence used by
the contract generator is only an input organization, not a new detection unit.

## 5. ThreadContract

A ThreadContract belongs to one thread context. It records what that thread
requires from the environment and what synchronization effects the thread may
provide.

Conceptually:

```text
ThreadContract(T):
  entry context
  clauses by resource/protocol object
  local requirements
  local hazards/effects
  local guarantee candidates
  evidence anchors
```

The key design point is that requirements can remain close to source code. A
requirement does not need to abstract the local statement into a universal event
such as `USE(R)`. It can directly refer to a concrete statement or region.

Example:

```text
ThreadContract(Sysfs callback)

Requirement:
  subject:
    the region in pattern_show() that reads and uses led_cdev->trigger_data

  resource:
    led_cdev->trigger_data

  obligation:
    trigger_data must remain live during this region

  hazards:
    deactivate() and kfree(trigger_data)

  evidence:
    source lines, call chain, callback domain, related environment operations
```

The deactivation thread may contain:

```text
ThreadContract(Trigger removal)

Hazard/effect:
  deactivate() may retire led_cdev->trigger_data

Guarantee candidate:
  device_remove_groups() is a close-and-drain operation for the sysfs callback
  domain
```

The checker composes these two contracts.

## 6. Requirement Vocabulary

Requirement terms should not be bug symptoms such as:

```text
UAF
data race
deadlock
lost wakeup
stale state
```

Symptoms grow without bound. Instead, requirements should describe the execution
obligation that local code relies on.

Existing concurrency-bug studies provide the motivation. Lu et al.'s ASPLOS
2008 study of real-world concurrency bugs found that the vast majority of
non-deadlock bugs in their sample were order violations or atomicity
violations, and that many examined bugs could be exposed by a small number of
cross-thread ordering constraints. This supports a vocabulary based on
bad-interleaving shapes rather than symptom names.

The current core requirement obligations are:

```text
ORDER(A, B)
  A must happen before B.

CONFLICT_MEDIATED(A, B)
  Two conflicting actions must not be ordinary unordered concurrent actions.
  They must be ordered, excluded, or linearized.

REGION_ISOLATED(R, Hazards)
  No hazard in Hazards may enter or disrupt region R.

STABLE_DURING(R, PredicateOrResource)
  A resource, lifetime predicate, or protocol predicate must remain stable while
  region R executes.
```

An optional extension is:

```text
PROGRESS_ENABLED(W, Enabler)
  A wait or blocking operation must have a reachable enabler and must not be
  trapped in an unresolvable wait cycle.
```

If the paper does not evaluate deadlock, lost wakeup, or blocking bugs, then
`PROGRESS_ENABLED` should be described as an extension rather than a core
obligation.

Examples:

```text
Use-after-free:
  STABLE_DURING(use_region, live(obj))

Data race:
  CONFLICT_MEDIATED(read(x), write(x))

Check-then-use:
  REGION_ISOLATED(check_use_region, invalidators)

Publish-before-init:
  ORDER(init(obj), publish(obj))

Callback teardown race:
  STABLE_DURING(callback_region, live(callback_data))
```

The LLM does not freely invent arbitrary requirements. It maps bounded static
evidence into this obligation vocabulary.

## 7. Requirement Generation

The requirement-generation story must avoid two weak extremes:

```text
Weak story 1:
  The LLM freely invents requirements.

Weak story 2:
  Static analysis enumerates every syntactic use/free or read/write pair and
  asks the LLM to classify all of them.
```

The current story is:

```text
Static analysis provides a compact evidence packet for each thread.
The LLM proposes anchored requirements only within this bounded evidence.
The checker mechanically generates violation queries from the requirement type.
```

The evidence packet is not a global surface and not a separate pipeline stage.
It is simply the organized input to the per-thread ContractGenerator.

An evidence packet may include:

- shared resources touched by the current thread;
- other concurrent entry contexts that touch the same resources;
- candidate hazards such as free, unregister, deactivate, close, state
  transition, final put, or publish;
- local guards, state checks, reference operations, and synchronization calls;
- call chains, locksets, possible concurrency, and known ordering facts;
- relevant framework or callback context.

The LLM output is constrained:

```text
Requirement:
  subject: concrete statement or region in the current thread
  resource/protocol: resource, predicate, domain, or object
  obligation: ORDER | CONFLICT_MEDIATED | REGION_ISOLATED |
              STABLE_DURING | optional PROGRESS_ENABLED
  hazards: candidate environment actions or action classes
  evidence: source lines and static facts
```

This avoids a Cartesian product over all accesses while still allowing semantic
requirements such as lifecycle, callback-domain, protocol-state, and ownership
requirements.

## 8. Guarantee Vocabulary: Two Levels

Guarantee terms require a stronger theory than requirement terms. We should not
say:

```text
Synchronization APIs are classified as lock, RCU, refcount, completion, ...
```

That would be an API catalog. It would not explain why the vocabulary is
complete or principled.

The better design is two-level:

```text
Level 0: synchronization atoms
  Primitive effects on execution histories.

Level 1: checker macros
  Engineering-level idioms that lower to Level-0 atoms.
```

The Level-0 atoms are:

```text
ORDER(a, b)
EXCLUDE(token, region, mode)
LINEARIZE(object, operation)
WAIT(wait_event, condition_or_enabler)
```

These four atoms have distinct theoretical sources.

### 8.1 ORDER

`ORDER(a, b)` means that event `a` must happen before event `b`, or that effects
before `a` become visible before or at `b`.

This comes from Lamport's happened-before relation and modern memory models.
C/C++ memory models use relations such as `synchronizes-with` and
`happens-before` to define cross-thread visibility and ordering.

Checker relation:

```text
HB(a, b)
```

### 8.2 EXCLUDE

`EXCLUDE(token, region, mode)` means that regions protected by incompatible
modes of the same token cannot overlap.

This is mutual exclusion. It covers mutexes, spinlocks, rwlocks, interrupt
disable regions, preemption disable regions, and single-thread executors.

It is not reducible to `ORDER`. Two critical sections may execute in either
runtime order; the checker only needs to know that they cannot overlap.

Checker relation:

```text
NO_OVERLAP(region1, region2)
```

### 8.3 LINEARIZE

`LINEARIZE(object, operation)` means that an operation on a synchronization or
concurrent object takes effect atomically at one logical point in that object's
abstract history.

This comes from atomic modification order and linearizability. Relaxed atomics
can be linearized on one object without creating a cross-thread happens-before
edge, which is why `LINEARIZE` is distinct from `ORDER`.

Checker relation:

```text
LIN(object, operation)
```

or, equivalently, an atomic transition on a ghost synchronization object.

### 8.4 WAIT

`WAIT(wait_event, condition_or_enabler)` means that a thread continuation is
disabled until a condition becomes true or an enabler event occurs.

This comes from condition synchronization. Condition variables, completions,
semaphores, channels, joins, and drains all have this dimension.

It is not reducible to `ORDER`. The edge `signal -> wait_return` explains the
successful return path, but it does not explain why the wait might never return.

Checker relation:

```text
WAIT_FOR(wait, condition_or_enabler)
ENABLES(enabler, wait)
```

## 9. Derived Guarantee Macros

Most source-level synchronization mechanisms are not Level-0 atoms. They are
macros over the atoms.

### 9.1 Mutex

```text
mutex_lock(m):
  WAIT(lock_acquire, owner(m) == none)
  LINEARIZE(m, owner := current_thread)

critical section:
  EXCLUDE(m, critical_region, exclusive)

mutex_unlock(m):
  LINEARIZE(m, owner := none)
  ORDER(unlock(m), later_lock(m))
```

### 9.2 Handoff

```text
handoff(signal, wait):
  WAIT(wait, signal_or_predicate)
  ORDER(signal_side_effects, wait_return)
```

Examples include completions, condition variables, and producer-consumer
signals.

### 9.3 Capability

Reference counting and similar ownership mechanisms are macros:

```text
capability(resource, acquire, release, retire):
  LINEARIZE(capability_state, acquire)
  LINEARIZE(capability_state, release)
  invariant: retire requires no active holder
```

Examples include refcount, kref, get/put, pin/unpin, and module get/put.

### 9.4 RCU Grace Period

```text
rcu_read_lock():
  LINEARIZE(rcu_domain, enter(reader))

rcu_read_unlock():
  LINEARIZE(rcu_domain, exit(reader))

synchronize_rcu():
  WAIT(grace_period, preexisting_readers_empty)
  ORDER(preexisting_reader_exit, synchronize_rcu_return)
```

This captures the important boundary that a normal RCU grace period waits for
pre-existing readers, not arbitrary future readers.

### 9.5 Callback Close-And-Drain

```text
callback entry:
  LINEARIZE(domain, enter(callback))

callback exit:
  LINEARIZE(domain, exit(callback))

close-and-drain API:
  LINEARIZE(domain, admission := closed)
  WAIT(api_call, active_callbacks_empty)
  ORDER(callback_exit, api_return)
```

This distinction matters because different APIs may be:

```text
close only
drain only
close and drain
```

Only the last one prevents new entries and waits for active callbacks.

### 9.6 Validation/Retry

Seqcount and seqlock readers are best treated as a macro:

```text
VALIDATE(region, witness):
  LINEARIZE(witness, version transitions)
  successful path condition:
    no observed invalidation of the region's snapshot
  failed path:
    discard or retry
```

Readers may overlap writers, so the useful synchronization effect is an
optimistic consistency check rather than region exclusion. The macro is built
from linearized version transitions and a successful-path condition.

### 9.7 Phase Guard

`PHASE_GUARD(predicate, region)` is not a guarantee. It is evidence.

A state check can help the LLM understand a protocol requirement, but a plain
state check does not by itself synchronize with other threads. It can discharge
a requirement only when the predicate is supported by `ORDER`, `EXCLUDE`,
`LINEARIZE`, or `WAIT` effects.

## 10. What Can Be Claimed About Coverage?

We should not claim:

```text
The guarantee vocabulary covers all possible synchronization APIs.
```

Arbitrary libraries can implement arbitrary protocols. A universal claim would
be indefensible.

The defensible claim is relative to a model:

```text
The Level-0 atoms are sufficient for synchronization mechanisms whose
safety-relevant semantics can be modeled as:

1. linearizable transitions over an abstract synchronization object;
2. optional blocking until a predicate over that object becomes true;
3. optional happens-before or visibility edges between program events;
4. optional mutual-exclusion invariants derived from the object state.
```

Proof sketch:

```text
If an API call changes synchronization object state, represent the state change
as LINEARIZE.

If an API call blocks until a state condition or event occurs, represent that as
WAIT.

If an API call establishes visibility or must-precede ordering between program
events, represent that as ORDER.

If the synchronization object invariant prevents incompatible regions from
overlapping, expose that invariant as EXCLUDE.
```

Therefore, every checker-visible effect of such an API can be represented by
the four atoms.

This covers common mechanisms used in our target systems: locks, rwlocks,
atomics, CAS, semaphores, completions, condition variables, joins, barriers,
refcounts, RCU grace periods, callback drains, and seqcount validation. APIs
whose semantics cannot be lowered to these atoms are outside the current
checker model.

## 11. Checker Semantics

The checker receives:

- anchored requirements from ThreadContracts;
- local hazards/effects from other ThreadContracts;
- guarantee macros and their lowered Level-0 atoms;
- static facts such as program order, call relations, locksets, alias facts,
  and possible concurrency.

It maintains relations such as:

```text
HB          // ORDER closure
NO_OVERLAP  // EXCLUDE constraints
LIN         // per-object linearization facts
WAIT_GRAPH  // WAIT and enablement facts
MAY_CONC    // static may-concurrency
ALIAS       // resource equivalence or may-alias
```

Each requirement type has a fixed discharge query:

```text
ORDER(A, B):
  Can HB(A, B) be proved?

CONFLICT_MEDIATED(A, B):
  Can the checker prove HB(A, B), HB(B, A), NO_OVERLAP, or LINEARIZE on the
  conflicting object?

REGION_ISOLATED(R, Hazards):
  Can the checker prove that no hazard can occur inside R?

STABLE_DURING(R, P):
  Can the checker prove that P cannot be invalidated during R?

PROGRESS_ENABLED(W, E):
  Can the checker prove an enabler path and absence of an unresolvable wait
  cycle? This is optional if progress bugs are out of scope.
```

If the requirement is not discharged, the checker asks whether a violating
interleaving is still feasible under static facts and lowered guarantees. If
yes, it reports a residual bug candidate.

## 12. Running Example With Level-0 Atoms

For CVE-2024-43830, the sysfs thread contributes:

```text
Requirement:
  STABLE_DURING(pattern_show_use_region, live(trigger_data))
```

The deactivation thread contributes:

```text
Hazard:
  RETIRE(trigger_data)

Guarantee macro:
  close_and_drain(sysfs_callbacks, device_remove_groups)
```

The macro lowers to:

```text
LINEARIZE(sysfs_domain, admission := closed)
WAIT(device_remove_groups, active_callbacks_empty)
ORDER(callback_exit, device_remove_groups_return)
```

In the buggy version, program order is:

```text
RETIRE(trigger_data)
  -> device_remove_groups()
```

The checker can derive that active callbacks finish before
`device_remove_groups()` returns. But because the retire occurs before the
close-and-drain call, the checker cannot derive:

```text
pattern_show_use_region_end -> RETIRE(trigger_data)
```

The violating interleaving remains feasible:

```text
RETIRE(trigger_data)
  -> pattern_show_use_region
  -> device_remove_groups_return
```

The requirement is not discharged, so Lace reports the bug.

In the fixed version, program order is:

```text
device_remove_groups()
  -> RETIRE(trigger_data)
```

Together with the close-and-drain macro:

```text
callback_exit -> device_remove_groups_return
device_remove_groups_return -> RETIRE(trigger_data)
```

the checker derives that the sysfs use region ends before the retire. The
requirement is discharged.

## 13. Boundary Against the Other Story

The alternate/new entry uses surface-like structures and mechanism knowledge.
This Lace story should stay distinct.

For this story:

- the unit is a per-thread ThreadContract;
- evidence packets are inputs to ContractGenerator, not global bug surfaces;
- there is no mechanism knowledge base;
- there is no CVE-specific pattern specialization;
- historical bug phenomena may guide obligation categories, but they must not
  become bug signatures.

Concise distinction:

```text
Surface-like structures are detection candidates.
Contract evidence packets are materials for writing per-thread contracts.
```

## 14. Claims To Use In The Paper

Do not write:

```text
Our vocabulary covers all concurrency bugs.
Our vocabulary covers all synchronization APIs.
The LLM proposes all necessary requirements.
```

Write:

```text
Lace expresses concurrency-bug hypotheses as anchored execution obligations
over local statements and regions, rather than as symptom labels.

Lace models synchronization mechanisms by their checker-visible effects:
order, exclusion, linearization, and wait.

Higher-level idioms such as RCU grace periods, reference capabilities, callback
drains, handoffs, and validation/retry protocols are derived macros over these
atoms.

Each admissible requirement has a fixed checker discharge rule over the lowered
atoms and static program facts.
```

## 15. Current One-Paragraph Story

Lace detects semantic concurrency defects by combining static evidence with
LLM-recovered synchronization semantics. Static analysis identifies thread
contexts, shared resources, related statements, possible hazards, and known
structural facts. For each thread, an LLM generates a ThreadContract: a
structured, evidence-bound description of local execution obligations,
hazards, and synchronization guarantee candidates. Requirements remain anchored
to concrete statements or regions and use a small obligation vocabulary such as
ordering, conflict mediation, region isolation, and stability. Guarantees are
normalized in two layers: high-level idioms lower to four primitive
synchronization atoms, namely order, exclusion, linearization, and wait. The
checker composes these atoms with static program facts and reports a residual
bug candidate when the composed constraints still permit a violating
interleaving. This lets Lace reason about framework-specific lifecycle and
synchronization protocols without directly asking the LLM to enumerate thread
interleavings.

## 16. Reference Anchors

These are useful anchors for the paper text and bibliography work:

- Lamport, "Time, Clocks and the Ordering of Events in a Distributed System":
  happened-before and partial orders over events.
- C/C++ memory-model materials: happens-before, synchronizes-with, data race,
  atomic modification order, and mutex synchronization.
- Andrews-style concurrent programming taxonomy: mutual exclusion and condition
  synchronization.
- Herlihy and Wing, "Linearizability": concurrent object operations that appear
  to take effect atomically at one point.
- Linux RCU requirements: grace-period guarantee and publish/subscribe
  guarantee.
- Linux kref/refcount documentation: reference ownership and final release.
- Linux seqlock/seqcount documentation: optimistic validation and retry.
- POSIX condition variable documentation: wait, signal, associated mutex, and
  lost-wakeup avoidance.
