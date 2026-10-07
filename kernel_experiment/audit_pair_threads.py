#!/usr/bin/env python3
"""Check whether each report's two sites can actually belong to different threads.

Two judges independently reported that many findings pair sites that are
program-ordered inside ONE thread (a use and the free that follows it on the same
teardown path). Reports name their sites by CCPG node id, and the surface records
a thread id for every access, so the claim is checkable without reading code: a
report is only meaningful if some access of side `a` and some access of side `b`
sit in DIFFERENT threads.

    python3 audit_pair_threads.py --snapshot final_20260907
"""
import argparse
import collections
import glob
import json
import os
import re

ROOT = os.path.dirname(os.path.abspath(__file__))
DUMP = os.path.join(os.path.dirname(ROOT), "LLM_dump")


def node_threads(surface_path):
    """node id -> set of thread ids that access it."""
    m = collections.defaultdict(set)
    d = json.load(open(surface_path))
    for o in d.get("shared_objects", []):
        for a in o.get("accesses", []):
            if a.get("node_id") is not None:
                m[a["node_id"]].add(a.get("thread_id"))
    return m


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

    tot = same = cross = unknown = 0
    per_case = {}
    for f in sorted(glob.glob(os.path.join(snap, "bugs", "*.txt"))):
        case = os.path.basename(f)[:-4]
        sp = os.path.join(dumps.get(case, ""), "vulnerability_surface.json")
        if not os.path.isfile(sp):
            continue
        nt = node_threads(sp)
        s = c = u = 0
        for blk in open(f, errors="ignore").read().split(
                "========== Mechanism-Rule Violation Detected ==========")[1:]:
            ids = [int(x) for x in re.findall(r"^\s+\w+ \(node (\d+)\):", blk, re.M)]
            if len(ids) < 2:
                continue
            tot += 1
            ta, tb = nt.get(ids[0]), nt.get(ids[1])
            if not ta or not tb:
                u += 1
                unknown += 1
            elif any(x != y for x in ta for y in tb):
                c += 1
                cross += 1
            else:
                s += 1
                same += 1
        if s or c or u:
            per_case[case] = (s, c, u)

    def pc(a):
        return f"{100*a/max(1,tot):.0f}%"

    print(f"reports with two node-identified sites : {tot}")
    print(f"  sites CAN be in different threads    : {cross}  ({pc(cross)})")
    print(f"  sites only ever in the SAME thread   : {same}  ({pc(same)})")
    print(f"  node not found in surface            : {unknown}  ({pc(unknown)})")
    print("\ncases with same-thread-only pairs (same / cross / unknown):")
    for case, (s, c, u) in sorted(per_case.items(), key=lambda kv: -kv[1][0]):
        if s:
            print(f"  {case:26s} {s:3d} / {c:3d} / {u:3d}")


if __name__ == "__main__":
    main()
