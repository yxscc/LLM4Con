# Lace

**Lace** is an LLM-enhanced static detector for concurrency vulnerabilities in C/C++ (Linux kernel slices in our evaluation). It combines a concurrent code property graph (CCPG), pointer analysis (Phasar), per-thread concurrency contracts (LLM), deterministic discharge, and LLM calibration.

This repository is the **paper artifact**: source code, the 72-case dataset (sources + labels), and the frozen main evaluation results.

## V2 module-contract prototype (2026-10-11)

This branch starts at `lace-v2` commit `3ff95ba`. It adds an official SDK harness
and an opt-in complete-module workflow; the frozen paper results below do not
evaluate these changes.

**Harness.** OpenAI-compatible calls now use `openai==3.26.0` and conversations
use `openai-agents==0.23.1` (`Agent`, `Runner`, function tools). C++ remains the
static tool service. A local JSON-lines bridge connects it to the Python SDK;
the SDK owns model/tool turns and preserves the complete conversation across
contract generation and review. There is no legacy 16k pruning on this route.
Chat Completions is the default for existing gateways; `LACE_MODEL_API=responses`
selects the SDK Responses adapter. No temperature is forced. Optional
gateway query authentication is passed as SDK `default_query`, not appended to
the API root. Standard version roots and `/chat/completions` URLs are supported;
an arbitrary full gateway endpoint is preserved with an HTTP-client request hook.
`LACE_ENDPOINT_MODE=root|exact` overrides automatic URL classification. Optional
`LACE_REASONING_EFFORT` is forwarded to the configured model. Traces remain local;
SDK remote trace export is disabled. Credentials travel through local IPC, never
through process arguments or temporary request files. Native Gemini and the
explicit `LACE_AGENT_HARNESS=legacy` ablation retain the old implementation.

**Module workflow.** Set `LACE_MODULE_CONTRACTS=1`, or use the runner below:

1. Input the target module source and its LLVM IR; optional pinned repository
   source is available through tools, without injecting the whole repository.
2. Reuse v2's CPG/LLVM/Phasar entry and shared-access extraction.
3. Generate conditional, quantified use-before-free and read/write mediation
   obligations from **all extracted objects and accesses**. Keep missing node
   anchors as coverage gaps. Operand sets represent groups without materializing
   an access-pair list; they are not sampled representatives.
4. Give one SDK conversation the complete supplied C/C++ module and an entry/
   function navigation inventory. It describes protocols across functions,
   supplements semantic requirements, and records node-anchored synchronization
   guarantees with applicability conditions. Static obligations are immutable.
5. Run v2's deterministic checker once over the module. Its first failing witness
   and apparent discharges are **hints**, not proofs: every requirement remains
   in the subsequent semantic review inventory. Conditional guarantees do not
   enter automatic discharge.
6. Continue the same SDK conversation to review complete obligation groups;
   batch groups sharing a justification, including apparent discharges.
7. Persist source-supported candidates, satisfied/not-applicable judgments,
   evidence and explicit unknowns separately in the contract/review ledger.

Requirements retain the paper's concurrency relations: `MustPrecede`,
`MustBeAtomic`, and `MustBeMediated`. Guarantees retain its synchronization
relations: `Order`, `Exclude`, `AtomicOp`, and `Wait`. API legality or general
functionality is evidence for understanding a protocol, not an extra contract
kind. Ordinary local order does not create a synchronization guarantee. Each
claim must identify its operations, resource/domain, applicability and source
basis. Scope that the checker cannot represent remains conditional for review.

The extraction task separates ordinary source facts (`record_protocol`),
concurrency obligations (`add_requirement`) and actual synchronization effects
(`add_guarantee`). It defines each form, requires the mechanism/domain for a
guarantee, and gives two contrasting examples: local construction order and a
completion handoff between producer and consumer.
`unconditional=false` cannot turn local program order into a guarantee. `Wait`
includes source-anchored enabling events/conditions, not only activity termination.
The exact task text is retained as `agent_instructions` in `module_contracts.json`
for review alongside the trajectory. Type/node/citation validation remains
structural; semantic correctness still needs source review and model evaluation.

