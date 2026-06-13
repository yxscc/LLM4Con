#!/usr/bin/env python3
"""B3 — Mythos / Claude Security agentic-scaffold baseline.

Faithful reproduction of the Mythos Research Edition v4 pipeline
(Keyvanhardani/mythos-research, Apache-2.0) for the CVE setting,
adapted to the ByteDance gateway / GPT-5.5 / Lace evaluation harness.

Pipeline phases (per CVE)
-------------------------
  Phase 1  Sink-guided slicing
           ripgrep (re-impl in Python) over $CVE/src using the
           upstream c-cpp.txt sink catalog.
  Phase 2  File ranking
           Single LLM call using upstream/file-ranking.md.
           Falls back to a deterministic sink-density rank when the
           CVE has ≤2 source files (the LLM rank is a no-op then).
  Phase 3  Agentic hunt (per top-N file)
           Single-shot LLM per file using upstream/vsp-c-cpp.md
           (system) + upstream/hunter-agent.md (mission). Note: the
           upstream prompt assumes Read/Grep/Glob/Bash tool-loop; we
           run plain Chat Completion. The full source contents of
           the file are inlined into the user message to compensate
           — same trade-off mythos-bench's harness.py makes vs.
           cc_harness.go (function vs. whole-file mode).
  Phase 3.5 Adversarial self-challenge (per finding)
           upstream/self-challenge.md → ADVERSE findings are dropped.
  Phase 4  Skeptical validation (per surviving finding)
           upstream/validation.md → FALSE_POSITIVE dropped.
  Phase 6  Aggregate
           Surviving findings → Lace bugs.txt format.

Known divergences from upstream Mythos v4 — documented in the
report under "Threats to Validity":
  * No multi-turn tool loop (Read/Grep/Glob/Bash) — single-shot per
    file. Mitigated by inlining full file contents.
  * No build sandbox / Phase 2.5 — kernel CVE bitcodes are
    pre-prepared but we don't compile/run the snippets here.
  * No Phase 5 (live exec validator) — that phase is held private
    in mythos-research too.
  * No pass-at-k diversity sampling — K=1. (Mythos default K=3.)
  * No FP-memory writeback — each CVE evaluated in isolation.
"""
from __future__ import annotations

import argparse
import json
import os
import re
import sys
import time
from dataclasses import dataclass, field
from datetime import datetime
from pathlib import Path
from typing import Any, Dict, List, Optional, Tuple

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE.parent))

from common import cve_loader  # noqa: E402
from common.llm_client import LLMClient, parse_json_block  # noqa: E402
from common.dump_writer import Finding, write_dump  # noqa: E402

from sink_slicer import (  # noqa: E402
    SinkHit, density_per_file, load_sink_catalog, slice_tree,
)


UPSTREAM = HERE / "upstream"
SINK_CATALOG = HERE / "sinks" / "c-cpp.txt"
DEFAULT_DUMP_BASE = os.environ.get(
    "BASELINE_DUMP_BASE",
    os.path.join(
        os.environ.get(
            "LLM4CON_HOME",
            "/mlx_devbox/users/mayunlong.39/playground/LLM4Con",
        ),
        "kernel_experiment", "baseline_dump",
    ),
)

# Hard caps to keep cost / latency bounded.
MAX_FILES_TO_HUNT = 8       # Mythos default --max-files 8
MAX_FILE_CHARS = 80_000     # ~ 2k LoC inlined per hunter call
HUNTER_MAX_TOKENS = 6000
RANK_MAX_TOKENS = 1500
JUDGE_MAX_TOKENS = 1500


@dataclass
class HunterFinding:
    """Raw hunter output — kept verbose so the challenger + validator
    have everything they need to reason."""

    raw: Dict[str, Any]
    source_file: str             # which file was being hunted
    challenge: Optional[Dict[str, Any]] = None
    validation: Optional[Dict[str, Any]] = None
    final_status: str = "PENDING"   # CONFIRMED | NEUTRAL | DROPPED


@dataclass
class HunterRun:
    """Per-(file, hunt) provenance kept verbatim for paper-level
    auditability — we want to be able to quote the model's reasoning
    even when it returns CLEAN."""
    source_file: str
    assistant_text: str
    parsed_verdict: Optional[str]
    parsed_findings_count: int
    elapsed_s: float


