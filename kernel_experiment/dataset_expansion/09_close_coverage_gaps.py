#!/usr/bin/env python3
"""Augment ground_truth.json + cve_survey.csv for the small set of
entries where the audit-identified racing pair was split across files
and only one half made it into the compiled bitcode. For each entry
below we add the sibling .c that defines the *other* racing function.

The additions were verified by `git grep` at the parent commit (see
08_verify_coverage.py / staging_coverage.json).
"""
from __future__ import annotations
import csv
import json
import os
from pathlib import Path

PROD = Path("/mlx_devbox/users/mayunlong.39/playground/LLM4Con/"
            "kernel_experiment")
CSV_PATH = PROD / "cve_survey.csv"

# bug_id -> list of additional .c files (paths relative to kernel root)
EXTRA: dict[str, list[str]] = {
    "SYZBOT-123b88b9ddea8e98": [
        # tcp_add_backlog / tcp_grow_window racing with sock.c / tipc
        "net/ipv4/tcp_ipv4.c",
        "net/ipv4/tcp_input.c",
    ],
    "SYZBOT-2e4de7fe846aba66": [
        # gro_normal_list / napi_busy_loop racing with tun_napi_poll
        "net/core/dev.c",
    ],
    "SYZBOT-5366159cc4c1d817": [
        # run_timer_softirq racing with hrtimer code
        "kernel/time/timer.c",
    ],
    "SYZBOT-5676077ba016d741": [
        # ip_finish_output2 racing with ip_tunnel_xmit / ip6_tunnel_xmit
        "net/ipv4/ip_output.c",
    ],
    "SYZBOT-5a486fef3de40e0d": [
        # io_sqe_files_register racing with __se_sys_io_uring_register
        "io_uring/rsrc.c",
    ],
    "SYZBOT-5cce5938c6c2c518": [
        # __ip_make_skb self-race; called from datagram/inet/dccp/chtls
        "net/ipv4/ip_output.c",
    ],
}


def update_gt(bug_id: str, extra: list[str]) -> list[str]:
    gt_path = PROD / bug_id / "ground_truth.json"
    gt = json.load(open(gt_path))
    files = list(gt.get("files", []))
    affected = list(gt.get("affected_files_from_patch", []))
    for f in extra:
        if f not in files:
            files.append(f)
    gt["files"] = files
    gt["affected_files_from_patch"] = list(dict.fromkeys(affected + extra))
    gt["coverage_augmentation"] = {
        "added_files": extra,
        "rationale": (
            "08_verify_coverage.py reported these source files as defining "
            "the audit-identified racing function whose body was not in the "
            "original compiled bitcode. Adding them so Lace can observe "
            "both sides of the race."
        ),
    }
    with open(gt_path, "w") as f:
        json.dump(gt, f, indent=2, ensure_ascii=False)
    return files


def update_csv(updates: dict[str, list[str]]) -> None:
    rows: list[list[str]] = []
    with open(CSV_PATH) as f:
        reader = csv.reader(f)
        header = next(reader)
        for row in reader:
            if not row:
                continue
            bug = row[0]
            if bug in updates:
                row[2] = ";".join(updates[bug])
            rows.append(row)
    with open(CSV_PATH, "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(header)
        for r in rows:
            w.writerow(r)


def main() -> int:
    updates: dict[str, list[str]] = {}
    for bug_id, extra in EXTRA.items():
        d = PROD / bug_id
        if not d.is_dir():
            print(f"  skip {bug_id}: no production dir")
            continue
        new_files = update_gt(bug_id, extra)
        updates[bug_id] = new_files
        print(f"  + {bug_id}  added {extra}  -> {len(new_files)} files")

        # Clear stale .ll + log so batch_prepare re-runs
        for ext in ("*.ll", "*_compile.log", "llvm-link.log",
                    "expansion.log", "expansion_report.json"):
            for p in d.glob(ext):
                p.unlink()

    update_csv(updates)
    print(f"\nupdated {CSV_PATH}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
