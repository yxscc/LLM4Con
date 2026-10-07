#!/usr/bin/env python3
"""Triage hunt findings against later kernel history.

A race reported on a fixed baseline (v7.2) is only interesting if nobody has
already fixed it upstream. We have the full history locally, so for every
source line a finding points at we can ask: does that line still exist in
mainline, and did any race-shaped commit touch that file since the baseline?

Verdicts, most to least interesting:

  PRESENT        every involved line is still in mainline verbatim and no
                 race-shaped commit touched the files -> best new-bug candidate
  PRESENT_RISKY  lines still present, but the file did receive race fixes
                 since the baseline -> read those commits before reporting
  CHANGED        some involved line is gone from mainline, no race-shaped
                 commit -> refactored; re-check against mainline
  FIXED_LIKELY   involved line gone AND a race-shaped commit touched the file
                 -> most likely already fixed upstream

IMPORTANT: this is a cheap prefilter, not a decision procedure. PRESENT means
the *code* is unchanged, not that the *bug* is unfixed: a race is often fixed
by adding synchronisation on the other side of the pair, leaving both cited
lines byte-identical. Treat PRESENT as "worth a human read" and always read the
listed race-fix commits before writing a report.

Usage:
  known_defect_filter.py <bugs.txt> [--tree DIR] [--base v7.2] [--head origin/master]
  known_defect_filter.py --dump-root hunt/dumps [...]      # every newest dump
"""
import argparse
import functools
import glob
import json
import os
import re
import subprocess
import sys

RACE_MSG = re.compile(
    r"\b(race|races|racy|deadlock|use-after-free|UAF|double[- ]free|"
    r"data.?race|KCSAN|lockdep|use after free|"
    r"lock(ing)? (bug|imbalance|inversion)|missing lock|unlocked)\b",
    re.I,
)

# bugs.txt node lines look like:
#   a (node 68): task->io_context = NULL  [<abs-or-rel path>:206]
NODE_RE = re.compile(r"^\s{2}\w+ \(node (\d+)\):\s*(.+?)\s*\[([^\]\s]+):(\d+)\]\s*$")
RULE_RE = re.compile(r"^Rule:\s*(\S+)")
MECH_RE = re.compile(r"^Mechanism:\s*(.+)$")
DESC_RE = re.compile(r"^Description:\s*(.+)$")


def git(tree, *args, timeout=120):
    try:
        p = subprocess.run(["git", "-C", tree, *args], capture_output=True,
                           text=True, timeout=timeout)
        return p.stdout if p.returncode == 0 else ""
    except (subprocess.SubprocessError, OSError):
        return ""


TOPDIRS = ("net", "fs", "drivers", "kernel", "block", "io_uring", "mm",
           "sound", "security", "virt", "crypto", "lib", "ipc", "arch")


@functools.lru_cache(maxsize=1)
def _basename_index(tree, ref):
    """basename -> [paths] for every .c/.h in the tree at ref.

    Needed because older case layouts flatten sources into src/ with no
    subdirectory, so the path in the dump carries no kernel-relative prefix.
    """
    out = git(tree, "ls-tree", "-r", "--name-only", ref, timeout=300)
    idx = {}
    for p in out.splitlines():
        if p.endswith((".c", ".h")):
            idx.setdefault(os.path.basename(p), []).append(p)
    return idx


def kernel_relpath(loc, tree=None, ref=None):
    """Map a dump location back to a path inside the kernel tree.

    prepare_module.sh mirrors the kernel layout below src/, e.g.
    .../targets/net-tipc/src/net/tipc/link.c, so the common case is a prefix
    strip. Flattened layouts fall back to a unique-basename lookup.
    """
    loc = loc.replace("\\", "/")
    m = re.search(r"/src/((?:" + "|".join(TOPDIRS) + r")/.+)$", loc)
    if m:
        return m.group(1)
    for root in TOPDIRS:
        i = loc.find("/" + root + "/")
        if i >= 0:
            return loc[i + 1:]
        if loc.startswith(root + "/"):
            return loc
    if tree and ref:
        hits = _basename_index(tree, ref).get(os.path.basename(loc), [])
        if len(hits) == 1:
            return hits[0]
    return None


@functools.lru_cache(maxsize=4096)
def file_at(tree, ref, path):
    return git(tree, "show", f"{ref}:{path}")


@functools.lru_cache(maxsize=2048)
def file_race_commits(tree, base, head, path):
    """(n_commits, [(sha, subject)]) race-shaped commits touching path."""
    out = git(tree, "log", "--no-merges", f"{base}..{head}",
              "--format=%h%x1f%s", "--", path)
    hits = []
    total = 0
    for line in out.splitlines():
        if "\x1f" not in line:
            continue
        sha, subj = line.split("\x1f", 1)
        total += 1
        if RACE_MSG.search(subj):
            hits.append((sha, subj))
    return total, hits


def norm_code(s):
    """Whitespace-insensitive form, so reindentation is not treated as change."""
    return re.sub(r"\s+", "", s)


def parse_bugs(path):
    """Split bugs.txt into findings with their involved nodes."""
    text = open(path, errors="ignore").read()
    findings = []
    for block in text.split("Mechanism-Rule Violation Detected")[1:]:
        rule = mech = desc = ""
        nodes = []
        in_nodes = False
        for line in block.splitlines():
            if m := RULE_RE.match(line):
                rule = m.group(1)
            elif m := MECH_RE.match(line):
                mech = m.group(1).strip()
            elif m := DESC_RE.match(line):
                desc = m.group(1).strip()
            elif "Involved Nodes ---" in line:
                in_nodes = True
            elif line.startswith("--- Verified Constraints"):
                in_nodes = False
            elif in_nodes:
                if m := NODE_RE.match(line):
                    nodes.append({"node": int(m.group(1)),
                                  "code": m.group(2),
                                  "loc": m.group(3),
                                  "line": int(m.group(4))})
        if rule or nodes:
            findings.append({"rule": rule, "mechanism": mech,
                             "description": desc, "nodes": nodes})
    return findings


