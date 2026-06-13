#!/usr/bin/env python3
"""Step 2: build a self-contained evidence packet for each candidate.

For every candidate in candidates/*.json we produce evidence/<bug_id>.md
plus evidence/<bug_id>.meta.json.  Contents:

  * Title, tier, source archive, CVE description.
  * Resolved mainline commit (we walk every SHA referenced by the
    metadata until we find one that exists in our local linux.git).
  * Commit message of the fix.
  * Full mainline patch (real `git show`, not the bot-blocked HTML
    blob that lives inside the linux_kernel archive).
  * For every modified C file: the FULL function bodies that contain
    each changed hunk, taken from the vulnerable parent commit
    (commit~1) — this is what I need to confirm two-thread evidence.
  * For every modified function: a list of OTHER call sites in the
    kernel at commit~1 (cheap `git grep`), which is what reveals the
    second execution context (e.g. another caller from softirq, RCU
    walker, work_struct, etc.).

The script is idempotent and skips candidates that already have an
evidence file.  Failures are recorded in evidence/<bug_id>.error
so we can pick a different sample.
"""
from __future__ import annotations

import argparse
import json
import os
import re
import subprocess
import sys
from pathlib import Path
from typing import Any, Dict, List, Optional, Tuple


HERE = Path(__file__).resolve().parent
LINUX_GIT = "/mlx_devbox/users/mayunlong.39/playground/linux.git"


def git(args: List[str], *, cwd: str = LINUX_GIT, timeout: int = 60) -> Tuple[int, str]:
    proc = subprocess.run(["git", "-C", cwd] + args,
                          capture_output=True, text=True, timeout=timeout)
    return proc.returncode, (proc.stdout or "") + (proc.stderr or "")


def all_shas_in_metadata(meta: Dict[str, Any], pctx: Dict[str, Any]) -> List[str]:
    sha_re = re.compile(r"\b([0-9a-f]{20,40})\b")
    out: List[str] = []
    seen: set = set()

    # metadata.json: cve.references[*].url
    cve = meta.get("cve", {}) if isinstance(meta, dict) else {}
    for r in cve.get("references", []) or []:
        url = r.get("url", "") or ""
        for m in sha_re.finditer(url):
            sha = m.group(1)
            if sha not in seen:
                seen.add(sha)
                out.append(sha)

    # patch_context.json: sections[*].source_url
    for sec in (pctx.get("sections", []) or []):
        url = sec.get("source_url", "") or ""
        for m in sha_re.finditer(url):
            sha = m.group(1)
            if sha not in seen:
                seen.add(sha)
                out.append(sha)
    return out


def resolve_mainline(shas: List[str]) -> Optional[str]:
    """Return the first SHA present in the local mainline repo."""
    for sha in shas:
        if len(sha) < 12:
            continue
        rc, _ = git(["cat-file", "-t", sha])
        if rc == 0:
            return sha
    return None


_FN_HEAD = re.compile(
    r'^[ \t]*(?:static\s+|inline\s+|noinline\s+|__always_inline\s+|extern\s+|const\s+)*'
    r'(?:[\w\s\*]+?)\b(\w+)\s*\(',
)


def is_c_source(p: str) -> bool:
    return p.endswith((".c", ".h"))


def show_blob(sha: str, path: str) -> Optional[str]:
    rc, out = git(["show", f"{sha}:{path}"])
    if rc != 0:
        return None
    return out


def find_enclosing_functions(c_src: str, hunk_lines: List[int]) -> List[Dict[str, Any]]:
    """For each changed line number, find the enclosing function block
    using a brace-balance walk.  Return a list of unique
    {name, start_line, end_line, body}.
    """
    if not c_src:
        return []
    lines = c_src.splitlines()
    n = len(lines)

    # Pass 1: collect all function-like definitions and their brace spans.
    # A "function" here is anywhere a top-level identifier( ... )\n{ appears,
    # captured with a brace counter.
    funcs: List[Dict[str, Any]] = []
    i = 0
    while i < n:
        ln = lines[i]
        if "(" in ln and not ln.lstrip().startswith(("//", "*", "/*", "#")):
            m = _FN_HEAD.match(ln)
            if m:
                # Look forward for the opening brace within ~6 lines
                j = i
                while j < min(n, i + 8) and "{" not in lines[j]:
                    j += 1
                if j < n and "{" in lines[j]:
                    # Walk braces.
                    depth = 0
                    start = i
                    k = j
                    while k < n:
                        for ch in lines[k]:
                            if ch == "{":
                                depth += 1
                            elif ch == "}":
                                depth -= 1
                        if depth <= 0:
                            break
                        k += 1
                    end = k
                    if end > start and end - start < 2000:
                        funcs.append({
                            "name": m.group(1),
                            "start_line": start + 1,  # 1-based
                            "end_line": end + 1,
                        })
                    i = end + 1
                    continue
        i += 1

    # Pass 2: for each changed line, pick the smallest function containing it.
    hits: Dict[Tuple[str, int, int], Dict[str, Any]] = {}
    for hl in hunk_lines:
        best: Optional[Dict[str, Any]] = None
        for f in funcs:
            if f["start_line"] <= hl <= f["end_line"]:
                if best is None or (f["end_line"] - f["start_line"] <
                                    best["end_line"] - best["start_line"]):
                    best = f
        if best is None:
            continue
        key = (best["name"], best["start_line"], best["end_line"])
        if key not in hits:
            body = "\n".join(lines[best["start_line"] - 1:best["end_line"]])
            d = dict(best)
            d["body"] = body
            hits[key] = d

    return list(hits.values())


