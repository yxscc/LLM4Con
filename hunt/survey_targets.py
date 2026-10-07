#!/usr/bin/env python3
"""Rank kernel subsystems as unknown-defect hunting targets.

Three things decide whether a subsystem is worth spending LLM budget on:

  size        A translation-unit group that is too large blows up the surface
              (and therefore Phase A/C cost); too small has nothing to find.
  concurrency How many synchronisation primitives the code actually uses. A
              subsystem with no locks/RCU/workqueues has no contracts to
              generate, however big it is.
  churn       How many of its commits in recent history are race/UAF fixes.
              A subsystem that keeps producing concurrency fixes is a
              subsystem where concurrency bugs still live.

Usage:
  survey_targets.py <kernel-tree> [--since v7.0] [--dirs-file f] [--json out]
"""
import argparse
import collections
import json
import os
import re
import subprocess
import sys

# Primitive families. Counted per family so that a file spamming one macro
# does not look more concurrent than one that genuinely mixes mechanisms.
PRIMS = {
    "lock":      r"\b(spin_lock|spin_unlock|raw_spin_lock|mutex_lock|mutex_unlock|"
                 r"down_write|down_read|up_write|up_read|write_lock|read_lock|"
                 r"spin_lock_bh|spin_lock_irqsave)\b",
    "rcu":       r"\b(rcu_read_lock|rcu_read_unlock|rcu_dereference\w*|"
                 r"rcu_assign_pointer|synchronize_rcu|call_rcu|kfree_rcu|"
                 r"RCU_INIT_POINTER|list_add_rcu|hlist_add_head_rcu)\b",
    "atomic":    r"\b(atomic_\w+|atomic64_\w+|refcount_\w+|cmpxchg|xchg|"
                 r"test_and_set_bit|test_and_clear_bit|set_bit|clear_bit)\b",
    "defer":     r"\b(INIT_WORK|INIT_DELAYED_WORK|queue_work|schedule_work|"
                 r"queue_delayed_work|cancel_work_sync|cancel_delayed_work_sync|"
                 r"flush_work|tasklet_\w+|timer_setup|mod_timer|del_timer\w*|"
                 r"hrtimer_\w+)\b",
    "thread":    r"\b(kthread_run|kthread_create|kthread_stop|request_irq|"
                 r"free_irq|napi_schedule|napi_enable|napi_disable)\b",
    "wait":      r"\b(wait_event\w*|wake_up\w*|complete|wait_for_completion\w*|"
                 r"completion|down_timeout|schedule_timeout)\b",
    "barrier":   r"\b(smp_mb|smp_rmb|smp_wmb|smp_store_release|smp_load_acquire|"
                 r"barrier|READ_ONCE|WRITE_ONCE)\b",
}
PRIMS = {k: re.compile(v) for k, v in PRIMS.items()}

# A commit that fixes a race is strong evidence the subsystem hosts more.
RACE_MSG = re.compile(
    r"\b(race|races|racy|deadlock|use-after-free|UAF|double[- ]free|"
    r"lockdep|data.?race|KCSAN|refcount|use after free|concurrent\w*|"
    r"lock(ing)? (bug|imbalance|inversion)|missing lock|unlocked)\b",
    re.I,
)

DEFAULT_DIRS = [
    # net/* : the subsystem family where the tool's measured recall is highest
    "net/sched", "net/tipc", "net/sctp", "net/smc", "net/tls", "net/rds",
    "net/unix", "net/nfc", "net/vmw_vsock", "net/bluetooth", "net/mptcp",
    "net/xfrm", "net/atm", "net/ax25", "net/rose", "net/netrom", "net/l2tp",
    "net/9p", "net/ceph", "net/sunrpc", "net/psample", "net/openvswitch",
    "net/mac80211", "net/wireless", "net/bridge", "net/dsa", "net/can",
    "net/key", "net/kcm", "net/qrtr", "net/caif", "net/x25", "net/llc",
    "net/dccp", "net/ethtool", "net/devlink", "net/handshake",
    # adjacent areas with proven hits in the 72-case set
    "io_uring", "block", "fs/jbd2", "fs/ext4", "fs/smb/client", "fs/fuse",
    "drivers/net/wireless/ath/ath11k", "drivers/net/wireless/mediatek/mt76",
    "drivers/vhost", "drivers/nvme/host", "drivers/md", "drivers/tty",
    "drivers/usb/gadget/function", "drivers/infiniband/core",
    "sound/core/seq", "virt/kvm",
]


