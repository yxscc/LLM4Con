#!/usr/bin/env python3
"""Locate where each ground-truth access is lost on its way to the surface.

Run the detector with LACE_SURFACE_DEBUG=1 and LACE_EARLY_EXIT_AFTER_SURFACE=1
first: it emits one ``[surface-acc]`` line per collected access, carrying the
group it landed in and whether that group survived the two-thread bar. Reading
those back tells us, for each ground-truth side, which of three things went
wrong:

  MISSING  never collected -- the access is not in the CCPG, or
           SharedFieldKey rejected it (stack-local, unresolvable root)
  DROPPED  collected and grouped, but the group had fewer than two threads
           and did not qualify as a self-race
  KEPT     on the surface

Usage:  diagnose_surface.py <case> ...     (default: every case with a log)
"""
import re
import sys
from collections import defaultdict
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from measure_surface import access_matches  # noqa: E402

HERE = Path(__file__).resolve().parent
LINE = re.compile(
    r"\[surface-acc\] survived=(\d) gkey=(\S+) tid=(-?\d+) type=(\w+) "
    r"fn=(\S+) code=(.*)$")


def latest_log(case: str):
    logs = sorted((HERE / case).glob("detection_manualentry_*.log"))
    return logs[-1] if logs else None


def parse(log: Path):
    out = []
    for ln in log.read_text(errors="ignore").splitlines():
        m = LINE.match(ln.strip())
        if m:
            out.append({
                "survived": m.group(1) == "1", "gkey": m.group(2),
                "tid": int(m.group(3)), "access_type": m.group(4),
                "containing_function": m.group(5), "code": m.group(6),
            })
    return out


def locate(accs, gt):
    """Return (state, gkeys) for one ground-truth side."""
    hits = [a for a in accs if access_matches(a, gt) >= 2]
    if not hits:
        return "MISSING", set()
    kept = {a["gkey"] for a in hits if a["survived"]}
    if kept:
        return "KEPT", kept
    return "DROPPED", {a["gkey"] for a in hits}


def main() -> int:
    import json
    cases = sys.argv[1:]
    if not cases:
        cases = sorted(p.parent.name for p in HERE.glob("*/flow_annotation.json"))

    tally = defaultdict(list)
    print(f"{'case':26s} {'side A':>9} {'side B':>9}  diagnosis")
    for case in cases:
        log = latest_log(case)
        if not log:
            continue
        accs = parse(log)
        if not accs:
            continue
        gt = json.loads((HERE / case / "flow_annotation.json").read_text())
        gt = gt.get("ground_truth_access") or {}
        sa, ka = locate(accs, gt.get("access_a") or {})
        sb, kb = locate(accs, gt.get("access_b") or {})
        if sa == "KEPT" and sb == "KEPT":
            verdict = "surfaced together" if ka & kb else "SPLIT across groups"
        elif "MISSING" in (sa, sb):
            verdict = "not collected"
        else:
            verdict = "dropped at the two-thread bar"
        tally[verdict].append(case)
        print(f"{case:26s} {sa:>9} {sb:>9}  {verdict}")

    print("\n" + "=" * 60)
    for k in sorted(tally, key=lambda x: -len(tally[x])):
        print(f"{len(tally[k]):>3}  {k}")
        for c in tally[k]:
            print(f"       {c}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