# ──────────────────────────────────────────────────────────────────
# Phase 2: file ranking (LLM-graded).
# ──────────────────────────────────────────────────────────────────

def build_ranking_user_msg(
    cve: cve_loader.CVE,
    files: List[str],
    density: Dict[str, Dict[str, int]],
) -> str:
    parts = [
        f"Codebase: linux-kernel subset prepared for {cve.cve_id}",
        "",
        "Files (one per line, with sink-category hit counts in brackets):",
        "",
    ]
    for f in files:
        cats = density.get(f, {})
        if cats:
            parts.append(
                f"  {f}    [" + ", ".join(
                    f"{k}={v}" for k, v in sorted(cats.items())
                ) + "]"
            )
        else:
            parts.append(f"  {f}    []")
    parts += [
        "",
        "Rank these files for vulnerability-research priority per your "
        "instructions. Output ONE 'RANK | filepath | reason' line per "
        "file, sorted by rank descending. Only include rank 3-5 files."
    ]
    return "\n".join(parts)


_RANK_LINE = re.compile(
    r"^\s*(\d)\s*[|│]\s*([^\s|│][^|│\n]*?)\s*[|│]\s*(.+?)\s*$"
)


def parse_rank_output(text: str) -> List[Tuple[int, str, str]]:
    """Parse the upstream ranker's '5 | path | reason' lines.
    Tolerates extra surrounding prose and code fences."""
    out: List[Tuple[int, str, str]] = []
    if not text:
        return out
    if text.startswith("```"):
        text = re.sub(r"^```[a-zA-Z]*\n?", "", text)
        text = re.sub(r"\n?```$", "", text)
    for line in text.splitlines():
        m = _RANK_LINE.match(line)
        if not m:
            continue
        try:
            rank = int(m.group(1))
        except ValueError:
            continue
        if not (1 <= rank <= 5):
            continue
        out.append((rank, m.group(2).strip(), m.group(3).strip()))
    return out


def rank_files(
    client: LLMClient,
    cve: cve_loader.CVE,
    files: List[str],
    density: Dict[str, Dict[str, int]],
    *,
    record: Dict[str, Any],
) -> List[Tuple[int, str, str]]:
    """Return a list of (rank, filepath, reason) sorted by rank desc.

    Fallback: when ≤2 candidate files exist, skip the LLM and produce
    a deterministic rank from sink-hit density."""
    if len(files) <= 2:
        ranked = sorted(
            files,
            key=lambda f: -sum(density.get(f, {}).values()),
        )
        return [(5, f, "single-file fallback") for f in ranked]

    system = (UPSTREAM / "file-ranking.md").read_text()
    user = build_ranking_user_msg(cve, files, density)
    t0 = time.time()
    resp = client.chat(
        [{"role": "system", "content": system},
         {"role": "user", "content": user}],
        max_tokens=RANK_MAX_TOKENS,
    )
    record.setdefault("phase_times", {})["rank"] = round(resp.elapsed_s, 1)
    if resp.error:
        record.setdefault("phase_errors", {})["rank"] = resp.error
        # Fallback to density rank on API failure.
        ranked = sorted(
            files,
            key=lambda f: -sum(density.get(f, {}).values()),
        )
        return [(4, f, "rank-API-error fallback") for f in ranked[:MAX_FILES_TO_HUNT]]
    parsed = parse_rank_output(resp.text)
    if not parsed:
        ranked = sorted(
            files,
            key=lambda f: -sum(density.get(f, {}).values()),
        )
        return [(4, f, "rank-parse-fail fallback") for f in ranked[:MAX_FILES_TO_HUNT]]
    parsed.sort(key=lambda t: -t[0])
    return parsed


# ──────────────────────────────────────────────────────────────────
# Phase 3: hunter (per-file).
# ──────────────────────────────────────────────────────────────────

def build_hunter_user_msg(
    cve: cve_loader.CVE,
    rel_file: str,
    full_path: str,
    file_hits: List[SinkHit],
) -> str:
    try:
        text = open(full_path, "r", encoding="utf-8", errors="ignore").read()
    except OSError:
        text = "<file read error>"
    if len(text) > MAX_FILE_CHARS:
        text = text[:MAX_FILE_CHARS] + "\n/* ...<truncated>... */\n"
    hit_lines = "\n".join(
        f"  L{h.line:>6}  [{h.category}]  {h.snippet}"
        for h in file_hits[:40]
    ) or "  (no sink-catalog hits in this file)"
    return (
        f"Assigned scope: file `{rel_file}` from CVE-prep tree for "
        f"{cve.cve_id}.\n\n"
        "Pre-computed sink-catalog hits in this file (use as entry "
        "points; do NOT re-grep the tree):\n\n"
        f"{hit_lines}\n\n"
        "## File contents\n\n"
        f"```c\n{text}\n```\n\n"
        "Per your hunter-agent.md instructions, output the strict "
        "JSON report. Emit ONLY the JSON object — no prose before "
        "or after."
    )


