"""Loader for kernel_experiment/CVE-*/ artifacts.

Mirrors the conventions of scripts/batch_prepare.sh and
scripts/evaluate_recall.py:
  * cve_dir/ground_truth.json  -- canonical ground truth (cve_id,
                                   files, patch, summary, cwes, ...)
  * cve_dir/src/                -- vulnerable source tree (post
                                   patch-expander, files referenced
                                   by the CVE patch are reachable)
  * cve_dir/*.ll                -- LLVM bitcode for the Lace detector
                                   (not used by baselines, but its
                                   absence is a SKIP signal — same
                                   gate as scripts/evaluate_recall.py)
"""
from __future__ import annotations

import glob
import json
import os
from dataclasses import dataclass, field
from typing import Dict, List, Optional


_LLM4CON_HOME = os.environ.get(
    "LLM4CON_HOME",
    "/mlx_devbox/users/mayunlong.39/playground/LLM4Con",
)
EXPERIMENT_BASE = os.environ.get(
    "EXPERIMENT_BASE", os.path.join(_LLM4CON_HOME, "kernel_experiment")
)


@dataclass
class CVE:
    cve_id: str
    cve_dir: str
    ground_truth: Dict
    src_dir: Optional[str]
    files_touched: List[str] = field(default_factory=list)
    has_bitcode: bool = False

    @property
    def description(self) -> str:
        return (
            self.ground_truth.get("description")
            or self.ground_truth.get("summary")
            or ""
        )

    @property
    def patch(self) -> str:
        return self.ground_truth.get("patch") or ""

    @property
    def cwes(self) -> List[str]:
        return self.ground_truth.get("cwes") or []


def list_cves(experiment_base: str = EXPERIMENT_BASE) -> List[CVE]:
    """Discover prepared bug entries. Skips dirs that lack
    ground_truth.json.

    Walks both `CVE-*` and `SYZBOT-*` directories — the v2 dataset
    expansion added syzbot KCSAN data-race reports alongside the
    original CVE-only set.

    The same SKIP rule scripts/evaluate_recall.py:370 enforces — no
    .ll file means the entry was never compiled and Lace could not
    have detected anything; we exclude it from baselines too to keep
    the comparison apples-to-apples (Lace's denominator == baseline's
    denominator)."""
    cves: List[CVE] = []
    candidates: List[str] = []
    for pat in ("CVE-*", "SYZBOT-*"):
        candidates.extend(glob.glob(os.path.join(experiment_base, pat)))
    for d in sorted(candidates):
        gt_path = os.path.join(d, "ground_truth.json")
        if not os.path.isfile(gt_path):
            continue
        try:
            with open(gt_path, "r", encoding="utf-8", errors="ignore") as f:
                gt = json.load(f)
        except Exception:
            continue
        cve_id = gt.get("cve_id") or os.path.basename(d)
        src_dir = os.path.join(d, "src")
        if not os.path.isdir(src_dir):
            src_dir = None
        files_touched = (
            gt.get("affected_files_from_patch")
            or gt.get("files")
            or []
        )
        has_bitcode = bool(glob.glob(os.path.join(d, "*.ll")))
        cves.append(
            CVE(
                cve_id=cve_id,
                cve_dir=d,
                ground_truth=gt,
                src_dir=src_dir,
                files_touched=list(files_touched),
                has_bitcode=has_bitcode,
            )
        )
    return cves


def read_source(cve: CVE, max_chars_per_file: int = 200_000) -> Dict[str, str]:
    """Read patch-touched source files for the CVE. The path lookup
    matches batch_prepare.sh's layout: kernel_experiment/<CVE>/src/...
    The function falls back to a recursive search for the file's
    basename if the exact path layout differs.
    """
    if cve.src_dir is None:
        return {}
    out: Dict[str, str] = {}
    for rel in cve.files_touched:
        candidates = [
            os.path.join(cve.src_dir, rel),
            os.path.join(cve.src_dir, os.path.basename(rel)),
        ]
        # Recursive search as last resort.
        if not any(os.path.isfile(p) for p in candidates):
            for found in glob.glob(
                os.path.join(cve.src_dir, "**", os.path.basename(rel)),
                recursive=True,
            ):
                candidates.append(found)
        path = next((p for p in candidates if os.path.isfile(p)), None)
        if path is None:
            continue
        try:
            with open(path, "r", encoding="utf-8", errors="ignore") as f:
                text = f.read()
        except Exception:
            continue
        if len(text) > max_chars_per_file:
            text = (
                text[:max_chars_per_file]
                + f"\n\n/* ...<truncated at {max_chars_per_file} chars>... */\n"
            )
        out[rel] = text
    return out


def list_src_files(cve: CVE, suffixes=(".c", ".h")) -> List[str]:
    """Enumerate all C/H files under cve.src_dir (rel paths). Used by
    the Mythos baseline's sink-slice + file-ranking phases."""
    if cve.src_dir is None:
        return []
    files: List[str] = []
    for root, _, names in os.walk(cve.src_dir):
        for n in names:
            if n.endswith(suffixes):
                rel = os.path.relpath(os.path.join(root, n), cve.src_dir)
                files.append(rel)
    return sorted(files)


def reverse_patch(patch: str) -> str:
    """Given a fix patch, emit a diff that represents 'the PR that
    re-introduced the bug' — i.e. fix lines become deletions, original
    lines become additions. This is the most-faithful adaptation of
    Anthropic's Claude-Code-Security-Review prompt (which expects a
    PR diff) to the CVE setting.

    Notes:
      * We only swap '+' <-> '-' on hunk content lines; metadata
        lines (`+++`, `---`, `@@`, `diff --git`) are preserved.
      * Multi-file patches are handled token-line-by-line so the
        per-file headers stay intact.
    """
    lines = patch.splitlines()
    out: List[str] = []
    for ln in lines:
        if ln.startswith(("+++", "---", "diff --git", "@@", "index ",
                          "new file mode", "deleted file mode", "Binary ",
                          "From ", "Date:", "Subject:", "Signed-off-by:",
                          "Cc:", "Fixes:")):
            out.append(ln)
            continue
        if ln.startswith("+"):
            out.append("-" + ln[1:])
        elif ln.startswith("-"):
            out.append("+" + ln[1:])
        else:
            out.append(ln)
    return "\n".join(out)
