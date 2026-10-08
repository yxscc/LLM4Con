"""Score lace3 entry discovery against the 72-case hand-configured roots.

Same protocol as kernel_experiment/eval_entry_rule.py so the two are directly
comparable: the same bitcode per case, the same name matching (syscall
wrappers match their core name), and a ground-truth root counts as covered
when it is a discovered entry or reachable from one.

Usage:
    python -m lace3.eval.entries72 [--case NAME] [--misses]
"""

import argparse
import json
import sys
import time
from pathlib import Path

from lace3.entries.discover import discover_entries, syscall_core, _reachable
from lace3.ir.module import load_program

REPO = Path(__file__).resolve().parents[2]
CASES = REPO / "kernel_experiment"


def pick_bitcode(case_dir):
    lls = sorted(case_dir.glob("*.ll"))
    names = [p.name for p in lls]
    if "merged.ll" in names:
        return case_dir / "merged.ll"
    if "snd-seq.ll" in names:
        return case_dir / "snd-seq.ll"
    return max(lls, key=lambda p: p.stat().st_size) if lls else None


def names_match(a, b):
    if a == b:
        return True
    ca, cb = syscall_core(a), syscall_core(b)
    return (ca is not None and (ca == cb or ca == b)) or (cb is not None and cb == a)


def equiv(prog, name):
    out = {name}
    if name in prog.aliases:
        out.add(prog.aliases[name])
    out |= {a for a, t in prog.aliases.items() if t == name}
    return out


def score_case(prog, gt_roots, self_race, prune):
    es = discover_entries(prog)
    roots = [e for e in es.entries if not (prune and e.reachable_from)]
    root_fns = {e.function for e in roots}
    pool_d = set()
    for e in roots:
        pool_d |= equiv(prog, e.function.name)
    reach_of = {fn: _reachable(prog, fn) for fn in root_fns}
    pool_r = set()
    for fns in reach_of.values():
        for fn in fns:
            pool_r |= equiv(prog, fn.name)

    scored = [g for g in gt_roots if any(prog.is_defined(n) for n in equiv(prog, g))
              or any(names_match(g, f.name) for f in prog.functions)]
    hit_d = [g for g in scored if any(names_match(g, p) for p in pool_d)]
    hit_r = [g for g in scored if any(names_match(g, p) for p in pool_r)]

    pairable = None
    if len(scored) >= 2 and not self_race:
        owners = []
        for g in scored:
            owners.append({fn for fn, fns in reach_of.items()
                           if any(names_match(g, n) for x in fns for n in equiv(prog, x.name))})
        pairable = all(owners) and not (len(owners) == 2 and len(owners[0]) == 1
                                        and owners[0] == owners[1])
    missed = [g for g in scored if g not in hit_r]
    return dict(scored=len(scored), unmatchable=len(gt_roots) - len(scored),
                direct=len(hit_d), reach=len(hit_r), roots=len(roots),
                pairable=pairable, missed=missed)


def main(argv=None):
    ap = argparse.ArgumentParser()
    ap.add_argument("--case")
    ap.add_argument("--misses", action="store_true")
    args = ap.parse_args(argv)

    gt_all = json.loads((CASES / "dataset_entrypoints.json").read_text())
    cases = [args.case] if args.case else sorted(gt_all)
    results, failed = {}, {}
    t0 = time.time()
    for case in cases:
        bc = pick_bitcode(CASES / case) if (CASES / case).is_dir() else None
        if bc is None:
            failed[case] = "no bitcode"
            continue
        try:
            prog = load_program([bc])
        except Exception as e:
            failed[case] = f"{type(e).__name__}: {str(e)[:160]}"
            continue
        gt = gt_all[case]["thread_roots"]
        sr = gt_all[case].get("self_race")
        results[case] = {p: score_case(prog, gt, sr, p) for p in (False, True)}

    print(f"cases scored {len(results)}/{len(cases)} in {time.time() - t0:.0f}s")
    for case, why in failed.items():
        print(f"  not loaded: {case}: {why}")
    for prune, label in ((False, "all facts"), (True, "+prune reachable")):
        rs = [r[prune] for r in results.values()]
        scored = sum(r["scored"] for r in rs)
        direct = sum(r["direct"] for r in rs)
        reach = sum(r["reach"] for r in rs)
        roots = sum(r["roots"] for r in rs) / max(len(rs), 1)
        pa = [r["pairable"] for r in rs if r["pairable"] is not None]
        print(f"{label:<18} root-hit {direct}/{scored}  +reach {reach}/{scored} "
              f"({reach / max(scored, 1) * 100:.1f}%)  roots/case {roots:.1f}  "
              f"pairable {sum(pa)}/{len(pa)}  unmatchable GT {sum(r['unmatchable'] for r in rs)}")
    if args.misses:
        print("\nmissed GT roots (all facts):")
        for case, r in sorted(results.items()):
            for g in r[False]["missed"]:
                print(f"  {case:<28} {g}")
    return 0 if not failed else 1


if __name__ == "__main__":
    sys.exit(main())
