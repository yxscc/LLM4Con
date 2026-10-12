# LACE: source-led module concurrency agent

This branch reads the complete supplied module source, extracts concurrency
Requirements (Req) and synchronization Guarantees (Gua), applies deterministic
sufficient-discharge rules over the complete input graph, then continues the
**same agent conversation** to review every Req and produce candidate reports.

## Workflow

1. Provide a module source directory and matching LLVM `.ll`/`.bc`. Optionally
   provide a read-only kernel repository plus its exact revision for queries.
2. Construct `module_model.json`: every source method retained by Joern and
   **every defined function and every instruction in the supplied IR**. Retain
   source AST/CFG, exact intraprocedural IR CFG and direct-call navigation edges.
   Functions are analysis units, **not inferred parallel entries**. No TCT,
   shared-object surface, automatic Req generation or entry-limited graph is
   used in this workflow. The queried external repository is not all compiled.
3. The agent receives a complete source-directory index, graph coverage and
   an extensible protocol catalog. All original source files are available in
   a read-only workspace through the SDK's native `exec_command/write_stdin`;
   source bodies are not pasted into the initial prompt. It identifies overlapping actors from source
   evidence and records resource protocols, Req and Gua. Detailed graph facts
   are obtained through paginated tools; they do not restrict source auditing.
4. E checks each Req using scoped Gua and exact IR control-flow relations.
5. The same agent reviews **all** Req, including discharges and graph unknowns,
   tracing synchronization semantics, instances, activations and source intent.
6. Write the contract ledger, evidence, unknowns, model history and final
   `stateful_bugs/reports.json`. Reports are source-supported, not reproduced.

## Handling incomplete graphs

Source and IR are separate representations. A source-coordinate match does not
establish operation equivalence. `get_source_operation(s)` validates exact text
in the input snapshot and returns binding status and candidate counts. When
automatic matching fails, the agent inspects candidate IR instructions using
`get_operations` and records a justified selection with `bind_source_operation`.
The selected instruction must have been inspected and belong to that anchor's
coordinates/function. Req/Gua retain their source IDs; feedback and E use the
same persisted correspondence, marked as an Agent judgment. Selections can be
corrected or withdrawn; snapshots and revision history preserve their provenance.
Ambiguous mappings are not silently selected. If none is suitable, use the validated module source ID
in `a_sources`, `b_sources` or `interference_sources`. The Req is retained, E
returns `requires_semantic_review`, and source review/reporting still proceeds.
No fabricated graph nodes, synthetic execution roots or ignored Req are used.

IR-only function bodies remain in the graph even if Joern misses their source
function. Source-only code remains readable even if missing from the IR due to
configuration, compilation scope, preprocessing or debug mapping. Coverage
metadata states retained source functions, all defined IR functions, instruction
counts and missing debug locations. Unresolved indirect call targets are listed
as graph gaps. This is **input-relative coverage**, not whole-kernel coverage.

## Contract interface

Req forms: `MustPrecede(A,B)`, `MustBeMediated(A,B)`, `MustBeAtomic(sequence)`.
Each Req records a concrete resource, instance/lifetime/activation `scope`,
at least two actors and source-based rationale. Atomicity Req also records the
identified interfering operation set. Unresolved applicability is explicit in
`assumptions`. No static thread IDs or shared-object IDs are required.

Gua forms: `Order`, `Wait`, `Exclude`, `AtomicOp`. They describe synchronization
actually supplied by code. Ordinary thread-local order/API functionality is
recorded as a protocol explanation, **never an Order Gua**. A Gua must use the
same resource/scope label as a Req it supports and retain mechanism evidence,
matching assumptions and conditions. Exclude records acquisition/release
`regions`; individual atomic effects do not make a compound transaction atomic.
Enum and anchor validation checks structural validity only. It does not prove
that the model's synchronization interpretation or concurrency claim is true.

Tools:

- SDK `exec_command/write_stdin`: source listing/search/reads and live raw contract queries in the read-only workspace.
- `get_module_functions`: paginated source/IR function inventory.
- `get_operations`: optional, paginated **IR-only** evidence by source ID or function/file/lines. AST operation queries are rejected.
- `get_source_operation(s)`: validates source path/lines/text and reports an unbound IR candidate count; returns no graph nodes, even for whole-function spans. A unique debug-line candidate is not automatically bound.
- `bind_source_operation`: after IR inspection, recover a failed/ambiguous source correspondence with an explicit node ID and reason. `node_id=-1` withdraws a previous selection. No IR candidate stays unknown; selection does not change compiler facts or supply a synchronization Gua.
- `get_module_calls`: paginated compact API/function/path/line index; read source for arguments. No AST records are returned.
- `repository_search/read`: pinned dependency lookup when configured.
- `record_protocol`, `record_pattern_review`, `record_gap`: source audit record.
- `add_requirement`, `add_guarantee`, `get_obligations`: contract ledger.
- `review_obligations`, `finalize_contract/review`: validated workflow accounting.

## E: implemented rules and their boundaries

The checker is pure JSON code, testable independently of Joern/LLVM/model calls.
Failed prefix/completion premises may include actual CFG bypass node paths;
these are navigation evidence with unverified feasibility, not executable
concurrent counterexamples. Intraprocedural relations are cached per model.
Only an exact, complete IR CFG supports its control-flow premises. Direct call
edges are navigation; arbitrary interprocedural must-order reasoning is not
implemented yet. No legacy reachability-based HB or inferred alias/locksets
are used for discharge.

- **MustPrecede:** source-operation completion must pass the Gua predecessor
  before exiting; the Gua continuation must dominate the target operation. At
  least one scoped `Order`/`Wait` handoff is mandatory. Chained handoffs compose
  with local prefix relations. Bypass cycles and ambiguous iteration occurrence
  order are retained conservatively. Local order alone never synchronizes two
  activations, even if they share one function or one physical operation node.
- **MustBeMediated:** sufficient synchronization order in either direction,
  continuous exclusive protection by one scoped token, or a declared compatible
  protocol over individually verified IR atomic operations.
- **MustBeAtomic:** preserve the intended local sequence order and discharge
  each explicitly identified interfering operation by whole-sequence ordering
  or one continuously protected region with compatible contender exclusion.
  Separate protected steps and individual atomics are insufficient. No identified
  interference domain means unknown rather than automatic satisfaction.
- **Exclude region checking:** verify prefix/completion closure around every
  protected operation; an opaque interior call requires effect summaries and
  is not automatically credited. Matching token/instance/acquisition semantics
  remain Gua premises to review, not facts independently proved by a CFG.

Results: `discharged`, `undischarged`, `requires_semantic_review`. Discharges
are **relative to the graph and declared Gua semantics/scope**; `proof=false`
means no program-safety proof is asserted. Results expose used Gua, local facts,
unresolved combinations and missing premises. Failure of a sufficient rule is
not a feasible interleaving witness. This version does **not** claim to construct
counterexample execution paths. Final review is mandatory even for discharges.

## Extensible protocol catalog

Default: `config/protocol_catalog.json`. Override with `LACE_PROTOCOL_CATALOG`
or `harness/run_module.py --catalog FILE`. JSON has a version, unique pattern
IDs, questions and provenance. Add patterns without changing graph/checker
algorithms. The initial questions concern retirement, asynchronous revocation,
publication, compound state, ownership handoff and protection context. They are
an operational synthesis informed by empirical studies, not an exhaustive
published taxonomy. Checklist accounting does not prove Req extraction recall.
The exact catalog and prompt are retained in each ledger/run manifest.

## Build and run

Requires LLVM/Phasar, Joern, Graphviz and the Python dependencies in
`requirements-harness.txt`. Use `bash build.sh`; it rebuilds cleanly and runs
CTest. Python calls/tools use the official OpenAI SDK and Agents SDK; C++ owns
static model construction and contract validation. Existing transport keeps
cross-phase history; this refactor does not replace the SDK harness.

The source workspace uses the SDK's `ExecCommandTool/WriteStdinTool`, including
its PTY handling and output formatting. Linux execution requires bubblewrap
(`bwrap` in PATH or `LACE_SOURCE_BWRAP`). Only supplied source files and the live,
read-only `contract_state.json` ledger are exposed. Native tool observations are
saved in `sdk_source_tools.jsonl`, including when extraction fails. External
Linux dependencies remain pinned repository queries; AST and full IR stay in
the static backend.

