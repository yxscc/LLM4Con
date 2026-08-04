#!/usr/bin/env python3
"""B4 -- Codex CLI agent + cybersecurity *static* skill baseline.

This is the "skill on an excellent agent" baseline: instead of running the
repo's Semgrep skill as a thin standalone script, we attach it to a real
coding agent (OpenAI Codex CLI) and let the agent read the CVE source,
optionally run semgrep, and reason about concurrency vulnerabilities.

Model-controlled like B1/B2/B3: Codex talks to the same GPT-5.5 gateway via
the local Responses<->Chat shim (shim.py), so the only moving variable is
the *scaffold* (Codex agent + skill) vs Lace's pipeline.

Per CVE:
  1. copy kernel_experiment/<CVE>/src/ into a scratch workdir
  2. write AGENTS.md = the attached SKILL.md + a concurrency-audit task
     contract (the agent picks AGENTS.md up automatically)
  3. run `codex exec` non-interactively (shell tool enabled, read-only intent)
  4. read the agent's findings (prefers __findings__.json it writes; falls
     back to parsing the final message) and emit a Lace-compatible dump so
     scripts/evaluate_recall.py scores it with zero changes.

Usage:
    # shim must be running:  python3 shim.py &   (see run_all.sh)
    source ../../../setup_env.sh
    python3 run.py --limit 2            # smoke
    python3 run.py --cve CVE-2024-27019
    python3 run.py --skip-existing      # full batch, resumable
"""
from __future__ import annotations

import argparse
import json
import os
import re
import shutil
import subprocess
import sys
import time
from datetime import datetime
from pathlib import Path
from typing import Any, Dict, List, Optional

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE.parent))

from common import cve_loader  # noqa: E402
from common.llm_client import parse_json_block  # noqa: E402
from common.dump_writer import Finding, write_dump  # noqa: E402

LLM4CON_HOME = os.environ.get("LLM4CON_HOME", str(HERE.parents[2]))
PLAYGROUND = os.path.dirname(LLM4CON_HOME)
EXTERNAL_BIN = os.path.join(PLAYGROUND, "external", "bin")
CODEX_BIN = os.environ.get("CODEX_BIN", os.path.join(EXTERNAL_BIN, "codex"))
BWRAP_BIN = os.environ.get("BWRAP_BIN", os.path.join(EXTERNAL_BIN, "bwrap"))
CODEX_HOME = os.environ.get("CODEX_HOME", str(HERE / "codex_home"))
SKILL_PATH = os.environ.get(
    "SKILL_PATH",
    "/tmp/Anthropic-Cybersecurity-Skills/skills/"
    "implementing-semgrep-for-custom-sast-rules/SKILL.md")
DEFAULT_DUMP_BASE = os.environ.get(
    "BASELINE_DUMP_BASE",
    os.path.join(LLM4CON_HOME, "kernel_experiment", "baseline_dump"))
PER_CVE_TIMEOUT = int(os.environ.get("B4_TIMEOUT", "1200"))
WORK_ROOT = os.environ.get("B4_WORK_ROOT", "/tmp/b4_codex_work")

FINDINGS_FILE = "__findings__.json"

TASK = f"""\
# TASK: Static concurrency-vulnerability audit (Linux kernel source)

You are auditing the Linux-kernel C source in THIS directory for
**concurrency vulnerabilities only** (data races, atomicity violations,
use-after-free / double-free caused by races, NULL-deref races, ordering /
publish-before-init bugs, refcount races). Ignore non-concurrency issues.

Use your shell tool to explore the tree (ls/grep/sed/cat). You also have the
`semgrep` CLI available; you MAY run it (see the attached skill in AGENTS.md)
but semgrep's public rules barely cover kernel concurrency, so rely mainly on
reading the code and reasoning about thread interleavings (syscall paths,
softirq/timer/work callbacks, RCU readers, IRQ handlers).

When done, WRITE your findings as JSON to a file named `{FINDINGS_FILE}` in
this directory (use the shell tool / apply_patch). The JSON MUST be:

{{
  "findings": [
    {{
      "category": "data_race|atomicity_violation|use_after_free|double_free|null_deref|order_violation|use_before_init|refcount|other_concurrency",
      "severity": "high|medium|low",
      "shared_object": "the shared variable/field/object that is raced on",
      "threads": ["execution context A (e.g. syscall write path)", "execution context B (e.g. timer callback)"],
      "description": "Root cause: which shared object, which two contexts race, the interleaving, and why it is a bug.",
      "locations": [
        {{"role": "writer|reader|free|use|init", "file": "relative/path.c", "line": 123, "code_snippet": "the racing line"}}
      ]
    }}
  ]
}}

If you find no concurrency vulnerability, write {{"findings": []}}. Report
only HIGH/MEDIUM-confidence concurrency bugs. After writing the file, reply
with the exact same JSON as your final message and nothing else.
"""


