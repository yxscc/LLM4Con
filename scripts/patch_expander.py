#!/usr/bin/env python3
"""patch_expander.py — Phase F (M7).

Given a kernel git repo, a fix commit and an initial list of seed source
files, produce an *expanded* list of .c/.h files that should be compiled
and llvm-linked together so the LLM detector has full visibility into
the patch's data-flow.

Why this exists:
  Several kernel CVEs (e.g. CVE-2024-43891 across 5 files,
  CVE-2025-37920 across 4 files) involve concurrency between a thread
  that lives in `foo.c` and another that lives in `bar.c`, both sharing
  state declared in the patch's `.h`. The original prepare_cve.sh only
  compiled the .c files explicitly listed in cve_inputs.json, so the
  detector saw an incomplete picture.

Expansion strategy:
  1) Always include every .c/.h file touched by the fix commit.
  2) For every .h file in the working set, scan all sibling (same
     directory) .c files: if a sibling `#include`s the .h AND mentions
     at least one symbol that the fix commit changed, pull it in.
  3) Output an expansion_report.json next to the experiment dir
     describing what was added and why, so it is auditable.

Usage:
  patch_expander.py --kernel-dir /home/ConCord/targets/linux.git \\
                    --commit 92964c79b357 \\
                    --seeds net/netlink/af_netlink.c net/netlink/af_netlink.h \\
                    --output /home/LLM4Con/kernel_experiment/CVE-2016-9806

Exit codes:
  0  expansion succeeded (or was a no-op, single-file fast-path)
  1  hard error (git show failed, kernel dir missing, etc.)

The script is intentionally side-effect-light: it only writes
`expansion_report.json` and prints the final file list (one per line)
to stdout. The shell wrapper (prepare_cve.sh) consumes stdout.
"""
from __future__ import annotations

import argparse
import json
import os
import re
import subprocess
import sys
from pathlib import Path
from typing import Iterable


HUNK_HEADER_RE = re.compile(r"^@@ .* @@\s*(.*)$")
SYMBOL_RE = re.compile(r"\b([A-Za-z_][A-Za-z0-9_]{2,})\b")
INCLUDE_RE = re.compile(r'#\s*include\s+["<]([^">]+)[">]')


def run_git(kernel_dir: Path, *args: str) -> str:
    """Run a git command in the kernel dir and return stdout (text)."""
    proc = subprocess.run(
        ["git", "-C", str(kernel_dir), *args],
        check=False, capture_output=True, text=True, errors="replace",
    )
    if proc.returncode != 0:
        raise RuntimeError(
            f"git {' '.join(args)} failed (rc={proc.returncode}): {proc.stderr.strip()}"
        )
    return proc.stdout


def patch_files(kernel_dir: Path, commit: str) -> list[str]:
    """Return all .c/.h files touched by `commit`."""
    out = run_git(kernel_dir, "show", "--name-only", "--pretty=", commit)
    return [l.strip() for l in out.splitlines()
            if l.strip().endswith((".c", ".h"))]


def changed_symbols(kernel_dir: Path, commit: str) -> set[str]:
    """Extract C-identifier-shaped symbols mentioned in the diff *code*
    of `commit`.

    Lessons from M7 canary on CVE-2024-43891:
      - `git show` includes the commit message; using its body would
        otherwise pull English words ("When", "and", "the") into the
        symbol set and falsely match every sibling .c file.
      - Even the diff body contains added/removed comment lines whose
        prose words must be skipped.
      - Common short English nouns ("data", "file", "buf") leak through
        a naive identifier match. We require a kernel-style shape:
        identifier must contain '_' OR an uppercase letter, and be
        at least 4 chars long.

    We use `git diff $commit~..$commit` so the commit message is
    excluded by construction. We then walk diff body lines that start
    with '+' or '-', skipping file headers and any line whose stripped
    body looks like a C/C++ comment.
    """
    try:
        out = run_git(kernel_dir, "diff",
                      "--no-color", "--unified=0",
                      f"{commit}~..{commit}")
    except RuntimeError:
        out = run_git(kernel_dir, "diff",
                      "--no-color", "--unified=0",
                      f"{commit}^..{commit}")

    syms: set[str] = set()
    for line in out.splitlines():
        if not line:
            continue
        if line.startswith(("+++", "---", "diff ", "index ", "@@")):
            continue
        if line[0] not in "+-":
            continue
        body = line[1:].lstrip()
        if (body.startswith("/*") or body.startswith("*")
                or body.startswith("//") or body.startswith("* ")):
            continue
        for m in SYMBOL_RE.finditer(body):
            tok = m.group(1)
            if tok in _NOISE_TOKENS:
                continue
            if len(tok) < 4:
                continue
            if "_" not in tok and not any(c.isupper() for c in tok):
                continue
            syms.add(tok)
    return syms


_NOISE_TOKENS = {
    "int", "char", "void", "long", "short", "unsigned", "signed",
    "struct", "union", "enum", "static", "const", "extern", "inline",
    "return", "if", "else", "for", "while", "do", "switch", "case",
    "break", "continue", "default", "goto", "sizeof", "typeof",
    "true", "false", "NULL", "EINVAL", "ENOMEM", "EFAULT",
    "READ_ONCE", "WRITE_ONCE", "smp_mb", "barrier",
}


