#!/usr/bin/env python3
"""
LLM-only recall + false-positive evaluation for the Lace concurrency
vulnerability detector.

For each prepared CVE we:
  1. Read ground_truth.json  (CVE description, CWEs, patch).
  2. Parse the latest detection output — bugs.txt and
     confirmed_hypotheses.log — into individual bug reports.
  3. Ask an LLM judge to, in ONE call per CVE:
       (a) decide whether the detector recalled the CVE (HIT / MISS);
       (b) classify every individual bug report as TP_MATCH / TP_RELATED
           / FP relative to the real CVE.
  4. Aggregate into dataset-wide recall and per-bug precision /
     false-positive-rate.

No more surface-level file-name or patch-function string matching.
"""

from __future__ import annotations

import argparse
import glob
import json
import os
import re
import sys
import time
from datetime import datetime
from pathlib import Path
from typing import Any, Dict, List, Optional, Tuple

_LLM4CON_HOME = os.environ.get("LLM4CON_HOME", "/home/LLM4Con")
EXPERIMENT_BASE = os.environ.get(
    "EXPERIMENT_BASE", f"{_LLM4CON_HOME}/kernel_experiment")
DUMP_BASE = os.environ.get("DUMP_BASE", f"{_LLM4CON_HOME}/LLM_dump")


# ──────────────────────────────────────────────────────────────────────
# Detection-result loading
# ──────────────────────────────────────────────────────────────────────

_BUG_DELIMITER = re.compile(
    r"=+\s*Hypothesis-Based Violation Detected\s*=+", re.IGNORECASE)


def find_latest_output_dir(cve_id: str) -> Optional[str]:
    """Return the most recent ``LLM_dump/<CVE>_<timestamp>`` folder that
    actually contains a bug or hypothesis file. Falls back to the newest
    folder overall if none carry bug artifacts."""
    if not os.path.isdir(DUMP_BASE):
        return None
    cands = sorted(
        glob.glob(os.path.join(DUMP_BASE, f"{cve_id}_*")), reverse=True)
    for c in cands:
        if (os.path.exists(os.path.join(c, "stateful_bugs", "bugs.txt"))
                or os.path.exists(os.path.join(c, "confirmed_hypotheses.log"))):
            return c
    return cands[0] if cands else None


def _read_if(p: str) -> str:
    try:
        with open(p, "r", errors="ignore") as f:
            return f.read()
    except Exception:
        return ""


def load_detection(cve_dir: str) -> Dict[str, Any]:
    """Collect all detection evidence for a CVE. We always prefer the
    latest LLM_dump output but also fall back to the per-CVE
    ``output_*`` subdirs if they exist."""
    cve_id = os.path.basename(cve_dir.rstrip("/"))
    result: Dict[str, Any] = {
        "cve_id": cve_id,
        "has_detection": False,
        "bugs_raw": "",
        "hypotheses_raw": "",
        "dump_dir": None,
    }

    dump_dir = find_latest_output_dir(cve_id)
    if dump_dir:
        result["dump_dir"] = dump_dir
        bugs = _read_if(os.path.join(dump_dir, "stateful_bugs", "bugs.txt"))
        hyp = _read_if(os.path.join(dump_dir, "confirmed_hypotheses.log"))
        if bugs.strip():
            result["bugs_raw"] = bugs
            result["has_detection"] = True
        if hyp.strip():
            result["hypotheses_raw"] = hyp
            result["has_detection"] = True

    # Per-CVE output_* (legacy path). Only consult when LLM_dump was empty.
    if not result["has_detection"]:
        for subdir in sorted(
                glob.glob(os.path.join(cve_dir, "output_*")), reverse=True):
            bugs = _read_if(os.path.join(subdir, "stateful_bugs", "bugs.txt"))
            hyp = _read_if(os.path.join(subdir, "confirmed_hypotheses.log"))
            if bugs.strip():
                result["bugs_raw"] = bugs
                result["has_detection"] = True
            if hyp.strip():
                result["hypotheses_raw"] = hyp
                result["has_detection"] = True
            if result["has_detection"]:
                result["dump_dir"] = subdir
                break

    return result


