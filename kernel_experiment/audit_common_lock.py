#!/usr/bin/env python3
"""Find reports that a common-lock argument can safely discharge.

The surface records, per access, the lock expressions in scope (`lock`). A report
is only safely suppressible when EVERY cross-thread pairing of its two sides is
covered by a shared MUTUALLY EXCLUSIVE lock. Two refinements matter, and skipping
either one turns real findings into suppressed ones:

  * `rcu_read_lock`, `local_bh_disable` and friends are not mutual exclusion. Two
    RCU read sections run concurrently by design, and bh-disable is per-CPU, so a
    shared entry of that kind says nothing about the pair.
  * Locks must not be unioned per node. A node reachable along two paths carries a
    different lockset on each, and intersecting the two unions reports a shared
    lock that no single execution ever holds.

    python3 audit_common_lock.py --snapshot lockfix_20260907_204407
"""
import argparse
import collections
import glob
import json
import os
import re

ROOT = os.path.dirname(os.path.abspath(__file__))
DUMP = os.path.join(os.path.dirname(ROOT), "LLM_dump")

# Primitives that appear in a lockset but grant no exclusion between two holders.
NON_EXCLUSIVE = {
    "rcu_read_lock", "rcu_read_unlock",
    "rcu_read_lock_bh", "rcu_read_lock_sched",
    "local_bh_disable", "local_bh_enable",
    "preempt_disable", "preempt_enable",
    "local_irq_save", "local_irq_disable",
    "srcu_read_lock", "get_cpu", "migrate_disable",
}


def lock_objects(expr):
    """Exclusive lock operands named by a `lock` field, e.g. '&ep->mtx' -> 'ep->mtx'.

    The field concatenates the acquisitions in scope with ' & ', each rendered as
    its original call, so the operand has to be pulled back out of the argument
    list. A call with no argument names a global context, not an object, and every
    such primitive we know of is non-exclusive.
    """
    out = set()
    if not expr:
        return out
    for part in expr.split(" & "):
        part = part.strip()
        m = re.match(r"([A-Za-z_]\w*)\s*\((.*)\)\s*$", part, re.S)
        if not m:
            continue
        callee, args = m.group(1), m.group(2).strip()
        if callee in NON_EXCLUSIVE:
            continue
        if not args:
            continue
        first = args.split(",")[0].strip()
        first = re.sub(r"^\(.*?\)\s*", "", first)  # drop a cast
        first = first.lstrip("&").strip()
        first = re.sub(r"\s+", "", first)
        if first:
            out.add(first)
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--snapshot", required=True)
    ap.add_argument("--judge", help="judge_results tag, to cross-check verdicts")
    args = ap.parse_args()
    snap = os.path.join(ROOT, "eval_snapshots", args.snapshot)

    dumps = {}
    for ln in open(os.path.join(snap, "dump_index.txt")):
        p = ln.split()
        if len(p) == 2 and p[1] != "-":
            dumps[p[0]] = os.path.join(DUMP, p[1])

    tot = safe = 0
    verdicts = collections.Counter()
    gt_hit = []
    per_case = collections.Counter()

    for f in sorted(glob.glob(os.path.join(snap, "bugs", "*.txt"))):
        case = os.path.basename(f)[:-4]
        sp = os.path.join(dumps.get(case, ""), "vulnerability_surface.json")
        if not os.path.isfile(sp):
            continue
        # node id -> list of (thread_id, exclusive lockset), one entry per access
        acc = collections.defaultdict(list)
        j = json.load(open(sp))
        for o in j.get("shared_objects", []):
            for a in o.get("accesses", []):
                nid = a.get("node_id")
                if nid is not None:
                    acc[nid].append((a.get("thread_id"), lock_objects(a.get("lock"))))

        jv, matched = {}, set()
        if args.judge:
            jf = os.path.join(ROOT, "judge_results", args.judge, case + ".json")
            if os.path.isfile(jf):
                jj = json.load(open(jf))
                jv = {r.get("id"): r.get("verdict") for r in jj.get("reports", [])}
                matched = set(jj.get("matching_report_ids") or [])

        text = open(f, errors="ignore").read()
        blocks = text.split("========== Mechanism-Rule Violation Detected ==========")[1:]
        for i, blk in enumerate(blocks, 1):
            ids = [int(x) for x in re.findall(r"node (\d+)\)", blk)]
            if len(ids) < 2 or ids[0] not in acc or ids[1] not in acc:
                continue
            tot += 1
            # Every way the two sides could be scheduled against each other has to
            # be covered; one uncovered interleaving is a real race.
            pairs = [(x, y) for x in acc[ids[0]] for y in acc[ids[1]]
                     if x[0] != y[0]]
            if not pairs or not all(x[1] & y[1] for x, y in pairs):
                continue
            safe += 1
            per_case[case] += 1
            verdicts[jv.get(i, "?")] += 1
            if i in matched:
                gt_hit.append((case, i))

    print(f"reports with both sides in the surface : {tot}")
    print(f"  every cross-thread pairing shares an exclusive lock : {safe}"
          f"  ({100*safe/tot:.0f}%)" if tot else "")
    if args.judge:
        print(f"  judged FP {verdicts['FP']} / TP {verdicts['TP']}"
              f" / unjudged {verdicts['?']}")
        print(f"  among GT-matching reports: {len(gt_hit)}"
              + ("  " + ", ".join(f"{c} r{i}" for c, i in gt_hit) if gt_hit else ""))
    if per_case:
        print("\ntop cases:")
        for case, n in per_case.most_common(12):
            print(f"  {case:28s} {n}")


if __name__ == "__main__":
    main()
