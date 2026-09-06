#!/usr/bin/env python3
"""Score candidate thread-entry discovery rules against the 72-case dataset.

The dataset ships hand-configured thread roots in dataset_entrypoints.json;
those are treated as ground truth here. Each rule is applied directly to the
case bitcode so that variants can be compared without rebuilding the detector.

Five independent facts are extracted per module, none of which needs a
per-subsystem name table:

  A  the address appears in a global variable initializer (ops/handler tables)
  E  the address appears in an EXPORT_SYMBOL marker global
  B  the address is passed to a callee this module only declares, so the
     pointer leaves the translation unit (call_rcu, timer_init_key, ...)
  S  the address is written into memory by a store, which is how the macro
     initialisers register a callback on a heap object (INIT_WORK sets
     work->func, so the callback never appears in a global or an argument)
  C  the name matches the syscall wrapper convention that SYSCALL_DEFINE
     emits kernel-wide (sys_*, SyS_*, __x64_sys_*, ...)
  D  the function has external linkage and no intra-module caller, so some
     other translation unit must be calling it

A ground-truth root counts as covered when it is itself a discovered root or
when it is reachable from one along the intra-module call graph, since thread
analysis explores outward from its root.

Usage:
    python3 eval_entry_rule.py [--detail] [--case NAME] [--depth N]
"""

import argparse
import json
import re
from collections import defaultdict, deque
from pathlib import Path

HERE = Path(__file__).resolve().parent

BOOKKEEPING_PREFIXES = ("llvm.", "__kstrtab", "____versions", "__func__", ".str")
DEAD_SECTIONS = (".discard", ".debug", ".note", ".modinfo", "llvm.metadata")
EXPORT_PREFIXES = ("__ksymtab", "__addressable_", "__UNIQUE_ID___addressable_")
SYSCALL_PREFIXES = ("__x64_sys_", "__ia32_sys_", "__arm64_sys_", "__se_sys_",
                    "__do_sys_", "__sys_", "SyS_", "sys_")


def syscall_core(name):
    for p in SYSCALL_PREFIXES:
        if name.startswith(p) and len(name) > len(p):
            return name[len(p):]
    return None


def names_match(a, b):
    if a == b:
        return True
    ca, cb = syscall_core(a), syscall_core(b)
    if ca is not None and cb is not None and ca == cb:
        return True
    if ca is not None and ca == b:
        return True
    if cb is not None and cb == a:
        return True
    return False


def pick_bitcode(case_dir):
    lls = sorted(case_dir.glob("*.ll"))
    names = [p.name for p in lls]
    if "merged.ll" in names:
        return case_dir / "merged.ll"
    if "snd-seq.ll" in names:
        return case_dir / "snd-seq.ll"
    if not lls:
        return None
    return max(lls, key=lambda p: p.stat().st_size)


RE_DEFINE = re.compile(r"^define\b([^@\n]*)@([A-Za-z0-9_.$]+)\(", re.M)
RE_GLOBAL = re.compile(r"^@([A-Za-z0-9_.$]+) = (.*)$", re.M)
RE_CALL = re.compile(r"\b(?:call|invoke)\b[^@\n]*@([A-Za-z0-9_.$]+)\((.*)$")
RE_STORE = re.compile(r"^\s*store\b")
RE_FNREF = re.compile(r"@([A-Za-z0-9_.$]+)")
LOCAL_LINKAGE = ("internal", "private")

# Sentinel: score the roots the shipped detector actually discovered.
SHIPPED = frozenset()