def collect_call_sites(sha: str, fn_name: str, exclude_file: str,
                       limit: int = 30) -> List[Tuple[str, int, str]]:
    """`git grep -n` the symbol at the parent commit, excluding the
    file we already show the function from.  Returns (path, lineno, snippet).
    """
    rc, out = git(["grep", "-n", "-I",
                   "--word-regexp", fn_name, sha, "--",
                   "*.c", "*.h"], timeout=60)
    if rc != 0:
        return []
    hits: List[Tuple[str, int, str]] = []
    for ln in out.splitlines():
        # format: <sha>:<path>:<line>:<text>
        parts = ln.split(":", 3)
        if len(parts) < 4:
            continue
        _, p, lno, txt = parts
        if p == exclude_file:
            continue
        try:
            lno_i = int(lno)
        except ValueError:
            continue
        hits.append((p, lno_i, txt.rstrip()))
        if len(hits) >= limit:
            break
    return hits


def extract_hunk_lines_from_pctx(pctx_files_for_path: List[Dict[str, Any]]) -> List[int]:
    """The dataset's patch_context.json already records changed_lines.
    These are -OLD- line numbers (pre-patch) ⇒ correct for parent commit.
    """
    out: List[int] = []
    for fi in pctx_files_for_path:
        for line_info in fi.get("changed_lines", []) or []:
            ln = line_info.get("old_line")
            if isinstance(ln, int):
                out.append(ln)
    return sorted(set(out))


def extract_hunk_lines_from_git_show(patch_text: str) -> Dict[str, List[int]]:
    """Parse the real `git show` patch and per-file return the OLD line
    numbers covered by every '-' or context line in each hunk.
    """
    by_file: Dict[str, List[int]] = {}
    cur_file: Optional[str] = None
    cur_old: int = 0
    for ln in patch_text.splitlines():
        if ln.startswith("diff --git "):
            cur_file = None
            cur_old = 0
            continue
        if ln.startswith("--- a/"):
            cur_file = ln[6:].strip()
            by_file.setdefault(cur_file, [])
            continue
        if ln.startswith("--- /dev/null"):
            cur_file = None
            continue
        m = re.match(r"^@@\s+-(\d+)(?:,(\d+))?\s+\+\d+", ln)
        if m and cur_file:
            cur_old = int(m.group(1))
            continue
        if cur_file is None:
            continue
        if ln.startswith("-") and not ln.startswith("---"):
            by_file[cur_file].append(cur_old)
            cur_old += 1
        elif ln.startswith("+") and not ln.startswith("+++"):
            pass  # no old-line advance
        elif ln.startswith(" "):
            cur_old += 1
    return by_file


