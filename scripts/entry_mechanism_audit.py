#!/usr/bin/env python3
"""Mechanism-level audit of static thread/entry coverage.

Unlike static_capability_validator.py (which asks the narrow question "is the
annotated entry name a thread root?"), this script takes the high-level view the
project cares about: a *thread* is a launch mechanism + a reachable body + a
lifecycle, and what ultimately matters for recall is whether the two ends of the
annotated race land in two *may-happen-in-parallel* contexts.

For every flow_annotation.json it extracts, per side (thread_a/thread_b):
  - entry.function and entry.kind         (the launch mechanism)
  - bug_site.function / ground_truth access functions (the race ends)
  - call_chain present_in_bitcode flags   (does the IR even contain the path?)

It then cross-references the latest vulnerability_surface.json dump:
  - entry_is_root   : entry (or syscall alias) appears as a thread root
  - body_reachable  : the bug-site / access function appears in *some* thread's
                      accesses (i.e. it is actually inside the static graph)
  - pair_ok         : both sides' race functions are present and at least one of
                      them is reachable in a thread distinct from the other side

Output is grouped by entry.kind so systematic mechanism gaps are visible rather
than per-CVE noise.
"""

from __future__ import annotations

import argparse
import json
from collections import defaultdict
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any


SYSCALL_PREFIXES = [
    "__x64_sys_",
    "__ia32_sys_",
    "__arm64_sys_",
    "__se_sys_",
    "__do_sys_",
    "SyS_",
    "sys_",
]


def read_json(path: Path) -> Any:
    with path.open("r", encoding="utf-8", errors="ignore") as f:
        return json.load(f)


def syscall_aliases(name: str) -> list[str]:
    for prefix in SYSCALL_PREFIXES:
        if name.startswith(prefix):
            suffix = name[len(prefix):]
            return [p + suffix for p in SYSCALL_PREFIXES] + [name]
    return [name]


@dataclass
class Side:
    role: str
    entry_fn: str
    entry_kind: str
    race_fns: list[str]              # bug_site + ground-truth access functions
    chain_in_bitcode: bool           # all call-chain hops marked present_in_bitcode
    chain_fns: list[str] = field(default_factory=list)


@dataclass
class CaseAudit:
    case_id: str
    sides: list[Side]
    # surface-derived
    has_surface: bool = False
    total_threads: int | None = None
    thread_roots: list[str] = field(default_factory=list)
    body_fns: set[str] = field(default_factory=set)   # functions present in any thread body
    body_fn_to_threads: dict[str, set[int]] = field(default_factory=dict)


def collect_sides(data: dict[str, Any]) -> list[Side]:
    ti = data.get("true_interleaving", {})
    sides: list[Side] = []
    if not isinstance(ti, dict):
        return sides
    for key in ("thread_a", "thread_b", "thread_c", "thread_d"):
        th = ti.get(key)
        if not isinstance(th, dict):
            continue
        entry = th.get("entry", {}) if isinstance(th.get("entry"), dict) else {}
        entry_fn = entry.get("function", "") if isinstance(entry.get("function"), str) else ""
        entry_kind = entry.get("kind", "") if isinstance(entry.get("kind"), str) else ""

        race_fns: list[str] = []
        bug = th.get("bug_site", {})
        if isinstance(bug, dict) and isinstance(bug.get("function"), str):
            race_fns.append(bug["function"])

        chain = th.get("call_chain", [])
        chain_fns: list[str] = []
        chain_in_bitcode = True
        if isinstance(chain, list):
            for hop in chain:
                if not isinstance(hop, dict):
                    continue
                if isinstance(hop.get("function"), str):
                    chain_fns.append(hop["function"])
                if hop.get("present_in_bitcode") is False:
                    chain_in_bitcode = False

        sides.append(Side(
            role=th.get("role", key),
            entry_fn=entry_fn,
            entry_kind=entry_kind or "<none>",
            race_fns=race_fns,
            chain_in_bitcode=chain_in_bitcode,
            chain_fns=chain_fns,
        ))

    # Augment race functions with ground_truth access functions.
    gt = data.get("ground_truth_access", {})
    if isinstance(gt, dict):
        for side, akey in ((0, "access_a"), (1, "access_b")):
            acc = gt.get(akey, {})
            if isinstance(acc, dict) and isinstance(acc.get("function"), str):
                if side < len(sides):
                    sides[side].race_fns.append(acc["function"])
    return sides


def latest_surface(case_id: str, dump_dir: Path) -> Path | None:
    cands = sorted(
        dump_dir.glob(f"{case_id}_*/vulnerability_surface.json"),
        key=lambda p: p.stat().st_mtime,
        reverse=True,
    )
    return cands[0] if cands else None


def load_surface(audit: CaseAudit, surface_path: Path) -> None:
    data = read_json(surface_path)
    audit.has_surface = True
    audit.total_threads = data.get("total_threads")
    roots = []
    tid_to_entry: dict[int, str] = {}
    for t in data.get("threads", []):
        if isinstance(t, dict):
            ef = str(t.get("entry_function", ""))
            roots.append(ef)
            if isinstance(t.get("thread_id"), int):
                tid_to_entry[t["thread_id"]] = ef
    audit.thread_roots = roots

    for obj in data.get("shared_objects", []):
        if not isinstance(obj, dict):
            continue
        for acc in obj.get("accesses", []):
            if not isinstance(acc, dict):
                continue
            tid = acc.get("thread_id")
            for fkey in ("function", "containing_function"):
                fn = acc.get(fkey)
                if isinstance(fn, str) and fn:
                    audit.body_fns.add(fn)
                    if isinstance(tid, int):
                        audit.body_fn_to_threads.setdefault(fn, set()).add(tid)
    # Thread root entries are also "in the graph".
    for ef in roots:
        if ef:
            audit.body_fns.add(ef)