def scan_dir(tree, d):
    path = os.path.join(tree, d)
    if not os.path.isdir(path):
        return None
    files = sorted(f for f in os.listdir(path) if f.endswith(".c"))
    if not files:
        return None
    loc = 0
    counts = collections.Counter()
    per_file = {}
    for f in files:
        fp = os.path.join(path, f)
        try:
            with open(fp, errors="ignore") as fh:
                text = fh.read()
        except OSError:
            continue
        n = text.count("\n")
        loc += n
        sub = collections.Counter()
        for name, rx in PRIMS.items():
            c = len(rx.findall(text))
            counts[name] += c
            sub[name] += c
        per_file[f] = {"loc": n, "prims": sum(sub.values())}
    return {
        "dir": d,
        "files": len(files),
        "loc": loc,
        "prims": dict(counts),
        "prim_total": sum(counts.values()),
        "families": sum(1 for v in counts.values() if v > 0),
        "per_file": per_file,
    }


def race_churn(tree, d, since):
    """Count recent commits touching d, and how many look like race fixes."""
    try:
        out = subprocess.run(
            ["git", "-C", tree, "log", "--no-merges", f"{since}..HEAD",
             "--format=%s", "--", d],
            capture_output=True, text=True, timeout=180,
        ).stdout
    except (subprocess.SubprocessError, OSError):
        return 0, 0
    subjects = [s for s in out.splitlines() if s.strip()]
    return len(subjects), sum(1 for s in subjects if RACE_MSG.search(s))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("tree")
    ap.add_argument("--since", default="v6.12",
                    help="churn window start ref (default v6.12)")
    ap.add_argument("--dirs-file")
    ap.add_argument("--json")
    ap.add_argument("--max-loc", type=int, default=60000,
                    help="flag dirs above this as needing TU splitting")
    args = ap.parse_args()

    dirs = DEFAULT_DIRS
    if args.dirs_file:
        dirs = [l.strip() for l in open(args.dirs_file) if l.strip()
                and not l.startswith("#")]

    rows = []
    for d in dirs:
        r = scan_dir(args.tree, d)
        if not r:
            print(f"  (skip, no .c) {d}", file=sys.stderr)
            continue
        r["commits"], r["race_fixes"] = race_churn(args.tree, d, args.since)
        kloc = max(r["loc"] / 1000.0, 0.001)
        r["prim_density"] = r["prim_total"] / kloc
        r["race_rate"] = (r["race_fixes"] / r["commits"]) if r["commits"] else 0.0
        # Hunting value: absolute race-fix history is the strongest prior, scaled
        # by how mechanism-diverse the code is; size is a cost, not a benefit,
        # so it only enters as the split flag below.
        r["score"] = round(r["race_fixes"] * r["families"] * (1 + r["race_rate"]), 1)
        r["needs_split"] = r["loc"] > args.max_loc
        rows.append(r)

    rows.sort(key=lambda r: -r["score"])

    hdr = (f"{'SUBSYSTEM':38s} {'files':>5s} {'kLOC':>6s} {'prims':>6s} "
           f"{'dens':>5s} {'fam':>3s} {'cmts':>5s} {'race':>5s} {'rate':>5s} "
           f"{'score':>7s}  split")
    print(hdr)
    print("-" * len(hdr))
    for r in rows:
        print(f"{r['dir']:38s} {r['files']:5d} {r['loc']/1000:6.1f} "
              f"{r['prim_total']:6d} {r['prim_density']:5.0f} {r['families']:3d} "
              f"{r['commits']:5d} {r['race_fixes']:5d} {r['race_rate']*100:4.0f}% "
              f"{r['score']:7.1f}  {'YES' if r['needs_split'] else ''}")

    if args.json:
        with open(args.json, "w") as f:
            json.dump(rows, f, indent=2)
        print(f"\nwrote {args.json}")


if __name__ == "__main__":
    sys.exit(main() or 0)