def load_skill() -> str:
    try:
        return Path(SKILL_PATH).read_text(encoding="utf-8", errors="ignore")
    except Exception:
        return "(skill file unavailable)"


def build_agents_md(skill_text: str) -> str:
    return (
        "# Attached skill: static SAST / concurrency audit\n\n"
        "You are operating with the following cybersecurity skill attached. "
        "Apply it while performing the audit described in the task prompt.\n\n"
        "---\n\n" + skill_text + "\n\n---\n\n"
        "Remember: the deliverable is the `" + FINDINGS_FILE + "` JSON file "
        "described in the task prompt.\n"
    )


def codex_env() -> Dict[str, str]:
    """Environment for the *outer* bwrap process. Deliberately carries NO
    proxy: Codex's model calls go to the localhost shim (direct), and the
    sandboxed agent must have no internet at all."""
    env = dict(os.environ)
    for k in ("HTTP_PROXY", "HTTPS_PROXY", "http_proxy", "https_proxy",
              "ALL_PROXY", "all_proxy"):
        env.pop(k, None)
    env["PATH"] = os.pathsep.join([
        EXTERNAL_BIN, os.path.expanduser("~/.local/bin"),
        "/usr/local/bin", "/usr/bin", "/bin"])
    env["NO_PROXY"] = env["no_proxy"] = "127.0.0.1,localhost"
    return env


def build_jail_cmd(workdir: Path, codexhome: Path) -> List[str]:
    """Codex inside a two-layer bubblewrap jail:

      * OUTER bwrap (here): filesystem jail -- binds ONLY the source copy,
        system dirs and the tool binaries. The CVE experiment tree (and its
        ground_truth.json) is NOT bound, so the agent literally cannot read
        the oracle. It shares the host network namespace so Codex's model
        calls reach the localhost shim.
      * INNER sandbox (Codex's own `-s workspace-write` +
        network_access=false): wraps every agent shell command in bwrap
        --unshare-net, so the agent has NO internet (cannot look the CVE up).

    Net effect: a fully offline, oracle-blind static audit on GPT-5.5.
    """
    local = os.path.expanduser("~/.local")
    cmd = [BWRAP_BIN,
           "--ro-bind", "/usr", "/usr",
           "--ro-bind", "/bin", "/bin",
           "--ro-bind", "/sbin", "/sbin",
           "--ro-bind", "/lib", "/lib",
           "--ro-bind", "/lib64", "/lib64",
           "--ro-bind", "/etc", "/etc",
           "--proc", "/proc", "--dev", "/dev",
           "--ro-bind", os.path.join(PLAYGROUND, "external"),
           os.path.join(PLAYGROUND, "external")]
    if os.path.isdir(local):
        cmd += ["--ro-bind", local, local]
    cmd += [
        "--bind", str(workdir), "/work",
        "--bind", str(codexhome), "/codexhome",
        "--setenv", "HOME", "/work",
        "--setenv", "CODEX_HOME", "/codexhome",
        "--setenv", "PATH", f"{EXTERNAL_BIN}:{local}/bin:/usr/bin:/bin",
        "--setenv", "GW_KEY", "dummy",
        "--setenv", "NO_PROXY", "127.0.0.1,localhost",
        "--setenv", "TERM", "xterm",
        "--chdir", "/work",
        CODEX_BIN, "exec",
        "-C", "/work",
        "--skip-git-repo-check",
        "-s", "workspace-write",
        "-c", "sandbox_workspace_write.network_access=false",
        "-c", "approval_policy=never",
        "-c", "shell_environment_policy.inherit=all",
        "-o", "/work/__last_msg__.txt",
        TASK,
    ]
    return cmd


