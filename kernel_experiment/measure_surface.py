#!/usr/bin/env python3
"""Measure the static vulnerability surface against ground truth, without LLMs.

Pair this with LACE_EARLY_EXIT_AFTER_SURFACE=1 to iterate on the surface
heuristics (object identity, grouping, suppressions, risk score) in minutes
instead of paying for a full LLM run.

For each case we locate the ground-truth access pair in flow_annotation.json
(ground_truth_access.access_a / access_b) and ask where it landed:

  BOTH    one shared object covers both GT accesses -- the checker can at
          least see the conflicting pair
  SPLIT   both GT accesses are on the surface but in *different* objects --
          object identity fragmented the pair apart
  PARTIAL only one GT side is on the surface
  ABSENT  neither side is on the surface

GT line numbers come from the upstream kernel tree while a case's src/ holds a
reduced copy, so accesses are matched on function name plus code text.

Usage:
  measure_surface.py <stamp>          # e.g. 2026-08-06, matches LLM_dump dirs
  measure_surface.py <stamp> --csv out.csv
"""
import argparse
import csv
import json
import os
import re
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
DUMP_ROOT = Path(os.environ.get("LACE_DUMP_ROOT", HERE.parent / "LLM_dump"))


def norm_code(s: str) -> str:
    """Collapse whitespace and drop the trailing [file:line] marker."""
    s = re.split(r"\s*\[[^\]]*:\d+\]", s or "")[0]
    return re.sub(r"\s+", " ", s).strip().rstrip(";").lower()


# GT "kind" is free text written by annotators; the surface only distinguishes
# Read/Write/Free. Map the former onto the latter, permissively.
_KIND_TO_TYPES = {
    "free": {"Free", "Write"}, "delete": {"Free", "Write"},
    "use": {"Read", "Write"}, "read": {"Read"}, "write": {"Write"},
    "register": {"Write"}, "unregister": {"Write", "Free"},
    "detach": {"Write"}, "lock": {"Read", "Write"},
    "missed_register": {"Write"},
}


def kind_compatible(acc: dict, gt: dict) -> bool:
    kinds = re.split(r"[/_]", (gt.get("kind") or "").lower())
    allowed = set()
    for k in kinds:
        allowed |= _KIND_TO_TYPES.get(k.strip(), set())
    if not allowed:
        return True
    return acc.get("access_type") in allowed


def access_matches(acc: dict, gt: dict) -> int:
    """0 = no match, 1 = function only, 2 = function plus kind, 3 = plus code.

    Code text is the strongest signal but is often unavailable: frees in
    particular are recorded synthetically (``[ir-fallback] free via kfree``)
    and never share text with the annotated ``kfree(data);``.
    """
    fn = (gt.get("function") or "").strip()
    if not fn:
        return 0
    if fn not in (acc.get("containing_function"), acc.get("function")):
        return 0
    gc, ac = norm_code(gt.get("code", "")), norm_code(acc.get("code", ""))
    if gc and ac and (gc in ac or ac in gc):
        return 3
    return 2 if kind_compatible(acc, gt) else 1


def latest_dump(case: str, stamp: str):
    cands = sorted(DUMP_ROOT.glob(f"{case}_{stamp}*"))
    return cands[-1] if cands else None


def classify(objs, gt_a, gt_b, need: int):
    """Return (verdict, rank_of_best_object, n_objects_touching_gt)."""
    hit_a, hit_b, both = set(), set(), []
    for i, o in enumerate(objs):
        a = any(access_matches(x, gt_a) >= need for x in o.get("accesses", []))
        b = any(access_matches(x, gt_b) >= need for x in o.get("accesses", []))
        if a:
            hit_a.add(i)
        if b:
            hit_b.add(i)
        if a and b:
            both.append(i)
    if both:
        return "BOTH", min(both) + 1, len(hit_a | hit_b)
    if hit_a and hit_b:
        return "SPLIT", min(hit_a | hit_b) + 1, len(hit_a | hit_b)
    if hit_a or hit_b:
        return "PARTIAL", min(hit_a | hit_b) + 1, len(hit_a | hit_b)
    return "ABSENT", 0, 0


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("stamp", help="dump-directory timestamp prefix, e.g. 2026-08-06")
    ap.add_argument("--csv", help="also write per-case rows here")
    ap.add_argument("--match", choices=("loose", "kind", "code"), default="kind",
                    help="GT matching strength: function only / plus access "
                         "type (default) / plus code text")
    args = ap.parse_args()
    need = {"loose": 1, "kind": 2, "code": 3}[args.match]

    rows = []
    for ann in sorted(HERE.glob("*/flow_annotation.json")):
        case = ann.parent.name
        gt = json.loads(ann.read_text()).get("ground_truth_access") or {}
        gt_a, gt_b = gt.get("access_a") or {}, gt.get("access_b") or {}
        d = latest_dump(case, args.stamp)
        if d is None:
            rows.append((case, None, "NO-RUN", 0, 0))
            continue
        sf = d / "vulnerability_surface.json"
        objs = []
        if sf.exists():
            try:
                objs = json.loads(sf.read_text()).get("shared_objects", [])
            except json.JSONDecodeError:
                pass
        verdict, rank, touched = classify(objs, gt_a, gt_b, need)
        rows.append((case, len(objs), verdict, rank, touched))

    rows.sort(key=lambda r: (r[2], -(r[1] or 0)))
    print(f"{'case':26s} {'objs':>5} {'verdict':>8} {'rank':>5} {'gt-objs':>7}")
    for case, n, v, rank, touched in rows:
        print(f"{case:26s} {('-' if n is None else n):>5} {v:>8} "
              f"{(rank or '-'):>5} {(touched or '-'):>7}")

    ran = [r for r in rows if r[1] is not None]
    counts = {}
    for r in ran:
        counts[r[2]] = counts.get(r[2], 0) + 1
    print("\n" + "=" * 60)
    print(f"cases with a surface run : {len(ran)}/{len(rows)}")
    for k in ("BOTH", "SPLIT", "PARTIAL", "ABSENT"):
        print(f"  {k:8s} {counts.get(k, 0):>3}")
    both = [r for r in ran if r[2] == "BOTH"]
    if both:
        ranks = sorted(r[3] for r in both)
        print(f"GT rank among BOTH       : median {ranks[len(ranks) // 2]}, "
              f"max {ranks[-1]}, top-10 {sum(1 for x in ranks if x <= 10)}/{len(ranks)}")
    sizes = sorted(r[1] for r in ran)
    if sizes:
        print(f"objects per case         : median {sizes[len(sizes) // 2]}, "
              f"max {sizes[-1]}, total {sum(sizes)}, "
              f"zero-object cases {sum(1 for x in sizes if x == 0)}")

    if args.csv:
        with open(args.csv, "w", newline="") as fh:
            w = csv.writer(fh)
            w.writerow(["case", "objects", "verdict", "gt_rank", "gt_objects"])
            w.writerows(rows)
        print(f"\nwrote {args.csv}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