def hunt_one_file(
    client: LLMClient,
    cve: cve_loader.CVE,
    rel_file: str,
    full_path: str,
    file_hits: List[SinkHit],
    *,
    record: Dict[str, Any],
    hunter_runs: Optional[List["HunterRun"]] = None,
) -> List[HunterFinding]:
    """Run the hunter agent on a single file. Returns 0..N findings."""
    system = (
        (UPSTREAM / "vsp-c-cpp.md").read_text()
        + "\n\n# Hunter agent mission brief\n\n"
        + (UPSTREAM / "hunter-agent.md").read_text()
    )
    user = build_hunter_user_msg(cve, rel_file, full_path, file_hits)
    resp = client.chat(
        [{"role": "system", "content": system},
         {"role": "user", "content": user}],
        max_tokens=HUNTER_MAX_TOKENS,
    )
    times = record.setdefault("phase_times", {}).setdefault("hunt", [])
    times.append({"file": rel_file, "s": round(resp.elapsed_s, 1)})
    if resp.error:
        record.setdefault("phase_errors", {}).setdefault(
            "hunt", []).append({"file": rel_file, "err": resp.error})
        if hunter_runs is not None:
            hunter_runs.append(HunterRun(
                source_file=rel_file,
                assistant_text=f"<api_error: {resp.error}>",
                parsed_verdict=None,
                parsed_findings_count=0,
                elapsed_s=round(resp.elapsed_s, 1),
            ))
        return []
    parsed = parse_json_block(resp.text)
    verdict = (parsed or {}).get("verdict") if isinstance(parsed, dict) else None
    raw_findings = (parsed or {}).get("findings") or [] if isinstance(parsed, dict) else []
    if hunter_runs is not None:
        hunter_runs.append(HunterRun(
            source_file=rel_file,
            assistant_text=resp.text,
            parsed_verdict=verdict,
            parsed_findings_count=len(raw_findings),
            elapsed_s=round(resp.elapsed_s, 1),
        ))
    if not isinstance(parsed, dict):
        return []
    return [
        HunterFinding(raw=r, source_file=rel_file)
        for r in raw_findings if isinstance(r, dict)
    ]


# ──────────────────────────────────────────────────────────────────
# Phase 3.5: self-challenge (per finding).
# ──────────────────────────────────────────────────────────────────

def challenge_finding(
    client: LLMClient,
    hf: HunterFinding,
    *,
    record: Dict[str, Any],
) -> None:
    system = (UPSTREAM / "self-challenge.md").read_text()
    user = (
        f"Finding under challenge (source_file={hf.source_file}):\n\n"
        f"```json\n{json.dumps(hf.raw, indent=2, ensure_ascii=False)}\n"
        "```\n\nProduce the strict JSON verdict per your instructions."
    )
    resp = client.chat(
        [{"role": "system", "content": system},
         {"role": "user", "content": user}],
        max_tokens=JUDGE_MAX_TOKENS,
    )
    times = record.setdefault("phase_times", {}).setdefault("challenge", [])
    times.append({"file": hf.source_file, "s": round(resp.elapsed_s, 1)})
    if resp.error:
        record.setdefault("phase_errors", {}).setdefault(
            "challenge", []).append({"file": hf.source_file,
                                     "err": resp.error})
        hf.challenge = None
        return
    hf.challenge = parse_json_block(resp.text)


# ──────────────────────────────────────────────────────────────────
# Phase 4: skeptical validation (per finding).
# ──────────────────────────────────────────────────────────────────