def normalize_findings(cve: cve_loader.CVE,
                       parsed: Optional[Dict[str, Any]]) -> List[Finding]:
    if not parsed:
        return []
    raw = parsed.get("findings")
    if not isinstance(raw, list):
        return []
    out: List[Finding] = []
    for i, r in enumerate(raw, 1):
        if not isinstance(r, dict):
            continue
        cat = str(r.get("category") or "other_concurrency").lower()
        sev = str(r.get("severity") or "unknown").lower()
        desc = str(r.get("description") or "")
        so = r.get("shared_object")
        threads = r.get("threads") or []
        bits = []
        if so:
            bits.append(f"shared object = {so}")
        if threads:
            bits.append("threads = " + " ; ".join(map(str, threads)))
        if bits:
            desc = "(" + "; ".join(bits) + ")\n" + desc
        locs_out: List[Dict[str, Any]] = []
        for j, loc in enumerate(r.get("locations") or [], 1):
            if not isinstance(loc, dict):
                continue
            locs_out.append({
                "role": str(loc.get("role") or f"site{j}"),
                "code": str(loc.get("code_snippet") or loc.get("code") or ""),
                "file": str(loc.get("file") or ""),
                "line": loc.get("line"),
                "node_id": j,
            })
        out.append(Finding(
            hypothesis_id=f"B4_codex_{cve.cve_id}_{i}",
            category=cat, description=desc, severity=sev,
            locations=locs_out,
            extras={"cwe": r.get("cwe")} if r.get("cwe") else {},
        ))
    return out


def extract_findings(workdir: Path, last_msg: str) -> Optional[Dict[str, Any]]:
    fpath = workdir / FINDINGS_FILE
    if fpath.is_file():
        try:
            return json.loads(fpath.read_text(encoding="utf-8", errors="ignore"))
        except Exception:
            blk = parse_json_block(fpath.read_text(errors="ignore"))
            if blk is not None:
                return blk
    return parse_json_block(last_msg or "")


def _copy_sessions(codexhome: Path, dest_dir: str) -> Optional[str]:
    """Copy Codex's session rollout(s) (full tool-call transcript) into the
    dump as an audit trail of exactly what the agent did/saw."""
    sess = list((codexhome / "sessions").rglob("rollout-*.jsonl"))
    if not sess:
        return None
    sess.sort(key=lambda p: p.stat().st_mtime)
    try:
        dst = os.path.join(dest_dir, "codex_session.jsonl")
        shutil.copy2(str(sess[-1]), dst)
        return dst
    except Exception:
        return None


def process_cve(cve: cve_loader.CVE, skill_text: str, dump_base: str,
                timestamp: str, dry_run: bool = False) -> Dict[str, Any]:
    rec: Dict[str, Any] = {"cve_id": cve.cve_id}
    if not cve.src_dir or not os.path.isdir(cve.src_dir):
        rec["status"] = "NO_SRC"
        return rec

    # Opaque workdir + fresh per-CVE CODEX_HOME (no CVE id reaches the agent;
    # both are bound into the jail, never visible by host path).
    os.makedirs(WORK_ROOT, exist_ok=True)
    workdir = Path(WORK_ROOT) / cve.cve_id          # host-side bookkeeping only
    codexhome = Path(WORK_ROOT) / (cve.cve_id + "__ch")
    for d in (workdir, codexhome):
        if d.exists():
            shutil.rmtree(d, ignore_errors=True)
    workdir.mkdir(parents=True, exist_ok=True)
    codexhome.mkdir(parents=True, exist_ok=True)
    shutil.copy2(str(HERE / "codex_home" / "config.toml"),
                 str(codexhome / "config.toml"))

    for entry in os.listdir(cve.src_dir):
        s = os.path.join(cve.src_dir, entry)
        d = workdir / entry
        if os.path.isdir(s):
            shutil.copytree(s, d, dirs_exist_ok=True)
        else:
            shutil.copy2(s, d)
    (workdir / "AGENTS.md").write_text(build_agents_md(skill_text))

    rec["n_c_files"] = sum(1 for _ in workdir.rglob("*.c"))
    if dry_run:
        rec["status"] = "DRY"
        shutil.rmtree(workdir, ignore_errors=True)
        shutil.rmtree(codexhome, ignore_errors=True)
        return rec

    cmd = build_jail_cmd(workdir, codexhome)
    t0 = time.time()
    status_override = None
    try:
        proc = subprocess.run(
            cmd, env=codex_env(), stdin=subprocess.DEVNULL,
            stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
            timeout=PER_CVE_TIMEOUT, text=True)
        rec["elapsed_s"] = round(time.time() - t0, 1)
        rec["codex_rc"] = proc.returncode
        codex_log = proc.stdout or ""
    except subprocess.TimeoutExpired as e:
        rec["elapsed_s"] = round(time.time() - t0, 1)
        status_override = "TIMEOUT"
        codex_log = (e.stdout or "") if isinstance(e.stdout, str) else ""

    last_msg = workdir / "__last_msg__.txt"
    last_text = last_msg.read_text(errors="ignore") if last_msg.is_file() else ""
    parsed = extract_findings(workdir, last_text)
    findings = normalize_findings(cve, parsed)
    rec["n_findings"] = len(findings)
    rec["status"] = status_override or ("OK" if parsed is not None else "PARSE_FAIL")

    out_dir = write_dump(
        dump_base, cve.cve_id, findings,
        raw_responses=[{
            "task": TASK,
            "skill_path": SKILL_PATH,
            "final_message": last_text,
            "parsed": parsed,
            "codex_log_tail": codex_log[-6000:],
        }],
        meta={
            "baseline": "B4_codex_skill",
            "agent": "codex-cli-0.137.0",
            "skill": os.path.basename(os.path.dirname(SKILL_PATH)),
            "model": os.environ.get("LLM_MODEL"),
            "sandbox": "outer-bwrap-fs-jail + inner-unshare-net (offline, oracle-blind)",
            "elapsed_s": rec.get("elapsed_s"),
            "codex_rc": rec.get("codex_rc"),
            "status": rec["status"],
            "n_findings": len(findings),
        },
        timestamp=timestamp)
    rec["dump"] = out_dir
    sess = _copy_sessions(codexhome, out_dir)
    rec["session_saved"] = bool(sess)

    # Clean up scratch (the dump retains findings, final message, log tail and
    # the full session transcript).
    shutil.rmtree(workdir, ignore_errors=True)
    shutil.rmtree(codexhome, ignore_errors=True)
    return rec