def split_bug_reports(bugs_raw: str, hypotheses_raw: str) -> List[str]:
    """Split the concatenated ``bugs.txt`` text into individual reports,
    each corresponding to one Hypothesis-Based Violation. Hypotheses-log
    reports are added only if they contain information not already
    covered by bugs.txt (we keep the union so the judge sees both)."""
    reports: List[str] = []

    def _split(text: str) -> List[str]:
        if not text.strip():
            return []
        chunks: List[str] = []
        # Use the delimiter as a separator but keep it in the chunk.
        parts = _BUG_DELIMITER.split(text)
        # The text before the first delimiter is the preamble; discard
        # unless it already looks like a bug body.
        for p in parts[1:]:
            p = p.strip()
            # Clean footer lines like "count N -----"
            p = re.sub(r"\n+count\s+\d+\s+-+\s*$", "", p, flags=re.IGNORECASE)
            # Clean trailing separator.
            p = p.strip("= \n")
            if p:
                chunks.append(p)
        return chunks

    reports.extend(_split(bugs_raw))
    # Hypotheses log: use only if bugs.txt was empty, otherwise it's
    # mostly duplicative noise.
    if not reports:
        reports.extend(_split(hypotheses_raw))

    # Truncate any single report that is absurdly long to keep prompt size sane.
    trimmed = []
    MAX_PER_REPORT = 2000
    for r in reports:
        if len(r) > MAX_PER_REPORT:
            r = r[:MAX_PER_REPORT] + "\n...<truncated>..."
        trimmed.append(r)
    return trimmed


# ──────────────────────────────────────────────────────────────────────
# Ground-truth rendering
# ──────────────────────────────────────────────────────────────────────

def format_ground_truth(gt: Dict[str, Any]) -> str:
    """Render the CVE ground truth as a short briefing for the judge.
    We keep it compact so the context budget is spent on bug reports."""
    cve_id = gt.get("cve_id") or "UNKNOWN"
    parts = [f"## Ground Truth — {cve_id}"]

    desc = (gt.get("description") or gt.get("summary") or "").strip()
    if desc:
        parts.append(f"\n### CVE description\n{desc[:2200]}")

    cwes = gt.get("cwes") or []
    if cwes:
        parts.append(f"\n### CWEs: {', '.join(cwes)}")

    patch = gt.get("patch") or ""
    if patch:
        # Keep only hunk headers and +/- lines so the judge can see which
        # functions / variables were actually touched, without burning
        # tokens on unchanged context.
        kept = []
        for line in patch.splitlines():
            if (line.startswith(("@@", "+++", "---", "diff --git"))
                    or (line[:1] in ("+", "-") and not line.startswith(("+++", "---")))):
                kept.append(line)
            if len(kept) > 120:
                kept.append("...<patch truncated>...")
                break
        if kept:
            parts.append("\n### Patch (essential lines)\n```diff\n"
                         + "\n".join(kept) + "\n```")

    return "\n".join(parts)


# ──────────────────────────────────────────────────────────────────────
# LLM judge
# ──────────────────────────────────────────────────────────────────────

JUDGE_SYSTEM = """You are an expert evaluator for static concurrency bug detectors targeting the Linux kernel.

You will be given:
  * A CVE ground-truth briefing (description, CWEs, patch diff).
  * A list of individual bug reports produced by the tool for ONE specific CVE target.

Your job is twofold.

1. Decide whether the tool RECALLED the CVE.
   A recall is a HIT iff at least one bug report in the list describes
   the same underlying concurrency vulnerability as the CVE. "Same
   underlying vulnerability" means the involved shared object / data
   structure matches (names may differ — IR vs source), the bug category
   is consistent (a UAF may surface as a data-race or double-free; count
   it as consistent if the root cause — unsynchronized shared access —
   is the same), and at least one of the involved functions / code
   regions overlaps with what the CVE patch actually changed.

2. Classify EVERY bug report in the list.
   For each report assign one label:
     * TP_MATCH   — this bug IS the CVE (same shared object, overlapping
                    functions, consistent pattern). Multiple reports may
                    share this label if they describe the same bug from
                    different angles.
     * TP_RELATED — a genuine-looking concurrency bug in code that is
                    adjacent to or shares state with the CVE fix site,
                    but NOT the CVE itself. (Not a false positive.)
     * FP         — the report does not plausibly describe a real
                    concurrency vulnerability, or the constraints /
                    locations it cites are clearly nonsensical (e.g.
                    all "involved nodes" map to the same single
                    statement, the "two threads" are really the same
                    caller, or the racing variable is thread-local).

Be strict about FP: mark a report FP only if you have specific,
stated reasons why it is not a real bug. Do NOT mark something FP
merely because it is not the CVE — that is what TP_RELATED is for.

Output a single JSON object with this schema and NOTHING else:
{
  "recall": "HIT" | "MISS",
  "recall_reason": "<=300 chars",
  "matched_bug_indices": [int, ...],    // indices (1-based) of reports voted TP_MATCH
  "bug_verdicts": [
    {"idx": int, "label": "TP_MATCH" | "TP_RELATED" | "FP", "reason": "<=200 chars"},
    ...
  ]
}

Produce one entry per input report. `idx` is 1-based in the order they
were presented. If the report list is empty, set recall=MISS,
matched_bug_indices=[], bug_verdicts=[].
"""


