#!/usr/bin/env python3
"""Aggregate per-case judgments into recall/precision and diff against the paper run.

Recall is per CASE (did any report name the known defect); precision is per REPORT
(is this report a real race). A case with no reports is a recall MISS contributing
no reports, so it needs no judgment file -- it is counted from the snapshot.

    python3 aggregate_judgments.py --snapshot final_20260907 \
        [--baseline eval_snapshots/full72_20260709_110902/cost_statistics.csv]
"""
import argparse
import csv
import json
import os
import re

ROOT = os.path.dirname(os.path.abspath(__file__))


def load_baseline_csv(path):
    """Per-case (recall, tp, fp) from a run's own statistics table.

    These verdicts were made at the time of that run, under whatever rubric was in
    use then, so they are only loosely comparable to a fresh judging pass.
    """
    out = {}
    if not os.path.isfile(path):
        return out
    for row in csv.DictReader(open(path)):
        rec = (row.get("recall") or "").strip()
        out[row["case"]] = {
            "recall": "HIT" if rec.upper() in ("HIT", "TP", "Y", "YES", "1") else
                      ("MISS" if rec in ("-", "", "MISS") else rec.upper()),
            "tp": int(row.get("reports_tp") or 0),
            "fp": int(row.get("reports_fp") or 0),
        }
    return out


def load_baseline_judgments(tag):
    """Per-case (recall, tp, fp) from a judging pass over another snapshot.

    Preferred over the CSV: comparing two runs is only meaningful when the same
    rubric graded both, and a report count difference otherwise reads as a
    precision change when it is really a change of grader.
    """
    out = {}
    res = os.path.join(ROOT, "judge_results", tag)
    snap = os.path.join(ROOT, "eval_snapshots", tag, "bugs")
    if not os.path.isdir(snap):
        return out
    for case in sorted(d for d in os.listdir(ROOT)
                       if os.path.isdir(os.path.join(ROOT, d))
                       and re.match(r"^(CVE|SYZBOT)-", d)):
        bugs = os.path.join(snap, case + ".txt")
        if not os.path.isfile(bugs):
            out[case] = {"recall": "MISS", "tp": 0, "fp": 0}
            continue
        jf = os.path.join(res, case + ".json")
        if not os.path.isfile(jf):
            continue
        try:
            j = json.load(open(jf))
        except Exception:
            continue
        v = [r.get("verdict") for r in j.get("reports", [])]
        out[case] = {"recall": j.get("recall", "?"),
                     "tp": sum(1 for x in v if x == "TP"),
                     "fp": sum(1 for x in v if x == "FP")}
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--snapshot", required=True)
    ap.add_argument("--baseline",
                    default="eval_snapshots/full72_20260709_110902/cost_statistics.csv",
                    help="a run's own statistics CSV (verdicts from that run's own era)")
    ap.add_argument("--baseline-judgments",
                    help="snapshot tag re-judged under THIS rubric; overrides --baseline")
    args = ap.parse_args()

    snap = os.path.join(ROOT, "eval_snapshots", args.snapshot)
    res = os.path.join(ROOT, "judge_results", args.snapshot)
    if args.baseline_judgments:
        base = load_baseline_judgments(args.baseline_judgments)
        base_label = f"BASELINE ({args.baseline_judgments}, same rubric)"
    else:
        base = load_baseline_csv(os.path.join(ROOT, args.baseline))
        base_label = "BASELINE (as graded at the time)"

    cases = sorted(d for d in os.listdir(ROOT)
                   if os.path.isdir(os.path.join(ROOT, d)) and re.match(r"^(CVE|SYZBOT)-", d))

    rows, unjudged, malformed = [], [], []
    for case in cases:
        bugs = os.path.join(snap, "bugs", case + ".txt")
        nrep = 0
        if os.path.isfile(bugs):
            nrep = len(re.findall(r"=+ Mechanism-Rule Violation Detected =+",
                                  open(bugs, errors="ignore").read()))
        jf = os.path.join(res, case + ".json")
        if nrep == 0:
            rows.append({"case": case, "recall": "MISS", "n": 0, "tp": 0, "fp": 0,
                         "why": "no reports"})
            continue
        if not os.path.isfile(jf):
            unjudged.append(case)
            continue
        try:
            j = json.load(open(jf))
        except Exception as e:
            malformed.append(f"{case}: {e}")
            continue
        verdicts = [r.get("verdict") for r in j.get("reports", [])]
        tp = sum(1 for v in verdicts if v == "TP")
        fp = sum(1 for v in verdicts if v == "FP")
        rows.append({"case": case, "recall": j.get("recall", "?"), "n": nrep,
                     "tp": tp, "fp": fp,
                     "why": j.get("recall_reason", "")[:90],
                     "judged": len(verdicts)})

    def pct(a, b):
        return f"{100*a/b:.1f}%" if b else "n/a"

    hits = sum(1 for r in rows if r["recall"] == "HIT")
    n_rep = sum(r["n"] for r in rows)
    tp = sum(r["tp"] for r in rows)
    fp = sum(r["fp"] for r in rows)
    total = len(rows)

    print(f"{'case':24s} {'recall':6s} {'rep':>4s} {'TP':>3s} {'FP':>3s}   "
          f"{'paper':6s} {'pTP':>4s} {'pFP':>4s}")
    for r in rows:
        b = base.get(r["case"], {})
        mark = ""
        if b:
            if b["recall"] == "HIT" and r["recall"] != "HIT":
                mark = "  LOST"
            elif b["recall"] != "HIT" and r["recall"] == "HIT":
                mark = "  GAINED"
        print(f"{r['case']:24s} {r['recall']:6s} {r['n']:>4d} {r['tp']:>3d} {r['fp']:>3d}   "
              f"{b.get('recall','?'):6s} {b.get('tp','?'):>4} {b.get('fp','?'):>4}{mark}")

    print()
    print(f"THIS RUN ({args.snapshot})")
    print(f"  cases judged      : {total}/{len(cases)}")
    print(f"  recall            : {hits}/{total}  ({pct(hits,total)})")
    print(f"  reports           : {n_rep}")
    print(f"  precision         : {tp}/{tp+fp}  ({pct(tp,tp+fp)})")
    if base:
        bh = sum(1 for c in base if base[c]["recall"] == "HIT")
        btp = sum(base[c]["tp"] for c in base)
        bfp = sum(base[c]["fp"] for c in base)
        print(base_label)
        print(f"  recall            : {bh}/{len(base)}  ({pct(bh,len(base))})")
        print(f"  reports           : {btp+bfp}")
        print(f"  precision         : {btp}/{btp+bfp}  ({pct(btp,btp+bfp)})")
        lost = [r["case"] for r in rows
                if base.get(r["case"], {}).get("recall") == "HIT" and r["recall"] != "HIT"]
        gained = [r["case"] for r in rows
                  if base.get(r["case"], {}).get("recall") not in ("HIT", None)
                  and r["recall"] == "HIT"]
        print(f"\n  lost vs paper   ({len(lost)}): {' '.join(lost) or '-'}")
        print(f"  gained vs paper ({len(gained)}): {' '.join(gained) or '-'}")
    if unjudged:
        print(f"\n  MISSING judgment ({len(unjudged)}): {' '.join(unjudged)}")
    if malformed:
        print(f"  MALFORMED: {'; '.join(malformed)}")


if __name__ == "__main__":
    main()
