# Source-led contract refactor — 2026-10-11

## Purpose and decisions

Implement the user's preferred initial broad model: every defined function and
instruction in the supplied LLVM IR, plus every Joern-retained source method.
Do not infer parallel entries to restrict graph construction or generate a
shared-object surface before agent auditing. No dynamic entry expansion is
implemented. External pinned source remains queryable, not all compiled.

Req express concurrency safety; Gua express actual synchronization. Ordinary
local sequencing and API preconditions are protocol facts. Use an extensible
protocol catalog for auditing, not an exhaustive bug taxonomy or a promise of
extraction completeness. Restore unified deterministic Req discharge with
Gua + model rather than replacing it with detached local-fact checklists.

## Implementation map

- `src/CCPG/ModuleGraph.cpp`: complete input-module model, separate source/IR
  origins, all actual IR instructions and CFG, direct-call navigation, coverage.
- `src/llm_main.cpp`: no entry flags/TCT/HB initialization. `--static-only` writes
  the input graph without a model. Removed the old `CCPG::build` entry filter.
- `src/PhasarUtil/PhasarPointerAnalysis.cpp`: load all IR definitions/location
  maps; no entry discovery or eager CHA/alias analysis in this path. Legacy
  alias queries initialize their backend lazily if explicitly invoked.
- `src/LLMUtil/ModuleContractAgent.cpp`: source-first extraction, catalog audit
  positions, no automatic Req/objects/thread IDs, detailed paginated graph tools,
  validated contracts, same-conversation review of ALL requirements and reports.
- `src/LLMUtil/ModuleRepository.cpp`: compact real snapshot anchors and callsite index;
  no inferred equivalence, pinned dependency queries unchanged.
- `src/LLMUtil/RequirementChecker.cpp`: sufficient-discharge rules over exact
  IR CFG and scoped Gua, cached intraprocedural relations and rule evidence.
- `config/protocol_catalog.json`: six initial questions/protocol families plus
  explicit empirical provenance and limitations; configurable/replaceable JSON.
- `harness/run_module.py`: input validation, optional catalog/static-only,
  input-relative graph metadata and exact catalog/hash in the run manifest.

Deleted unused surface generator, automatic module Req builder/tests and old
synthetic entry fallback/discovery helpers. Some legacy CCPG/TCT/HB utility
classes remain compiled in the shared static core, but this workflow does not
initialize/use those analyses. Do not claim every legacy class was rewritten.
The static core is shared with the direct IR construction regression test.

## Missing graph nodes

Source packet covers every supplied C/C++ file. `get_source_operation(s)` checks
that path/span/text actually exists and retains it under a module source ID.
Source registration returns path/lines/text, binding status and an IR candidate
count, never AST/IR candidate records. Even a unique debug-line candidate is not
automatically considered equivalent. Optional `get_operations` queries return
only paginated IR instructions for a registered source ID or narrow coordinates;
AST operation queries are rejected. Source-only and ambiguous operands retain E unknown and enter
final review/reporting. Function IDs are rejected as operation operands.
No placeholder graph operations or execution roots are created for missing code.
IR-only functions remain available even if Joern retains no corresponding method.
The source callsite index is paginated and contains only API/function/path/line.
The full static model is retained on disk/backend, not injected into the Agent.
Source snapshots are passed to E so its source-only gap records reflect actual
snapshot presence; this does not establish executable-node correspondence.

## What E currently establishes

A discharge is conditional on declared Gua semantics, matching instance/lifetime
and activation scope. It is NOT an independently established program-safety
proof. Unresolved Req assumptions prevent automatic discharge.

- MustPrecede: a Gua handoff is mandatory. The source operation must complete
  through the handoff predecessor on all continuations before exits; the
  post-handoff continuation must dominate the target. Compose further scoped
  handoffs with local prefix relations. Reject bypass cycles/ambiguous iteration
  order. Plain local reachability or same physical node does not synchronize
  different activations.
- MustBeMediated: sufficient synchronized order either way, scoped continuous
  exclusive protection, or compatible actual individual IR atomic effects.
- MustBeAtomic: ordered sequence plus its explicitly identified interfering
  operations; each interference is separated by synchronized order or whole-
  sequence exclusion. Individual atomics/separate protected steps do not suffice.
- Exclusion: CFG prefix/completion closure for matching declared boundaries;
  opaque interior calls are unsupported without effect summaries. Actual lock
  semantics/identity and protection domain are still Gua/source-review premises.

Statuses: discharged / undischarged / requires_semantic_review. Failure of a
sufficient rule is NOT proof of a bug. CFG bypass paths are local graph evidence
with unresolved feasibility, not concurrent executable counterexamples. Results
explicitly set `proof=false` and `counterexample_constructed=false`.