def validate_finding(
    client: LLMClient,
    hf: HunterFinding,
    *,
    record: Dict[str, Any],
) -> None:
    system = (UPSTREAM / "validation.md").read_text()
    user = (
        f"Vulnerability report for review (source_file={hf.source_file}):\n\n"
        f"```json\n{json.dumps(hf.raw, indent=2, ensure_ascii=False)}\n"
        "```\n\nProduce the strict JSON verdict per your instructions."
    )
    resp = client.chat(
        [{"role": "system", "content": system},
         {"role": "user", "content": user}],
        max_tokens=JUDGE_MAX_TOKENS,
    )
    times = record.setdefault("phase_times", {}).setdefault("validate", [])
    times.append({"file": hf.source_file, "s": round(resp.elapsed_s, 1)})
    if resp.error:
        record.setdefault("phase_errors", {}).setdefault(
            "validate", []).append({"file": hf.source_file,
                                    "err": resp.error})
        hf.validation = None
        return
    hf.validation = parse_json_block(resp.text)


# ──────────────────────────────────────────────────────────────────
# Aggregate hunter raw → dump_writer.Finding.
# ──────────────────────────────────────────────────────────────────

def hunter_to_finding(
    hf: HunterFinding,
    cve: cve_loader.CVE,
    idx: int,
) -> Finding:
    r = hf.raw
    cat = str(r.get("category") or r.get("cwe") or "other_security").lower()
    sev = str(r.get("severity") or "unknown").lower()
    title_bits = []
    if r.get("impact"):
        title_bits.append(r["impact"])
    if r.get("cwe"):
        title_bits.append(r["cwe"])
    title = " / ".join(title_bits)
    desc_bits: List[str] = []
    if title:
        desc_bits.append(f"[{title}]")
    if r.get("exploit_path"):
        desc_bits.append("Exploit: " + r["exploit_path"])
    if r.get("taint_path"):
        desc_bits.append("Taint: " + " → ".join(r["taint_path"]))
    if r.get("sanitizer_status"):
        desc_bits.append("Sanitizer: " + r["sanitizer_status"])
    if r.get("source_description"):
        desc_bits.append("Source: " + r["source_description"])
    if r.get("fix_suggestion"):
        desc_bits.append("Fix: " + r["fix_suggestion"])
    description = "\n".join(desc_bits) or json.dumps(r)[:600]

    locs: List[Dict[str, Any]] = []
    if r.get("sink_file"):
        locs.append({
            "role": "sink",
            "code": r.get("sink_function") or "",
            "file": r["sink_file"],
            "line": r.get("sink_line"),
            "node_id": 1,
        })
    if r.get("source_file") and r.get("source_file") != r.get("sink_file"):
        locs.append({
            "role": "source",
            "code": r.get("source_description") or "",
            "file": r["source_file"],
            "line": r.get("source_line"),
            "node_id": 2,
        })

    extras: Dict[str, Any] = {}
    if r.get("confidence") is not None:
        extras["confidence"] = r["confidence"]
    if r.get("cvss_estimate") is not None:
        extras["cvss_estimate"] = r["cvss_estimate"]
    if hf.challenge:
        extras["challenge_verdict"] = hf.challenge.get("verdict")
        extras["severity_after_challenge"] = hf.challenge.get(
            "severity_after_challenge"
        )
    if hf.validation:
        extras["validator_verdict"] = hf.validation.get("verdict")
        extras["validator_adjusted_severity"] = hf.validation.get(
            "adjusted_severity"
        )

    return Finding(
        hypothesis_id=f"B3_mythos_{cve.cve_id}_{idx}",
        category=cat,
        description=description,
        severity=sev,
        locations=locs,
        extras=extras,
    )


# ──────────────────────────────────────────────────────────────────
# Per-CVE orchestrator.
# ──────────────────────────────────────────────────────────────────

