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


_DEREF = re.compile(r"[a-z_][a-z0-9_]*(?:\s*->\s*[a-z_][a-z0-9_]*)+")


def _deref_chains(code: str) -> set:
    return {re.sub(r"\s*->\s*", "->", m) for m in _DEREF.findall(code)}


def access_matches(acc: dict, gt: dict) -> int:
    """0 = no match, 1 = function only, 2 = function plus kind, 3 = plus code.

    Matching on the annotated function name alone systematically undercounts:
    the ground truth names the function as written, but small static helpers
    are inlined, and then the access reports its inlining parent instead. So
    an access also matches when its code carries a field-dereference chain
    from the annotation, whatever function it now sits in.

    Code text is the strongest signal but is often unavailable: frees in
    particular are recorded synthetically (``[ir-fallback] free via kfree``)
    and never share text with the annotated ``kfree(data);``.
    """
    gc, ac = norm_code(gt.get("code", "")), norm_code(acc.get("code", ""))
    fn = (gt.get("function") or "").strip()
    same_fn = bool(fn) and fn in (acc.get("containing_function"),
                                  acc.get("function"))
    if gc and ac and (gc in ac or ac in gc):
        return 3
    if not same_fn:
        chains = _deref_chains(gc) & _deref_chains(ac)
        return 3 if chains else 0
    return 2 if kind_compatible(acc, gt) else 1


_STRUCT = re.compile(r"struct\s+(\w+)")


def object_present(objs, gt):
    """Is the ground-truth *object* on the surface, touched by two threads?

    The pair metric asks where two annotated access sites landed, which the
    annotation style keeps breaking: ground truth names the call site while
    the analyzer records the access inside the callee, and inlining rewrites
    function names underneath both. Since objects are now grouped at the
    owning struct, asking for the object directly sidesteps all of that. It
    is the coarser question -- the right struct type, not the right field --
    so read it as an upper bound on what the surface can support.

    Returns (found, rank) with rank 1-based, or (False, 0).
    """
    # Reading the struct name out of the annotation's prose was missing
    # objects that are demonstrably on the surface: it only fires when the
    # prose happens to spell the same struct the analyzer keyed on. The two
    # annotated sites do name their containing functions, so ask first
    # whether some multi-thread object carries accesses from both of them --
    # that is the condition the contract stage actually needs, and it does
    # not depend on how the object was described.
    want = {(gt.get(s) or {}).get("function") for s in ("access_a", "access_b")}
    want.discard(None)
    names = {f"struct.{m}" for m in _STRUCT.findall(gt.get("object", ""))}
    chains = _deref_chains(norm_code(gt.get("object", "")))
    if not want and not names and not chains:
        return False, 0
    for i, o in enumerate(objs):
        accesses = o.get("accesses", [])
        if len(o.get("accessing_thread_ids") or
               {a.get("thread_id") for a in accesses}) < 2:
            continue
        if want and want <= {a.get("containing_function") for a in accesses}:
            return True, i + 1
        nm = o.get("name", "")
        if any(n in nm for n in names):
            return True, i + 1
        if chains and any(chains & _deref_chains(norm_code(a.get("code", "")))
                          for a in accesses):
            return True, i + 1
    return False, 0


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
            rows.append((case, None, "NO-RUN", 0, 0, False, None))
            continue
        sf = d / "vulnerability_surface.json"
        objs = []
        if sf.exists():
            try:
                objs = json.loads(sf.read_text()).get("shared_objects", [])
            except json.JSONDecodeError:
                pass
        verdict, rank, touched = classify(objs, gt_a, gt_b, need)
        found, orank = object_present(objs, gt)
        rows.append((case, len(objs), verdict, rank, touched,
                     "yes" if found else "no", orank))

    rows.sort(key=lambda r: (r[2], -(r[1] or 0)))
    print(f"{'case':26s} {'objs':>5} {'pair':>8} {'rank':>5} {'obj?':>5} {'rank':>5}")
    for case, n, v, rank, touched, found, orank in rows:
        print(f"{case:26s} {('-' if n is None else n):>5} {v:>8} "
              f"{(rank or '-'):>5} {found:>5} {(orank or '-'):>5}")

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
    objfound = [r for r in ran if r[5] == "yes"]
    print(f"GT object on the surface  : {len(objfound)}/{len(ran)}", end="")
    if objfound:
        oranks = sorted(r[6] for r in objfound)
        print(f"  (median rank {oranks[len(oranks) // 2]}, "
              f"top-10 {sum(1 for x in oranks if x <= 10)}/{len(oranks)})")
    else:
        print()
    sizes = sorted(r[1] for r in ran)
    if sizes:
        print(f"objects per case         : median {sizes[len(sizes) // 2]}, "
              f"max {sizes[-1]}, total {sum(sizes)}, "
              f"zero-object cases {sum(1 for x in sizes if x == 0)}")

    if args.csv:
        with open(args.csv, "w", newline="") as fh:
            w = csv.writer(fh)
            w.writerow(["case", "objects", "verdict", "gt_rank", "gt_objects",
                        "gt_object_found", "gt_object_rank"])
            w.writerows(rows)
        print(f"\nwrote {args.csv}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
