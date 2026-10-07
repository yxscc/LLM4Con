#!/usr/bin/env python3
"""Run the L2 contract checker over hunt targets to look for unknown defects.

Differs from kernel_experiment/run_manual_entry.py in the one way that matters:
that runner feeds each case its ground-truth thread roots via LACE_ENTRYPOINTS.
Here there is no ground truth, so entry discovery is left to the detector's own
structural pass (concurrency-creation APIs plus kernel entry heuristics). That
finds far more roots than a curated pair -- vsock yields 112 threads -- so
Phase A cost is bounded explicitly rather than implicitly.

Usage:
  run_hunt.py ALL                      # every target with bitcode
  run_hunt.py net-tipc net-smc         # named targets
  run_hunt.py --static-only ALL        # surfaces only, no LLM spend

Env:
  LLM_BASE_URL / LLM_API_KEY / LLM_MODEL   endpoint (required unless static-only)
  THREAD_CAP (default 24)      max per-thread contracts in Phase A
  SESSION_CAP (default 24)     max Phase C calibration sessions
  TARGET_PARALLELISM (default 1)
  LACE_CONTRACT_PARALLELISM (default 3)
  TARGET_TIMEOUT (default 10800)
"""
import argparse
import json
import os
import re
import subprocess
import sys
import threading
import time
from concurrent.futures import ThreadPoolExecutor, as_completed
from pathlib import Path

HERE = Path(__file__).resolve().parent
HOME = HERE.parent
DETECTOR = Path(os.environ.get("DETECTOR", HOME / "Release-build" / "llm_detector"))
TARGETS_DIR = Path(os.environ.get("HUNT_TARGETS", HERE / "targets"))
DUMP_ROOT = Path(os.environ.get("HUNT_DUMPS", HERE / "dumps"))
TIMEOUT = int(os.environ.get("TARGET_TIMEOUT", "10800"))
STAMP = time.strftime("%Y%m%d_%H%M%S")
_lock = threading.Lock()

STATS = [
    ("threads", r"Threads:\s*(\d+),"),
    ("pairs", r"Conflicting pairs:\s*(\d+),"),
    ("objs", r"Shared objects:\s*(\d+)"),
    ("contracts", r"\[contract-parallel\] generated (\d+)/"),
    ("undischarged", r"undischarged_total=(\d+)"),
    ("kept", r"kept_after_calibration=(\d+)"),
    ("hyps", r"L2 Static-Composition Analysis Finished:\s*(\d+) hypotheses"),
    ("bugs", r"(\d+) potential bug\(s\) found"),
    ("reqs", r"LLM API Requests:\s*(\d+)"),
    ("prompt_tok", r"Total Prompt Tokens:\s*(\d+)"),
    ("compl_tok", r"Total Completion Tokens:\s*(\d+)"),
    ("secs", r"Total Time:\s*(\d+) seconds"),
]


def pick_bitcode(d: Path) -> str:
    if (d / "merged.ll").is_file():
        return "merged.ll"
    lls = sorted(d.glob("*.ll"))
    return max(lls, key=lambda p: p.stat().st_size).name if lls else ""


def parse_log(text: str) -> dict:
    out = {}
    for key, rx in STATS:
        m = re.search(rx, text)
        out[key] = m.group(1) if m else "-"
    out["api_err"] = str(len(re.findall(r"-2004|资源不足|no model permission", text)))
    return out


