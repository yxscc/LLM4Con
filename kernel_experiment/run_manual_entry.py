#!/usr/bin/env python3
"""Run the static-composition detector with MANUAL thread-entry restriction.

For each case we read its thread roots from dataset_entrypoints.json (derived
from flow_annotation.json's true_interleaving.thread_a/thread_b.entry.function)
and pass them via LACE_ENTRYPOINTS, which disables automatic entry discovery
and uses only those roots (see include/CCPG/ManualEntryConfig.h).

Scientific guardrail: we only configure thread ENTRY functions (the concurrent
contexts). The racing object / field is NEVER provided -- the detector still
discovers it on its own.

Usage:
  LLM_BASE_URL=... LLM_API_KEY=... LLM_MODEL=gpt-5.5-2026-04-24 \
    python3 run_manual_entry.py CVE-2016-7911 CVE-2013-1792 ...
  (no args -> a default representative set)

Env knobs: CASE_TIMEOUT (default 5400), LACE_CONTRACT_PARALLELISM (default 2).
"""
import json
import os
import re
import subprocess
import sys
import time
from pathlib import Path

HERE = Path(__file__).resolve().parent
TABLE = HERE / "dataset_entrypoints.json"
DETECTOR = os.environ.get(
    "DETECTOR",
    str(HERE.parent / "Release-build" / "llm_detector"),
)
CASE_TIMEOUT = int(os.environ.get("CASE_TIMEOUT", "5400"))
STAMP = time.strftime("%Y%m%d_%H%M%S")

DEFAULT_CASES = [
    # known HIT (sanity) + previously-MISS cases (recall-floor candidates)
    "CVE-2016-7911",   # 2-root, io_context UAF (was HIT)
    "CVE-2013-1792",   # self-race, keyring
    "CVE-2024-53124",  # 2? sk_forward_alloc (was recall-floor MISS)
    "CVE-2024-46704",  # work->data (was adjacent MISS)
    "CVE-2016-9806",   # netlink_dump cb->skb (was buried)
    "CVE-2017-15265",  # self-race snd_seq (was buried)
]


def pick_bitcode(case_dir: Path) -> str:
    lls = sorted(case_dir.glob("*.ll"))
    names = [p.name for p in lls]
    if "merged.ll" in names:
        return "merged.ll"
    if "snd-seq.ll" in names:
        return "snd-seq.ll"
    if not lls:
        return ""
    return max(lls, key=lambda p: p.stat().st_size).name


def parse_log(text: str) -> dict:
    out = {}
    m = re.search(r"Threads:\s*(\d+),\s*Conflicting pairs:\s*(\d+),\s*Shared objects:\s*(\d+)", text)
    if m:
        out["threads"], out["pairs"], out["objs"] = m.group(1), m.group(2), m.group(3)
    m = re.search(r"Modeled (\d+) self-race object", text)
    out["self_race_objs"] = m.group(1) if m else "0"
    m = re.search(r"(\d+) potential bug\(s\) found", text)
    out["bugs"] = m.group(1) if m else "0"
    m = re.search(r"Static-Composition Analysis Finished:\s*(\d+) hypotheses", text)
    out["hyps"] = m.group(1) if m else "?"
    m = re.search(r"LLM API Requests:\s*(\d+)", text)
    out["reqs"] = m.group(1) if m else "?"
    out["api_2004"] = str(len(re.findall(r"-2004|资源不足", text)))
    return out


_print_lock = __import__("threading").Lock()


