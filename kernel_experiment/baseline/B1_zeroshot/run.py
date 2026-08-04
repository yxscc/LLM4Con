#!/usr/bin/env python3
"""B1 — Zero-shot whole-file concurrency-audit baseline.

For every prepared CVE in kernel_experiment/CVE-*, send a single
chat-completion to the configured LLM with:
  * a fixed concurrency-audit system prompt (B1_zeroshot/prompt.md)
  * the patch-touched .c source file(s) as the user message
The LLM is given **no patch information** — this prevents the
trivial localization advantage that B2 (PR-diff baseline) gets.

Outputs are written under
  $BASELINE_DUMP_BASE/B1_zeroshot/<CVE>_<TS>/
so scripts/evaluate_recall.py can pick them up by setting DUMP_BASE.

Usage
-----
    source ../../../setup_env.sh
    python3 run.py                     # whole 50-CVE batch
    python3 run.py --cve CVE-2024-27019 CVE-2024-26974
    python3 run.py --limit 3           # smoke test (first 3 CVEs)
    python3 run.py --dry-run           # print prompt sizes, no API
"""
from __future__ import annotations

import argparse
import json
import os
import re
import sys
import time
from datetime import datetime
from pathlib import Path
from typing import Any, Dict, List, Optional

# Make `common` importable when this script is run directly.
HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE.parent))

from common import cve_loader  # noqa: E402
from common.llm_client import LLMClient, parse_json_block  # noqa: E402
from common.dump_writer import Finding, write_dump  # noqa: E402


PROMPT_FILE = HERE / "prompt.md"
DEFAULT_DUMP_BASE = os.environ.get(
    "BASELINE_DUMP_BASE",
    os.path.join(
        os.environ.get("LLM4CON_HOME", str(HERE.parents[2])),
        "kernel_experiment", "baseline_dump",
    ),
)


def load_system_prompt() -> str:
    text = PROMPT_FILE.read_text()
    # The system prompt is the prose section labelled "System message".
    m = re.search(
        r"System message\s*\n=+\s*\n(.*?)(?:\n#+ |\Z)",
        text,
        flags=re.S,
    )
    body = m.group(1).strip() if m else text
    return body


def build_user_message(cve: cve_loader.CVE) -> str:
    sources = cve_loader.read_source(cve)
    if not sources:
        return (
            "## Target\n\n"
            f"CVE: {cve.cve_id}\n"
            "No patch-touched source files were resolvable for this CVE."
        )
    parts: List[str] = [
        "## Target",
        f"CVE: {cve.cve_id}",
        f"Files in scope: {len(sources)}",
        "",
        "## Source",
        "",
    ]
    for rel, code in sources.items():
        parts.append(f"### File: {rel}")
        parts.append("```c")
        parts.append(code.rstrip())
        parts.append("```")
        parts.append("")
    parts.append(
        "## Task\n\nReport every HIGH/MEDIUM-confidence concurrency "
        "vulnerability per the system prompt's JSON schema. Output ONLY "
        "the JSON object — no markdown prose, no commentary."
    )
    return "\n".join(parts)


def normalize_findings(
    cve: cve_loader.CVE,
    parsed: Optional[Dict[str, Any]],
) -> List[Finding]:
    if not parsed:
        return []
    raw = parsed.get("findings") or []
    if not isinstance(raw, list):
        return []
    out: List[Finding] = []
    for i, r in enumerate(raw, 1):
        if not isinstance(r, dict):
            continue
        cat = str(r.get("category") or "other_concurrency").lower()
        sev = str(r.get("severity") or "unknown").lower()
        desc = str(r.get("description") or "")
        # Augment description with shared-object + threads so the
        # judge sees them even though the bugs.txt schema does not
        # have dedicated slots.
        so = r.get("shared_object")
        threads = r.get("threads") or []
        prefix_bits = []
        if so:
            prefix_bits.append(f"shared object = {so}")
        if threads:
            prefix_bits.append("threads = " + " ; ".join(map(str, threads)))
        if prefix_bits:
            desc = "(" + "; ".join(prefix_bits) + ")\n" + desc
        locs_in = r.get("locations") or []
        locs_out: List[Dict[str, Any]] = []
        for j, loc in enumerate(locs_in, 1):
            if not isinstance(loc, dict):
                continue
            locs_out.append({
                "role": str(loc.get("role") or f"site{j}"),
                "code": str(loc.get("code_snippet") or ""),
                "file": str(loc.get("file") or ""),
                "line": loc.get("line"),
                "node_id": j,
            })
        extras: Dict[str, Any] = {}
        if r.get("fix_suggestion"):
            extras["fix_suggestion"] = r["fix_suggestion"]
        out.append(Finding(
            hypothesis_id=f"B1_zs_{cve.cve_id}_{i}",
            category=cat,
            description=desc,
            severity=sev,
            locations=locs_out,
            extras=extras,
        ))
    return out