def entry_is_root(entry_fn: str, roots: list[str]) -> bool:
    if not entry_fn:
        return False
    rootset = set(roots)
    return any(a in rootset for a in syscall_aliases(entry_fn))


def side_body_reachable(side: Side, audit: CaseAudit) -> bool:
    return any(fn in audit.body_fns for fn in side.race_fns if fn)


def diagnose(audit: CaseAudit) -> dict[str, Any]:
    """Return a per-case mechanism diagnosis dict."""
    out: dict[str, Any] = {"case_id": audit.case_id, "sides": []}
    if not audit.has_surface:
        out["status"] = "NO_SURFACE"
    for side in audit.sides:
        is_root = entry_is_root(side.entry_fn, audit.thread_roots)
        reachable = side_body_reachable(side, audit)
        out["sides"].append({
            "role": side.role,
            "entry_fn": side.entry_fn,
            "kind": side.entry_kind,
            "entry_is_root": is_root,
            "body_reachable": reachable,
            "chain_in_bitcode": side.chain_in_bitcode,
            "race_fns": side.race_fns,
        })

    if audit.has_surface:
        roots_ok = all(entry_is_root(s.entry_fn, audit.thread_roots) for s in audit.sides if s.entry_fn)
        body_ok = all(side_body_reachable(s, audit) for s in audit.sides)
        # Distinct-thread check: the union of threads that host each side's race
        # functions must contain >= 2 distinct thread ids.
        host_threads: set[int] = set()
        per_side_threads: list[set[int]] = []
        for s in audit.sides:
            tids: set[int] = set()
            for fn in s.race_fns:
                tids |= audit.body_fn_to_threads.get(fn, set())
            per_side_threads.append(tids)
            host_threads |= tids
        distinct_ok = len(host_threads) >= 2 or any(
            len(per_side_threads[i] - per_side_threads[j]) > 0
            for i in range(len(per_side_threads))
            for j in range(len(per_side_threads)) if i != j
        )

        if roots_ok and body_ok:
            out["status"] = "PASS"
        elif body_ok and distinct_ok:
            out["status"] = "BODY_OK_ROOT_MISMATCH"   # reachable but entry not a literal root
        elif body_ok:
            out["status"] = "BODY_OK_SINGLE_CONTEXT"   # reachable but not in 2 distinct threads
        else:
            out["status"] = "BODY_MISS"                 # race fn not in graph at all
    return out


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--kernel-experiment", type=Path, default=Path("kernel_experiment"))
    ap.add_argument("--dump-dir", type=Path, default=Path("LLM_dump"))
    ap.add_argument("--case", action="append", default=[])
    ap.add_argument("--json-out", type=Path)
    ap.add_argument("--show-cases", action="store_true")
    args = ap.parse_args()

    anns = sorted(args.kernel_experiment.glob("*/flow_annotation.json"))
    if args.case:
        wanted = set(args.case)
        anns = [p for p in anns if p.parent.name in wanted]

    diagnoses: list[dict[str, Any]] = []
    # mechanism kind -> counters
    kind_stats: dict[str, dict[str, int]] = defaultdict(lambda: defaultdict(int))

    for ann in anns:
        data = read_json(ann)
        case_id = data.get("bug_id") or ann.parent.name
        audit = CaseAudit(case_id=case_id, sides=collect_sides(data))
        sp = latest_surface(case_id, args.dump_dir)
        if sp:
            load_surface(audit, sp)
        diag = diagnose(audit)
        diagnoses.append(diag)

        for sd in diag["sides"]:
            k = sd["kind"]
            kind_stats[k]["total"] += 1
            if sd["entry_is_root"]:
                kind_stats[k]["entry_is_root"] += 1
            if sd["body_reachable"]:
                kind_stats[k]["body_reachable"] += 1
            if not audit.has_surface:
                kind_stats[k]["no_surface"] += 1

    status_counts: dict[str, int] = defaultdict(int)
    for d in diagnoses:
        status_counts[d.get("status", "?")] += 1

    summary = {
        "total_cases": len(diagnoses),
        "by_status": dict(sorted(status_counts.items())),
        "by_kind": {
            k: dict(v) for k, v in sorted(
                kind_stats.items(),
                key=lambda kv: kv[1].get("total", 0),
                reverse=True,
            )
        },
    }
    print(json.dumps(summary, indent=2))

    if args.show_cases:
        print("\n=== CASE DETAIL ===")
        for d in sorted(diagnoses, key=lambda x: x.get("status", "")):
            print(f"\n[{d.get('status')}] {d['case_id']}")
            for sd in d["sides"]:
                print(f"   {sd['role']:<28} kind={sd['kind']:<26} "
                      f"root={sd['entry_is_root']!s:<5} body={sd['body_reachable']!s:<5} "
                      f"entry={sd['entry_fn']}")

    if args.json_out:
        args.json_out.parent.mkdir(parents=True, exist_ok=True)
        args.json_out.write_text(
            json.dumps({"summary": summary, "diagnoses": diagnoses}, indent=2),
            encoding="utf-8",
        )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