def build_judge_prompt(gt_text: str, reports: List[str]) -> str:
    if not reports:
        bug_block = "(tool produced no bug reports)"
    else:
        bug_block_parts = []
        for i, r in enumerate(reports, 1):
            bug_block_parts.append(f"### Report #{i}\n{r}")
        bug_block = "\n\n".join(bug_block_parts)

    return (
        f"{gt_text}\n\n---\n\n## Detector output ({len(reports)} bug report(s))\n\n"
        f"{bug_block}\n\n---\n\n"
        "Produce the JSON verdict object described in your instructions. "
        "Make sure the `bug_verdicts` array contains one entry per report "
        "above in the same numbering."
    )


def call_llm(prompt: str, api_key: str, model: str, base_url: str,
             retries: int = 2, timeout: int = 180) -> Dict[str, Any]:
    import urllib.request
    import urllib.error

    payload: Dict[str, Any] = {
        "model": model,
        "messages": [
            {"role": "system", "content": JUDGE_SYSTEM},
            {"role": "user", "content": prompt},
        ],
        "max_tokens": 4000,
    }
    # GPT-5 / GPT-5.5 reasoning models reject any temperature != 1.
    # Skip the field for them; non-reasoning models default to 1 anyway,
    # so the determinism loss is negligible for a one-shot judge call.
    if not re.match(r"^gpt-?5", model, re.IGNORECASE):
        payload["temperature"] = 0.1
    headers = {
        "Content-Type": "application/json",
        "Authorization": f"Bearer {api_key}",
    }
    # If the caller passes an already-complete endpoint (anything containing
    # `?` or ending in `/chat/completions` / `/crawl`), use it as-is. Only the
    # canonical OpenAI-style root URL gets `/chat/completions` appended.
    if "?" in base_url or base_url.rstrip("/").endswith(
            ("/chat/completions", "/crawl", "/completions")):
        endpoint = base_url
    else:
        endpoint = f"{base_url.rstrip('/')}/chat/completions"

    last_err: Optional[str] = None
    for attempt in range(retries + 1):
        try:
            req = urllib.request.Request(
                endpoint,
                data=json.dumps(payload).encode(),
                headers=headers,
                method="POST",
            )
            with urllib.request.urlopen(req, timeout=timeout) as resp:
                result = json.loads(resp.read().decode())
            content = result["choices"][0]["message"]["content"].strip()
            # Strip ```json fences if present.
            if content.startswith("```"):
                content = re.sub(r"^```[a-zA-Z]*\n?", "", content)
                content = re.sub(r"\n?```$", "", content)
                content = content.strip()
            # Tolerate a leading "Here is..." paragraph by grabbing the
            # outermost JSON object.
            brace = content.find("{")
            if brace > 0:
                content = content[brace:]
            # Truncate anything trailing after the last closing brace.
            last = content.rfind("}")
            if last > 0 and last < len(content) - 1:
                content = content[:last + 1]
            return json.loads(content)
        except Exception as e:
            last_err = f"{type(e).__name__}: {e}"
            time.sleep(1.5 * (attempt + 1))
    return {
        "recall": "ERROR",
        "recall_reason": f"LLM call failed after {retries + 1} attempts: {last_err}",
        "matched_bug_indices": [],
        "bug_verdicts": [],
    }


