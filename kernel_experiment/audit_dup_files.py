#!/usr/bin/env python3
"""Count reports whose two sides are the same source line reached via duplicate files.

Several case trees carry a file twice -- once at its kernel-relative path and once
flattened into `src/` -- so the CPG builds two node sets for one function and the
surface treats them as separate contexts. Any report pairing the two copies is an
artifact of the tree layout, not a finding: same file content, same line, no second
thread. Reported separately from same-line self-pairs inside one file, which are a
different (chained-assignment) artifact.

    python3 audit_dup_files.py --snapshot lockfix_20260907_204407
"""
import argparse
import collections
import glob
import hashlib
import os
import re

ROOT = os.path.dirname(os.path.abspath(__file__))


def dup_groups(case_dir):
    """basename-relative duplicate sets: content hash -> [paths] with >1 entry."""
    by_hash = collections.defaultdict(list)
    src = os.path.join(case_dir, "src")
    for dirpath, _dirs, files in os.walk(src):
        for fn in files:
            if not fn.endswith((".c", ".h")):
                continue
            p = os.path.join(dirpath, fn)
            try:
                h = hashlib.sha1(open(p, "rb").read()).hexdigest()
            except OSError:
                continue
            by_hash[h].append(p)
    return {h: v for h, v in by_hash.items() if len(v) > 1}


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--snapshot", required=True)
    args = ap.parse_args()
    snap = os.path.join(ROOT, "eval_snapshots", args.snapshot)

    tot = dup_pair = same_line = 0
    per_case = collections.Counter()
    dup_cases = 0

    for f in sorted(glob.glob(os.path.join(snap, "bugs", "*.txt"))):
        case = os.path.basename(f)[:-4]
        case_dir = os.path.join(ROOT, case)
        if not os.path.isdir(case_dir):
            continue
        groups = dup_groups(case_dir)
        if groups:
            dup_cases += 1
        # path -> canonical id shared by every duplicate of the same content
        canon = {}
        for i, (_h, paths) in enumerate(groups.items()):
            for p in paths:
                canon[os.path.abspath(p)] = i

        text = open(f, errors="ignore").read()
        blocks = text.split("========== Mechanism-Rule Violation Detected ==========")[1:]
        for blk in blocks:
            # Only the node listing carries the pair under audit. The Description
            # line ends in a bracketed `[dedup: ... file.c:NNN]` note, and a
            # bracket-delimited search over the whole block matches that instead.
            body = blk.split("--- Involved Nodes ---", 1)
            if len(body) < 2:
                continue
            body = body[1].split("--- Verified Constraints ---", 1)[0]
            locs = re.findall(r"\[(/?[^\[\]]+?\.c):(\d+)\]", body)
            if len(locs) < 2:
                continue
            tot += 1
            (fa, la), (fb, lb) = locs[0], locs[1]
            pa = os.path.abspath(fa if fa.startswith("/") else "/" + fa)
            pb = os.path.abspath(fb if fb.startswith("/") else "/" + fb)
            if fa == fb and la == lb:
                same_line += 1
                per_case[case] += 1
            elif (pa in canon and pb in canon and canon[pa] == canon[pb]
                  and la == lb):
                dup_pair += 1
                per_case[case] += 1

    def pct(x):
        return f"{100*x/tot:.0f}%" if tot else "n/a"

    print(f"cases whose src/ holds duplicate copies : {dup_cases}")
    print(f"reports with two file:line sites        : {tot}")
    print(f"  same line via DUPLICATE file copies   : {dup_pair}  ({pct(dup_pair)})")
    print(f"  identical file:line on both sides     : {same_line}  ({pct(same_line)})")
    if per_case:
        print("\ntop cases:")
        for case, n in per_case.most_common(12):
            print(f"  {case:28s} {n}")


if __name__ == "__main__":
    main()
