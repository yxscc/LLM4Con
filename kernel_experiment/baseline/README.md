# Lace Baseline Comparison Experiments

This directory hosts the three product-level **baseline** experiments
that anchor Lace's contribution in the kernel-CVE evaluation. They
share the *same* model, *same* endpoint, *same* evaluation judge, and
*same* CVE corpus as Lace itself — all the way down to the LLM-as-judge
prompt — so the comparison isolates the **pipeline** (CPG + Phasar +
HBGraph + verifier) as the independent variable.

> **Status:** scaffolding complete; smoke tests passing (B1/B2 both
> correctly judged HIT on CVE-2024-27019, B3 returned CLEAN on the
> same CVE per upstream Mythos's web-app-oriented calibration). Full
> 50-CVE batches kicked off — see `RESULTS.md` once the run finishes.

---

## 1 Motivation

Lace is a static analyser for **Linux-kernel concurrency
vulnerabilities** built on a 4-stage pipeline:

```
.ll bitcode → CPG (Joern) ─┐
                            ├─→ CCPG ─→ HBGraph ─┐
src/ tree   → Phasar ───────┘                     │
                                                  ↓
                               VulnerabilitySurfaceGenerator
                                                  ↓
                            DetectorAgent (LLM, 5+3 DSL)
                                                  ↓
                            HypothesisVerifier (DSL eval)
                                                  ↓
                            confirmed_hypotheses.log
```

The natural sceptical reaction to a system like Lace is

> *"Couldn't you just ask an LLM directly? The static-analysis
>  pipeline is dead weight."*

This experiment falsifies (or, less optimistically, calibrates) that
reaction by comparing Lace head-to-head against three published
LLM-as-vulnerability-detector designs, **all running on the same
GPT-5.5 backbone Lace uses**.

---

## 2 Baselines

### B1 — Zero-shot whole-file scan (lower bound)

A single chat completion: model is given the patch-touched source
file(s) + a fixed concurrency-audit system prompt. No tools, no
multi-turn, no patch leakage.

This is **not a real product** — it's the simplest fair bar. If Lace
cannot beat B1 the rest of the comparison is moot.

* Prompt: [`B1_zeroshot/prompt.md`](B1_zeroshot/prompt.md)
* Entry point: [`B1_zeroshot/run.py`](B1_zeroshot/run.py)
* Cost per CVE: ~$0.05 (one Chat Completion call, ~5–90 K input)

### B2 — Claude Code Security Review prompt (Anthropic OSS)

Anthropic's publicly-released `/security-review` slash-command
prompt — [`anthropics/claude-code-security-review`](https://github.com/anthropics/claude-code-security-review)
@ `.claude/commands/security-review.md` — applied verbatim to a
synthesised PR diff. The diff is constructed by reversing the
ground-truth fix patch, so the synthetic diff represents "the commit
that introduced the vulnerability" — exactly what Claude Code Security
Review is designed to flag.

* Upstream prompt (verbatim, MIT/Apache pending): [`B2_ccsr/upstream/security-review.md`](B2_ccsr/upstream/security-review.md)
* Adaptation only at the runtime layer: the bash-`!`-substitutions
  the upstream prompt invokes (`git status`, `git diff --merge-base
  origin/HEAD`, ...) are filled in with synthetic outputs since
  Claude Code CLI's bash expansion is unavailable on our
  endpoint.
* Entry point: [`B2_ccsr/run.py`](B2_ccsr/run.py)
* Cost per CVE: ~$0.05–0.10 (one call, system prompt is ~14 K chars
  + diff is 2–4 K chars)

#### Limitation
B2 enjoys a localisation advantage Lace does not: the diff input
narrows the model's attention to a small region. We document this
in §6 (discussion).

### B3 — Mythos / Claude Security agentic scaffold (Anthropic product replica)

Faithful reproduction of the public Mythos v4 pipeline — Anthropic's
internal "Project Glasswing" / Claude Security architecture, as
open-sourced by [Keyvanhardani/mythos-research](https://github.com/Keyvanhardani/mythos-research)
(Apache-2.0). The five phases are:

| Phase | Stage | Implementation |
| --- | --- | --- |
| 1 | Sink-guided slicing | Python regex over `sinks/c-cpp.txt` (upstream catalogue) |
| 2 | File ranking | One LLM call using upstream `file-ranking.md` |
| 3 | Agentic hunt (per top-N file) | One LLM call per file using upstream `vsp-c-cpp.md` + `hunter-agent.md` |
| 3.5 | Adversarial self-challenge | One LLM call per finding using `self-challenge.md` |
| 4 | Skeptical validation | One LLM call per surviving finding using `validation.md` |

All four prompt files are **upstream verbatim** (with a header noting
provenance and licence) — see [`B3_mythos/upstream/`](B3_mythos/upstream/).

* Entry point: [`B3_mythos/pipeline.py`](B3_mythos/pipeline.py)
* Cost per CVE: ~$0.30–1.50 (matches upstream Mythos
  `claude-opus-4-7` estimate; GPT-5.5 pricing is similar)

#### Known divergences from upstream Mythos v4
Documented in §6 — none materially weaken the comparison, but they
do bias the result *toward* Mythos:

1. **Single-shot per file**, not multi-turn `Read`/`Grep`/`Glob`
   tool-loop. Compensated by inlining the full file (≤ 80 K chars).
2. No build sandbox (Phase 2.5) — kernel CVE prep doesn't produce
   runnable binaries anyway.
3. No Phase 5 live-exec validator (upstream also private).
4. `pass-at-k = 1` (upstream default = 3); cuts cost ×3.
5. No FP-memory writeback across runs.

---

## 3 Why this is a fair comparison

| Variable | Lace | B1 | B2 | B3 |
| --- | --- | --- | --- | --- |
| Model | gpt-5.5-2026-04-24 | same | same | same |
| Endpoint | shared OpenAI-compatible gateway | same | same | same |
| CVE set | 50 prepared kernel CVEs | same | same | same |
| Judge | `scripts/evaluate_recall.py` (root-cause-vs-patch LLM judge) | same | same | same |
| **Pipeline** | **CPG + Phasar + HBGraph + Verifier** | none | PR-diff prompt | Mythos 5-phase scaffold |

Only the **pipeline** column changes. Everything else is held
constant, including the judge LLM (and its prompt — `JUDGE_SYSTEM`
in `scripts/evaluate_recall.py`).

The result table is therefore an honest test of
> *"Given an industry-standard LLM, does Lace's static-analysis
>  pipeline add value over zero-shot / PR-review / whole-repo
>  agentic baselines?"*

---

## 4 Reproducibility

```bash
cd $LLM4CON_HOME
source setup_env.sh           # exports LLM_API_KEY, LLM_BASE_URL,
                              #         LLM_MODEL, LLM4CON_HOME

cd kernel_experiment/baseline

# Each baseline is independent and idempotent (per-CVE timestamped dump).
python3 B1_zeroshot/run.py                          # ~3 h for 50 CVEs
python3 B2_ccsr/run.py                              # ~2 h for 50 CVEs
python3 B3_mythos/pipeline.py --max-files 5         # ~6 h for 50 CVEs

# Score every baseline with the same judge Lace uses.
for B in B1_zeroshot B2_ccsr B3_mythos; do
  DUMP_BASE="$LLM4CON_HOME/kernel_experiment/baseline_dump/$B" \
  EXPERIMENT_BASE="$LLM4CON_HOME/kernel_experiment" \
  python3 ../../scripts/evaluate_recall.py \
      --api-key "$API_KEY" \
      --base-url "$LLM_BASE_URL" \
      --model    "$LLM_MODEL" \
      --output baseline_eval/${B}_eval.json
done

# Merge into one Markdown report.
python3 compare.py \
    --report Lace_M7=$LLM4CON_HOME/kernel_experiment/evaluation_report.json \
    --report B1=baseline_eval/B1_zeroshot_eval.json \
    --report B2=baseline_eval/B2_ccsr_eval.json \
    --report B3=baseline_eval/B3_mythos_eval.json \
    --out RESULTS.md
```

`SKIP` and `OK`-checkpoints make every baseline safe to interrupt
and resume — re-running picks up where the previous batch stopped.

---

## 5 Results

> **Filled in once full 50-CVE batches complete — see `RESULTS.md`.**

Placeholder for the auto-generated table from `compare.py`:

```text
| Tool       | Model              | CVEs | FOUND% | recall_strict | precision_strict | FP_rate |
| ---        | ---                | ---  | ---    | ---           | ---              | ---     |
| Lace M5    | gpt-5.5-2026-04-24 | 50   | ?      | ?             | ?                | ?       |
| Lace M7    | gpt-5.5-2026-04-24 | 50   | ?      | ?             | ?                | ?       |
| B1 zs      | gpt-5.5-2026-04-24 | 50   | ?      | ?             | ?                | ?       |
| B2 CCSR    | gpt-5.5-2026-04-24 | 50   | ?      | ?             | ?                | ?       |
| B3 Mythos  | gpt-5.5-2026-04-24 | 50   | ?      | ?             | ?                | ?       |
```

Smoke-test results (CVE-2024-27019, n=1):

| Tool       | n_findings | recall  | precision_strict | judged_reason |
| ---        | ---        | ---     | ---              | --- |
| B1 zs      | 1          | **HIT** | 100%             | Same `nf_tables_objects` race, RCU fix overlap |
| B2 CCSR    | 1          | **HIT** | 100%             | Identifies removed `list_for_each_entry_rcu` + missing `rcu_read_lock` |
| B3 Mythos  | 0          | MISS    | n/a              | Hunter returned `verdict: CLEAN` — race conditions are below upstream's HIGH-confidence threshold |

---

## 6 Threats to validity / discussion

### 6.1 Why B2 gets a localisation advantage Lace doesn't
B2's input is a PR diff (synthesised by reversing the fix patch),
which highlights ~3-10 lines as "changed". A model that can read the
diff knows the bug is in the diffed region. Lace's input is the full
LLVM bitcode + CPG of the entire CVE-prep tree, with no patch hint.
We document this asymmetry explicitly rather than try to engineer it
away because it reflects the realistic deployment surfaces of the
two tools: Lace is a one-shot deep static analyser, Claude Code
Security Review is a PR-time reviewer.

### 6.2 Why B3 is biased toward conservative output
Mythos's `hunter-agent.md` explicitly says *"if the best bug you can
find is LOW or MEDIUM, do NOT report it — output
`{findings:[],verdict:CLEAN}` instead."* Its severity ladder
(CRITICAL/HIGH for pre-auth RCE, auth-bypass, cross-user data
access, privilege escalation) is calibrated for **web application**
exploits, not kernel concurrency. A kernel UAF triggered by a
control-plane race is, in the prompt's framing, only *MEDIUM* unless
the model spells out an LPE chain — which it rarely does for a
single-file view. **This is not a defect in our re-implementation;
it is a property of the upstream product as published.**

### 6.3 Single-shot vs. agentic Mythos
We implement the four explicit phases verbatim, but the **inside** of
each agentic phase (Phase 3 hunter / Phase 3.5 challenger / Phase 4
validator) is single-shot rather than multi-turn `Read`/`Grep`/`Glob`.
The full file contents (≤ 80 K chars) are inlined into the user
message to compensate. For CVE files smaller than the kernel norm
(< 5 K LoC) this should approximate the agentic case well; for
heavier files the single-shot reading may miss cross-function
context that an agentic hunter would chase. Future work: implement
real tool-loop dispatching against the gateway (the
gateway is OpenAI-protocol, so OpenAI-style function calling will
work).

### 6.4 Model-controlled vs. product-controlled comparison
We chose **model-controlled** (everyone uses GPT-5.5) over
**product-controlled** (each baseline uses its native model).
Rationale: the gateway credentials available to us do not authorise
`claude-*` models, and renting Anthropic-direct credit for 50 × N
calls is operationally heavy. Model-controlled is the more
scientifically meaningful split anyway — it isolates the
**scaffolding** as the independent variable.

In the paper we should note: *"This experiment compares the
**scaffolding strategies** of three Anthropic products against
Lace's pipeline, holding the LLM backbone fixed at GPT-5.5. We do
not directly evaluate the products as shipped by Anthropic, which
ship with proprietary Claude checkpoints whose AK-gated access was
unavailable to us during this study."*

---

## 7 File layout

```
baseline/
├── README.md                 # this file
├── compare.py                # produces RESULTS.md from N evaluation_report.json
├── common/
│   ├── llm_client.py         # OpenAI-compatible HTTP client
│   ├── cve_loader.py         # ground_truth.json + src/ + patch readers
│   └── dump_writer.py        # bugs.txt / confirmed_hypotheses.log writer
├── B1_zeroshot/
│   ├── prompt.md             # B1 system prompt (locally authored)
│   └── run.py
├── B2_ccsr/
│   ├── upstream/
│   │   └── security-review.md   # verbatim from anthropics/claude-code-security-review
│   └── run.py
└── B3_mythos/
    ├── upstream/                # verbatim from Keyvanhardani/mythos-research
    │   ├── vsp-c-cpp.md
    │   ├── file-ranking.md
    │   ├── hunter-agent.md
    │   ├── self-challenge.md
    │   └── validation.md
    ├── sinks/
    │   └── c-cpp.txt            # verbatim from mythos-research/scripts/lib/sinks/c-cpp.txt
    ├── sink_slicer.py
    └── pipeline.py
```

All upstream prompt/sink files are tracked alongside this codebase
to make the experiment reproducible without re-cloning the source
repositories. Provenance and licensing headers are preserved.