def process_cve(
    cve: cve_loader.CVE,
    client: LLMClient,
    catalog,
    dump_base: str,
    timestamp: str,
    *,
    max_files: int = MAX_FILES_TO_HUNT,
    dry_run: bool = False,
) -> Dict[str, Any]:
    record: Dict[str, Any] = {
        "cve_id": cve.cve_id,
        "max_files": max_files,
    }
    if cve.src_dir is None:
        record["status"] = "SKIP_NO_SRC"
        return record

    # Phase 1: sink-slice.
    hits = slice_tree(cve.src_dir, catalog)
    density = density_per_file(hits)
    files_all = sorted(density.keys())
    if not files_all:
        # No sink hits → still include the patch-touched files as
        # last-resort scope (mythos would skip but for fair comparison
        # to Lace we hunt anyway).
        files_all = sorted(
            os.path.relpath(os.path.join(r, n), cve.src_dir)
            for r, _, names in os.walk(cve.src_dir)
            for n in names if n.endswith((".c", ".h"))
        )

    # De-dup (basename, size)-duplicated copies introduced by
    # patch_expander.py: it places each patch-touched file both at
    # its canonical kernel-style path and at the src root. We prefer
    # the deeper (informative) path so the ranker reasons about the
    # subsystem layout correctly.
    def _dedup(files: List[str]) -> List[str]:
        groups: Dict[Tuple[str, int], List[str]] = {}
        for rel in files:
            full = os.path.join(cve.src_dir, rel)
            try:
                sz = os.path.getsize(full)
            except OSError:
                sz = -1
            key = (os.path.basename(rel), sz)
            groups.setdefault(key, []).append(rel)
        out: List[str] = []
        for cands in groups.values():
            cands.sort(key=lambda p: (-p.count("/"), len(p)))
            out.append(cands[0])
        return sorted(out)

    files_all = _dedup(files_all)
    record["phase1_n_hits"] = len(hits)
    record["phase1_n_files_with_hits"] = sum(1 for f in files_all if density.get(f))

    if dry_run:
        record["status"] = "DRY"
        record["files_considered"] = files_all[:max_files]
        return record

    # Phase 2: rank.
    ranked = rank_files(client, cve, files_all, density, record=record)
    top = ranked[:max_files]
    record["phase2_top_files"] = [
        {"rank": r, "file": f, "reason": rs} for r, f, rs in top
    ]

    # Phase 3: hunt per file.
    hits_by_file: Dict[str, List[SinkHit]] = {}
    for h in hits:
        hits_by_file.setdefault(h.file, []).append(h)
    all_findings: List[HunterFinding] = []
    hunter_runs: List[HunterRun] = []
    # patch_expander.py duplicates each patch-touched file twice under
    # src/ (once at the kernel-style path, once flattened to the root)
    # so we de-dup by (basename, file-size) which is cheap and catches
    # those copies without paying SHA256 on every file.
    seen_basename_size: set = set()
    for _, rel_file, _ in top:
        full_path = os.path.join(cve.src_dir, rel_file)
        if not os.path.isfile(full_path):
            base = os.path.basename(rel_file)
            cands = []
            for root, _, names in os.walk(cve.src_dir):
                for n in names:
                    if n == base:
                        cands.append(os.path.join(root, n))
            if cands:
                full_path = cands[0]
            else:
                continue
        try:
            sz = os.path.getsize(full_path)
        except OSError:
            sz = -1
        dedup_key = (os.path.basename(full_path), sz)
        if dedup_key in seen_basename_size:
            continue
        seen_basename_size.add(dedup_key)
        all_findings.extend(hunt_one_file(
            client, cve, rel_file, full_path,
            hits_by_file.get(rel_file, []),
            record=record,
            hunter_runs=hunter_runs,
        ))
    record["phase3_raw_findings"] = len(all_findings)

    # Phase 3.5: self-challenge (drop ADVERSE).
    surviving: List[HunterFinding] = []
    for hf in all_findings:
        challenge_finding(client, hf, record=record)
        verdict = (hf.challenge or {}).get("verdict", "")
        if verdict == "ADVERSE":
            hf.final_status = "DROPPED_BY_CHALLENGE"
            continue
        surviving.append(hf)
    record["phase35_after_challenge"] = len(surviving)

    # Phase 4: validation (drop FALSE_POSITIVE).
    confirmed: List[HunterFinding] = []
    for hf in surviving:
        validate_finding(client, hf, record=record)
        verdict = (hf.validation or {}).get("verdict", "")
        if verdict == "FALSE_POSITIVE":
            hf.final_status = "DROPPED_BY_VALIDATOR"
            continue
        hf.final_status = "CONFIRMED" if verdict == "CONFIRMED" else "NEUTRAL"
        confirmed.append(hf)
    record["phase4_confirmed"] = len(confirmed)

    # Phase 6: aggregate.
    finding_objs = [hunter_to_finding(hf, cve, i + 1) for i, hf in enumerate(confirmed)]
    record["n_findings_emitted"] = len(finding_objs)
    write_dump(
        dump_base, cve.cve_id, finding_objs,
        raw_responses=[{
            "phase1_sink_hits": [
                {"category": h.category, "file": h.file,
                 "line": h.line, "snippet": h.snippet} for h in hits
            ][:200],
            "phase2_ranked": [
                {"rank": r, "file": f, "reason": rs} for r, f, rs in ranked
            ],
            "phase3_hunter_runs": [
                {"source_file": hr.source_file,
                 "elapsed_s": hr.elapsed_s,
                 "parsed_verdict": hr.parsed_verdict,
                 "parsed_findings_count": hr.parsed_findings_count,
                 "assistant_text": hr.assistant_text}
                for hr in hunter_runs
            ],
            "phase3_hunter_findings": [
                {"source_file": hf.source_file, "raw": hf.raw,
                 "challenge": hf.challenge, "validation": hf.validation,
                 "final_status": hf.final_status}
                for hf in all_findings
            ],
        }],
        meta={
            "baseline": "B3_mythos",
            "model": client.model,
            "endpoint": client.endpoint,
            "phase_times": record.get("phase_times", {}),
            "phase_errors": record.get("phase_errors", {}),
            "phase_counts": {
                "phase1_hits": record["phase1_n_hits"],
                "phase2_top_files": len(top),
                "phase3_raw_findings": record["phase3_raw_findings"],
                "phase35_after_challenge": record["phase35_after_challenge"],
                "phase4_confirmed": record["phase4_confirmed"],
            },
            "max_files": max_files,
            "status": "OK",
        },
        timestamp=timestamp,
    )
    record["status"] = "OK"
    return record


