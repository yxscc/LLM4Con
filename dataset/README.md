# Lace kernel concurrency dataset (72 cases)

Derived from `kernel_experiment/`. Each case directory contains:

- `ground_truth.json` — CVE/bug metadata and description
- `flow_annotation.json` — true interleaving / thread roles (for evaluation)
- `src/` — vulnerable source slice used by the experiment

**Not included:** LLVM bitcode (`.ll`), detection logs, or LLM dumps.

Cases: **72**

Thread entry functions for reproduction can be read from `flow_annotation.json`
(`true_interleaving.thread_a/b.entry.function`).