This path branches before object triage, top-20 thread seeding, object/thread/
session cost budgets and cluster splitting. It does not call the old truncated
Phase-0 wrapper prompt. It never creates one agent session per thread/object/pair.
It does **not** claim completeness of the inherited static front-end, all possible
semantic requirements, or verified program safety. Missing dependencies/anchors,
unreviewed groups and unknowns make `coverage.complete` false. Protocol accounting
over access-containing functions is an audit checklist, not proof that every
developer intention was inferred. The current source loader handles regular
`.c/.h/.cc/.cpp/.cxx/.hh/.hpp` files; symlinks are explicit gaps. The supplied
source directory determines module scope. Oversized contexts fail visibly rather
than silently dropping code; large-repository decomposition remains future work.

Install into a Python 3.10+ environment, build C++ as below, and use your existing
approved endpoint and model configuration:

```bash
python -m pip install -r requirements-harness.txt
export LLM_API_KEY='...'
export LLM_BASE_URL='https://your-approved-endpoint/v1'
export LLM_MODEL='your-model'
# Joern/Java must also be configured as for v2.
python harness/run_module.py --source /path/to/src --bitcode /path/to/merged.ll \
  --output module_runs/example --max-turns 32
```

The output directory must be new. The runner uses its own Python executable for
the SDK worker; direct C++ users set `LACE_PYTHON=/path/to/venv/bin/python`.
`--max-turns` is an explicit per-phase execution budget, not an object cap;
exhaustion returns an error and retains the incomplete ledger. `0` disables the
turn cap. `LACE_LLM_TIMEOUT` controls the SDK request timeout. Entry configuration
is inherited from v2 and recorded as automatic/configured in `run.json`.
Module phases retain SDK `tool_choice=required` across turns and terminate only
when their validated `finalize_*` tool succeeds; a prose claim is not completion.

Read `module_contracts.json` for the complete requirement/guarantee/review ledger,
source manifest, coverage gaps and first checker witnesses. `llm_simplified_trace.log`
records full SDK input items, tools and usage locally. `run.json` records model,
entry mode, paths, execution budget and final coverage. A source-supported report
is a candidate; it has not been reproduced or formally proved.

**Repository context.** To make omitted dependencies available without injecting
or compiling the entire repository, add a read-only Git repository and a full
commit SHA. The source directory must mirror repository-relative paths; every
supplied source file is checked byte-for-byte against that commit before analysis.

```bash
python harness/run_module.py --source /path/to/src --bitcode /path/to/merged.ll \
  --repository /path/to/linux.git --revision FULL_40_CHARACTER_COMMIT_SHA \
  --output module_runs/with-context --max-turns 96
```

The same conversation receives the complete target source, static object/node
inventory and module callsites/arguments. `repository_search` navigates textual
symbol occurrences; `repository_read` returns source at the pinned version with
path, line numbers, content hash and a citable source ID. Paging limits a single
response, not the reachable files or number of dependencies. No checkout, patch,
commit messages, labels or future revisions are exposed through these tools.
Existing bitcode provenance is recorded as **unverified**; matching source bytes
does not prove the bitcode's build configuration or revision.

The Agent directly submits the same three requirements and four guarantees
through `add_requirement` and `add_guarantee`. External source supplies evidence;
there is no API-summary registration step or requirement to summarize every
external call. Optional `repository_evidence_ids` on contracts and judgments
are checked against source excerpts actually read. An API's documented calling
condition alone does not establish a concurrency defect: the Agent must identify
the conflicting operations, same-resource applicability and harmful interleaving.

If static extraction omitted a resource, `add_requirement` accepts `object_ids=[]`
and a concrete `resource` expression, alongside real module operation nodes,
rationale and any repository citations. The requirement still has one of the
three concurrency forms. Its missing static mapping is explicit (`object_id=-1`);
the legacy checker labels it `requires_semantic_review`, and it is reviewed in
the same Phase C inventory. Neither the expression nor its citations prove
aliasing or feasible concurrency. `requirements_outside_static_objects` keeps
this static coverage gap visible even after review. External source receives
inspection only; it is not compiled into module IR. Missing operation anchors
still require a gap instead of an invented node. This is a dependency-access
prototype, not whole-kernel analysis.

Offline validation (no external model requests):

```bash
python -m pytest -q tests/test_sdk_worker.py tests/test_repository_context.py
clang++-16 -std=c++17 -Iinclude tests/module_requirements_test.cpp -o /tmp/module-req-test
/tmp/module-req-test
python tests/offline_module_pilot.py --source /path/to/src --bitcode /path/to/merged.ll \
  --output module_runs/offline-example
```