# ──────────────────────────────────────────────────────────────────────
# Orchestration
# ──────────────────────────────────────────────────────────────────────

def evaluate_all(api_key: str, model: str, base_url: str,
                 cve_filter: Optional[List[str]] = None,
                 output_report: Optional[str] = None,
                 max_reports_per_cve: int = 25) -> Dict[str, Any]:
    results: List[Dict[str, Any]] = []

    cve_dirs: List[str] = []
    for pat in ("CVE-*", "SYZBOT-*"):
        cve_dirs.extend(glob.glob(os.path.join(EXPERIMENT_BASE, pat)))
    cve_dirs = sorted(cve_dirs)
    print(f"\n{'='*64}")
    print(f" LLM-only Recall + FP Evaluation ({len(cve_dirs)} CVE dirs)")
    print(f" Model: {model}   base_url: {base_url}")
    print(f"{'='*64}\n")

    total_bugs = 0
    tp_match = 0
    tp_related = 0
    fp = 0
    llm_errors = 0

    for cve_dir in cve_dirs:
        cve_id = os.path.basename(cve_dir)
        if cve_filter and cve_id not in cve_filter:
            continue

        gt_file = os.path.join(cve_dir, "ground_truth.json")
        if not os.path.exists(gt_file):
            print(f"  [{cve_id}] SKIP (no ground_truth.json)")
            continue
        if not glob.glob(os.path.join(cve_dir, "*.ll")):
            print(f"  [{cve_id}] SKIP (no .ll files)")
            continue

        with open(gt_file) as f:
            gt = json.load(f)
        gt.setdefault("cve_id", cve_id)

        det = load_detection(cve_dir)
        reports = split_bug_reports(det["bugs_raw"], det["hypotheses_raw"])

        # Hard cap to keep prompts sane. If a CVE has many reports we
        # keep the first N (bugs.txt is already ordered by detection
        # priority, usually most interesting first).
        if len(reports) > max_reports_per_cve:
            reports_for_judge = reports[:max_reports_per_cve]
            truncated = True
        else:
            reports_for_judge = reports
            truncated = False

        if not det["has_detection"]:
            # Short-circuit: no bug output means forced MISS.
            verdict = {
                "recall": "MISS",
                "recall_reason": "Tool produced no bug output for this CVE.",
                "matched_bug_indices": [],
                "bug_verdicts": [],
            }
            print(f"  [{cve_id}] MISS (no detector output)")
        else:
            prompt = build_judge_prompt(format_ground_truth(gt),
                                        reports_for_judge)
            print(f"  [{cve_id}] judging {len(reports_for_judge)}"
                  f"{'(+more)' if truncated else ''} report(s)... ",
                  end="", flush=True)
            verdict = call_llm(prompt, api_key, model, base_url)
            if verdict["recall"] == "ERROR":
                llm_errors += 1
                print(f"ERROR — {verdict['recall_reason'][:80]}")
            else:
                print(verdict["recall"])

        # Tally per-bug labels for precision / FP rate.
        seen_idx = set()
        for bv in verdict.get("bug_verdicts", []):
            lbl = str(bv.get("label", "")).upper()
            idx = bv.get("idx")
            if idx is None or idx in seen_idx:
                continue
            seen_idx.add(idx)
            total_bugs += 1
            if lbl == "TP_MATCH":
                tp_match += 1
            elif lbl == "TP_RELATED":
                tp_related += 1
            elif lbl == "FP":
                fp += 1

        results.append({
            "cve_id": cve_id,
            "dump_dir": det["dump_dir"],
            "has_detection": det["has_detection"],
            "n_reports_total": len(reports),
            "n_reports_judged": len(reports_for_judge),
            "judged_truncated": truncated,
            "recall": verdict.get("recall", "ERROR"),
            "recall_reason": verdict.get("recall_reason", ""),
            "matched_bug_indices": verdict.get("matched_bug_indices", []),
            "bug_verdicts": verdict.get("bug_verdicts", []),
        })

    # Aggregate.
    total_cves = len(results)
    eval_cves = sum(1 for r in results if r["has_detection"])
    hit = sum(1 for r in results if r["recall"] == "HIT")
    miss = sum(1 for r in results if r["recall"] == "MISS")
    err = sum(1 for r in results if r["recall"] == "ERROR")

    def _pct(num: int, den: int) -> float:
        return round(num / den * 100, 2) if den else 0.0

    summary = {
        "timestamp": datetime.now().isoformat(timespec="seconds"),
        "model": model,
        "total_cves": total_cves,
        "cves_with_detection": eval_cves,
        "recall_hits": hit,
        "recall_misses": miss,
        "judge_errors": err,
        "recall_overall": _pct(hit, total_cves),
        "recall_among_with_output": _pct(hit, eval_cves),
        "per_bug": {
            "total": total_bugs,
            "tp_match": tp_match,
            "tp_related": tp_related,
            "fp": fp,
            "fp_rate": _pct(fp, total_bugs),
            "precision_strict": _pct(tp_match, total_bugs),
            "precision_lenient": _pct(tp_match + tp_related, total_bugs),
        },
        "llm_error_rate": _pct(llm_errors, total_cves),
        "details": results,
    }

    if output_report is None:
        output_report = os.path.join(EXPERIMENT_BASE, "evaluation_report.json")
    with open(output_report, "w") as f:
        json.dump(summary, f, indent=2, ensure_ascii=False)

    print(f"\n{'='*64}")
    print(f" SUMMARY")
    print(f"{'='*64}")
    print(f"  CVEs total / with-output : {total_cves} / {eval_cves}")
    print(f"  Recall (HIT/MISS/ERROR)  : {hit} / {miss} / {err}")
    print(f"  Recall @ overall         : {summary['recall_overall']}%")
    print(f"  Recall @ with-output     : {summary['recall_among_with_output']}%")
    print(f"  --- per-bug classification ({total_bugs} bugs) ---")
    print(f"  TP_MATCH   : {tp_match}  ({_pct(tp_match, total_bugs)}%)")
    print(f"  TP_RELATED : {tp_related}  ({_pct(tp_related, total_bugs)}%)")
    print(f"  FP         : {fp}  ({_pct(fp, total_bugs)}%)")
    print(f"  precision (strict)  = {summary['per_bug']['precision_strict']}%")
    print(f"  precision (lenient) = {summary['per_bug']['precision_lenient']}%")
    print(f"  FP rate             = {summary['per_bug']['fp_rate']}%")
    print(f"{'='*64}")
    print(f"  Report: {output_report}")
    return summary


