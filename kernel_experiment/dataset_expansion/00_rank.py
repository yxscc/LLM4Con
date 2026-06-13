#!/usr/bin/env python3
"""Step 1: build a ranked candidate list from the two source archives.

Strong concurrency signals we trust enough to short-list for manual
verification:

  TIER A  Syzbot KCSAN (data-race detector, 208 in archive)
          Title format "KCSAN: data-race in fn1 / fn2" identifies the
          two racing functions directly; KCSAN by construction only
          fires on a confirmed two-thread race.

  TIER B  Syzbot deadlock (possible deadlock detector, 414 in archive)
          Lockdep splat usually identifies the two acquisition orders;
          we still verify on a per-bug basis since some "deadlocks"
          are recursive locks (one thread).

  TIER C  Linux CVE with CWE-362 (race condition) — 461 in archive.
          The CVE description typically calls out the two contexts.

  TIER D  Linux CVE with CWE-667 (improper locking) — 363 in archive.
          Heuristically noisy; only keep those whose description has
          explicit concurrency keywords.

Output: candidates/<TIER>_<bug_id>.json — one JSON per candidate with
the minimal fields downstream tools need.
"""
from __future__ import annotations

import argparse
import json
import os
import re
from pathlib import Path
from typing import Any, Dict, List, Optional


HERE = Path(__file__).resolve().parent
DEFAULT_DS_ROOT = "/tmp/ds_inspect"


def desc_en(metadata: Dict[str, Any]) -> str:
    cve = metadata.get("cve", {}) if isinstance(metadata, dict) else {}
    for d in cve.get("descriptions", []):
        if d.get("lang") == "en":
            return d.get("value", "")
    return ""


def cwes_of(metadata: Dict[str, Any]) -> List[str]:
    cve = metadata.get("cve", {}) if isinstance(metadata, dict) else {}
    out: List[str] = []
    for w in cve.get("weaknesses", []):
        for dd in w.get("description", []):
            v = dd.get("value", "")
            if v.startswith("CWE-"):
                out.append(v)
    return out


_COMMIT_RE = re.compile(r"[?&]id=([0-9a-f]{20,40})\b")
_COMMIT_RE2 = re.compile(r"/commit/(?:\?id=)?([0-9a-f]{20,40})\b")
_KCSAN_TITLE_RE = re.compile(
    r"KCSAN:\s*data-race\s+in\s+([\w:.<>\-+*&|]+)\s*/\s*([\w:.<>\-+*&|]+)",
    re.IGNORECASE,
)


def extract_commit_sha(patch_context: Dict[str, Any]) -> Optional[str]:
    """Find the upstream-mainline commit SHA from patch_context.json.
    Prefer torvalds/linux URLs; fall back to gregkh/linux; otherwise
    take the first SHA seen.
    """
    sections = patch_context.get("sections", []) if isinstance(patch_context, dict) else []
    candidates: List[tuple] = []
    for sec in sections:
        url = sec.get("source_url", "") or ""
        for rx in (_COMMIT_RE, _COMMIT_RE2):
            m = rx.search(url)
            if not m:
                continue
            sha = m.group(1)
            score = 0
            if "torvalds" in url:
                score = 100
            elif "stable" in url:
                score = 50
            elif "gregkh" in url:
                score = 40
            candidates.append((score, sha, url))
            break
    if not candidates:
        return None
    candidates.sort(key=lambda t: -t[0])
    return candidates[0][1]


def files_from_patch_context(patch_context: Dict[str, Any]) -> List[str]:
    out: List[str] = []
    for sec in patch_context.get("sections", []) or []:
        for fi in sec.get("files", []) or []:
            p = fi.get("path") or ""
            if p:
                out.append(p)
    seen = set()
    dedup: List[str] = []
    for p in out:
        if p in seen:
            continue
        seen.add(p)
        dedup.append(p)
    return dedup


