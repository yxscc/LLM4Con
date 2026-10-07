#!/usr/bin/env python3
"""Timestamp pipeline milestones while a detection run is in flight.

The detector's log records which phase and session it reached but not when, so
the only way to see where wall time goes is to watch the file as it grows.
Every new milestone line is appended to <log>.timing.tsv with the seconds since
that log first appeared.

Usage:
    python3 phase_timer.py <stamp> [--interval 3]
"""

import argparse
import re
import time
from pathlib import Path

HERE = Path(__file__).resolve().parent

MILESTONES = re.compile(
    r"\[(Phase \d[^\]]*|Phase [ABC][^\]]*|L2 session \d+/\d+|contract-parallel[^\]]*|"
    r"contract-thread-budget|session-budget|phaseA-b0|dedup)\]|"
    r"Threads: \d+, Conflicting pairs|"
    r"Static-Composition Analysis Finished|"
    r"Total Time:")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("stamp", help="log stamp, e.g. 20260906_180000")
    ap.add_argument("--interval", type=float, default=3.0)
    ap.add_argument("--idle-stop", type=float, default=300.0,
                    help="stop after this many seconds with no new output")
    args = ap.parse_args()

    logs = sorted(HERE.glob(f"*/detection_manualentry_{args.stamp}.log"))
    while not logs:
        time.sleep(args.interval)
        logs = sorted(HERE.glob(f"*/detection_manualentry_{args.stamp}.log"))
    print(f"watching {len(logs)} log(s)", flush=True)

    t0 = {p: time.time() for p in logs}
    seen = {p: 0 for p in logs}
    out = {p: (p.parent / f"{p.stem}.timing.tsv").open("w") for p in logs}
    for p in logs:
        out[p].write("elapsed_s\tdelta_s\tmilestone\n")
    last = {p: time.time() for p in logs}
    prev = {p: 0.0 for p in logs}

    while True:
        active = False
        for p in logs:
            try:
                lines = p.read_text(errors="ignore").splitlines()
            except OSError:
                continue
            if len(lines) <= seen[p]:
                continue
            now = time.time()
            for line in lines[seen[p]:]:
                if not MILESTONES.search(line):
                    continue
                el = now - t0[p]
                out[p].write(f"{el:.1f}\t{el - prev[p]:.1f}\t{line.strip()[:150]}\n")
                prev[p] = el
            out[p].flush()
            seen[p] = len(lines)
            last[p] = now
            active = True
        if not active and all(time.time() - t > args.idle_stop for t in last.values()):
            break
        time.sleep(args.interval)

    for f in out.values():
        f.close()
    print("done", flush=True)


if __name__ == "__main__":
    main()
