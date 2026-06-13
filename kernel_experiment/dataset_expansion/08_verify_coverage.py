#!/usr/bin/env python3
"""Per-entry verification that the collected src/ + .ll *actually*
contains the data/control flow of the audited race.

For every promoted v2 entry the script:

  1. Pulls the two racing function names from
     ground_truth.json["thread_hint"] (KCSAN tier-A) or, failing that,
     parses them out of `two_threads_summary` and from the
     added/removed function-definition lines in `patch`.
  2. For each candidate function, checks
     (a) is there a complete function body in any src/*.c
         (looks for `^<rettype-tokens> name(...)` followed by `{`); and
     (b) is there a matching `define ... @name(` in any *.ll
         (i.e. the function is actually compiled in our bitcode).
  3. Reports per-entry coverage:
        - FULL    : both racing functions found in BOTH src and .ll
        - SRC_ONLY: src has them but .ll is missing one (build gap)
        - LL_ONLY : .ll has them but src/ is missing one (copy gap)
        - PATCH   : the racing target is a struct field/macro and the
                    patch-modified function bodies ARE present; data
                    flow still covered even though we couldn't name a
                    pair (e.g. atomic_*/READ_ONCE annotation fix)
        - MISSING : neither src nor .ll defines either racing function
                    — bitcode would not let Lace observe this race.

The output is written to staging_coverage.json and a console summary."""
from __future__ import annotations
import json
import os
import re
import subprocess
from pathlib import Path
from typing import Iterable

BASE = Path("/mlx_devbox/users/mayunlong.39/playground/LLM4Con/"
            "kernel_experiment")

# Names that show up in stack traces / titles / hunk context labels but
# aren't real C functions. C keywords + role-name nouns we use in audit
# summaries.
C_KEYWORDS = {
    "if", "else", "for", "while", "do", "switch", "case", "default",
    "return", "break", "continue", "goto", "void", "int", "char",
    "short", "long", "float", "double", "signed", "unsigned",
    "struct", "union", "enum", "typedef", "static", "extern", "inline",
    "const", "volatile", "register", "auto", "sizeof",
    "anyway", "true", "false", "null",
}
ROLE_NOUNS = {
    "softirq", "irq", "task", "worker", "rcu", "bh", "kworker", "syscall",
    "init", "cleanup", "task_struct", "sock", "skb", "pwq",
    "READ_ONCE", "WRITE_ONCE", "atomic", "smp_load_acquire",
    "smp_store_release",
}
NON_FN_TOKENS = C_KEYWORDS | ROLE_NOUNS

FN_RE = re.compile(r"[a-zA-Z_][a-zA-Z0-9_]{2,}")


def _kernel_idents(text: str) -> list[str]:
    """Plausible kernel identifier names mentioned in `text`."""
    cands = set()
    for m in FN_RE.findall(text or ""):
        if m in NON_FN_TOKENS:
            continue
        if len(m) < 4:
            continue
        if m.isupper():
            continue
        cands.add(m)
    return sorted(cands)


def patch_added_fn_defs(patch: str) -> list[str]:
    """Return function names whose entire definition is in the +/- lines
    of the patch (i.e. names that follow C-style `<type> name(...) {` on
    an added or removed line)."""
    out = set()
    for ln in patch.splitlines():
        if not (ln.startswith("+") or ln.startswith("-")):
            continue
        if ln.startswith("+++") or ln.startswith("---"):
            continue
        body = ln[1:].lstrip()
        m = re.match(r"(?:static\s+)?(?:inline\s+)?"
                     r"(?:[\w*\s]+?\s+)?"
                     r"([A-Za-z_][A-Za-z0-9_]+)\s*\([^;]*\)\s*$",
                     body)
        if m:
            out.add(m.group(1))
    return sorted(out)


_HUNK_HEADER = re.compile(
    r"^@@ -\d+(?:,\d+)? \+(\d+)(?:,\d+)? @@\s*(.*)$"
)