def looks_concurrency(text: str) -> List[str]:
    """Return matched keywords. We use the count later to bias ranking."""
    keys = [
        "data race", "data-race", "race condition", "race in ",
        "concurrent", "concurrently", "kcsan", "kasan",
        "deadlock", "use-after-free", "use after free", "uaf",
        "rcu_read_lock", "rcu_dereference", "list_for_each_entry_rcu",
        "spin_lock", "mutex_lock", "atomic_", "WRITE_ONCE", "READ_ONCE",
        "lockdep", "softirq", "tasklet", "workqueue", "work_struct",
        "interrupt", "irq", "preempt", "smp_",
    ]
    tl = text.lower()
    return [k for k in keys if k in tl]


def load_metadata_safely(path: str) -> Optional[Dict[str, Any]]:
    try:
        return json.load(open(path))
    except Exception:
        return None


def process_linux_cve(d: Path, existing: set) -> Optional[Dict[str, Any]]:
    cve_id = d.name
    if cve_id in existing:
        return None
    meta = load_metadata_safely(str(d / "metadata.json"))
    if not meta:
        return None
    desc = desc_en(meta)
    cwes = cwes_of(meta)
    if not desc:
        return None

    pctx_path = d / "patch_context.json"
    pctx = load_metadata_safely(str(pctx_path)) or {"sections": []}
    fix_sha = extract_commit_sha(pctx)
    files = files_from_patch_context(pctx)

    # The Linux archive's patch.diff is broken (Anubis HTML); patches
    # have to be re-fetched, but we DO have the per-file +/- lines
    # from patch_context.json. Capture them for the audit packet.
    diff_lines: List[Dict[str, Any]] = []
    for sec in pctx.get("sections", []) or []:
        for fi in sec.get("files", []) or []:
            cl = fi.get("changed_lines") or []
            if cl:
                diff_lines.append({
                    "path": fi.get("path"),
                    "changed_lines": cl,
                    "hunks": fi.get("hunks") or [],
                })

    # Tier scoring
    kws = looks_concurrency(desc)
    if "CWE-362" in cwes:
        tier = "C"
        score = 100 + len(kws)
    elif "CWE-667" in cwes and len(kws) >= 2:
        tier = "D"
        score = 60 + len(kws)
    elif any(c in cwes for c in ("CWE-416", "CWE-415", "CWE-367")) and len(kws) >= 3:
        tier = "D"
        score = 50 + len(kws)
    else:
        return None

    if not fix_sha:
        # Won't be able to verify; downgrade to nearly-rejected
        return None
    if not files:
        # No file info → hard to verify. Skip.
        return None

    return {
        "tier": tier,
        "score": score,
        "bug_id": cve_id,
        "source": "linux_cve",
        "title": cve_id,
        "description": desc[:2000],
        "cwes": cwes,
        "matched_keywords": kws,
        "fix_commit": fix_sha,
        "affected_files": files,
        "patch_context_hunks": diff_lines,
        "patch_diff_path": str(d / "patch.diff"),  # likely junk for linux_kernel
        "summary_md_path": str(d / "summary.md"),
        "src_dir": str(d),
    }


