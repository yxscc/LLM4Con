#!/usr/bin/env python3
"""Distill each evidence/<bug>.md into a 1-screen audit card.

Output: audit/<bug>.txt — title, commit one-liner, KCSAN/lockdep
stack-trace block, and the unified diff itself (no source bodies).
That keeps the audit packet narrow enough to make a verdict from a
single shell `cat` per candidate.
"""
from __future__ import annotations
import argparse
import json
import re
from pathlib import Path


HERE = Path(__file__).resolve().parent


def extract_section(md_text: str, header_re: str) -> str:
    m = re.search(header_re, md_text, re.MULTILINE)
    if not m:
        return ""
    start = m.end()
    nxt = re.search(r"^##\s", md_text[start:], re.MULTILINE)
    end = start + (nxt.start() if nxt else len(md_text) - start)
    return md_text[start:end].strip()


def extract_stack_block(commit_msg: str) -> str:
    """Pull the KCSAN/lockdep report from the commit message if present."""
    lines = commit_msg.splitlines()
    out = []
    inside = False
    for ln in lines:
        if ("BUG: KCSAN" in ln or "WARNING:" in ln
                or "possible recursive locking" in ln.lower()
                or "deadlock" in ln.lower() and "BUG" in ln):
            inside = True
        if inside:
            out.append(ln)
            if ln.strip().startswith("Fixes:") or ln.strip().startswith("Signed-off-by:"):
                break
    if out:
        return "\n".join(out[:60])
    # Try alternative: find "read to" / "write to" block, common for KCSAN
    for i, ln in enumerate(lines):
        if ln.strip().startswith("read to ") or ln.strip().startswith("write to "):
            return "\n".join(lines[max(0, i-1):i+40])
    return ""


def trimmed_diff(patch: str, max_lines: int = 120) -> str:
    lines = patch.splitlines()
    # Find diff start
    start = 0
    for i, ln in enumerate(lines):
        if ln.startswith("diff --git"):
            start = i
            break
    diff = "\n".join(lines[start:start + max_lines])
    if len(lines) - start > max_lines:
        diff += "\n... (diff truncated) ..."
    return diff


def card_for(md_path: Path) -> str:
    text = md_path.read_text()
    meta_path = md_path.with_suffix(".meta.json")
    meta = json.load(open(meta_path)) if meta_path.exists() else {}

    head = []
    for line in text.splitlines()[:10]:
        if line.startswith("- ") or line.startswith("# "):
            head.append(line)

    commit_msg = extract_section(text, r"^##\s+Commit message")
    patch = extract_section(text, r"^##\s+Full mainline patch")
    stack = extract_stack_block(commit_msg.strip("`").strip())

    out: list = []
    out.append("=" * 70)
    out.extend(head)
    out.append("")
    out.append("## Commit subject")
    subj_lines = commit_msg.splitlines()
    # commit_msg starts with ``` then sha then subject
    for ln in subj_lines:
        s = ln.strip()
        if s and not s.startswith("```") and not re.match(r"^[0-9a-f]{40}$", s):
            out.append(f"  {s}")
            break
    if stack:
        out.append("")
        out.append("## Reporter stack trace (excerpt)")
        for ln in stack.splitlines():
            out.append(f"  {ln}")
    out.append("")
    out.append("## Patch (truncated)")
    out.append("```diff")
    out.append(trimmed_diff(patch.strip("`").strip(), max_lines=80))
    out.append("```")
    return "\n".join(out)


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--evidence-dir", default=str(HERE / "evidence"))
    ap.add_argument("--out-dir", default=str(HERE / "audit"))
    args = ap.parse_args()

    ev_dir = Path(args.evidence_dir)
    out_dir = Path(args.out_dir)
    out_dir.mkdir(parents=True, exist_ok=True)

    n = 0
    for md in sorted(ev_dir.glob("*.md")):
        card = card_for(md)
        (out_dir / (md.stem + ".txt")).write_text(card)
        n += 1
    print(f"[audit_summary] wrote {n} cards to {out_dir}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
