# Lace

**Lace** is an LLM-enhanced static detector for concurrency vulnerabilities in C/C++ (Linux kernel slices in our evaluation). It combines a concurrent code property graph (CCPG), pointer analysis (Phasar), per-thread concurrency contracts (LLM), deterministic discharge, and LLM calibration.

This repository is the **paper artifact**: source code, the 72-case dataset (sources + labels), and the frozen main evaluation results.

## Repository layout

| Path | Contents |
|------|----------|
| `src/`, `include/` | Lace implementation |
| `dataset/` | 72-case kernel concurrency dataset (sources + ground truth; **no** bitcode) |
| `results/lace_full72_20260709/` | **Paper main result** (frozen) |
| `kernel_experiment/` | Experiment runners / baseline harnesses (optional for reproduction) |
| `scripts/`, `build.sh`, `CMakeLists.txt` | Build & utility scripts |

## Paper main result

Frozen snapshot: [`results/lace_full72_20260709/`](./results/lace_full72_20260709/)

| Metric | Value |
|--------|-------|
| Dataset | 72 kernel concurrency cases |
| Recall (TP cases) | **36/72 (50.0%)** |
| Model | `gpt-5.5-2026-04-24` |

Details: `results/lace_full72_20260709/cost_statistics.md`, `run_summary.txt`, and per-case dumps under `bugs/`.

## Dataset

See [`dataset/README.md`](./dataset/README.md). Each case provides:

- `ground_truth.json` — bug metadata / description  
- `flow_annotation.json` — true interleaving (evaluation)  
- `src/` — vulnerable source slice  

LLVM bitcode is **not** shipped. To re-run Lace you must produce `.ll`/`.bc` for the slice (same files historically used under `kernel_experiment/<CASE>/`).

Thread entry functions used in the paper’s manual-entry setting are recorded in `flow_annotation.json` (`true_interleaving.thread_a/b.entry.function`).

## Build

### Dependencies (high level)

- CMake, Clang/LLVM **16** (tool build), Clang **15** often used for kernel bitcode  
- Joern (`joern-parse` / `joern-export` on `PATH`)  
- Phasar (pointer analysis), Boost, libcurl, OpenSSL, Z3, Graphviz  

Exact machine setup used in our experiments is documented in `setup_env.sh` (ByteDance internal paths — adapt locally). **Do not commit API keys.**

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
python3 kernel_experiment/run_manual_entry.py            # all configured cases
python3 kernel_experiment/run_manual_entry.py CVE-2024-43830
python3 kernel_experiment/score_l2_run.py                # recall / precision over the newest dump per case
```

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

## What is not in this artifact

- LLVM bitcode (`.ll`) and live `LLM_dump/` traces  
- Obsolete trees (`LinConVul/`, `experimental_result/`, …)  
- Later exploratory full runs that are **not** the paper main table  

## Citation

If you use Lace or this dataset, please cite the paper (TODO: add final bibtex upon publication).

## License

See repository license files where present; third-party kernel sources retain their original licenses.
