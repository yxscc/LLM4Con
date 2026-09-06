#!/usr/bin/env python3
"""Record what the shipped automatic entry discovery finds on the 72 cases.

The dataset is normally run with LACE_ENTRYPOINTS set to the hand-configured
roots. Here that variable is deliberately left unset so the detector falls back
to structural auto-discovery, and the resulting roots are written out so they
can be scored against the same ground truth as a candidate rule.

Only the static half of the pipeline runs (LACE_EARLY_EXIT_AFTER_SURFACE), so
no LLM credit is consumed.

Usage:
    python3 run_auto_entry_baseline.py [--jobs N] [--case NAME ...]
"""

import argparse
import concurrent.futures as cf
import json
import os
import re
import subprocess
import time
from pathlib import Path

HERE = Path(__file__).resolve().parent
ROOT = HERE.parent
DETECTOR = Path(os.environ.get("DETECTOR", ROOT / "Release-build" / "llm_detector"))
DUMP_ROOT = HERE / "auto_entry_dumps"
OUT = HERE / "auto_entry_baseline.json"
STAMP = time.strftime("%Y%m%d_%H%M%S")
CASE_TIMEOUT = int(os.environ.get("CASE_TIMEOUT", "1200"))

RE_STRUCT = re.compile(
    r"\[Auto-Entry Structural\] (\d+) init/exit section, (\d+) EXPORT_SYMBOL, "
    r"(\d+) syscall wrapper, (\d+) ops-table member, (\d+) indirect-fork sink, "
    r"(\d+) public-wrapper alias, (\d+) cross-TU public")
RE_TIER = re.compile(r"\[Auto-Entry Tiering\] thread roots: (\d+)/(\d+) "
                     r"\(strong=(\d+), weak_kept=(\d+)")
SIGNAL_KEYS = ["section_init", "export_symbol", "syscall", "ops_member",
               "fork_sink", "public_alias", "cross_tu"]


def pick_bitcode(case_dir):
    lls = sorted(case_dir.glob("*.ll"))
    names = [p.name for p in lls]
    if "merged.ll" in names:
        return "merged.ll"
    if "snd-seq.ll" in names:
        return "snd-seq.ll"
    if not lls:
        return ""
    return max(lls, key=lambda p: p.stat().st_size).name


def newest_surface(case):
    cands = sorted(DUMP_ROOT.glob(f"{case}_*/vulnerability_surface.json"),
                   key=lambda p: p.stat().st_mtime)
    return cands[-1] if cands else None


def run_case(case):
    d = HERE / case
    bc = pick_bitcode(d) if d.is_dir() else ""
    if not bc:
        return case, {"status": "NO_BITCODE"}

    env = os.environ.copy()
    env.update({
        "LACE_STATIC_COMPOSE": "1",
        "LACE_CONTRACT_L2": "1",
        "LACE_ENABLE_FLOW_PRIOR": "0",
        "LACE_EARLY_EXIT_AFTER_SURFACE": "1",
        "LACE_DUMP_ROOT": str(DUMP_ROOT),
    })
    # Leaving these unset is what selects automatic discovery.
    env.pop("LACE_ENTRYPOINTS", None)
    env.pop("LACE_ENTRYPOINTS_FILE", None)
    env.pop("LACE_SELF_RACE", None)

    log = d / f"auto_entry_{STAMP}.log"
    cmd = ["timeout", str(CASE_TIMEOUT), str(DETECTOR),
           "--input-bc", bc, "--input-src", "src",
           "--legacy-workflow", "--abl-contract", "on",
           "--llm-provider", "openai",
           "--llm-url", "http://localhost", "--llm-key", "dummy",
           "--llm-model", "dummy"]

    t0 = time.time()
    with log.open("w") as lf:
        proc = subprocess.run(cmd, cwd=d, env=env, stdout=lf,
                              stderr=subprocess.STDOUT)
    wall = int(time.time() - t0)
    text = log.read_text(errors="ignore")

    rec = {"status": "OK" if proc.returncode in (0, 1) else f"rc={proc.returncode}",
           "wall_s": wall, "bitcode": bc}
    if proc.returncode == 124:
        rec["status"] = "TIMEOUT"

    m = RE_STRUCT.search(text)
    if m:
        rec["signals"] = dict(zip(SIGNAL_KEYS, (int(x) for x in m.groups())))
    m = RE_TIER.search(text)
    if m:
        rec["tiering"] = {"kept": int(m.group(1)), "total": int(m.group(2)),
                          "strong": int(m.group(3)), "weak_kept": int(m.group(4))}

    surf = newest_surface(case)
    if surf:
        try:
            data = json.loads(surf.read_text())
            rec["entries"] = sorted({t["entry_function"]
                                     for t in data.get("threads", [])})
        except (json.JSONDecodeError, KeyError) as exc:
            rec["surface_error"] = str(exc)
    rec["n_entries"] = len(rec.get("entries", []))
    return case, rec


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--jobs", type=int, default=4)
    ap.add_argument("--case", nargs="*")
    args = ap.parse_args()

    gt = json.loads((HERE / "dataset_entrypoints.json").read_text())
    cases = args.case or sorted(gt)
    DUMP_ROOT.mkdir(exist_ok=True)

    if not DETECTOR.exists():
        raise SystemExit(f"detector not found: {DETECTOR}")

    out = {}
    if OUT.exists():
        out = json.loads(OUT.read_text())

    done = 0
    with cf.ThreadPoolExecutor(max_workers=args.jobs) as ex:
        futs = {ex.submit(run_case, c): c for c in cases}
        for fut in cf.as_completed(futs):
            case, rec = fut.result()
            out[case] = rec
            done += 1
            print(f"[{done}/{len(cases)}] {case:<28} {rec['status']:<10} "
                  f"entries={rec['n_entries']:<4} {rec.get('wall_s', 0)}s",
                  flush=True)
            OUT.write_text(json.dumps(out, indent=1, sort_keys=True))

    ok = [c for c, r in out.items() if r.get("status") == "OK"]
    tot = sum(out[c]["n_entries"] for c in ok)
    print(f"\ncompleted {len(ok)} case(s); mean entries/case "
          f"{tot / max(len(ok), 1):.1f}")
    print(f"wrote {OUT}")


if __name__ == "__main__":
    main()
