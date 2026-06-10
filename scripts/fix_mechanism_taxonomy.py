#!/usr/bin/env python3
"""Mine ground_truth patches to derive the empirical fix-mechanism taxonomy.

Read-only. For each case we look at the patch diff (added/removed lines) plus
the CWE / description, and classify:

  * fix_mechanism: what synchronization guarantee the patch INTRODUCES
    (this tests Guarantee-vocabulary completeness)
  * bug_class:     what the violated invariant is (UAF / data-race / null / ...)
    (this tests Assume-vocabulary completeness)

The output is a histogram plus an explicit "unmapped" list: cases whose fix
mechanism does not map onto any proposed Guarantee predicate. Those are the
completeness gaps we must discuss.
"""

from __future__ import annotations

import json
import re
from collections import Counter
from pathlib import Path

BASE = Path("/mlx_devbox/users/mayunlong.39/playground/LLM4Con/kernel_experiment")

# --- Guarantee-mechanism detectors (matched on ADDED patch lines) ---
MECH = {
    "annot_atomic": re.compile(r"\b(READ_ONCE|WRITE_ONCE|data_race|ASSERT_ONCE)\s*\("),
    "rcu": re.compile(r"\b(rcu_read_lock|rcu_read_unlock|synchronize_rcu|call_rcu|"
                      r"rcu_assign_pointer|rcu_dereference\w*|kfree_rcu|rcu_replace_pointer)\b"),
    "lock": re.compile(r"\b(spin_lock\w*|spin_unlock\w*|raw_spin_\w+|mutex_lock\w*|mutex_unlock|"
                       r"read_lock\w*|write_lock\w*|down_\w+|up_\w+|up|down|seqlock|write_seqlock|"
                       r"read_seqbegin|local_bh_disable|local_bh_enable|local_irq_save|"
                       r"local_irq_restore|preempt_disable|preempt_enable|lockdep|"
                       r"guard|scoped_guard|lock_sock\w*|release_sock|bh_lock_sock\w*|"
                       r"bh_unlock_sock|spin_lock_irqsave|spin_unlock_irqrestore)\b"),
    "barrier": re.compile(r"\b(smp_mb|smp_wmb|smp_rmb|smp_store_release|smp_load_acquire|"
                          r"wmb|rmb|mb|barrier|dma_wmb|dma_rmb)\s*\("),
    "refcount": re.compile(r"\b(refcount_\w+|kref_\w+|atomic_inc_not_zero|"
                           r"\w*_get|\w*_put|get_\w+|put_\w+|hold|\w*_hold|"
                           r"\w*_ref_freeze|\w*_ref_unfreeze|folio_ref_\w+|page_ref_\w+|"
                           r"try_get\w*|\w*_tryget\w*)\b"),
    "atomic_op": re.compile(r"\b(atomic_\w+|atomic64_\w+|cmpxchg\w*|xchg|try_cmpxchg|"
                            r"test_and_set_bit|test_and_clear_bit|set_bit|clear_bit|test_bit)\b"),
    "wait_join": re.compile(r"\b(flush_work\w*|cancel_work_sync|cancel_delayed_work\w*|"
                            r"kthread_stop|wait_for_completion\w*|complete\w*|del_timer_sync|"
                            r"timer_delete_sync|synchronize_irq|flush_workqueue|drain_\w+|"
                            r"wait_event\w*|napi_disable|napi_synchronize)\b"),
    "flag_state": re.compile(r"\b(set_bit|clear_bit|test_bit|test_and_\w+_bit|"
                             r"\w+->\w*(flags|state|valid|active|dead|closing|done)\w*)\b"),
}

# Guarantee predicates each mechanism maps onto, in the proposed vocabulary.
MECH_TO_PREDICATE = {
    "annot_atomic": "atomic(loc)",          # single-location access atomicity
    "lock": "serialize(L,region)",
    "rcu": "order(a<b via rcu) / serialize(rcu_rs)",
    "barrier": "order(a<b via barrier)",
    "refcount": "counts(R)",
    "atomic_op": "atomic(loc) / order(via RMW)",
    "wait_join": "order(a<b via join)",
    "flag_state": "order(a<b via published-flag)",
}

CWE_BUG = {
    "CWE-416": "UAF",
    "CWE-415": "double_free",
    "CWE-476": "null_deref",
    "CWE-362": "data_race",
    "CWE-457": "uninit",
    "CWE-125": "oob_read",
    "CWE-787": "oob_write",
    "CWE-190": "overflow",
    "CWE-200": "infoleak",
    "CWE-667": "lock",
    "CWE-833": "deadlock",
}

DESC_BUG = [
    (re.compile(r"use[- ]after[- ]free|\bUAF\b", re.I), "UAF"),
    (re.compile(r"double[- ]free", re.I), "double_free"),
    (re.compile(r"null[- ]?ptr|null pointer|NULL deref", re.I), "null_deref"),
    (re.compile(r"data[- ]race|KCSAN", re.I), "data_race"),
    (re.compile(r"uninitializ", re.I), "uninit"),
    (re.compile(r"out[- ]of[- ]bounds|overflow|OOB", re.I), "oob"),
    (re.compile(r"deadlock", re.I), "deadlock"),
    (re.compile(r"refcount|reference count|use count", re.I), "refcount"),
]

