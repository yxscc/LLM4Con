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

## Running Lace (sketch)

```bash
export LLM_BASE_URL="https://<your-openai-compatible-endpoint>/v1/chat/completions"
export LLM_API_KEY="..."
export LLM_MODEL="gpt-5.5-2026-04-24"

# Example: analyze one prepared case directory that contains src/ + bitcode
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

Batch / manual-entry orchestration lives under `kernel_experiment/run_manual_entry.py` (expects a case directory layout compatible with the historical experiment tree). Point it at a working directory that includes bitcode you generated locally.

Useful environment flags (see source / experiment scripts):

- `LACE_STATIC_COMPOSE=1` — thread-contract pipeline  
- `LACE_ENTRYPOINTS=fn1,fn2` — restrict to configured thread roots  
- `LACE_ENABLE_SELF_RACE=1` — model reentrant self-race objects  

## Pipeline (paper)

1. **Static surface** — CCPG + shared-object / conflict candidates  
2. **Phase A** — LLM emits per-thread concurrency contracts  
3. **Phase B** — deterministic requirement discharge (locks / hard order)  
4. **Phase C** — LLM calibration / filtering of remaining candidates  

## What is not in this artifact

- LLVM bitcode (`.ll`) and live `LLM_dump/` traces  
- Obsolete trees (`LinConVul/`, `experimental_result/`, …)  
- Later exploratory full runs that are **not** the paper main table  

## Citation

If you use Lace or this dataset, please cite the paper (TODO: add final bibtex upon publication).

## License

See repository license files where present; third-party kernel sources retain their original licenses.