def process_syzbot(d: Path, existing: set) -> Optional[Dict[str, Any]]:
    bug_id = d.name
    if bug_id in existing:
        return None
    meta = load_metadata_safely(str(d / "metadata.json"))
    if not meta:
        return None
    desc = desc_en(meta)
    if not desc:
        return None
    title = desc.split("\n")[0].strip()
    tl = title.lower()

    pctx = load_metadata_safely(str(d / "patch_context.json")) or {"sections": []}
    fix_sha = extract_commit_sha(pctx)
    files = files_from_patch_context(pctx)

    diff_lines: List[Dict[str, Any]] = []
    for sec in pctx.get("sections", []) or []:
        for fi in sec.get("files", []) or []:
            cl = fi.get("changed_lines") or []
            if cl:
                diff_lines.append({
                    "path": fi.get("path"),
                    "changed_lines": cl,
                    "hunks": fi.get("hunks") or [],
                })

    kcsan = _KCSAN_TITLE_RE.search(title)
    if kcsan:
        fn1, fn2 = kcsan.group(1), kcsan.group(2)
        tier = "A"
        score = 200
        thread_hint = {"kind": "kcsan", "fn_a": fn1, "fn_b": fn2}
    elif "deadlock" in tl:
        tier = "B"
        score = 100
        thread_hint = {"kind": "deadlock", "raw_title": title}
    else:
        # KASAN UAF / 'race' titles — those are weaker signals, skip
        # at this pass to keep audit budget bounded. Can re-enable
        # with --include-uaf if we run short on candidates.
        return None

    if not fix_sha or not files:
        return None

    return {
        "tier": tier,
        "score": score,
        "bug_id": bug_id,
        "source": "syzbot",
        "title": title,
        "description": desc[:2000],
        "cwes": [],
        "thread_hint": thread_hint,
        "fix_commit": fix_sha,
        "affected_files": files,
        "patch_context_hunks": diff_lines,
        "patch_diff_path": str(d / "patch.diff"),
        "summary_md_path": str(d / "summary.md"),
        "src_dir": str(d),
    }


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--ds-root", default=DEFAULT_DS_ROOT)
    ap.add_argument("--existing-list",
                    default="/tmp/existing_cves.txt",
                    help="One-per-line list of CVE/bug IDs already in dataset")
    ap.add_argument("--out-dir", default=str(HERE / "candidates"))
    ap.add_argument("--max-per-tier", type=int, default=80)
    args = ap.parse_args()

    existing = set()
    if os.path.isfile(args.existing_list):
        with open(args.existing_list) as f:
            for ln in f:
                ln = ln.strip()
                if ln:
                    existing.add(ln)
    print(f"[rank] excluding {len(existing)} already-in-dataset entries")

    ds_root = Path(args.ds_root)
    out_dir = Path(args.out_dir)
    out_dir.mkdir(parents=True, exist_ok=True)

    all_candidates: List[Dict[str, Any]] = []

    lk_root = ds_root / "linux_kernel"
    if lk_root.is_dir():
        for d in sorted(lk_root.iterdir()):
            if not d.name.startswith("CVE-"):
                continue
            c = process_linux_cve(d, existing)
            if c:
                all_candidates.append(c)
    sz_root = ds_root / "syzbot"
    if sz_root.is_dir():
        for d in sorted(sz_root.iterdir()):
            if not d.name.startswith("SYZBOT-"):
                continue
            c = process_syzbot(d, existing)
            if c:
                all_candidates.append(c)

    # Rank within each tier.
    by_tier: Dict[str, List[Dict[str, Any]]] = {}
    for c in all_candidates:
        by_tier.setdefault(c["tier"], []).append(c)
    for tier, lst in by_tier.items():
        lst.sort(key=lambda c: -c["score"])

    kept: List[Dict[str, Any]] = []
    for tier in ("A", "B", "C", "D"):
        lst = by_tier.get(tier, [])
        cap = args.max_per_tier
        keep_n = min(len(lst), cap)
        print(f"[rank] tier {tier}: {len(lst)} candidates, keeping top {keep_n}")
        kept.extend(lst[:keep_n])

    # Write per-candidate stubs.
    for c in kept:
        out_path = out_dir / f"{c['tier']}_{c['bug_id']}.json"
        with open(out_path, "w") as f:
            json.dump(c, f, indent=2, ensure_ascii=False)

    # Index.
    index = {
        "n_total": len(kept),
        "by_tier": {t: len(by_tier.get(t, [])) for t in "ABCD"},
        "kept_by_tier": {
            t: sum(1 for c in kept if c["tier"] == t) for t in "ABCD"
        },
        "candidates": [
            {"tier": c["tier"], "bug_id": c["bug_id"],
             "score": c["score"], "title": c["title"]}
            for c in kept
        ],
    }
    with open(out_dir / "_index.json", "w") as f:
        json.dump(index, f, indent=2, ensure_ascii=False)
    print(f"[rank] wrote {len(kept)} candidate stubs to {out_dir}")
    print(f"[rank] index: {out_dir / '_index.json'}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