def run_target(name: str, args) -> dict:
    d = TARGETS_DIR / name
    if not d.is_dir():
        return {"target": name, "status": "SKIP_NO_DIR"}
    bc = pick_bitcode(d)
    if not bc:
        return {"target": name, "status": "SKIP_NO_BITCODE"}
    if not (d / "src").is_dir():
        return {"target": name, "status": "SKIP_NO_SRC"}

    env = os.environ.copy()
    env.update({
        "LACE_STATIC_COMPOSE": "1",
        "LACE_CONTRACT_L2": "1",
        # no oracle input of any kind on a hunt
        "LACE_ENABLE_FLOW_PRIOR": "0",
        "LACE_DUMP_ROOT": str(DUMP_ROOT),
        # Auto entry discovery yields ~40-120 roots per module, so bound the
        # two costs that scale with it instead of taking the size-based
        # defaults (which reach 90 contracts / 70 sessions).
        "LACE_CONTRACT_THREAD_CAP": str(args.thread_cap),
        "LACE_SESSION_CAP": str(args.session_cap),
        "LACE_CONTRACT_PARALLELISM": os.environ.get(
            "LACE_CONTRACT_PARALLELISM", "3"),
    })
    env.pop("LACE_ENTRYPOINTS", None)
    env.pop("LACE_ENTRYPOINTS_FILE", None)
    if args.static_only:
        env["LACE_EARLY_EXIT_AFTER_SURFACE"] = "1"

    key = os.environ.get("LLM_API_KEY", "")
    url = os.environ.get("LLM_BASE_URL", "")
    model = os.environ.get("LLM_MODEL", "")
    if args.static_only:
        key, url, model = key or "dummy", url or "http://localhost", model or "dummy"

    log_path = d / f"hunt_{STAMP}.log"
    cmd = ["timeout", str(TIMEOUT), str(DETECTOR),
           "--input-bc", bc, "--input-src", "src",
           "--legacy-workflow", "--abl-contract", "on",
           "--llm-provider", "openai",
           "--llm-url", url, "--llm-key", key, "--llm-model", model]

    t0 = time.time()
    with log_path.open("w") as lf:
        lf.write(f"[hunt] target={name} bc={bc} stamp={STAMP} "
                 f"thread_cap={args.thread_cap} session_cap={args.session_cap} "
                 f"static_only={args.static_only}\n")
        lf.flush()
        proc = subprocess.run(cmd, cwd=d, env=env, stdout=lf,
                              stderr=subprocess.STDOUT)
    wall = int(time.time() - t0)
    text = log_path.read_text(errors="ignore")
    s = parse_log(text)

    if proc.returncode == 124:
        status = "TIMEOUT"
    elif "POTENTIAL" in text and "VIOLATION" in text:
        status = "FINDINGS"
    elif "Analysis Finished" in text or "No bugs detected" in text:
        status = "CLEAN"
    elif proc.returncode != 0:
        status = f"FAIL(rc={proc.returncode})"
    else:
        status = "UNKNOWN"

    s.update({"target": name, "status": status, "wall": str(wall),
              "bitcode": bc, "log": str(log_path)})
    return s


COLS = ["target", "status", "threads", "objs", "contracts", "undischarged",
        "kept", "bugs", "reqs", "prompt_tok", "compl_tok", "api_err", "wall"]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("targets", nargs="+", help="target names, or ALL")
    ap.add_argument("--static-only", action="store_true",
                    help="stop after the surface; spends no LLM budget")
    ap.add_argument("--thread-cap", type=int,
                    default=int(os.environ.get("THREAD_CAP", "24")))
    ap.add_argument("--session-cap", type=int,
                    default=int(os.environ.get("SESSION_CAP", "24")))
    ap.add_argument("--json")
    args = ap.parse_args()

    if args.targets[0].upper() == "ALL":
        names = sorted(p.name for p in TARGETS_DIR.iterdir()
                       if p.is_dir() and (list(p.glob("*.ll")) or
                                          (p / "merged.ll").is_file()))
    else:
        names = args.targets

    if not args.static_only and not os.environ.get("LLM_API_KEY"):
        print("error: LLM_API_KEY not set (source setup_env.sh) "
              "or pass --static-only", file=sys.stderr)
        return 2

    par = int(os.environ.get("TARGET_PARALLELISM", "1"))
    print(f"[hunt] detector = {DETECTOR}")
    print(f"[hunt] model    = {os.environ.get('LLM_MODEL','(static-only)')}")
    print(f"[hunt] targets  = {len(names)}  parallelism={par}  "
          f"thread_cap={args.thread_cap} session_cap={args.session_cap}")
    print(f"[hunt] stamp    = {STAMP}")
    print("=" * 118)

    rows = []

    def emit(r):
        with _lock:
            rows.append(r)
            print("  ".join(f"{r.get(c,'-'):>{max(len(c),6)}}" for c in COLS))
            sys.stdout.flush()

    print("  ".join(f"{c:>{max(len(c),6)}}" for c in COLS))
    if par <= 1:
        for n in names:
            emit(run_target(n, args))
    else:
        with ThreadPoolExecutor(max_workers=par) as ex:
            futs = {ex.submit(run_target, n, args): n for n in names}
            for fut in as_completed(futs):
                emit(fut.result())

    print("=" * 118)
    tp = sum(int(r.get("prompt_tok", 0) or 0) for r in rows
             if str(r.get("prompt_tok", "")).isdigit())
    tc = sum(int(r.get("compl_tok", 0) or 0) for r in rows
             if str(r.get("compl_tok", "")).isdigit())
    tr = sum(int(r.get("reqs", 0) or 0) for r in rows
             if str(r.get("reqs", "")).isdigit())
    nf = sum(1 for r in rows if r.get("status") == "FINDINGS")
    print(f"targets={len(rows)}  with-findings={nf}  requests={tr}  "
          f"prompt_tokens={tp}  completion_tokens={tc}")
    print(f"\nNext: python3 {HERE.name}/known_defect_filter.py "
          f"--dump-root {DUMP_ROOT} --verbose")

    if args.json:
        Path(args.json).write_text(json.dumps(rows, indent=2))
        print(f"wrote {args.json}")


if __name__ == "__main__":
    sys.exit(main() or 0)