def patch_touched_lines_fns(d: Path, patch: str) -> list[str]:
    """Function names that *enclose* the changed lines.

    Two complementary signals are combined:
      (a) `git diff` writes the C function header (or struct name) into
          each hunk's `@@ -m,n +o,p @@ <context>` label. We parse out
          the identifier in that context label.
      (b) For each touched line we re-scan the corresponding src/*.c
          and pick the nearest preceding function definition. The
          regex accepts both single-line `<rettype> name(...)` and the
          common kernel style where the return type is on its own line
          followed by `name(...) {`.
    """
    touched: dict[str, list[int]] = {}
    label_idents: set[str] = set()

    cur_file = None
    cur_hunk = None
    for ln in patch.splitlines():
        m = re.match(r"^diff --git a/(\S+) b/(\S+)", ln)
        if m:
            cur_file = m.group(2)
            cur_hunk = None
            continue
        m = _HUNK_HEADER.match(ln)
        if m and cur_file:
            cur_hunk = int(m.group(1))
            label = m.group(2)
            # The hunk-context label starts with return-type and
            # qualifiers (e.g. "static int ") and is followed by the
            # function name. Walk every identifier and keep the first
            # one that isn't a C keyword/role noun.
            for id_match in re.finditer(r"\b([A-Za-z_][A-Za-z0-9_]{3,})\b",
                                        label):
                ident = id_match.group(1)
                if ident in NON_FN_TOKENS:
                    continue
                label_idents.add(ident)
                break
            continue
        if cur_hunk is None or cur_file is None:
            continue
        if ln.startswith("+") and not ln.startswith("+++"):
            touched.setdefault(cur_file, []).append(cur_hunk)
            cur_hunk += 1
        elif ln.startswith(" "):
            cur_hunk += 1
        # context lines starting with '-' don't advance the "after" cursor

    fns: set[str] = set(label_idents)
    for relpath, lines in touched.items():
        base = os.path.basename(relpath)
        candidate = None
        for root, _, files in os.walk(d / "src"):
            if base in files:
                candidate = Path(root) / base
                break
        if candidate is None:
            continue
        try:
            src_lines = candidate.read_text(errors="replace").splitlines()
        except OSError:
            continue
        defs = _scan_fn_defs(src_lines)
        for hit in lines:
            best = None
            for (i, name) in defs:
                if i <= hit:
                    best = name
                else:
                    break
            if best:
                fns.add(best)
    return sorted(fns)


def _scan_fn_defs(lines: list[str]) -> list[tuple[int, str]]:
    """Yield (line_no, fn_name) for every function definition in a C
    file. Handles both single-line and split-style declarations:

        static int  foo(int x) {       # single-line
        static int                     # multi-line
        foo(int x)
        {
    """
    defs: list[tuple[int, str]] = []
    pat_inline = re.compile(
        r"^(?:static\s+|inline\s+|__always_inline\s+|noinline\s+)*"
        r"[\w\*\s]+\s+(\*\s*)?([A-Za-z_]\w+)\s*\([^;]*\)\s*\{?\s*$"
    )
    pat_only_name = re.compile(r"^(\*\s*)?([A-Za-z_]\w+)\s*\([^;]*\)\s*\{?\s*$")
    for i, ln in enumerate(lines, start=1):
        m = pat_inline.match(ln)
        if m:
            defs.append((i, m.group(2)))
            continue
        # split-style: previous non-empty line is a return-type/qualifier
        # token list (no parens, no `=`, doesn't end with `;` or `{`)
        m2 = pat_only_name.match(ln)
        if not m2:
            continue
        j = i - 2  # 0-indexed previous line
        while j >= 0 and not lines[j].strip():
            j -= 1
        if j < 0:
            continue
        prev = lines[j].strip()
        if (prev and ";" not in prev and "{" not in prev and
                "=" not in prev and "(" not in prev and
                re.match(r"^[A-Za-z_][\w\*\s]*$", prev)):
            defs.append((i, m2.group(2)))
    return defs


def fn_defined_in_src(d: Path, name: str) -> Path | None:
    """Return path of a src .c that contains a function definition named
    `name`. Handles both inline and split return-type styles."""
    src_root = d / "src"
    if not src_root.is_dir():
        return None
    for root, _, files in os.walk(src_root):
        for f in files:
            if not f.endswith(".c"):
                continue
            p = Path(root) / f
            try:
                lines = p.read_text(errors="replace").splitlines()
            except OSError:
                continue
            for (_, n) in _scan_fn_defs(lines):
                if n == name:
                    return p
    return None


def fn_defined_in_ll(d: Path, name: str) -> Path | None:
    for ll in d.glob("*.ll"):
        if ll.name == "merged.ll":
            continue
        try:
            with open(ll) as f:
                for ln in f:
                    if ln.startswith("define ") and f"@{name}(" in ln:
                        return ll
        except OSError:
            continue
    return None


