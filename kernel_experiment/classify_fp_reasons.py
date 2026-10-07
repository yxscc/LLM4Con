#!/usr/bin/env python3
"""Rank false-positive causes by reading the judges' per-report reasons.

Mechanical audits of the reports can only check what the surface records, and for
a self-race case the surface duplicates every access under a second thread id, so
"are the two sides in different threads" answers yes vacuously. The judges read
the source instead, so their reasons are the better measurement -- this bins them
into causes so the causes can be ranked by how many reports they explain.

    python3 classify_fp_reasons.py --snapshot final_20260907
"""
import argparse
import collections
import glob
import json
import os
import re

ROOT = os.path.dirname(os.path.abspath(__file__))

# Ordered: the first pattern that matches wins, so put the specific before the
# general (a caller-held lock is a locking miss, not a generic "already ordered").
CLASSES = [
    # A lock name in these reasons is written as an expression, e.g. "under
    # acct->lock" or "inside binder_inner_proc_lock", so the name pattern has to
    # allow `->`, `.` and `_` rather than a single separator character.
    ("read/read pair",
     r"read/read|both (?:sides |of them )?(?:are |sites are )?reads|"
     r"read-only|two reads|both .{0,24}\bare reads\b|cannot race.{0,24}read"),
    ("READ_ONCE / WRITE_ONCE / data_race annotated",
     r"READ_ONCE|WRITE_ONCE|data_race\(|ONCE annotation|already annotated"),
    ("lock held (incl. caller-held)",
     r"lock is (?:actually )?held|(?:under|inside|within|hold(?:s|ing)?|held)"
     r"[^.;]{0,40}?\b[\w.>-]*(?:lock|mutex|sem|_lk)\b|"
     r"\b[\w.>-]*(?:lock|mutex)\b[^.;]{0,30}(?:covers|protects|serializ)|"
     r"caller[^.;]{0,40}(?:holds|held|lock)|lock_sock|_ilocked|"
     r"mutex_lock|spin_lock|read_lock|write_lock|rwlock|inherited"),
    ("RCU / refcount / atomic protocol",
     r"\brcu\b|call_rcu|synchronize_rcu|refcount|atomic_dec_and_test|kref|"
     r"refcnt|not_zero|srcu"),
    ("same statement / sub-expression pair",
     r"sub-?expression|same statement|one statement|chained assignment|"
     r"same line|nested|two statements earlier"),
    ("object not shared yet (unpublished / private)",
     r"not yet published|unpublished|freshly (?:k?z?alloc|allocated)|"
     r"k?z?alloc(?:'d|ed)? |thread-private|stack local|caller-supplied|"
     r"not (?:yet )?(?:visible|runnable|reachable|exist)|before publication|"
     r"probe[- ]time|still owns|exclusively|only reachable through|"
     r"cannot run before|before the worker|not yet exist|does not yet exist"),
    ("contexts cannot overlap (init / drain)",
     r"cannot overlap|one-?time init|drain|quiesce|unregister|teardown only|"
     r"never concurrent|unreachable until|after .{0,30}are gone|"
     r"pernet exit|mutually exclu|cannot be concurrent"),
    ("program-ordered in one thread",
     r"program[- ]order|sequential path|straight-?line|adjacent statement|"
     r"consecutive (?:line|statement)|single(?:-| )threaded path|same path|"
     r"one path|same thread|same .{0,24}(?:context|activation)|"
     r"precedes|happens before|on the next line"),
    ("different objects, same type/field name",
     r"different (?:object|instance|struct|skb|urb|header|allocation)|"
     r"distinct (?:object|instance|allocation)|not the same storage|"
     r"same field name|merely share|different fields"),
    ("not an access (label / decl / macro / call node)",
     r"not an access|case label|declaration|macro artifact|"
     r"function[- ]call node|call node|WARN_ON|mis-?attribut"),
    ("advisory value",
     r"advisory|benign|stale value (?:is )?(?:fine|harmless)|"
     r"cannot change behaviour"),
]


def classify(text):
    t = text or ""
    for name, pat in CLASSES:
        if re.search(pat, t, re.I):
            return name
    return "unclassified"


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--snapshot", required=True)
    ap.add_argument("--show-unclassified", action="store_true")
    args = ap.parse_args()

    res = os.path.join(ROOT, "judge_results", args.snapshot)
    counts = collections.Counter()
    per_class_cases = collections.defaultdict(set)
    unclassified = []
    tp = fp = 0
    for f in sorted(glob.glob(os.path.join(res, "*.json"))):
        try:
            j = json.load(open(f))
        except Exception:
            continue
        case = j.get("case", os.path.basename(f)[:-5])
        for r in j.get("reports", []):
            if r.get("verdict") == "TP":
                tp += 1
                continue
            fp += 1
            c = classify(r.get("reason", ""))
            counts[c] += 1
            per_class_cases[c].add(case)
            if c == "unclassified":
                unclassified.append(f"{case} #{r.get('id')}: {r.get('reason','')[:110]}")

    print(f"judged reports: {tp+fp}   TP {tp}   FP {fp}")
    print(f"\nFP causes ({fp} reports):")
    for c, n in counts.most_common():
        print(f"  {n:4d}  ({100*n/max(1,fp):4.1f}%)  {c}   [{len(per_class_cases[c])} cases]")
    if args.show_unclassified and unclassified:
        print(f"\nunclassified ({len(unclassified)}):")
        for u in unclassified[:40]:
            print("  " + u)


if __name__ == "__main__":
    main()