def classify(tree, base, head, finding):
    present, missing, unresolved = [], [], []
    race_commits = {}
    files_seen = set()

    for n in finding["nodes"]:
        rel = kernel_relpath(n["loc"], tree, head)
        n["kernel_path"] = rel
        if not rel:
            unresolved.append(n)
            continue
        files_seen.add(rel)
        head_text = file_at(tree, head, rel)
        if not head_text:
            # file gone from mainline entirely
            n["status"] = "file_removed"
            missing.append(n)
            continue
        n["status"] = ("present"
                       if norm_code(n["code"]) in norm_code(head_text)
                       else "absent")
        (present if n["status"] == "present" else missing).append(n)

    for rel in sorted(files_seen):
        total, hits = file_race_commits(tree, base, head, rel)
        if hits:
            race_commits[rel] = {"commits_total": total,
                                 "race_fixes": [{"sha": s, "subject": t}
                                                for s, t in hits]}

    has_race_fix = bool(race_commits)
    if missing:
        verdict = "FIXED_LIKELY" if has_race_fix else "CHANGED"
    elif not present and unresolved:
        verdict = "UNRESOLVED"
    else:
        verdict = "PRESENT_RISKY" if has_race_fix else "PRESENT"

    finding["verdict"] = verdict
    finding["race_commits"] = race_commits
    finding["n_present"] = len(present)
    finding["n_absent"] = len(missing)
    return finding


ORDER = ["PRESENT", "PRESENT_RISKY", "CHANGED", "FIXED_LIKELY", "UNRESOLVED"]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("bugs", nargs="*", help="bugs.txt file(s)")
    ap.add_argument("--dump-root",
                    help="scan the newest dump per target under this root")
    ap.add_argument("--tree",
                    default=os.environ.get("LINUX_REPO", ""),
                    help="kernel git tree (default $LINUX_REPO)")
    ap.add_argument("--base", default="v7.2")
    ap.add_argument("--head", default="origin/master")
    ap.add_argument("--json")
    ap.add_argument("--verbose", action="store_true")
    args = ap.parse_args()

    if not args.tree or not os.path.isdir(args.tree):
        print("error: --tree must point at the kernel git tree "
              "(source setup_env.sh, or pass --tree)", file=sys.stderr)
        return 2

    files = list(args.bugs)
    if args.dump_root:
        by_target = {}
        for d in glob.glob(os.path.join(args.dump_root, "*_*")):
            bf = os.path.join(d, "stateful_bugs", "bugs.txt")
            if not os.path.isfile(bf):
                continue
            target = os.path.basename(d).split("_20")[0]
            if (target not in by_target
                    or os.path.getmtime(d) > os.path.getmtime(by_target[target])):
                by_target[target] = d
        files += [os.path.join(d, "stateful_bugs", "bugs.txt")
                  for d in by_target.values()]
    if not files:
        print("error: no bugs.txt given (pass paths or --dump-root)",
              file=sys.stderr)
        return 2

    results = []
    for bf in sorted(files):
        target = "?"
        m = re.search(r"dumps/([^/]+?)_20\d\d-", bf.replace("\\", "/"))
        if m:
            target = m.group(1)
        for f in parse_bugs(bf):
            f["target"] = target
            f["source"] = bf
            results.append(classify(args.tree, args.base, args.head, f))

    results.sort(key=lambda f: (ORDER.index(f["verdict"])
                                if f["verdict"] in ORDER else 99,
                                f["target"], f["rule"]))

    counts = {}
    for f in results:
        counts[f["verdict"]] = counts.get(f["verdict"], 0) + 1

    print(f"baseline {args.base} -> mainline {args.head}\n")
    hdr = f"{'VERDICT':15s} {'TARGET':18s} {'RULE':9s} {'MECHANISM':28s} lines"
    print(hdr)
    print("-" * len(hdr))
    for f in results:
        print(f"{f['verdict']:15s} {f['target']:18s} {f['rule']:9s} "
              f"{f['mechanism'][:28]:28s} "
              f"present={f['n_present']} absent={f['n_absent']}")
        if args.verbose:
            for n in f["nodes"]:
                print(f"    [{n.get('status','?'):12s}] "
                      f"{n.get('kernel_path') or n['loc']}:{n['line']}  "
                      f"{n['code'][:64]}")
            for path, rc in f["race_commits"].items():
                for c in rc["race_fixes"][:4]:
                    print(f"    (race fix) {path}: {c['sha']} {c['subject'][:70]}")
    print("-" * len(hdr))
    print("  ".join(f"{k}={v}" for k, v in
                    sorted(counts.items(), key=lambda kv: ORDER.index(kv[0])
                           if kv[0] in ORDER else 99)))
    print("\nReview order: PRESENT first (still in mainline, no race fix "
          "nearby), then PRESENT_RISKY.")

    if args.json:
        with open(args.json, "w") as fh:
            json.dump(results, fh, indent=2)
        print(f"wrote {args.json}")


if __name__ == "__main__":
    sys.exit(main() or 0)
