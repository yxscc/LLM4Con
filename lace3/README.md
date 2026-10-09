# lace3

Concurrency-defect detector for Linux kernel IR. The new offline path builds a
static interaction index and orchestrates concurrency scenarios into planned
review sessions. LLM detection still uses the legacy pipeline described below.
Design: `../LACE_V3_DESIGN.md`. Deadlocks / lock ordering are not analyzed.

## Setup

```
python3.11 -m venv .venv-lace3
.venv-lace3/bin/pip install llvmlite==0.50.* openai openai-agents pytest
```

IR must come from clang-16 with `-g` (any `-O` level).

## Run

### Factorized candidate index (new, offline)

```
python -m lace3 index --ir path/to/main.ll --src path/to/src --out out/relations
python -m lace3 pairs --index out/relations --relation R0 --limit 20
# Continue with the returned next_cursor, e.g. --cursor 3 7.
```

`index` stores complete left/right member pools and a pairing rule, without
enumerating the Cartesian product. The four relation kinds are conflict,
atomicity, guard, and lifetime. Same-entry concurrent executions are retained
unless the existing front end classifies the entry as non-reentrant. Lock
facts, container paths and source order do not exclude pairs.

This is the first replacement stage: **relations are not agent tasks**.
There is no model call, reviewed/safe result, or conditional proof reuse on
this path. `run` below still uses the legacy item/clique pipeline. The new
index has no 6-context, 8-use, 12-step, 40-closure, or representative-sampling
limit. The existing front end still has `--max-depth 8` / `--max-steps 4000`,
and all its cuts/unknown calls remain explicit in the output. This preserves
the bounded model, not every feasible execution of the entire program.

The directory contains `index.json` (schema, counts, timings and limits),
`relations.jsonl`, `pools.jsonl`, `members.jsonl`, and `boundaries.jsonl`.
`operations.jsonl` stores a function's static operations once;
`bindings.jsonl` retains every activation/step's argument origins, container
path, locks, sections and call frames (interned in `frames.jsonl`). Sharing
an operation does not merge its locked and unlocked callers. The complete
entry provenance, sections and unresolved links are in `activations.jsonl`.
No concrete pairs are written until requested through `pairs`; a page's
`next_cursor` exposes remaining pairs and does not mark them reviewed.

Counts can differ from the legacy counter: duplicate provenance records for
the very same access are collapsed, while previously capped dependencies and
guard regions are retained. The counting units are explicit in `index.json`;
these are neither recall numbers nor an estimate of LLM calls. IR files in a
single invocation must have distinct stems because the current front end
uses the stem in its module identity; ambiguous input is rejected.

Offline acceptance (never pass patches or bug labels):

```
python -m lace3.eval.relations --ir path/to/main.ll --src path/to/src \
  --out out/acceptance.json --compare-legacy --write-index
```

`--compare-legacy` intentionally enumerates old contexts **in the evaluator**,
checking every context against new complete side members and the pairing
predicate. It can be expensive; `index` does not do this. This is a regression
check against the old bounded model, not a correctness/recall oracle.

### Concurrency scenario orchestration (offline prototype)

```
python -m lace3 plan --index out/relations --src path/to/src --out out/scenarios
# Optional sensitivity experiment; this is SOURCE BYTES, not a model token limit:
python -m lace3 plan --index out/relations --out out/scenarios-small \
  --source-byte-budget 200000
```

The planner gives each member an object-origin anchor. Parameter roots remain
qualified by entry and containing member path; loads follow their originating
pointer. Global objects and allocation/return sites have stable identities.
An anchor is an organizational role, never a proof of object identity.
When a known origin coexists with unknown or nullable alternatives, the known
origin determines ownership and **all alternatives remain recorded** in
`ownership.jsonl`. Fully unknown roots stay scoped to their static operation.

For each relation, scenarios partition only its left members. Every view
references the complete original right pool and retains the original ordering
predicate for symmetric relations. Cross-scenario candidates, same-entry
concurrency and distinct lock bindings survive. Structural validation checks
that the partition preserves all implicit combinations without enumerating them.