def list_dir_files_at(kernel_dir: Path, commit: str, dir_path: str) -> list[str]:
    """List files in `dir_path` at `commit` via git ls-tree (no checkout)."""
    arg = f"{commit}:{dir_path}/" if dir_path else f"{commit}:"
    try:
        out = run_git(kernel_dir, "ls-tree", "--name-only", arg)
    except RuntimeError:
        return []
    return [l.strip() for l in out.splitlines() if l.strip()]


def show_file_at(kernel_dir: Path, commit: str, file_path: str) -> str | None:
    """Read `file_path` at `commit` via git show (no checkout)."""
    try:
        return run_git(kernel_dir, "show", f"{commit}:{file_path}")
    except RuntimeError:
        return None


SIBLING_MIN_SYMS = 2  # sibling must reference >= N changed symbols
SIBLING_TOPK = 3      # cap newly-added siblings (avoids whole-dir explosion)
PATCH_LARGE_THRESHOLD = 3  # if patch already touches >= N files, skip expansion


def expand_with_siblings(kernel_dir: Path,
                         commit: str,
                         seeds: Iterable[str],
                         patch_count: int,
                         changed_syms: set[str]) -> tuple[list[str], dict]:
    """Pull in same-directory .c files that include any seed .h and
    *strongly* reference changed symbols.

    Skip expansion entirely when the patch already touches a lot of
    files (>= PATCH_LARGE_THRESHOLD) — the patch itself already
    provides cross-file context, and broad headers like `trace.h` would
    otherwise drag in dozens of unrelated TUs.

    For each candidate sibling, require it to mention at least
    `SIBLING_MIN_SYMS` distinct changed symbols. Then keep only the
    top `SIBLING_TOPK` candidates by match count, breaking ties
    alphabetically. This bounds the expansion size and prefers the
    siblings that are most likely to participate in the bug's data flow.

    Implementation reads files via `git show $commit:path` so we do not
    need a checked-out worktree.
    """
    final: set[str] = {p.strip() for p in seeds if p.strip()}
    reasons: dict[str, str] = {}

    if patch_count >= PATCH_LARGE_THRESHOLD:
        return sorted(final), reasons

    headers = sorted(p for p in final if p.endswith(".h"))
    candidates: list[tuple[int, str, str, list[str]]] = []
    for hpath in headers:
        h_basename = Path(hpath).name
        sib_dir = str(Path(hpath).parent)
        if sib_dir == ".":
            sib_dir = ""

        for fname in list_dir_files_at(kernel_dir, commit, sib_dir):
            if not fname.endswith(".c"):
                continue
            rel = f"{sib_dir}/{fname}" if sib_dir else fname
            if rel in final:
                continue
            text = show_file_at(kernel_dir, commit, rel)
            if text is None:
                continue
            includes = INCLUDE_RE.findall(text)
            if not any(Path(inc).name == h_basename for inc in includes):
                continue
            mentioned = sorted(s for s in changed_syms
                               if re.search(rf"\b{re.escape(s)}\b", text))
            if len(mentioned) < SIBLING_MIN_SYMS:
                continue
            candidates.append((len(mentioned), rel, h_basename, mentioned))

    candidates.sort(key=lambda c: (-c[0], c[1]))
    for n_match, rel, h_basename, mentioned in candidates[:SIBLING_TOPK]:
        final.add(rel)
        preview = ", ".join(mentioned[:3])
        if len(mentioned) > 3:
            preview += f", +{len(mentioned)-3} more"
        reasons[rel] = (
            f"includes {h_basename}; references {n_match} changed symbols: {preview}"
        )

    return sorted(final), reasons


def main() -> int:
    p = argparse.ArgumentParser(description=__doc__,
                                formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("--kernel-dir", required=True,
                   help="Path to the kernel git checkout.")
    p.add_argument("--commit", required=True,
                   help="Fix commit hash; expansion uses files this commit touches.")
    p.add_argument("--seeds", nargs="*", default=[],
                   help="User-supplied source files (will be unioned with patch files).")
    p.add_argument("--output", required=False,
                   help="If set, write expansion_report.json into this dir.")
    args = p.parse_args()

    kernel_dir = Path(args.kernel_dir)
    if not (kernel_dir / ".git").exists():
        print(f"[patch_expander] not a git repo: {kernel_dir}", file=sys.stderr)
        return 1

    try:
        touched = patch_files(kernel_dir, args.commit)
        syms = changed_symbols(kernel_dir, args.commit)
    except RuntimeError as e:
        print(f"[patch_expander] {e}", file=sys.stderr)
        return 1

    seeds = list({*args.seeds, *touched})
    expanded, reasons = expand_with_siblings(
        kernel_dir, args.commit, seeds, len(touched), syms)

    report = {
        "commit": args.commit,
        "user_seeds": sorted(args.seeds),
        "patch_files": sorted(touched),
        "changed_symbols_sample": sorted(list(syms))[:30],
        "expanded": expanded,
        "added_by_expander": reasons,
    }

    if args.output:
        out_dir = Path(args.output)
        out_dir.mkdir(parents=True, exist_ok=True)
        (out_dir / "expansion_report.json").write_text(
            json.dumps(report, indent=2, ensure_ascii=False))
        print(f"[patch_expander] wrote {out_dir/'expansion_report.json'}",
              file=sys.stderr)

    for line in expanded:
        print(line)
    return 0


if __name__ == "__main__":
    sys.exit(main())
