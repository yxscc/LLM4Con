# Lace full-72 evaluation (paper main result)

Snapshot stamp: `20260709_110902`

This directory freezes the **paper-aligned** Lace run on the 72-case kernel concurrency dataset.

## Headline numbers

See `cost_statistics.md` / `cost_statistics.csv` and `run_summary.txt`.

Key figures from the frozen stats:

- Cases: 72
- Recall (TP cases): **36/72 (50.0%)** (see `cost_statistics.md`)
- Model: GPT-5.5 (`gpt-5.5-2026-04-24`)
- Mode: manual-entry + static composition (thread-contract pipeline)

## Contents

| Path | Description |
|------|-------------|
| `run_summary.txt` | Per-case FOUND/CLEAN summary from the detector |
| `reports.txt` | Aggregated report listing |
| `bugs/` | Per-case bug text dumps |
| `cost_statistics.md` / `.csv` | Time / token / cost breakdown |
| `dump_index.txt` | Index into original dump locations |

## Notes

- This is the **canonical** result package for the paper tables.
- Later exploratory runs (e.g. L2-opt) are **not** included here.
- Bitcode and live LLM dumps are not shipped in this artifact; see top-level README for reproduction.
