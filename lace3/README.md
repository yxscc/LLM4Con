# lace3

Concurrency-defect detector for Linux kernel IR: static interaction index,
sequence-preserving analysis cliques, direct LLM detection with targeted fact
queries. Design: `../LACE_V3_DESIGN.md`. Deadlocks / lock ordering are not
analyzed.

## Setup

```
python3.11 -m venv .venv-lace3
.venv-lace3/bin/pip install llvmlite==0.50.* openai openai-agents pytest
```

IR must come from clang-16 with `-g` (any `-O` level).

## Run

```
# static part only: entries, items, tasks, evidence packets; no model calls
.venv-lace3/bin/python -m lace3 run --ir path/to/main.ll --src path/to/src --out out/ --offline

# with the model (needs a budget; never a full paid scan by default)
source setup_env.sh          # LLM_API_KEY, LLM_BASE_URL (Azure-style gateway), LLM_MODEL
.venv-lace3/bin/python -m lace3 run --ir path/to/main.ll --src path/to/src --out out/ \
    --max-tasks 5            # or --token-budget 500000
```

`--ir` takes a `.ll` file or a directory of them and can be repeated. `--src`
is the only source the detector reads; keep ground truth, patches and CVE
notes out of it (they are refused by name even inside it). Entries are always
discovered from the IR.

Other limits: `--max-turns` (12), `--max-expansions` (3), `--max-depth` (8),
`--max-steps` (4000 per activation), `--max-participants` (4),
`--max-items-per-task` (24), `--packet-chars` (30000).

Without a key the run falls back to offline and records
`model_calls: not made: ...` in the results.

## Output

```
out/results.json   schema lace3.results/1
out/ledger.json    coverage counts
out/report.md      findings, ledger, open items
out/packets/T*.txt evidence packet per task
out/tasks/T*.json  per task: status, tool calls, expansions, usage, raw submission
```

`results.json`:

- `scope`: analyzed / not analyzed (deadlock).
- `run`: inputs, limits, backend, `model_calls`, token usage, timings, truncated activations.
- `completeness`: `assignment` (items and contexts placed in tasks), `review`
  (task statuses, verdicts, `safe_on_part_of_contexts`, incomplete / pending /
  error / not-run items), `complete`, and `recall` (not computed here).
- `findings`: `id, task, title, kind, items, interleaving, consequence,
  citations, invalid_citations, confidence, model_added, notes`.
- `items`: every ledger item with `kind, keys, summary, contexts_seen,
  contexts, task, status, verdict, contexts_reviewed, reason, citations, notes`.
  Status is one of `assigned` (not run), `pending` (over budget), `reviewed`,
  `incomplete` (no verdict / turn limit), `error`.
- `tasks`, `activations`: clique membership, expansions, notes; per entry its
  provenance, reentrancy, steps, sections, link counts.

## Tests

```
.venv-lace3/bin/python -m pytest -q lace3/tests
```