def process_cve(
    cve: cve_loader.CVE,
    client: LLMClient,
    system_prompt: str,
    dump_base: str,
    timestamp: str,
    dry_run: bool = False,
) -> Dict[str, Any]:
    user = build_user_message(cve)
    record: Dict[str, Any] = {
        "cve_id": cve.cve_id,
        "user_msg_chars": len(user),
        "n_files_in_scope": len(cve_loader.read_source(cve)),
    }
    if dry_run:
        record["status"] = "DRY"
        return record
    t0 = time.time()
    resp = client.chat([
        {"role": "system", "content": system_prompt},
        {"role": "user", "content": user},
    ])
    record["elapsed_s"] = round(resp.elapsed_s, 1)
    if resp.error:
        record["status"] = "API_ERROR"
        record["error"] = resp.error
        write_dump(
            dump_base, cve.cve_id, [],
            raw_responses=[{"error": resp.error}],
            meta={
                "baseline": "B1_zeroshot",
                "model": client.model,
                "endpoint": client.endpoint,
                "user_msg_chars": len(user),
                "status": "API_ERROR",
            },
            timestamp=timestamp,
        )
        return record
    parsed = parse_json_block(resp.text)
    findings = normalize_findings(cve, parsed)
    record["n_findings"] = len(findings)
    record["status"] = "OK" if findings or parsed else "PARSE_FAIL"
    write_dump(
        dump_base, cve.cve_id, findings,
        raw_responses=[{
            "system": system_prompt,
            "user": user,
            "assistant_text": resp.text,
            "parsed": parsed,
        }],
        meta={
            "baseline": "B1_zeroshot",
            "model": client.model,
            "endpoint": client.endpoint,
            "elapsed_s": round(resp.elapsed_s, 1),
            "user_msg_chars": len(user),
            "status": record["status"],
        },
        timestamp=timestamp,
    )
    return record


def main(argv: Optional[List[str]] = None) -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--cve", nargs="*", help="Restrict to these CVE IDs.")
    ap.add_argument("--limit", type=int, default=0,
                    help="Process at most N CVEs (after filters).")
    ap.add_argument("--dump-base", default=DEFAULT_DUMP_BASE)
    ap.add_argument("--dry-run", action="store_true",
                    help="Don't call the LLM, just enumerate prompts.")
    ap.add_argument("--skip-existing", action="store_true",
                    help="Skip CVEs that already have a B1 dump in the "
                         "configured dump base (any timestamp).")
    args = ap.parse_args(argv)

    system_prompt = load_system_prompt()
    cves = cve_loader.list_cves()
    if args.cve:
        wanted = set(args.cve)
        cves = [c for c in cves if c.cve_id in wanted]
    if args.limit > 0:
        cves = cves[: args.limit]

    dump_base = os.path.join(args.dump_base, "B1_zeroshot")
    os.makedirs(dump_base, exist_ok=True)

    if args.skip_existing:
        kept = []
        for c in cves:
            existing = list(Path(dump_base).glob(f"{c.cve_id}_*"))
            if existing:
                print(f"[{c.cve_id}] SKIP (existing dump)")
                continue
            kept.append(c)
        cves = kept

    client: Optional[LLMClient] = None
    if not args.dry_run:
        client = LLMClient()
        err = client.preflight()
        if err:
            print(f"[Preflight FAIL] {err}", file=sys.stderr)
            return 2
        print(
            f"[Preflight OK] model={client.model} "
            f"endpoint={client.endpoint.split('?')[0]}?..."
        )

    timestamp = datetime.now().strftime("%Y-%m-%d_%H-%M-%S")
    print(f"\n=== B1 zero-shot batch (n={len(cves)}, ts={timestamp}) ===")
    summary: List[Dict[str, Any]] = []
    for i, cve in enumerate(cves, 1):
        print(f"  [{i}/{len(cves)}] {cve.cve_id} ... ", end="", flush=True)
        rec = process_cve(
            cve, client, system_prompt, dump_base, timestamp,
            dry_run=args.dry_run,
        )
        if args.dry_run:
            print(f"DRY (prompt={rec['user_msg_chars']} chars)")
        elif rec["status"] == "OK":
            print(f"OK ({rec['n_findings']} finding(s), "
                  f"{rec['elapsed_s']}s)")
        elif rec["status"] == "PARSE_FAIL":
            print(f"PARSE_FAIL ({rec['elapsed_s']}s)")
        else:
            print(f"{rec['status']}: "
                  f"{rec.get('error', '')[:80]}")
        summary.append(rec)

    summary_path = os.path.join(dump_base, f"run_summary_{timestamp}.json")
    with open(summary_path, "w") as f:
        json.dump({
            "timestamp": timestamp,
            "n_cves": len(cves),
            "dry_run": args.dry_run,
            "records": summary,
        }, f, indent=2, ensure_ascii=False)
    print(f"\nRun summary: {summary_path}")
    print(f"Dump base:   {dump_base}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