def main(argv: Optional[List[str]] = None) -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--cve", nargs="*", help="Restrict to these CVE IDs.")
    ap.add_argument("--limit", type=int, default=0)
    ap.add_argument("--dump-base", default=DEFAULT_DUMP_BASE)
    ap.add_argument("--max-files", type=int, default=MAX_FILES_TO_HUNT,
                    help="Top-N files to hunt (Mythos default 8).")
    ap.add_argument("--dry-run", action="store_true")
    ap.add_argument("--skip-existing", action="store_true")
    args = ap.parse_args(argv)

    catalog = load_sink_catalog(str(SINK_CATALOG))
    print(f"[B3] sink catalog: {len(catalog)} patterns loaded")

    cves = cve_loader.list_cves()
    if args.cve:
        wanted = set(args.cve)
        cves = [c for c in cves if c.cve_id in wanted]
    if args.limit > 0:
        cves = cves[: args.limit]

    dump_base = os.path.join(args.dump_base, "B3_mythos")
    os.makedirs(dump_base, exist_ok=True)
    if args.skip_existing:
        kept = []
        for c in cves:
            if list(Path(dump_base).glob(f"{c.cve_id}_*")):
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
        print(f"[Preflight OK] model={client.model}")

    timestamp = datetime.now().strftime("%Y-%m-%d_%H-%M-%S")
    print(f"\n=== B3 Mythos batch (n={len(cves)}, ts={timestamp}) ===")
    summary: List[Dict[str, Any]] = []
    for i, cve in enumerate(cves, 1):
        print(f"  [{i}/{len(cves)}] {cve.cve_id} ", end="", flush=True)
        rec = process_cve(
            cve, client, catalog, dump_base, timestamp,
            max_files=args.max_files, dry_run=args.dry_run,
        )
        if args.dry_run:
            print(
                f"DRY (hits={rec.get('phase1_n_hits')}, "
                f"files={rec.get('phase1_n_files_with_hits')})"
            )
        elif rec["status"] == "OK":
            pc = {
                "p1": rec["phase1_n_hits"],
                "p2": len(rec.get("phase2_top_files", [])),
                "p3": rec["phase3_raw_findings"],
                "p35": rec["phase35_after_challenge"],
                "p4": rec["phase4_confirmed"],
            }
            print(f"OK p1={pc['p1']} p2={pc['p2']} p3={pc['p3']} "
                  f"p35={pc['p35']} p4={pc['p4']}")
        else:
            print(f"{rec['status']}")
        summary.append(rec)

    summary_path = os.path.join(dump_base, f"run_summary_{timestamp}.json")
    with open(summary_path, "w") as f:
        json.dump({
            "timestamp": timestamp,
            "n_cves": len(cves),
            "dry_run": args.dry_run,
            "max_files": args.max_files,
            "records": summary,
        }, f, indent=2, ensure_ascii=False)
    print(f"\nRun summary: {summary_path}")
    print(f"Dump base:   {dump_base}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