def main(argv: Optional[List[str]] = None) -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--cve", nargs="*", help="Restrict to these CVE IDs.")
    ap.add_argument("--limit", type=int, default=0)
    ap.add_argument("--dump-base", default=DEFAULT_DUMP_BASE)
    ap.add_argument("--dry-run", action="store_true")
    ap.add_argument("--skip-existing", action="store_true")
    ap.add_argument("--shard", default="", help="i/N: process CVEs where "
                    "index %% N == i (for parallel workers).")
    args = ap.parse_args(argv)

    if not os.path.isfile(CODEX_BIN):
        print(f"[FATAL] codex binary not found at {CODEX_BIN}", file=sys.stderr)
        return 2

    skill_text = load_skill()
    cves = cve_loader.list_cves()
    if args.cve:
        wanted = set(args.cve)
        cves = [c for c in cves if c.cve_id in wanted]
    if args.shard:
        i, n = (int(x) for x in args.shard.split("/"))
        cves = [c for idx, c in enumerate(cves) if idx % n == i]
    if args.limit > 0:
        cves = cves[: args.limit]

    dump_base = os.path.join(args.dump_base, "B4_codex_skill")
    os.makedirs(dump_base, exist_ok=True)
    if args.skip_existing:
        kept = []
        for c in cves:
            if list(Path(dump_base).glob(f"{c.cve_id}_*")):
                print(f"[{c.cve_id}] SKIP (existing dump)")
                continue
            kept.append(c)
        cves = kept

    timestamp = datetime.now().strftime("%Y-%m-%d_%H-%M-%S")
    print(f"=== B4 codex+skill batch (n={len(cves)}, ts={timestamp}) ===")
    summary = []
    for i, cve in enumerate(cves, 1):
        print(f"  [{i}/{len(cves)}] {cve.cve_id} ... ", end="", flush=True)
        rec = process_cve(cve, skill_text, dump_base, timestamp, dry_run=args.dry_run)
        st = rec.get("status")
        if st == "OK":
            print(f"OK ({rec['n_findings']} finding(s), {rec.get('elapsed_s')}s)")
        elif st == "DRY":
            print(f"DRY ({rec.get('n_c_files')} .c files)")
        else:
            print(f"{st} ({rec.get('elapsed_s','?')}s)")
        summary.append(rec)

    spath = os.path.join(dump_base, f"run_summary_{timestamp}.json")
    with open(spath, "w") as f:
        json.dump({"timestamp": timestamp, "n_cves": len(cves),
                   "dry_run": args.dry_run, "records": summary}, f, indent=2)
    print(f"\nRun summary: {spath}\nDump base:   {dump_base}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
