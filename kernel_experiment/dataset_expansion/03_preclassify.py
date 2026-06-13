#!/usr/bin/env python3
"""Pre-classify each evidence card so manual review focuses on the
uncertain ones.

Rules (all heuristic; do NOT auto-accept):

  PROBABLY_GOOD: KCSAN-tier (A) AND the patch contains real
                 synchronization primitives — adds a lock, atomic_*,
                 READ_ONCE+WRITE_ONCE on same field, smp_*, percpu
                 conversion — AND no obvious red flag.

  PROBABLY_BAD : patch is only data_race() annotation, or just
                 ASSERT_*, or `nbd` socket-type-style defensive
                 input check, or removes a lock without adding one.

  NEEDS_REVIEW : everything else.

Output: preclassify.tsv  (bug_id\ttier\tlabel\tnotes\ttitle)
"""
from __future__ import annotations
import json
import re
import sys
from pathlib import Path


HERE = Path(__file__).resolve().parent


def patch_metrics(patch: str) -> dict:
    """Count primitives only on '+' lines (real additions),
    deletions on '-' lines.  Anything outside +/- (context/headers)
    is ignored.
    """
    added_lines = [ln[1:] for ln in patch.splitlines()
                   if ln.startswith("+") and not ln.startswith("+++")]
    removed_lines = [ln[1:] for ln in patch.splitlines()
                     if ln.startswith("-") and not ln.startswith("---")]
    added_blob = "\n".join(added_lines)
    removed_blob = "\n".join(removed_lines)
    pm = {
        "data_race_annot": len(re.findall(r"\bdata_race\s*\(", added_blob)),
        "assert_excl": len(re.findall(r"\bASSERT_EXCLUSIVE_", added_blob)),
        "read_once": len(re.findall(r"\bREAD_ONCE\s*\(", added_blob)),
        "write_once": len(re.findall(r"\bWRITE_ONCE\s*\(", added_blob)),
        "atomic_call": len(re.findall(r"\batomic[0-9_a-z]*_(?:read|set|inc|dec|add|sub|or|and|xor|cmpxchg|xchg|fetch)\s*\(", added_blob)),
        "atomic_call_rm": len(re.findall(r"\batomic[0-9_a-z]*_(?:read|set|inc|dec|add|sub|or|and|xor|cmpxchg|xchg|fetch)\s*\(", removed_blob)),
        "smp_call": len(re.findall(r"\bsmp_(?:rmb|wmb|mb|load_acquire|store_release|cond_load_acquire)\b", added_blob)),
        "lock_add": len(re.findall(r"\b(spin_lock|raw_spin_lock|mutex_lock|read_lock|write_lock|rcu_read_lock|down_(?:read|write)|lock_sock|bh_lock_sock|local_irq_save|local_bh_disable|preempt_disable)", added_blob)),
        "lock_del": len(re.findall(r"\b(spin_lock|raw_spin_lock|mutex_lock|read_lock|write_lock|rcu_read_lock|down_(?:read|write)|lock_sock|bh_lock_sock|local_irq_save|local_bh_disable|preempt_disable)", removed_blob)),
        "barrier_add": len(re.findall(r"\bbarrier\s*\(\s*\)", added_blob)),
        "lines_added": len(added_lines),
        "lines_removed": len(removed_lines),
        "files_touched": len(re.findall(r"^diff --git ", patch, re.MULTILINE)),
    }
    # Treat conversion atomic_op → atomic_op (same count rm/add) as no net change.
    pm["atomic_net_add"] = max(0, pm["atomic_call"] - pm["atomic_call_rm"])
    return pm


def classify(tier: str, title: str, subj: str, patch: str) -> tuple:
    pm = patch_metrics(patch)
    notes = []

    # Hard reject patterns
    if pm["data_race_annot"] >= 1 and pm["lock_add"] == 0 and pm["atomic_call"] == 0:
        notes.append(f"data_race_annot={pm['data_race_annot']}")
        return "PROBABLY_BAD", notes, pm
    if "false positive" in subj.lower() or "annotate" in subj.lower() and pm["lock_add"] == 0:
        # 'annotate' with no lock add => annotation only
        if pm["read_once"] + pm["write_once"] == 0 and pm["atomic_call"] == 0:
            notes.append("subject=annotate-only")
            return "PROBABLY_BAD", notes, pm
    if pm["lock_del"] > 0 and pm["lock_add"] == 0:
        notes.append(f"net-lock-removed={pm['lock_del']}")
        return "PROBABLY_BAD", notes, pm

    # Net-new synchronization actually added on '+' lines?
    real_sync = (
        pm["lock_add"] > pm["lock_del"]
        or pm["atomic_net_add"] > 0
        or pm["smp_call"] > 0
        or pm["barrier_add"] > 0
        or (pm["read_once"] > 0 and pm["write_once"] > 0)
        # KCSAN race + at least one *_ONCE on '+' lines is acceptable,
        # since the other side is often already protected.
        or (tier == "A" and (pm["read_once"] > 0 or pm["write_once"] > 0))
    )

    if tier == "A" and real_sync:
        notes.append("kcsan+real_sync")
        return "PROBABLY_GOOD", notes, pm

    if tier in ("C", "D") and real_sync:
        notes.append("cve+real_sync")
        return "PROBABLY_GOOD", notes, pm

    if tier == "B":
        # Deadlock cases are inherently noisy — never auto-good.
        return "NEEDS_REVIEW", ["tier=B_deadlock"], pm

    if not real_sync:
        notes.append("no_real_sync_primitive")
        return "NEEDS_REVIEW", notes, pm

    return "NEEDS_REVIEW", notes, pm


def extract_subject(md: str) -> str:
    m = re.search(r"^##\s+Commit subject\s*\n\s*([^\n]+)", md, re.MULTILINE)
    if m:
        return m.group(1).strip()
    return ""


def extract_title(md: str) -> str:
    m = re.search(r"^-\s+title:\s*(.+)$", md, re.MULTILINE)
    return m.group(1).strip() if m else ""


def extract_tier(md: str) -> str:
    m = re.search(r"\btier\s+([A-D])", md)
    return m.group(1) if m else "?"


def extract_patch_text(md: str) -> str:
    """Pull the contents of the first ```diff fenced block."""
    m = re.search(r"```diff\s*\n(.*?)\n```", md, re.DOTALL)
    return m.group(1) if m else ""


def main() -> int:
    ev_dir = HERE / "evidence"
    rows = []
    for md_path in sorted(ev_dir.glob("*.md")):
        md = md_path.read_text()
        bug_id = md_path.stem
        tier = extract_tier(md)
        title = extract_title(md)
        subj = extract_subject(md)
        patch = extract_patch_text(md)
        label, notes, pm = classify(tier, title, subj, patch)
        rows.append((bug_id, tier, label, ";".join(notes),
                     pm["lock_add"], pm["lock_del"], pm["read_once"],
                     pm["write_once"], pm["atomic_call"], pm["data_race_annot"],
                     title))

    out = HERE / "preclassify.tsv"
    with open(out, "w") as f:
        f.write("bug_id\ttier\tlabel\tnotes\tlock+\tlock-\tR_ONCE\tW_ONCE\tatomic\tdata_race\ttitle\n")
        for r in rows:
            f.write("\t".join(str(x) for x in r) + "\n")
    print(f"[preclassify] wrote {len(rows)} rows to {out}")

    cnt = {}
    for r in rows:
        cnt[r[2]] = cnt.get(r[2], 0) + 1
    for k, v in sorted(cnt.items()):
        print(f"  {k}: {v}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