class Module:
    def __init__(self, path):
        text = path.read_text(errors="ignore")

        self.defined = {}          # name -> has external linkage
        for attrs, name in RE_DEFINE.findall(text):
            self.defined[name] = not any(k in attrs.split() for k in LOCAL_LINKAGE)

        self.alias_of = {}
        globals_ = []
        for name, body in RE_GLOBAL.findall(text):
            if " alias " in body:
                tgt = [t for t in RE_FNREF.findall(body) if t in self.defined]
                if tgt:
                    self.alias_of[name] = tgt[0]
                continue
            globals_.append((name, body))

        # A / E: address stored in a global initializer.
        self.fact_global = defaultdict(set)
        self.fact_export = defaultdict(set)
        for name, body in globals_:
            if name.startswith(BOOKKEEPING_PREFIXES):
                continue
            if any(f'section "{s}' in body for s in DEAD_SECTIONS):
                continue
            bucket = (self.fact_export if name.startswith(EXPORT_PREFIXES)
                      else self.fact_global)
            for f in set(RE_FNREF.findall(body)):
                if f in self.defined:
                    bucket[f].add(name)

        # B: address handed to a callee this module does not define.
        # Also build the intra-module call graph while walking the body.
        self.fact_extern = defaultdict(set)
        self.fact_store = defaultdict(set)
        self.callees = defaultdict(set)
        self.called_directly = set()
        current = None
        for line in text.splitlines():
            if line.startswith("define"):
                m = RE_DEFINE.match(line)
                current = m.group(2) if m else None
                continue
            if RE_STORE.search(line):
                for f in set(RE_FNREF.findall(line)):
                    if f in self.defined:
                        self.fact_store[f].add(current or "?")
            m = RE_CALL.search(line)
            if not m:
                continue
            callee, argtext = m.group(1), m.group(2)
            self.called_directly.add(callee)
            if current and callee in self.defined:
                self.callees[current].add(callee)
            if callee in self.defined or callee.startswith("llvm."):
                continue
            for f in set(RE_FNREF.findall(argtext)):
                if f in self.defined and f != callee:
                    self.fact_extern[f].add(callee)

        # C: syscall naming convention.
        self.fact_syscall = {f for f in self.defined if syscall_core(f)}
        # D: externally linked and never called here.
        self.fact_orphan = {f for f, ext in self.defined.items()
                            if ext and f not in self.called_directly}

    def roots(self, use, prune="none"):
        """Discovered roots. `prune` drops roots that already sit inside
        another root's slice, which means they are not independent threads.

          weak  only D-derived roots are dropped this way; roots carrying
                positive registration evidence are always kept
          all   any root reachable from another root is dropped
        """
        out = self._facts_union(use)
        if prune == "weak" and isinstance(use, str) and "D" in use:
            anchored = self._facts_union(use.replace("D", ""))
            inside = self.reachable(anchored, None) - anchored
            out = anchored | {f for f in out - anchored if f not in inside}
        elif prune == "all":
            inside = set()
            for r in out:
                inside |= (self.reachable({r}, None) - {r}) & out
            out = out - inside
        return out

    def _facts_union(self, use):
        if isinstance(use, (set, frozenset)):
            return {f for f in use if f in self.defined}
        out = set()
        if "A" in use:
            out |= set(self.fact_global)
        if "E" in use:
            out |= set(self.fact_export)
        if "B" in use:
            out |= set(self.fact_extern)
        if "S" in use:
            out |= set(self.fact_store)
        if "C" in use:
            out |= self.fact_syscall
        if "D" in use:
            out |= self.fact_orphan
        return out

    def reachable(self, roots, depth):
        seen = set(roots)
        frontier = deque((r, 0) for r in roots)
        while frontier:
            fn, d = frontier.popleft()
            if depth is not None and d >= depth:
                continue
            for c in self.callees.get(fn, ()):
                if c not in seen:
                    seen.add(c)
                    frontier.append((c, d + 1))
        return seen

    def equiv(self, fn):
        out = {fn}
        if fn in self.alias_of:
            out.add(self.alias_of[fn])
        out |= {a for a, t in self.alias_of.items() if t == fn}
        return out

    def facts_for(self, fn):
        tags = []
        eq = self.equiv(fn)
        for label, store in (("A", self.fact_global), ("E", self.fact_export),
                             ("B", self.fact_extern), ("S", self.fact_store),
                             ("C", self.fact_syscall), ("D", self.fact_orphan)):
            if any(e in store for e in eq):
                tags.append(label)
        if not any(e in self.defined for e in eq):
            tags.append("NOT-DEFINED")
        return tags


def covers(mod, pool, gt_name):
    return any(names_match(gt_name, p) for p in pool)