The scripted local API tests actual SDK function calling, >70k-character context
retention, cross-phase continuation, explicit finish and turn-limit errors.
The C++ regression tests cover 35 objects, 120 uses grouped under one lifetime
obligation, reentrancy, read/read exclusion and visible missing anchors. The
offline end-to-end pilot deliberately marks all semantics unknown; its report
must remain incomplete and must not be interpreted as a detection experiment.
Repository tests cover isolation from later fixes, source/version mismatch,
pagination, rejected forged citations, direct omitted-resource requirements, and
preservation of semantic-review status through the C++/SDK workflow. The
repository-enabled offline pilot also rejects API-precondition requirement kinds
and non-synchronization guarantee kinds without changing the ledger. It uses the
public ILA fixture and a deliberately hypothetical concurrency relation;
pass `--repository` and `--revision` to exercise this integration path.

Official API guidance: [Agents SDK quickstart](https://developers.openai.com/api/docs/guides/agents/quickstart?lang=python)
and [models and providers](https://developers.openai.com/api/docs/guides/agents/models).

**Frozen multi-case pilots.** `tests/module_batch_plan.json` selects three public
development cases with different synchronization mechanisms and reserves two
validation cases. Commit the implementation before running either split:

```bash
python harness/run_batch.py --plan tests/module_batch_plan.json --split development \
  --dataset /path/to/kernel_experiment --repository /path/to/linux.git \
  --output module_runs/diverse-batch-01 --jobs 2 --max-turns 96
```

The outer runner schedules independent `run_module.py` processes; each retains
the existing SDK agent and isolated artifacts. It verifies all source revisions
before making requests, records the implementation and binary hashes, and
summarizes contracts, reviews, tool calls and completed-phase usage in `batch.json`.
It does not load labels or fixes. Inspect complete trajectories before changing
the prompt, and use a new output directory for each frozen batch. Public known
cases support development diagnostics, not claims of unseen-defect detection.

## Repository layout

| Path | Contents |
|------|----------|
| `src/`, `include/` | Lace implementation |
| `dataset/` | 72-case kernel concurrency dataset (sources + ground truth; **no** bitcode) |
| `results/lace_full72_20260709/` | **Paper main result** (frozen) |
| `kernel_experiment/` | Experiment runners, scoring scripts, and baseline harnesses |
| `scripts/`, `build.sh`, `CMakeLists.txt` | Build & utility scripts |

## Paper main result

Frozen snapshot: [`results/lace_full72_20260709/`](./results/lace_full72_20260709/)

| Metric | Value |
|--------|-------|
| Dataset | 72 kernel concurrency cases |
| Recall (TP cases) | **36/72 (50.0%)** |
| Model | `gpt-5.5-2026-04-24` |

Details: `results/lace_full72_20260709/cost_statistics.md`, `run_summary.txt`, and per-case dumps under `bugs/`.

Real-world unknown-defect issue links are listed in
[`results/real_world_issues.md`](./results/real_world_issues.md).

## Dataset

See [`dataset/README.md`](./dataset/README.md). Each case provides:

- `ground_truth.json` — bug metadata / description  
- `flow_annotation.json` — true interleaving (evaluation)  
- `src/` — vulnerable source slice  

LLVM bitcode is **not** shipped. To re-run Lace, compile each slice into a single `.ll`/`.bc` and place it in the case directory.

Thread entry functions used in the paper’s manual-entry setting are recorded in `flow_annotation.json` (`true_interleaving.thread_a/b.entry.function`).

## Build

### Dependencies

- CMake and Clang/LLVM **16** to build the tool; the kernel bitcode in our evaluation was produced with Clang **15**
- Joern, with `joern-parse` and `joern-export` on `PATH`
- Phasar (pointer analysis), Boost, libcurl, OpenSSL, Z3, Graphviz

`setup_env.example.sh` lists the environment variables the tool reads. Copy it and fill in the paths and LLM endpoint for your machine.

### Compile

```bash
./build.sh          # Release → Release-build/llm_detector
# or
./build.sh debug    # → Debug-build/llm_detector
```

## Running Lace

Lace talks to any OpenAI-compatible **Chat Completions** endpoint.

```bash
export LLM_BASE_URL="https://<your-openai-compatible-endpoint>/v1/chat/completions"
export LLM_API_KEY="..."
export LLM_MODEL="gpt-5.5-2026-04-24"
```

### Single case

Run from a prepared case directory containing `src/` and the bitcode you generated:

```bash
LACE_STATIC_COMPOSE=1 \
LACE_CONTRACT_L2=1 \
LACE_ENTRYPOINTS="led_trigger_set,pattern_show / pattern_store" \
./Release-build/llm_detector \
  --input-bc merged.ll \
  --input-src src \
  --legacy-workflow \
  --abl-contract on \
  --llm-provider openai \
  --llm-url "$LLM_BASE_URL" \
  --llm-key "$LLM_API_KEY" \
  --llm-model "$LLM_MODEL"
```

`LACE_CONTRACT_L2=1` selects the checker described in the paper: contracts are anchored to concrete CCPG node ids and every candidate comes from an undischarged requirement rather than from a second free-form LLM pass.

### Batch

```bash
python3 kernel_experiment/run_manual_entry.py ALL             # every case in the entry table
python3 kernel_experiment/run_manual_entry.py CVE-2024-43830  # one or more named cases
python3 kernel_experiment/score_l2_run.py                     # recall / precision over the newest dump per case
```

`CASE_PARALLELISM` (default 1) controls how many cases run at once and `LACE_CONTRACT_PARALLELISM` (default 2) how many per-thread contracts are generated concurrently within a case.

The runner reads each case's thread roots from `dataset_entrypoints.json` (derived from `flow_annotation.json`) and enables the paper checker by default. It expects a case directory layout compatible with the experiment tree, including locally generated bitcode.

### Environment flags

| Flag | Default | Effect |
|------|---------|--------|
| `LACE_STATIC_COMPOSE=1` | off | Thread-contract pipeline |
| `LACE_CONTRACT_L2=1` | off (on in the batch runner) | Paper checker: node-anchored contracts, requirement-driven discharge, evidence-bounded calibration |
| `LACE_ENTRYPOINTS=fn1,fn2` | unset | Restrict to configured thread roots, disabling automatic entry discovery. Both `,` and `/` separate names, so interchangeable role entries can be grouped as `"a / b / c"` |
| `LACE_SELF_RACE=1` | off | Model reentrant entries as racing against themselves |
| `LACE_L2_SYNC_ALL=1` | off | Ablation: let protocol-level HB edges (RCU, completion, lifecycle flags) discharge requirements instead of requiring an LLM guarantee |
| `LACE_HB_REQUIRE_SYNC=0` | on | Ablation: accept plain control flow as cross-thread ordering |
| `LACE_DUMP_ROOT=<dir>` | `LLM_dump/` | Redirect dumps so ablation configurations do not clobber each other |

## Pipeline

1. **Static surface** — build the CCPG (control flow, calls, data flow, thread contexts, locksets) and derive the shared objects with potentially conflicting cross-thread accesses. Thread entries come from the standard concurrency-creation APIs (`pthread_create`, `kthread_run`, `request_irq`, tasklet/timer/work/RCU callbacks) unioned with any configured roots.

2. **Phase A — contract generation.** For each thread, the LLM reads a bounded object-centered slice and emits a ThreadContract over a closed vocabulary, with every operand naming concrete CCPG node ids:
   - *Requirements* — `MustPrecede`, `MustBeAtomic`, `MustBeMediated`
   - *Guarantees* — `Order`, `Exclude`, `AtomicOp`, `Wait`

3. **Phase B — deterministic discharge.** Contracts are composed across threads without further LLM involvement. `Order`/`Wait` extend the ordering evidence graph `E_ord`; `Exclude`/`AtomicOp` extend the protection map `P`. Each requirement is then discharged against that evidence — `MustPrecede` needs a must-order witness (source-side closure, cross-thread synchronization, target-side prefix), not mere reachability. Requirements that cannot be discharged become violation candidates.

   Only structural synchronization (lock release–acquire, fork, join) counts as an ordering edge here. Protocol-level mechanisms such as RCU grace periods and completions must be recovered by the LLM as `Order`/`Wait` guarantees rather than inferred from an API name table.

4. **Phase C — calibration.** Candidates are batched back to the LLM with their slices and checker evidence. The calibrator may only retain or reject; it keeps a candidate unless some guarantee supported by concrete evidence in the supplied context discharges the requirement. Retained candidates are emitted as reports.

## Citation

If you use Lace or this dataset, please cite the accompanying paper. BibTeX will be added once it appears.

## License

The kernel sources under `dataset/` are derived from Linux and retain their original licenses. See the license files in this repository for the Lace implementation.