def build_evidence(cand_json: Path, out_dir: Path, force: bool = False) -> str:
    c = json.load(open(cand_json))
    bug_id = c["bug_id"]
    out_md = out_dir / f"{bug_id}.md"
    out_meta = out_dir / f"{bug_id}.meta.json"
    out_err = out_dir / f"{bug_id}.error"

    if (out_md.exists() or out_err.exists()) and not force:
        return "skip"

    src_dir = Path(c["src_dir"])
    meta = json.load(open(src_dir / "metadata.json"))
    pctx = json.load(open(src_dir / "patch_context.json"))

    shas = all_shas_in_metadata(meta, pctx)
    mainline = resolve_mainline(shas)
    if not mainline:
        out_err.write_text(f"no mainline sha among {shas[:8]}\n")
        return "no_sha"

    rc, full_msg = git(["log", "-1", "--format=%H%n%s%n%n%b", mainline])
    if rc != 0:
        out_err.write_text(f"git log failed for {mainline}\n")
        return "git_log_fail"

    rc, patch = git(["show", "--no-color", mainline])
    if rc != 0:
        out_err.write_text(f"git show failed for {mainline}\n")
        return "git_show_fail"

    # Per-file changed OLD line numbers from the real patch.
    hunk_lines_by_path = extract_hunk_lines_from_git_show(patch)

    # Filter to .c / .h only (Lace handles C).
    c_paths = [p for p in hunk_lines_by_path if is_c_source(p)]
    if not c_paths:
        out_err.write_text("no .c/.h files in patch\n")
        return "no_c_files"

    parent = f"{mainline}^"

    file_blocks: List[Dict[str, Any]] = []
    for path in c_paths[:6]:  # cap files per CVE
        src = show_blob(parent, path)
        if src is None:
            file_blocks.append({"path": path, "error": "blob not found at parent"})
            continue
        hunks = sorted(set(hunk_lines_by_path[path]))
        enclosing = find_enclosing_functions(src, hunks)
        funcs = []
        for f in enclosing[:5]:
            callers = collect_call_sites(parent, f["name"], path)
            funcs.append({
                "name": f["name"],
                "start_line": f["start_line"],
                "end_line": f["end_line"],
                "body_preview_lines": f["end_line"] - f["start_line"] + 1,
                "body": f["body"][:8000],
                "external_call_sites": [
                    {"path": p, "line": l, "text": t}
                    for (p, l, t) in callers
                ],
            })
        file_blocks.append({
            "path": path,
            "n_changed_old_lines": len(hunks),
            "changed_old_lines_preview": hunks[:30],
            "enclosing_functions": funcs,
        })

    # Write markdown audit packet.
    lines: List[str] = []
    lines.append(f"# Evidence: {bug_id}  (tier {c['tier']}, score {c.get('score')})\n")
    lines.append(f"- source archive: **{c['source']}**")
    lines.append(f"- title: {c['title']}")
    lines.append(f"- mainline fix commit: `{mainline}`")
    lines.append(f"- candidate JSON: `{cand_json.name}`")
    if c.get("thread_hint"):
        lines.append(f"- thread_hint: `{json.dumps(c['thread_hint'])}`")
    if c.get("cwes"):
        lines.append(f"- CWEs: {', '.join(c['cwes'])}")
    lines.append("")
    lines.append("## Commit message\n")
    lines.append("```")
    lines.append(full_msg.strip())
    lines.append("```\n")
    lines.append("## CVE / bug description\n")
    lines.append("```")
    lines.append(c.get("description", "").strip())
    lines.append("```\n")
    lines.append("## Full mainline patch\n")
    lines.append("```diff")
    lines.append(patch[:20000])
    lines.append("```\n")
    for fb in file_blocks:
        lines.append(f"## File: `{fb['path']}`\n")
        if "error" in fb:
            lines.append(f"_{fb['error']}_\n")
            continue
        lines.append(f"- changed old-lines: {fb['changed_old_lines_preview']}\n")
        for fn in fb.get("enclosing_functions", []):
            lines.append(f"### Function `{fn['name']}` "
                         f"(L{fn['start_line']}–L{fn['end_line']}, "
                         f"{fn['body_preview_lines']} lines)\n")
            lines.append("```c")
            lines.append(fn["body"][:6000])
            lines.append("```\n")
            cs = fn.get("external_call_sites") or []
            if cs:
                lines.append(f"**External callers of `{fn['name']}` at parent**\n")
                for cc in cs[:20]:
                    lines.append(f"- `{cc['path']}:{cc['line']}` — `{cc['text']}`")
                lines.append("")
            else:
                lines.append(f"_(no external call sites found for `{fn['name']}`)_\n")

    out_md.write_text("\n".join(lines))
    meta_out = {
        "bug_id": bug_id,
        "tier": c["tier"],
        "title": c["title"],
        "mainline_sha": mainline,
        "all_shas_seen": shas,
        "c_paths": c_paths,
        "n_files_shown": len([fb for fb in file_blocks if "error" not in fb]),
        "n_functions_shown": sum(len(fb.get("enclosing_functions", []))
                                  for fb in file_blocks if "error" not in fb),
    }
    out_meta.write_text(json.dumps(meta_out, indent=2, ensure_ascii=False))
    return "ok"


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--candidates", default=str(HERE / "candidates"))
    ap.add_argument("--out-dir", default=str(HERE / "evidence"))
    ap.add_argument("--tier", default="",
                    help="comma-separated tier filter, e.g. 'A,B'")
    ap.add_argument("--limit", type=int, default=0,
                    help="max candidates to process this run (0 = all)")
    ap.add_argument("--force", action="store_true")
    args = ap.parse_args()

    out_dir = Path(args.out_dir)
    out_dir.mkdir(parents=True, exist_ok=True)

    cand_dir = Path(args.candidates)
    cand_files = sorted([p for p in cand_dir.glob("*.json")
                          if p.name != "_index.json"])
    tiers = set(t.strip() for t in args.tier.split(",") if t.strip())
    if tiers:
        cand_files = [p for p in cand_files if any(p.name.startswith(t + "_") for t in tiers)]
    if args.limit > 0:
        cand_files = cand_files[: args.limit]

    counts: Dict[str, int] = {}
    for i, p in enumerate(cand_files, 1):
        try:
            status = build_evidence(p, out_dir, force=args.force)
        except Exception as e:
            status = "exc"
            (out_dir / f"{p.stem}.error").write_text(repr(e))
        counts[status] = counts.get(status, 0) + 1
        if i % 5 == 0 or i == len(cand_files):
            print(f"  [{i}/{len(cand_files)}] last={p.stem} → {status}  "
                  f"counts={counts}", flush=True)

    print(f"[evidence] done. counts={counts}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
