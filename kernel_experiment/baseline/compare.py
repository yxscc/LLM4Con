#!/usr/bin/env python3
"""compare.py — Merge multiple evaluation_report.json files into a
single side-by-side markdown table.

Each baseline (B1/B2/B3) and Lace itself produces an
evaluation_report.json via scripts/evaluate_recall.py. This script
joins them on CVE-id and emits:
  * a 'summary' table (one row per baseline, FOUND% / recall / precision)
  * a 'per-CVE' table (one row per CVE, one column per baseline)
The Markdown is drop-in for the kernel_experiment/baseline/README.md
results section.

Usage
-----
    python3 compare.py \\
        --report Lace_M7=../../evaluation_report.json \\
        --report B1=./baseline_eval/B1_eval.json \\
        --report B2=./baseline_eval/B2_eval.json \\
        --report B3=./baseline_eval/B3_eval.json \\
        --out RESULTS.md
"""
from __future__ import annotations

import argparse
import json
import os
import sys
from typing import Any, Dict, List, Tuple


def load_report(path: str) -> Dict[str, Any]:
    with open(path, "r", encoding="utf-8") as f:
        return json.load(f)


def per_cve_index(report: Dict[str, Any]) -> Dict[str, Dict[str, Any]]:
    return {d["cve_id"]: d for d in report.get("details", [])}


def summary_row(label: str, report: Dict[str, Any]) -> Dict[str, Any]:
    pb = report.get("per_bug", {})
    return {
        "label": label,
        "model": report.get("model", "?"),
        "n_cves": report.get("total_cves", 0),
        "with_output": report.get("cves_with_detection", 0),
        "hit": report.get("recall_hits", 0),
        "miss": report.get("recall_misses", 0),
        "judge_err": report.get("judge_errors", 0),
        "recall_overall_pct": report.get("recall_overall", 0.0),
        "recall_with_output_pct": report.get("recall_among_with_output", 0.0),
        "found_pct": (
            round(
                report.get("cves_with_detection", 0)
                / max(report.get("total_cves", 1), 1) * 100,
                2,
            )
        ),
        "n_bugs_total": pb.get("total", 0),
        "tp_match": pb.get("tp_match", 0),
        "tp_related": pb.get("tp_related", 0),
        "fp": pb.get("fp", 0),
        "precision_strict_pct": pb.get("precision_strict", 0.0),
        "precision_lenient_pct": pb.get("precision_lenient", 0.0),
        "fp_rate_pct": pb.get("fp_rate", 0.0),
    }


def render_summary_table(rows: List[Dict[str, Any]]) -> str:
    headers = [
        "Tool", "Model", "CVEs",
        "FOUND%", "recall@overall", "recall@with-output",
        "n_bugs", "TP_match", "TP_related", "FP",
        "precision_strict", "precision_lenient", "FP_rate",
    ]
    lines = ["| " + " | ".join(headers) + " |",
             "| " + " | ".join(["---"] * len(headers)) + " |"]
    for r in rows:
        lines.append("| " + " | ".join([
            r["label"], r["model"], f"{r['n_cves']}",
            f"{r['found_pct']}%",
            f"{r['recall_overall_pct']}%",
            f"{r['recall_with_output_pct']}%",
            f"{r['n_bugs_total']}",
            f"{r['tp_match']}",
            f"{r['tp_related']}",
            f"{r['fp']}",
            f"{r['precision_strict_pct']}%",
            f"{r['precision_lenient_pct']}%",
            f"{r['fp_rate_pct']}%",
        ]) + " |")
    return "\n".join(lines)


def render_per_cve_table(
    cve_ids: List[str],
    indexes: List[Tuple[str, Dict[str, Dict[str, Any]]]],
) -> str:
    headers = ["CVE"] + [label for label, _ in indexes] + ["any_hit"]
    lines = ["| " + " | ".join(headers) + " |",
             "| " + " | ".join(["---"] * len(headers)) + " |"]

    def cell(d: Dict[str, Any]) -> str:
        recall = d.get("recall", "—")
        n_match = len(d.get("matched_bug_indices", []) or [])
        if recall == "HIT":
            return f"HIT ({n_match})"
        return recall

    for cve in cve_ids:
        cells = [cve]
        any_hit = False
        for _label, idx in indexes:
            d = idx.get(cve)
            if not d:
                cells.append("—")
                continue
            cells.append(cell(d))
            if d.get("recall") == "HIT":
                any_hit = True
        cells.append("✓" if any_hit else "")
        lines.append("| " + " | ".join(cells) + " |")
    return "\n".join(lines)


def main(argv: List[str] = None) -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--report", action="append", required=True,
                    metavar="LABEL=PATH",
                    help="One per tool. Example: --report Lace_M7=eval.json")
    ap.add_argument("--out", default="-",
                    help="Output Markdown file; '-' = stdout (default).")
    args = ap.parse_args(argv)

    pairs: List[Tuple[str, str]] = []
    for entry in args.report:
        if "=" not in entry:
            print(f"--report '{entry}' missing '=' (use LABEL=PATH)",
                  file=sys.stderr)
            return 2
        label, path = entry.split("=", 1)
        if not os.path.isfile(path):
            print(f"--report '{label}': {path} not found", file=sys.stderr)
            return 2
        pairs.append((label, path))

    reports = [(label, load_report(p)) for label, p in pairs]
    rows = [summary_row(lbl, r) for lbl, r in reports]
    indexes = [(lbl, per_cve_index(r)) for lbl, r in reports]
    all_cves = sorted({
        cve for _, idx in indexes for cve in idx.keys()
    })

    md_parts: List[str] = []
    md_parts.append("## Summary\n")
    md_parts.append(render_summary_table(rows))
    md_parts.append("\n\n## Per-CVE recall (HIT means `evaluate_recall.py` judged the report matches the patch's root cause)\n")
    md_parts.append(render_per_cve_table(all_cves, indexes))
    md = "\n".join(md_parts) + "\n"

    if args.out == "-":
        sys.stdout.write(md)
    else:
        with open(args.out, "w", encoding="utf-8") as f:
            f.write(md)
        print(f"[compare] wrote {args.out} ({len(md)} chars)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