Native context management reuses the SDK's `Compaction` capability and Runner
input hook. `LACE_CONTEXT_MANAGEMENT=auto` enables it on the Responses path;
`native` explicitly requires `LACE_MODEL_API=responses`, and `off` disables it.
`LACE_COMPACT_THRESHOLD` is optional; the default is the SDK policy. Full audit
history is retained, while later model requests use the SDK's opaque compaction
items. Chat Completions does not silently emulate this feature. The existing
experimental gateway rejected the standard Responses compatibility probe, so
actual native compaction is pending a compatible endpoint; the model is unchanged.

The proposed completion/state/relation changes are documented in
[AGENT_RELATION_DESIGN_20261012.md](docs/AGENT_RELATION_DESIGN_20261012.md).
The current E still uses resource/scope label matching; independent Gua relation
construction is a design, not an implemented result.

```sh
Release-build/llm_detector --input-src /path/module --input-bc /path/merged.ll --static-only

# Model credentials and endpoint are provided through the existing environment.
python harness/run_module.py --source /path/module --bitcode /path/merged.ll \
  --output /path/new-run --model MODEL \
  --repository /path/linux --revision FULL_COMMIT_SHA
```

Optional `--autoconf` must be the actual configuration used to compile the IR;
source/config/IR equivalence is not inferred merely from providing a header.
`--static-only` on the runner also permits a graph-only run without credentials.
Entry configuration and `--only-thread-entry` are removed rather than silently
limiting graph scope. The runner defaults to `--max-turns 0` (no turn cap),
letting the validated finish tool terminate extraction. An explicit positive
cap is an execution guard, not an algorithmic coverage boundary. Exhaustion
still runs E on accepted partial contracts, disables final review, preserves
the error/nonzero exit and marks extraction incomplete. No requirement sampling
is performed; partial E reports do not imply complete analysis.

## Validation and interpretation

`tests/requirement_checker_test.cpp` exercises must-order closure, CFG bypass,
wrong wait placement, loops, same-node/different-activation interference, scope
mismatch, conditional Gua, missing/source-only anchors, individual atomics and
whole-region exclusion. `tests/offline_module_pilot.py` runs real source/IR through
the native graph builder and local scripted API, checking the official harness,
pagination, pattern accounting, rejected invalid claims, same-conversation
review and source-only report plumbing. `tests/module_graph_test.cpp` separately verifies that a plain IR-only
function with no recognized entry and no debug locations retains every actual
instruction and its CFG. Synthetic judgments do **not** measure
model recall and must not be counted as discovered bugs.

`coverage.workflow_complete` means the two agent phases accounted for their
records. Unknown judgments, mapping gaps and graph coverage remain separate;
workflow completion never means all program requirements were found or proved.

### E-only stage diagnostics (2026-10-12)

`harness/run_module.py --no-review` runs source-led contract extraction followed by deterministic E and disables Phase C. `harness/run_batch.py --no-review --plan tests/e_stage_batch_plan.json ...` applies the same policy to seven development cases, including four legacy reports manually matched to GT. Their selection is for failure diagnosis, not an unbiased recall estimate.

Source anchors optionally supply `operation_kind`, `symbol` and `function`. Binding combines debug coordinates (including inline chains) with compiler-derived callee/access facts. Failed automatic matches with IR candidates are investigated through IR queries and explicit Agent correspondence selection. Missing IR and unresolved correspondence remain explicit gaps. A persisted selection is revalidated against its source/instruction snapshots, retains the automatic result and selection reason, and applies equally to Req operands, Gua evidence and region boundaries. Binding does not prove alias, object instance, overlapping activations or atomicity. No AST is injected. Byte-identical flat build copies are omitted from the Agent packet only when their canonical copy verifies at the pinned revision; all aliases remain in provenance.

Each E record saves operand bindings, resolved IR operations, every Gua's eligibility/rejection reasons, rules attempted, CFG bypasses, and discharge facts. Requirement applicability assumptions no longer prevent checking independently grounded graph relations. Failed combinations produce `static_contract_candidate` reports with `origin: deterministic_E`; they are candidates, not proved feasible interleavings. A partially resolved requirement cannot be discharged.

`Release-build/contract_replay model.json contracts.json checks.json` reruns binding/E without any model request. Preserve original ledgers; save replay results separately. Failure attribution is a manual reading of the final report, complete tool trajectory, structured contracts, source and historical fix. Assess graph coverage/binding, Gua semantics, Req semantics and E composition separately. Runtime diagnostics help locate evidence; schema acceptance and automated GT-site matches do not establish correctness or recall. GT/fixes are never passed to the detector.