REORDER_HINT = re.compile(r"\b(before|after|reorder|order|move|earlier|prior to|"
                          r"publish|initializ\w+ before)\b", re.I)


def added_removed(patch: str) -> tuple[str, str]:
    add, rem = [], []
    for ln in patch.splitlines():
        if ln.startswith("+") and not ln.startswith("+++"):
            add.append(ln[1:])
        elif ln.startswith("-") and not ln.startswith("---"):
            rem.append(ln[1:])
    return "\n".join(add), "\n".join(rem)


def classify_mechanisms(added: str) -> list[str]:
    hits = []
    for name, rx in MECH.items():
        if rx.search(added):
            hits.append(name)
    # refcount/flag_state/atomic_op are noisy; only keep them if they are the
    # *dominant* signal (no lock/rcu/annot present) OR clearly refcount-shaped.
    return hits


def bug_class(cwes: list[str], desc: str) -> str:
    for c in cwes:
        if c in CWE_BUG and CWE_BUG[c] not in ("lock",):
            # prefer a more specific desc match for data_race
            pass
    cls = set()
    for c in cwes:
        if c in CWE_BUG:
            cls.add(CWE_BUG[c])
    for rx, lab in DESC_BUG:
        if rx.search(desc):
            cls.add(lab)
    # priority ordering for a single primary label
    for pref in ("UAF", "double_free", "null_deref", "uninit", "oob", "oob_read",
                 "oob_write", "overflow", "refcount", "deadlock", "data_race", "infoleak"):
        if pref in cls:
            return pref
    return "other"


def main() -> int:
    cases = sorted(p.parent.name for p in BASE.glob("*/ground_truth.json"))
    rows = []
    mech_hist: Counter[str] = Counter()
    pred_hist: Counter[str] = Counter()
    bug_hist: Counter[str] = Counter()
    primary_mech_hist: Counter[str] = Counter()
    unmapped = []

    # rank for choosing a single "primary" mechanism (strongest ordering signal first)
    primary_rank = ["rcu", "wait_join", "barrier", "refcount", "lock",
                    "annot_atomic", "atomic_op", "flag_state"]

    for case in cases:
        gt = json.loads((BASE / case / "ground_truth.json").read_text(errors="ignore"))
        patch = gt.get("patch", "") or ""
        desc = (gt.get("description", "") or "") + "\n" + (gt.get("fix_commit_message", "") or "")
        cwes = gt.get("cwes", []) or []
        added, removed = added_removed(patch)
        mechs = classify_mechanisms(added)
        bug = bug_class(cwes, desc)

        primary = next((m for m in primary_rank if m in mechs), None)
        # annotation-only refinement: if the ONLY mechanism is annot_atomic
        only_annot = mechs == ["annot_atomic"] or (set(mechs) == {"annot_atomic"})
        # publish/reorder = no sync primitive added, but statements moved/removed
        # or the description explicitly talks about ordering of init/expose.
        reorder = (not mechs) and (bool(REORDER_HINT.search(desc)) or not added.strip())

        for m in mechs:
            mech_hist[m] += 1
            pred_hist[MECH_TO_PREDICATE[m]] += 1
        bug_hist[bug] += 1
        if primary:
            primary_mech_hist[primary] += 1
        elif reorder:
            primary_mech_hist["reorder/publish(no-primitive)"] += 1
        else:
            primary_mech_hist["UNMAPPED"] += 1
            unmapped.append((case, bug, cwes, added[:160].replace("\n", " ")))

        rows.append({
            "case": case, "bug": bug, "cwes": ",".join(cwes),
            "mechs": "|".join(mechs) or ("reorder?" if reorder else "NONE"),
            "primary": primary or ("reorder/publish" if reorder else "UNMAPPED"),
            "only_annot": only_annot,
        })

    out = BASE / "fix_mechanism_taxonomy.json"
    out.write_text(json.dumps({
        "n_cases": len(cases),
        "mechanism_hits": mech_hist.most_common(),
        "predicate_hits": pred_hist.most_common(),
        "primary_mechanism": primary_mech_hist.most_common(),
        "bug_class": bug_hist.most_common(),
        "rows": rows,
        "unmapped": unmapped,
    }, indent=2, ensure_ascii=False))

    print(f"cases={len(cases)}  wrote {out}")
    print("\n== primary fix mechanism (one per case) ==")
    for k, v in primary_mech_hist.most_common():
        print(f"  {k:34s} {v:3d}  ({v*100//len(cases)}%)")
    print("\n== bug class ==")
    for k, v in bug_hist.most_common():
        print(f"  {k:20s} {v:3d}")
    print("\n== mechanism co-occurrence (any hit) ==")
    for k, v in mech_hist.most_common():
        print(f"  {k:14s} {v:3d}  -> {MECH_TO_PREDICATE[k]}")
    print(f"\n== UNMAPPED ({len(unmapped)}) ==")
    for case, bug, cwes, snip in unmapped:
        print(f"  {case:30s} bug={bug:12s} cwe={cwes} add='{snip}'")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
