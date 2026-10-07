#!/usr/bin/env python3
"""Assemble one self-contained judging packet per case.

Judging a run means answering two questions per case: did any report name the
known defect (recall), and is each individual report a real race (precision).
Both need the fix diff -- the diff is the only artifact that says exactly which
storage the upstream maintainers considered unsynchronized -- so each packet
carries the CVE text, the patch hunks, and the run's reports together.

    python3 build_judge_packets.py --snapshot <tag> [--out judge_packets/<tag>]
"""
import argparse
import json
import os
import re

ROOT = os.path.dirname(os.path.abspath(__file__))


def patch_hunks(patch, limit=9000):
    """Keep the parts of a patch that identify the racing storage.

    Kernel patches carry a long commit message and often touch several files;
    what matters for judging is the file headers and the changed lines, so drop
    context lines once the budget runs short rather than truncating mid-file.
    """
    if not patch:
        return "(no patch recorded)"
    lines = patch.splitlines()
    start = next((i for i, l in enumerate(lines) if l.startswith("diff --git")), 0)
    head = "\n".join(lines[:start])
    msg = re.split(r"\n---\n|\nSigned-off-by:", head)[0].strip()
    body = lines[start:]
    out, used = [], 0
    for l in body:
        if l.startswith(("diff --git", "@@", "+++", "---", "+", "-")):
            out.append(l)
            used += len(l)
        elif used < limit * 0.6:
            out.append(l)
            used += len(l)
        if used > limit:
            out.append("... (patch truncated)")
            break
    return (msg[:1500] + "\n\n" + "\n".join(out))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--snapshot", required=True)
    ap.add_argument("--out")
    args = ap.parse_args()

    snap = os.path.join(ROOT, "eval_snapshots", args.snapshot)
    out = args.out or os.path.join(ROOT, "judge_packets", args.snapshot)
    os.makedirs(out, exist_ok=True)

    verdicts = {}
    for ln in open(os.path.join(snap, "run_summary.txt")):
        p = ln.split()
        if p:
            verdicts[p[0]] = ln.strip()

    written, empty = 0, []
    for case in sorted(verdicts):
        gt_path = os.path.join(ROOT, case, "ground_truth.json")
        if not os.path.isfile(gt_path):
            continue
        gt = json.load(open(gt_path))
        rep_path = os.path.join(snap, "bugs", case + ".txt")
        reports = open(rep_path, errors="ignore").read() if os.path.isfile(rep_path) else ""
        if not reports.strip():
            empty.append(case)

        n = len(re.findall(r"=+ Mechanism-Rule Violation Detected =+", reports))
        doc = [
            f"# Judging packet: {case}",
            "",
            f"CWE: {', '.join(gt.get('cwes', []))}",
            f"Files touched by the fix: {', '.join(gt.get('files', []))}",
            f"Fix commit: {gt.get('fix_commit', '')}",
            f"Run summary line: {verdicts[case]}",
            f"Reports in this run: {n}",
            "",
            "## Known defect (CVE/syzbot text)",
            "",
            gt.get("description", "").strip() or "(none)",
            "",
            "## Upstream fix (this is the authority on which storage was racing)",
            "",
            "```diff",
            patch_hunks(gt.get("patch", "")),
            "```",
            "",
            "## Reports produced by the run",
            "",
        ]
        if n:
            doc.append(reports)
        else:
            doc.append("(this run produced no reports for this case)")
        open(os.path.join(out, case + ".md"), "w").write("\n".join(doc))
        written += 1

    print(f"packets: {out}")
    print(f"  written: {written}   cases with no reports: {len(empty)}")
    if empty:
        print("  " + " ".join(empty))


if __name__ == "__main__":
    main()
