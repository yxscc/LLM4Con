#!/usr/bin/env python3
"""Check reports against invariants that need no judgement call.

Two of these came out of judging and are decidable mechanically:

  read/read     A pair of reads cannot race, whatever the locking. If both sides
                of a report are only ever reads in the surface, the report is
                unconditionally wrong.
  data_race()   The kernel marks reads it knows are racy and accepts as benign
                with `data_race(...)`. Reporting one is reporting a documented
                non-bug, and judges saw it happen while the unannotated read on
                the next line -- the one the patch fixed -- went unreported.

A third is reported for context rather than as a verdict: a report whose sites
are not in the surface at all cannot be checked this way, which is itself worth
knowing.

    python3 audit_report_invariants.py --snapshot final_20260907
"""
import argparse
import collections
import glob
import json
import os
import re

ROOT = os.path.dirname(os.path.abspath(__file__))
DUMP = os.path.join(os.path.dirname(ROOT), "LLM_dump")


def load_surface(path):
    """node id -> (set of access kinds, set of 'file:line')."""
    kinds = collections.defaultdict(set)
    locs = collections.defaultdict(set)
    d = json.load(open(path))
    for o in d.get("shared_objects", []):
        for a in o.get("accesses", []):
            n = a.get("node_id")
            if n is None:
                continue
            kinds[n].add(a.get("access_type"))
            if a.get("location"):
                locs[n].add(a["location"])
    return kinds, locs


def source_line(case, loc):
    """The source text at a surface location, or None."""
    m = re.match(r"(.*):(\d+)$", loc)
    if not m:
        return None
    path, line = m.group(1), int(m.group(2))
    # Surface locations are recorded without a leading slash.
    for cand in ("/" + path, os.path.join(ROOT, case, path)):
        if os.path.isfile(cand):
            try:
                with open(cand, errors="ignore") as fh:
                    for i, text in enumerate(fh, 1):
                        if i == line:
                            return text
            except OSError:
                return None
    return None


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--snapshot", required=True)
    args = ap.parse_args()
    snap = os.path.join(ROOT, "eval_snapshots", args.snapshot)

    dumps = {}
    for ln in open(os.path.join(snap, "dump_index.txt")):
        p = ln.split()
        if len(p) == 2 and p[1] != "-":
            dumps[p[0]] = os.path.join(DUMP, p[1])

    tot = rr = dr = unknown = 0
    rr_cases, dr_cases = collections.Counter(), collections.Counter()
    examples = []
    for f in sorted(glob.glob(os.path.join(snap, "bugs", "*.txt"))):
        case = os.path.basename(f)[:-4]
        sp = os.path.join(dumps.get(case, ""), "vulnerability_surface.json")
        if not os.path.isfile(sp):
            continue
        kinds, locs = load_surface(sp)
        for blk in open(f, errors="ignore").read().split(
                "========== Mechanism-Rule Violation Detected ==========")[1:]:
            ids = [int(x) for x in re.findall(r"^\s+\w+ \(node (\d+)\):", blk, re.M)]
            if len(ids) < 2:
                continue
            tot += 1
            ka, kb = kinds.get(ids[0]), kinds.get(ids[1])
            if not ka or not kb:
                unknown += 1
                continue
            if ka <= {"Read"} and kb <= {"Read"}:
                rr += 1
                rr_cases[case] += 1
                if len(examples) < 6:
                    examples.append(f"{case}: nodes {ids[0]}/{ids[1]} both Read-only")
            annotated = 0
            for n in ids:
                for loc in locs.get(n, ()):
                    t = source_line(case, loc)
                    if t and "data_race(" in t:
                        annotated += 1
                        break
            if annotated:
                dr += 1
                dr_cases[case] += 1

    def pc(a):
        return f"{100*a/max(1,tot):.0f}%"

    print(f"reports with two node-identified sites : {tot}")
    print(f"  BOTH sides read-only (cannot race)   : {rr}  ({pc(rr)})")
    print(f"  a side is a data_race() read         : {dr}  ({pc(dr)})")
    print(f"  sites absent from the surface        : {unknown}  ({pc(unknown)})")
    if rr_cases:
        print("\nread/read by case:")
        for c, n in rr_cases.most_common():
            print(f"  {c:26s} {n}")
    if dr_cases:
        print("\ndata_race() by case:")
        for c, n in dr_cases.most_common():
            print(f"  {c:26s} {n}")
    for e in examples:
        print("  eg " + e)


if __name__ == "__main__":
    main()