The scheduler deduplicates complete source files, including resolvable inline
and caller locations. Scenarios sharing source can enter one planned session.
Without a source budget, all connected source footprints can share a session;
this **does not establish that a model can fit or review the whole session**.
The measured bytes exclude future prompts, candidate descriptions, tool history
and output. No LLM is invoked and nothing is marked reviewed or safe.

With an explicit source-byte budget, a large scenario may split at relation
view boundaries. An indivisible view exceeding the budget stays pending rather
than losing members or source. The current file-granularity prototype cannot
split a single large source file. Missing source and upstream frontend gaps are
reported explicitly. Budgets must not be used to compare apparent cost savings
without comparing the pending scope too.

Output: `plan.json` (coverage, budgets, cost units and upstream gaps),
`scenarios.jsonl`, `views.jsonl`, `ownership.jsonl`, `sessions.jsonl`,
`pending.json`, and complete line-numbered `source/Q*.txt` packets. Sessions
are plans, not model requests. `run` remains the legacy detection pipeline.

Reproduce the offline measurements using only IR/source input:

```
python -m lace3.eval.scenarios --ir path/to/main.ll --src path/to/src \
  --out out/scenario-measurement --budget-fractions 0.5,0.25
```

The fractions are explicit sensitivity probes of the unique rendered source
size, not proposed model limits. The record includes the implementation and IR
hashes. Compare the independent-scenario, planned-session, and whole-input
source baselines before drawing conclusions about model cost or bug detection.

### Legacy detection pipeline

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

Review runs in two stages by default (`--review two-stage`): a direct pass on
the full packet with no tools, then an evidence pass with the fact tools for
the bug / unknown / unjudged items only, on a smaller packet (12000 chars) and
a fact-tool budget (`--tool-calls`, 12; later calls are refused with a request
to submit). `--review direct` or `--review evidence` runs a single stage.

Other limits: `--max-turns` (12 per evidence pass), `--max-expansions` (3),
`--max-depth` (8), `--max-steps` (4000 per activation), `--max-participants`
(6), `--max-items-per-task` (24), `--packet-chars` (30000).

Without a key the run falls back to offline and records
`model_calls: not made: ...` in the results.

## Output

```
out/results.json   schema lace3.results/1
out/ledger.json    coverage counts
out/report.md      findings, ledger, open items
out/packets/T*.txt evidence packet per task (T*.evidence.txt: second-stage packet)
out/tasks/T*.json  per task: status, per-stage reviews (items, usage, submission), tool calls
```

`results.json`:

- `scope`: analyzed / not analyzed (deadlock).
- `run`: inputs, limits, backend, `model_calls`, token usage, timings, truncated activations.
- `completeness`: `assignment` (items and contexts placed in tasks; assigned
  is not analyzed), `review` (task statuses, verdicts, `safe_means`,
  `safe_on_part_of_contexts`, `safe_with_open_boundaries`, incomplete /
  pending / error / not-run items), `boundaries` (open dependencies by kind,
  resolved by review), `static_gaps`, `complete`, `fully_resolved`, and
  `recall` (not computed here).
- `boundaries`: every open boundary `B<n>` (kind, activation, step, status,
  resolution, citations).
- `findings`: `id, task, title, kind, items, interleaving, consequence,
  citations, invalid_citations, confidence, model_added, notes`.
- `items`: every ledger item with `kind` (conflict / atomicity / guard /
  lifetime), `keys, summary, contexts_seen, contexts, boundaries, task, status,
  verdict, contexts_reviewed, reason, citations, notes` (notes record each
  stage's verdict).
  Status is one of `assigned` (not run), `pending` (over budget), `reviewed`,
  `incomplete` (no verdict / turn limit), `error`.
- `tasks`, `activations`: clique membership, expansions, notes; per entry its
  provenance, reentrancy, steps, sections, link counts.

## Tests

```
.venv-lace3/bin/python -m pytest -q lace3/tests
```
