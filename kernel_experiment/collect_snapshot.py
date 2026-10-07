#!/usr/bin/env python3
"""Collect one run's per-case artifacts into eval_snapshots/<tag>/.

A run leaves its reports in LLM_dump/<case>_<stamp>/stateful_bugs/bugs.txt and its
console output in <case>/detection_manualentry_<stamp>.log. Judging needs both in
one place, keyed by case, so this mirrors the layout the paper snapshot uses:

    eval_snapshots/<tag>/bugs/<case>.txt      reports, or absent when none
    eval_snapshots/<tag>/run_summary.txt      one line per case
    eval_snapshots/<tag>/dump_index.txt       case -> dump dir actually used

Cases are matched to dumps by "newest dump at or after --since", so a case rerun
after a fix picks up the rerun rather than the original attempt.
"""
import argparse
import datetime as dt
import os
import re
import shutil

ROOT = os.path.dirname(os.path.abspath(__file__))
DUMP = os.path.join(os.path.dirname(ROOT), "LLM_dump")


def parse_since(s):
    return dt.datetime.strptime(s, "%Y-%m-%d %H:%M").timestamp()


def newest_dump(case, since):
    """Newest LLM_dump dir for `case` whose reports were written after `since`.

    Dump dirs are named <case>_<stamp>, and a case name can be a prefix of
    another (CVE-2024-3597 vs CVE-2024-35977), so anchor on the underscore.
    """
    best, best_t = None, -1.0
    if not os.path.isdir(DUMP):
        return None
    for d in os.listdir(DUMP):
        if not d.startswith(case + "_"):
            continue
        p = os.path.join(DUMP, d)
        t = os.path.getmtime(p)
        if t >= since and t > best_t:
            best, best_t = p, t
    return best


def newest_log(case, since):
    d = os.path.join(ROOT, case)
    if not os.path.isdir(d):
        return None
    best, best_t = None, -1.0
    for f in os.listdir(d):
        if not f.startswith("detection_manualentry_"):
            continue
        p = os.path.join(d, f)
        t = os.path.getmtime(p)
        if t >= since and t > best_t:
            best, best_t = p, t
    return best


def summarize(log):
    """One summary line per case, in the same shape as the paper snapshot."""
    t = open(log, errors="ignore").read()

    def grab(pat, cast=int, default=None):
        m = re.search(pat, t)
        return cast(m.group(1)) if m else default

    bugs = grab(r"Bug detection complete\. (\d+) potential bug")
    objs = grab(r"Found (\d+) shared objects")
    thr = grab(r"vulnerability surface for (\d+) threads")
    cand = grab(r"\[L2\] undischarged_total=(\d+)")
    kept = grab(r"kept_after_calibration=(\d+)")
    secs = grab(r"Total Time: (\d+) seconds")
    reqs = grab(r"LLM API Requests: (\d+)")
    toks = grab(r"Total Tokens: (\d+)")
    # The bug-count line is only printed when the checker reaches its reporting
    # step; a case that ends with nothing to report skips it but still logs the
    # closing banner. Only the absence of BOTH means the run died.
    if bugs is None:
        verdict = ("CLEAN/DONE" if "LLM-guided analysis complete" in t
                   else "INCOMPLETE")
    elif bugs > 0:
        verdict = "FOUND"
    else:
        verdict = "CLEAN/DONE"

    def f(name, v):
        return f"{name}={'?' if v is None else v}"

    return " ".join([
        f"{verdict:12s}",
        f("threads", thr), f("objs", objs),
        f("cand", cand), f("kept", kept), f("bugs", bugs),
        f("reqs", reqs), f("tokens", toks),
        (f"{secs}s" if secs is not None else "?s"),
    ])


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--tag", required=True, help="snapshot dir name under eval_snapshots/")
    ap.add_argument("--since", required=True, help='earliest run time, "YYYY-MM-DD HH:MM"')
    ap.add_argument("--cases", help="file with one case name per line; default: all case dirs")
    args = ap.parse_args()

    since = parse_since(args.since)
    if args.cases:
        cases = [l.strip() for l in open(args.cases) if l.strip()]
    else:
        cases = sorted(d for d in os.listdir(ROOT)
                       if os.path.isdir(os.path.join(ROOT, d))
                       and re.match(r"^(CVE|SYZBOT)-", d))

    out = os.path.join(ROOT, "eval_snapshots", args.tag)
    os.makedirs(os.path.join(out, "bugs"), exist_ok=True)
    summary, index, missing = [], [], []

    for case in cases:
        log = newest_log(case, since)
        if log is None:
            missing.append(case)
            summary.append(f"{case:24s} NO-RUN")
            continue
        summary.append(f"{case:24s} {summarize(log)}")
        dump = newest_dump(case, since)
        if dump is None:
            index.append(f"{case:24s} -")
            continue
        index.append(f"{case:24s} {os.path.basename(dump)}")
        bugs = os.path.join(dump, "stateful_bugs", "bugs.txt")
        if os.path.isfile(bugs) and os.path.getsize(bugs) > 0:
            shutil.copyfile(bugs, os.path.join(out, "bugs", case + ".txt"))

    open(os.path.join(out, "run_summary.txt"), "w").write("\n".join(summary) + "\n")
    open(os.path.join(out, "dump_index.txt"), "w").write("\n".join(index) + "\n")
    n = len(os.listdir(os.path.join(out, "bugs")))
    print(f"snapshot: {out}")
    print(f"  cases: {len(cases)}   with reports: {n}   no run since --since: {len(missing)}")
    if missing:
        print("  missing: " + " ".join(missing))


if __name__ == "__main__":
    main()
