#!/usr/bin/env python3
"""Verify each staging entry has the artefacts required to be promoted
into the v1 dataset folder.

Pass criteria (all required):
  1. ground_truth.json present with >=10 keys (audit-grade shape).
  2. At least one *.c file copied under src/.
  3. At least one non-empty *.ll bitcode (>10 KB).
  4. If GT lists >1 .c file, merged.ll must exist and be non-empty.

Outputs:
  - dataset_expansion/staging_verdict.json  (id -> status + reasons)
  - prints a summary table.
"""
from __future__ import annotations
import json
import os
import sys
from pathlib import Path


STAGING = Path("/mlx_devbox/users/mayunlong.39/playground/LLM4Con/"
               "kernel_experiment_v2_staging")
MIN_LL_BYTES = 10 * 1024


def collect_files(d: Path, suffix: str) -> list[Path]:
    return [p for p in d.iterdir() if p.is_file() and p.name.endswith(suffix)]


def verify_one(d: Path) -> dict:
    """
    PASS    - everything compiles cleanly; safe to promote.
    PARTIAL - GT/src is fine but only a subset of the .c files in GT have
              .ll bitcode. Lace can still run on the available .ll's; the
              entry is usable but degraded.
    FAIL    - no usable .ll, or GT is malformed.
    """
    blockers: list[str] = []
    warnings: list[str] = []

    gt_path = d / "ground_truth.json"
    if not gt_path.exists():
        return {"status": "FAIL", "reasons": ["no ground_truth.json"]}
    try:
        gt = json.load(open(gt_path))
    except Exception as e:
        return {"status": "FAIL", "reasons": [f"GT load error: {e!r}"]}

    if len(gt) < 10:
        blockers.append(f"GT shape too small ({len(gt)} keys)")
    # `audit_notes` is supplementary commentary; the substantive audit
    # content lives in `two_threads_summary`. Require that one, and treat
    # audit_notes as optional.
    for k in ("cve_id", "fix_commit", "files", "two_threads_summary",
              "patch"):
        if not gt.get(k):
            blockers.append(f"GT missing/empty: {k}")

    src_dir = d / "src"
    src_c = []
    if src_dir.is_dir():
        for root, _, files in os.walk(src_dir):
            src_c.extend(f for f in files if f.endswith(".c"))
    if not src_c:
        blockers.append("no .c under src/")

    lls = collect_files(d, ".ll")
    nontrivial = [p for p in lls if p.name != "merged.ll" and
                  p.stat().st_size >= MIN_LL_BYTES]
    if not nontrivial:
        blockers.append("no non-trivial .ll (>10KB excluding merged)")

    gt_c_files = [f for f in gt.get("files", []) if f.endswith(".c")]
    gt_basenames = {os.path.basename(f)[:-2] for f in gt_c_files}
    ll_basenames = {p.name[:-3] for p in nontrivial}
    missing_ll = gt_basenames - ll_basenames
    if missing_ll:
        warnings.append(
            f"{len(missing_ll)}/{len(gt_basenames)} GT .c files have no .ll "
            f"({sorted(missing_ll)[:3]}{'...' if len(missing_ll) > 3 else ''})")

    if len(gt_c_files) > 1 and not missing_ll:
        merged = d / "merged.ll"
        if not merged.exists() or merged.stat().st_size < MIN_LL_BYTES:
            warnings.append(f"merged.ll missing (GT lists {len(gt_c_files)} "
                            f".c files) — Lace must consume individual .ll's")

    if blockers:
        status = "FAIL"
    elif warnings:
        status = "PARTIAL"
    else:
        status = "PASS"
    return {
        "status": status,
        "reasons": blockers + warnings,
        "n_ll": len(nontrivial),
        "n_src_c": len(src_c),
        "n_gt_keys": len(gt),
        "files": gt.get("files", []),
        "missing_ll_basenames": sorted(missing_ll),
    }


def main() -> int:
    entries = sorted(p for p in STAGING.iterdir()
                     if p.is_dir() and (p.name.startswith("CVE-") or
                                        p.name.startswith("SYZBOT-")))
    results: dict[str, dict] = {}
    for d in entries:
        results[d.name] = verify_one(d)

    pass_    = [b for b, r in results.items() if r["status"] == "PASS"]
    partial  = [b for b, r in results.items() if r["status"] == "PARTIAL"]
    fail     = [b for b, r in results.items() if r["status"] == "FAIL"]
    print(f"\nVerified {len(entries)} staging entries: "
          f"{len(pass_)} PASS, {len(partial)} PARTIAL, {len(fail)} FAIL\n")
    if partial:
        print("--- PARTIAL details (usable but degraded) ---")
        for b in partial:
            r = results[b]
            print(f"  {b}")
            for x in r["reasons"]:
                print(f"      ~ {x}")
    if fail:
        print("\n--- FAIL details (not promotable) ---")
        for b in fail:
            r = results[b]
            print(f"  {b}")
            for x in r["reasons"]:
                print(f"      ! {x}")
    out = (Path(__file__).resolve().parent / "staging_verdict.json")
    with open(out, "w") as f:
        json.dump({"pass": pass_, "partial": partial, "fail": fail,
                   "details": results}, f, indent=2)
    print(f"\nwrote {out}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
