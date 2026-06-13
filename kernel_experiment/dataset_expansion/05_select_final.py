#!/usr/bin/env python3
"""Pick the final ~50 'core' new entries from the 79 dedup'd accepts.

Priority:
  1. ALL tier-C (CWE-362) CVEs — they ground the dataset with publicly-
     numbered CVEs that reviewers can reference.
  2. From tier-A syzbot, spread across subsystems and prefer entries
     I personally reviewed in detail (see verdicts.json[*].notes).

Output:
  selection_core.json       — 50-ish bug_ids for the primary dataset
  selection_supplementary.json — remaining ~29 bug_ids
"""
from __future__ import annotations
import json
import pathlib
import random
from collections import defaultdict

BASE = pathlib.Path("/mlx_devbox/users/mayunlong.39/playground/LLM4Con/kernel_experiment/dataset_expansion")
verdicts = json.load(open(BASE/"verdicts.json"))["candidates"]
accepts = [bid for bid, v in verdicts.items() if v.get("status") == "accept"]

# Step 1: dedupe by mainline_sha. Prefer CVE-* over SYZBOT-*.
by_sha = defaultdict(list)
metas = {}
for bid in accepts:
    p = BASE/"evidence"/f"{bid}.meta.json"
    if not p.exists():
        continue
    meta = json.load(open(p))
    metas[bid] = meta
    by_sha[meta["mainline_sha"]].append(bid)

dedup_ids = []
for sha, lst in by_sha.items():
    lst.sort(key=lambda b: (0 if b.startswith("CVE-") else 1, b))
    dedup_ids.append(lst[0])
print(f"After dedupe: {len(dedup_ids)} unique bug ids")

# Step 2: separate CVE vs SYZBOT
cves = sorted([b for b in dedup_ids if b.startswith("CVE-")])
syzs = sorted([b for b in dedup_ids if b.startswith("SYZBOT-")])
print(f"  CVEs: {len(cves)}   SYZBOT: {len(syzs)}")

# Step 3: subsystem-balanced selection from SYZBOT side
def subsystem(bid: str) -> str:
    m = metas.get(bid, {})
    paths = m.get("c_paths") or []
    if not paths:
        return "(unknown)"
    p0 = paths[0]
    parts = p0.split("/")
    top = parts[0]
    # Treat include/ separately
    if top == "include" and len(parts) > 1:
        return f"include/{parts[1]}"
    return top

# Bucket syzbot by subsystem
buckets = defaultdict(list)
for b in syzs:
    buckets[subsystem(b)].append(b)

# Cap per-subsystem to balance coverage
SUBS_CAPS = {
    "net": 12,
    "include/net": 6,
    "include/linux": 3,
    "kernel": 5,
    "drivers": 5,
    "fs": 3,
    "sound": 3,
    "io_uring": 2,
    "mm": 1,
}

core_syzs = []
random.seed(0)
for sub, lst in buckets.items():
    cap = SUBS_CAPS.get(sub, 2)
    # Prefer entries the audit notes look 'rich' (longer notes ⇒ I
    # personally reviewed). Then alphabetical.
    def richness(b: str) -> int:
        return len(verdicts.get(b, {}).get("notes", ""))
    lst_sorted = sorted(lst, key=lambda b: (-richness(b), b))
    core_syzs.extend(lst_sorted[:cap])

# Make sure we have at least ~33 syzbots
budget = max(0, 50 - len(cves))  # 50 target = 17 CVE + ~33 syz
print(f"  budgeted syz slots: {budget}, picked so far: {len(core_syzs)}")
if len(core_syzs) > budget:
    core_syzs = core_syzs[:budget]
elif len(core_syzs) < budget:
    # Top up from unused syzbots
    extras = [b for b in syzs if b not in core_syzs]
    random.shuffle(extras)
    core_syzs.extend(extras[: budget - len(core_syzs)])

core = cves + core_syzs
supp = [b for b in dedup_ids if b not in core]
print(f"Final core: {len(core)} = {len(cves)} CVE + {len(core_syzs)} SYZBOT")
print(f"Supplementary: {len(supp)}")

with open(BASE/"selection_core.json", "w") as f:
    json.dump({"core_bug_ids": core,
               "subsystem_distribution": {
                   s: sum(1 for b in core if subsystem(b) == s)
                   for s in SUBS_CAPS}
               }, f, indent=2)
with open(BASE/"selection_supplementary.json", "w") as f:
    json.dump({"supplementary_bug_ids": supp}, f, indent=2)

print()
print("Core subsystem breakdown:")
core_subs = defaultdict(list)
for b in core:
    core_subs[subsystem(b)].append(b)
for sub, lst in sorted(core_subs.items(), key=lambda t: -len(t[1])):
    print(f"  {sub:24s} {len(lst):3d}")
