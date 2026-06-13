#!/usr/bin/env python3
"""Promote the staging entries that passed verification into the
production `kernel_experiment/` dataset folder.

Inputs:
  final_selection.json (`final_bug_ids`)
  staging_verdict.json (skipped if final id is not classified PASS/PARTIAL)

Actions:
  1. shutil.move each staging/<bug_id>/  ->  kernel_experiment/<bug_id>/
  2. Append a row to kernel_experiment/cve_survey.csv (header preserved).
  3. Append to dataset_expansion/cve_survey_promoted.csv as an audit trail.
  4. Refuse to overwrite an existing destination dir.

Run as:  python3 07_promote.py [--dry-run]
"""
from __future__ import annotations
import argparse
import csv
import json
import shutil
from pathlib import Path

HERE = Path(__file__).resolve().parent
ROOT = HERE.parent.parent  # .../LLM4Con
STAGING = ROOT / "kernel_experiment_v2_staging"
PROD = ROOT / "kernel_experiment"


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--dry-run", action="store_true")
    args = ap.parse_args()

    sel = json.load(open(HERE / "final_selection.json"))["final_bug_ids"]
    ver = json.load(open(HERE / "staging_verdict.json"))
    allowed = set(ver.get("pass", [])) | set(ver.get("partial", []))
    moves: list[tuple[str, str]] = []  # (bug_id, status)
    skipped: list[tuple[str, str]] = []
    for b in sel:
        src = STAGING / b
        dst = PROD / b
        if dst.exists():
            skipped.append((b, "dest exists in production"))
            continue
        if not src.is_dir():
            skipped.append((b, "missing from staging"))
            continue
        if b not in allowed:
            skipped.append((b, "not PASS/PARTIAL in verdict"))
            continue
        status = "PASS" if b in ver.get("pass", []) else "PARTIAL"
        moves.append((b, status))

    print(f"will promote {len(moves)} entries, skip {len(skipped)}")
    for b, why in skipped:
        print(f"  SKIP {b}: {why}")

    if args.dry_run:
        print("\n--- dry-run: would move ---")
        for b, st in moves:
            print(f"  {b}  [{st}]")
        return 0

    rows: list[list[str]] = []
    for b, st in moves:
        src = STAGING / b
        dst = PROD / b
        shutil.move(str(src), str(dst))
        gt = json.load(open(dst / "ground_truth.json"))
        rows.append([b, "YES",
                     ";".join(gt.get("files", [])),
                     gt.get("fix_commit", "")])
        print(f"  moved {b}  [{st}]")

    csv_path = PROD / "cve_survey.csv"
    existing_ids = set()
    if csv_path.exists():
        with open(csv_path) as f:
            next(f, None)
            for line in f:
                if line.strip():
                    existing_ids.add(line.split(",", 1)[0])
    new_rows = [r for r in rows if r[0] not in existing_ids]
    if new_rows:
        write_header = not csv_path.exists()
        with open(csv_path, "a", newline="") as f:
            w = csv.writer(f)
            if write_header:
                w.writerow(["CVE", "HAS_PATCH", "FILES", "FIX_COMMIT"])
            for r in new_rows:
                w.writerow(r)
        print(f"appended {len(new_rows)} rows to {csv_path}")

    trail = HERE / "cve_survey_promoted.csv"
    with open(trail, "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["CVE", "HAS_PATCH", "FILES", "FIX_COMMIT", "STATUS"])
        for (b, st), r in zip(moves, rows):
            w.writerow(r + [st])
    print(f"audit trail written to {trail}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