if __name__ == "__main__":
    ap = argparse.ArgumentParser(
        description="LLM-only recall + FP evaluation for Lace.")
    ap.add_argument("--api-key",
                    default=os.environ.get("LLM_API_KEY")
                    or os.environ.get("API_KEY")
                    or os.environ.get("OPENAI_API_KEY")
                    or os.environ.get("DEFAULT_KEY"),
                    help="LLM API key (defaults to $LLM_API_KEY / $API_KEY / "
                         "$OPENAI_API_KEY / $DEFAULT_KEY).")
    ap.add_argument("--model",
                    default=os.environ.get("LLM_MODEL", "gpt-4o"),
                    help="Judge model (default: $LLM_MODEL or gpt-4o).")
    ap.add_argument("--base-url",
                    default=os.environ.get("LLM_BASE_URL",
                                           "https://api.chatanywhere.tech/v1"),
                    help="LLM API endpoint. If it already points at a "
                         "complete chat-completion path (contains ?ak= or "
                         "ends in /chat/completions /crawl /completions), "
                         "it is used verbatim; otherwise /chat/completions "
                         "is appended.")
    ap.add_argument("--cve", nargs="*", help="Evaluate specific CVEs only.")
    ap.add_argument("--max-reports", type=int, default=25,
                    help="Cap on bug reports per CVE in the judge prompt "
                         "(default: 25). Extras are not judged but are "
                         "counted in total_reports.")
    ap.add_argument("--output",
                    default=os.path.join(EXPERIMENT_BASE, "evaluation_report.json"),
                    help="Where to dump the JSON summary.")
    args = ap.parse_args()

    if not args.api_key:
        print("ERROR: no --api-key and no $OPENAI_API_KEY / $DEFAULT_KEY in env.",
              file=sys.stderr)
        sys.exit(2)

    evaluate_all(args.api_key, args.model, args.base_url,
                 cve_filter=args.cve,
                 output_report=args.output,
                 max_reports_per_cve=args.max_reports)
