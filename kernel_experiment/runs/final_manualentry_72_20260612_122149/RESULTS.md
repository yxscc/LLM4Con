# Final manual-entry evaluation — `20260612_122149`

## Run configuration

| Item | Value |
|------|-------|
| Stamp | `20260612_122149` |
| Cases | 72 (`dataset_entrypoints.json`: 56 CVE + 16 SYZBOT) |
| Mode | manual-entry, static-composition, legacy-workflow |
| Concurrency | `CASE_PARALLELISM=4`, `LACE_CONTRACT_PARALLELISM=4` |
| Timeout | 4800s/case |
| Model | `gpt-5.5-2026-04-24` |

## Detector output summary

| Metric | Value |
|--------|-------|
| FOUND / CLEAN | 58 / 14 |
| Total bugs reported | 380 |
| LLM API requests | 2069 |
| api2004 errors | 0 |
| TIMEOUT / FAIL | 0 |

## Strict HIT vs ground truth (manual review)

| Metric | 72-case | CVE-56 | SYZBOT-16 |
|--------|---------|--------|-----------|
| **HIT** | **24** | **18** | **6** |
| NEAR | 26 | 20 | 6 |
| MISS | 22 | 18 | 10 |
| Strict recall | 33.3% | 32.1% | 37.5% |

Baseline reference (56 CVE, stamp `20260611_173800`): 16 HIT (28.6%).

## HIT cases (24)

### CVE (18)

CVE-2013-1792, CVE-2016-7911, CVE-2017-6346, CVE-2022-48830, CVE-2022-49215,
CVE-2022-49607, CVE-2023-53046, CVE-2024-27019, CVE-2024-27404, CVE-2024-35898,
CVE-2024-36938, CVE-2024-40953, CVE-2024-42234, CVE-2024-56788, CVE-2025-23151,
CVE-2025-38078, CVE-2025-38217, CVE-2025-38383

### SYZBOT (6, first benchmark)

SYZBOT-2e4de7fe846aba66, SYZBOT-3cc3a12efa69aa6f, SYZBOT-417aeb05fd190f3a,
SYZBOT-44cf88a58d91b12b, SYZBOT-4a03518df1e31b53, SYZBOT-5676077ba016d741

## Notable vs prior runs

- **CVE-2024-27404** `remote_id`: HIT (regression fixed vs v3 56-run)
- **CVE-2025-38078** `dma_area`: HIT (restored vs v3 CLEAN)
- **CVE-2024-26974** `reset_data`: still CLEAN / MISS

## Artifacts in this directory

- `summary.out` — orchestrator log (`run_manual_entry.py ALL`)
- `hit_review.json` — per-case FOUND/CLEAN + HIT/NEAR/MISS
- `llm_dump_manifest.json` — canonical `LLM_dump` path per case
- `per_case_logs/` — copy of `detection_manualentry_20260612_122149.log`

Per-case logs also remain at `kernel_experiment/<CASE>/detection_manualentry_20260612_122149.log`.