# label, facts (or SHIPPED for the detector's own output), prune mode
VARIANTS = [
    ("A",        "A",      "none"),
    ("AEB",      "AEB",    "none"),
    ("AEBS",     "AEBS",   "none"),
    ("AEBSC",    "AEBSC",  "none"),
    ("AEBSCD",   "AEBSCD", "none"),
    ("+prune-w", "AEBSCD", "weak"),
    ("+prune-a", "AEBSCD", "all"),
    ("shipped",  SHIPPED,  "none"),
    ("ship-pa",  SHIPPED,  "all"),
]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--detail", action="store_true")
    ap.add_argument("--case")
    ap.add_argument("--depth", type=int, default=None,
                    help="call-graph depth for coverage (default unbounded)")
    ap.add_argument("--strict", default="AEBSCD")
    ap.add_argument("--prune", default="weak")
    args = ap.parse_args()

    gt_all = json.loads((HERE / "dataset_entrypoints.json").read_text())
    cases = sorted(gt_all) if not args.case else [args.case]

    mods, skipped = {}, []
    for case in cases:
        d = HERE / case
        bc = pick_bitcode(d) if d.is_dir() else None
        if bc:
            mods[case] = Module(bc)
        else:
            skipped.append(case)
    if skipped:
        print(f"no bitcode for {len(skipped)} case(s): {', '.join(skipped)}\n")

    # Ground-truth names that do not denote a function in the module at all
    # are annotation artifacts ("host ring consumer", "a / b / c"); report
    # them separately instead of charging them against a rule.
    bogus = set()
    for case, mod in mods.items():
        for g in gt_all[case]["thread_roots"]:
            if "NOT-DEFINED" in mod.facts_for(g):
                bogus.add((case, g))
    ngt = sum(len(gt_all[c]["thread_roots"]) for c in mods)
    print(f"cases {len(mods)}   GT roots {ngt}   "
          f"unmatchable GT labels {len(bogus)}   scored {ngt - len(bogus)}\n")

    hdr = (f"{'variant':<9} {'root-hit':>9} {'+reach':>9} {'recall':>7} "
           f"{'roots/case':>11} {'pairable':>9}")
    print(hdr)
    print("-" * len(hdr))
    shipped = {}
    bl_path = HERE / "auto_entry_baseline.json"
    if bl_path.exists():
        for case, rec in json.loads(bl_path.read_text()).items():
            if rec.get("status") == "OK" and rec.get("entries"):
                shipped[case] = set(rec["entries"])

    table = {}
    for label, use, prune in VARIANTS:
        direct = reach = scored = nroots = ncases = 0
        pairable = npair = 0
        per_case = {}
        for case, mod in mods.items():
            gt = [g for g in gt_all[case]["thread_roots"]
                  if (case, g) not in bogus]
            if use is SHIPPED:
                if case not in shipped:
                    continue
                roots = mod.roots(shipped[case], prune)
            else:
                roots = mod.roots(use, prune)
            pool_d = set(roots)
            for r in list(roots):
                pool_d |= mod.equiv(r)
            pool_r = mod.reachable(roots, args.depth)
            for r in list(pool_r):
                pool_r |= mod.equiv(r)
            ncases += 1
            hits_d = [g for g in gt if covers(mod, pool_d, g)]
            hits_r = [g for g in gt if covers(mod, pool_r, g)]
            direct += len(hits_d)
            reach += len(hits_r)
            scored += len(gt)
            nroots += len(roots)
            per_case[case] = (len(hits_r), len(gt), len(roots))
            # Pairability: a two-thread case needs its two GT roots to sit
            # under *different* discovered roots to be modelled as a race.
            if len(gt) >= 2 and not gt_all[case].get("self_race"):
                npair += 1
                owners = []
                for g in gt:
                    own = {r for r in roots
                           if covers(mod, mod.reachable({r}, args.depth)
                                     | mod.equiv(r), g)}
                    owners.append(own)
                if all(owners) and not (len(owners) == 2 and
                                        len(owners[0]) == 1 and
                                        owners[0] == owners[1]):
                    pairable += 1
        table[label] = per_case
        print(f"{label:<9} {f'{direct}/{scored}':>9} {f'{reach}/{scored}':>9} "
              f"{reach / scored * 100:>6.1f}% {nroots / max(ncases, 1):>11.1f} "
              f"{f'{pairable}/{npair}':>9}")

    use, prune = args.strict, args.prune
    print(f"\n=== still missed under '{use}' prune={prune} ===")
    nmiss = 0
    for case, mod in sorted(mods.items()):
        roots = mod.roots(use, prune)
        pool = mod.reachable(roots, args.depth)
        for r in list(pool):
            pool |= mod.equiv(r)
        for g in gt_all[case]["thread_roots"]:
            if (case, g) in bogus or covers(mod, pool, g):
                continue
            nmiss += 1
            f = mod.facts_for(g) or ["no-fact"]
            print(f"  {case:<28} {g:<36} facts={','.join(f)}")
    print(f"  total {nmiss}")

    if args.detail:
        print(f"\n=== per case ('{use}') ===")
        for case, (h, t, r) in sorted(table[args.strict if args.strict in table else "AEBSCD"].items()):
            print(f"{case:<30} GT {t:>2}  hit {h:>2}  roots {r:>4}"
                  f"{'' if h == t else '   <-- miss'}")


if __name__ == "__main__":
    main()