Still unsupported: general interprocedural must-order with matched returns,
recursive/iteration-sensitive ordering, opaque-call lock effects, independent
proof of dynamic object identity/admission closure, and weak-memory reasoning.
Retain unknown/undischarged results; do not reintroduce legacy HB reachability
or guessed aliases to turn them into unconditional discharges.

## Validation already observed

Native binary builds. Both CTest regressions (complete module graph and
requirement checker) pass. The empty-source-CPG regression retains both actual
IR definitions and all six instructions without debug information or inferred
parallel entries. Existing official
SDK/repository tests: 12 passed. Three actual dataset source/IR inputs passed
scripted API end-to-end tests (NOT real-model detection experiments):

| Case | IR input | Retained source methods | IR definitions | IR instructions |
| --- | --- | ---: | ---: | ---: |
| CVE-2024-26984 | nv50.ll | 32 | 39 | 2331 |
| CVE-2022-48830 | isotp.ll | 42 | 84 | 6046 |
| CVE-2024-35986 | merged.ll | 63 | 134 | 5784 |

Each scripted test accounts for a seven-pattern extended catalog, rejects an
invalid Req kind and nonexistent operation ID without mutating the ledger,
retains one source-only Req, receives E unknown, continues the same conversation,
and finishes review. One deliberately synthetic source-supported report exercises
report output; it is NOT a discovered bug. All model calls stay on localhost.
Source/frontend caches were reused by the dataset tests. Direct IR construction
is separately exercised by `tests/module_graph_test.cpp` with an empty source
CPG, two IR-only definitions, an external declaration and no debug information.

Full scripted test artifacts are separate under remote
`/tmp/lace-source-led-validation-final-20261011`; the local summary is
`../analysis_outputs/lace-source-led-20261011/summary.json` relative to the repo.
Existing paper/full optimized and previous real three-case experiment archives
were not changed. These initial scripted tests establish no model recall result.

## No-AST observation correction and real pilot (2026-10-11)

Implementation commit `8f66600b24631700da75fe6329c7d8ef82ebe904` removes
automatic AST/IR candidate records from source-anchor replies, exposes only
on-demand IR instruction queries, and returns a paginated compact callsite index.
The complete backend graph is unchanged. Source-only snapshots are passed to E;
debug-line candidate counts never establish operation equivalence automatically.
Three updated scripted tests passed with the default six-pattern catalog. They
exercise a whole-source-file anchor without graph records, explicit AST query
rejection, source-filtered IR pagination, compact callsite pagination, retained
source-only requirements and report output. These remain synthetic tests. Both
CTests and all 12 Python SDK/repository tests passed again.

One real pilot used `gpt-5.5-2026-04-24`, official OpenAI/Agents SDK, the pinned
source revision and no GT inputs, with 48 turns per phase. CVE-2024-26984 completed
extraction and final review: 8 Req, 4 Gua, one final source-supported report.
Post-run manual GT/patch comparison matches the maps/refcount publication and
`memory.ptrs` visibility defect: the fix adds paired `smp_wmb`/`smp_rmb`.
It was not dynamically reproduced. All eight E results are
`requires_semantic_review` due to source operands; the report is the Agent's
source-review conclusion, NOT a static or weak-memory proof.

The preceding completed same-case run at `5d16c69` produced no reports and used
13,489,683 cumulative input tokens, 53 API requests, 705 seconds. The new pilot
used 4,677,155 input tokens, 76 requests, 784 seconds. Input decreased 65.33%, but
runtime did not decrease. This is one run per implementation with differing
prompts/trajectories and turn caps (96 vs 48), not a controlled ablation or a
general recall result. Token totals do not establish provider billing/cache cost.

Local full traces and summary: `../analysis_outputs/lace-no-ast-20261011/`.
Remote real run: `/mlx_devbox/users/mayunlong.39/playground/lace-no-ast-real-20261011/`.
The previous three-case batch was terminated at the user's request and preserved
separately. Next priority is backend source-operation grounding: avoid requiring
the Agent to enumerate/translate IR, preserve ambiguity, and provide usable E
evidence without silently turning debug-location coincidences into proofs.

## Next evaluation

Use several pinned-version cases with the actual model and examine FINAL reports
against GT, keeping labels out of detector inputs. Separate failures into missing
Req, unsupported/misinterpreted Gua, source/IR binding gaps, insufficient E rules
and incorrect final review. Do not count contract quantities, workflow completion
or synthetic reports as recall. Prefer diagnosing missing requirements and general
mechanisms; false-positive assessment remains deferred as the user requested.