def run_case(case_id: str, table: dict, contract_par: str) -> str:
    case_dir = HERE / case_id
    entry = table.get(case_id)
    if not case_dir.is_dir():
        return f"{case_id:18s}  SKIP (no dir)"
    if not entry:
        return f"{case_id:18s}  SKIP (no entry config)"
    roots = entry["thread_roots"]
    bc = pick_bitcode(case_dir)
    if not bc:
        return f"{case_id:18s}  SKIP (no bitcode)"

    log_path = case_dir / f"detection_manualentry_{STAMP}.log"
    env = os.environ.copy()
    env.update({
        "LACE_STATIC_COMPOSE": "1",
        # Paper-faithful checker: node-anchored contracts + requirement-driven
        # discharge. Set LACE_CONTRACT_L2=0 to fall back to the legacy path.
        "LACE_CONTRACT_L2": os.environ.get("LACE_CONTRACT_L2", "1"),
        "LACE_CONTRACT_PARALLELISM": contract_par,
        "LACE_ENABLE_FLOW_PRIOR": "0",
        "LACE_ENTRYPOINTS": ",".join(roots),
        "LACE_SELF_RACE": "1" if entry.get("self_race") else "0",
    })
    cmd = [
        "timeout", str(CASE_TIMEOUT), DETECTOR,
        "--input-bc", bc, "--input-src", "src",
        "--legacy-workflow", "--abl-contract", "on",
        "--llm-provider", "openai",
        "--llm-url", os.environ.get("LLM_BASE_URL", ""),
        "--llm-key", os.environ.get("LLM_API_KEY", ""),
        "--llm-model", os.environ.get("LLM_MODEL", ""),
    ]
    started = time.time()
    with log_path.open("w") as lf:
        lf.write(f"[manual-entry] case={case_id} roots={roots} bc={bc} stamp={STAMP}\n")
        lf.flush()
        proc = subprocess.run(cmd, cwd=case_dir, env=env, stdout=lf, stderr=subprocess.STDOUT)
    secs = int(time.time() - started)
    text = log_path.read_text(errors="ignore")
    s = parse_log(text)
    if proc.returncode == 124:
        status = "TIMEOUT"
    elif "POTENTIAL" in text and "VIOLATION" in text:
        status = "FOUND"
    elif "No bugs detected" in text or "Static-Composition Analysis Finished" in text:
        status = "CLEAN/DONE"
    elif proc.returncode != 0:
        status = f"FAIL(rc={proc.returncode})"
    else:
        status = "UNKNOWN"
    return (f"{case_id:18s}  {status:14s} roots={len(roots)} "
            f"threads={s.get('threads','?')} objs={s.get('objs','?')} "
            f"selfrace={s.get('self_race_objs','0')} bugs={s.get('bugs','0')} "
            f"hyps={s.get('hyps','?')} reqs={s.get('reqs','?')} "
            f"api2004={s.get('api_2004','0')} {secs}s")


def main():
    from concurrent.futures import ThreadPoolExecutor, as_completed
    table = json.loads(TABLE.read_text())
    args = sys.argv[1:]
    if not args:
        print(f"usage: {os.path.basename(sys.argv[0])} ALL | SMOKE | <case-id> ...\n"
              f"  ALL    every case in {TABLE.name} ({len(table)} cases)\n"
              f"  SMOKE  the {len(DEFAULT_CASES)}-case sanity subset",
              file=sys.stderr)
        return 1
    if args[0].upper() == "ALL":
        cases = sorted(table.keys())
    elif args[0].upper() == "SMOKE":
        cases = DEFAULT_CASES
    else:
        cases = args
    contract_par = os.environ.get("LACE_CONTRACT_PARALLELISM", "2")
    case_par = int(os.environ.get("CASE_PARALLELISM", "1"))

    print(f"[manual-entry] detector={DETECTOR}")
    print(f"[manual-entry] model={os.environ.get('LLM_MODEL','')} stamp={STAMP}")
    print(f"[manual-entry] {len(cases)} case(s), timeout={CASE_TIMEOUT}s, "
          f"case_par={case_par}, contract_par={contract_par}")
    print("=" * 80)

    if case_par <= 1:
        for case_id in cases:
            line = run_case(case_id, table, contract_par)
            with _print_lock:
                print(line); sys.stdout.flush()
        return

    done = 0
    with ThreadPoolExecutor(max_workers=case_par) as ex:
        futs = {ex.submit(run_case, c, table, contract_par): c for c in cases}
        for fut in as_completed(futs):
            line = fut.result()
            done += 1
            with _print_lock:
                print(f"[{done}/{len(cases)}] {line}"); sys.stdout.flush()


if __name__ == "__main__":
    sys.exit(main() or 0)