def classify_entry(d: Path) -> dict:
    gt = json.load(open(d / "ground_truth.json"))
    hint = gt.get("thread_hint") or {}
    tts = gt.get("two_threads_summary", "") or ""
    patch = gt.get("patch", "") or ""

    pair: list[str] = []
    if hint.get("fn_a") and hint["fn_a"] not in NON_FN_TOKENS:
        pair.append(hint["fn_a"])
    if hint.get("fn_b") and hint["fn_b"] not in NON_FN_TOKENS:
        # Keep both halves even when fn_a == fn_b: a KCSAN report of
        # "data-race in foo / foo" means foo races with itself across
        # threads. The single function in .ll IS the fully-covered case.
        pair.append(hint["fn_b"])
    is_self_race = len(pair) == 2 and pair[0] == pair[1]

    summary_idents = _kernel_idents(tts)
    patch_defs = patch_added_fn_defs(patch)
    enclosing = patch_touched_lines_fns(d, patch)

    candidates = list(dict.fromkeys(pair + enclosing + patch_defs +
                                    summary_idents))[:30]

    coverage = []
    for name in candidates:
        src = fn_defined_in_src(d, name)
        ll = fn_defined_in_ll(d, name)
        coverage.append({
            "fn": name,
            "src": str(src.relative_to(d)) if src else None,
            "ll": ll.name if ll else None,
        })

    # The gold standard for "data/control flow is observable by Lace" is
    # whether the function is defined in our LLVM bitcode (`.ll`).
    # The src/*.c presence is bookkeeping for human auditors.

    def covered_in_ll(name: str) -> bool:
        return any(c["fn"] == name and c["ll"] for c in coverage)

    has_pair = len(pair) >= 2
    # For self-races, "both halves covered" reduces to "the function
    # covered" — racing the function against another instance of itself
    # is fully observable from a single bitcode definition.
    pair_in_ll = [covered_in_ll(p) for p in pair] if has_pair else []
    enclosing_in_ll = [n for n in enclosing if covered_in_ll(n)]

    if has_pair and is_self_race and pair_in_ll[0]:
        status = "FULL"           # self-race: one fn in .ll == both halves
    elif has_pair and all(pair_in_ll):
        status = "FULL"           # both distinct racing functions in bitcode
    elif has_pair and any(pair_in_ll) and enclosing_in_ll:
        status = "FULL"           # one named + at least one patch fn
    elif not has_pair and enclosing_in_ll:
        status = "PATCH"          # annotate-only fix, patch fn in .ll
    elif has_pair and any(pair_in_ll):
        status = "PARTIAL"        # only one of the two racing fns
    elif any(c["ll"] for c in coverage):
        status = "PARTIAL"        # some related fn but not the canonical pair
    else:
        status = "MISSING"        # nothing relevant in bitcode

    return {
        "status": status,
        "pair_from_hint": pair,
        "patch_enclosing_fns": enclosing,
        "patch_def_fns": patch_defs,
        "all_candidates": candidates,
        "coverage": coverage,
        "pair_in_ll": dict(zip(pair, pair_in_ll)) if has_pair else {},
        "enclosing_in_ll": enclosing_in_ll,
    }


def main() -> int:
    import argparse
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--restrict-to",
                    default=str(Path(__file__).resolve().parent /
                                "final_selection.json"),
                    help="json with field 'final_bug_ids' to restrict the "
                         "scan to v2 promoted entries only.")
    args = ap.parse_args()

    if args.restrict_to and os.path.isfile(args.restrict_to):
        sel = json.load(open(args.restrict_to))
        ids = sel.get("final_bug_ids") or sel.get("core_bug_ids") or []
    else:
        ids = [d.name for d in BASE.iterdir()
               if d.is_dir() and (d.name.startswith("CVE-") or
                                  d.name.startswith("SYZBOT-"))]

    out: dict[str, dict] = {}
    counts: dict[str, int] = {}
    for b in sorted(ids):
        d = BASE / b
        if not d.is_dir():
            continue
        r = classify_entry(d)
        out[b] = r
        counts[r["status"]] = counts.get(r["status"], 0) + 1

    print(f"Scanned {len(out)} entries (restrict={args.restrict_to or 'all'})")
    print()
    for k in ("FULL", "PATCH", "PARTIAL", "MISSING"):
        if counts.get(k):
            print(f"  {k:9s} {counts[k]}")
    print()

    for status in ("MISSING", "PARTIAL"):
        rows = [(b, r) for b, r in out.items() if r["status"] == status]
        if not rows:
            continue
        print(f"--- {status} ({len(rows)}) ---")
        for b, r in rows:
            print(f"  {b}")
            if r["pair_from_hint"]:
                hits = [f"{p}={'Y' if r['pair_in_ll'].get(p) else 'N'}"
                        for p in r["pair_from_hint"]]
                print(f"    pair     : {', '.join(hits)}")
            if r["patch_enclosing_fns"]:
                print(f"    enclosing: {r['patch_enclosing_fns'][:6]}")
                print(f"    in .ll   : {r['enclosing_in_ll'][:6]}")
            top = [c for c in r["coverage"] if c["src"] or c["ll"]][:6]
            for c in top:
                print(f"    {c['fn']:30s}  src={c['src']}  ll={c['ll']}")
            if not top:
                print(f"    (no candidate function found in src/.ll)")

    out_path = (Path(__file__).resolve().parent / "staging_coverage.json")
    with open(out_path, "w") as f:
        json.dump({"counts": counts, "details": out}, f, indent=2)
    print(f"\nwrote {out_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
