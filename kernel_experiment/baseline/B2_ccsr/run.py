#!/usr/bin/env python3
"""B2 — Claude Code Security Review baseline.

Replays Anthropic's publicly released ``/security-review`` prompt
(``anthropics/claude-code-security-review`` → ``.claude/commands/
security-review.md``) verbatim against GPT-5.5 over the ByteDance
gateway. The prompt is designed for PR-diff inputs; we adapt the
CVE setting to it by **reversing the fix patch** so the synthetic
diff represents "the commit that introduced the vulnerability".

What's verbatim vs. what's adapted
----------------------------------
VERBATIM
  * The system prompt body — OBJECTIVE, CRITICAL INSTRUCTIONS,
    SECURITY CATEGORIES, ANALYSIS METHODOLOGY, OUTPUT FORMAT,
    SEVERITY GUIDELINES, FALSE POSITIVE FILTERING, and the 3-step
    "START ANALYSIS" instruction. These are pasted unchanged from
    upstream/security-review.md.
  * The 4 bash-substituted blocks (GIT STATUS / FILES MODIFIED /
    COMMITS / DIFF CONTENT). We substitute the `!`-prefixed bash
    commands with synthetic output rather than running git, since
    Claude Code CLI's bash-expansion is unavailable when the prompt
    is shipped to a plain Chat Completion endpoint.

ADAPTED
  * Sub-tasks. The prompt asks the model to "use a sub-task" via
    Claude Code's `Task` tool. Single-shot Chat Completion has no
    such tool, so we append a single-paragraph addendum at the END
    of the user message explaining that the analysis must be
    completed in-line. The system prompt is left untouched.

Disclaimers
-----------
This is NOT "Claude Code Security Review the product" — that product
ships with Claude Sonnet as the backbone model and a Claude Code CLI
runtime with file-system tooling. We hold the model+endpoint constant
with Lace (GPT-5.5 over the same ByteDance gateway) so the comparison
controls for *capability*, not *product surface*. The baseline label
in the final report is therefore:
  "Claude Code Security Review prompt (Anthropic OSS, commit-pinned),
   executed against GPT-5.5 for fair model-controlled comparison."
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

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE.parent))

from common import cve_loader  # noqa: E402
from common.llm_client import LLMClient  # noqa: E402
from common.dump_writer import Finding, write_dump  # noqa: E402


UPSTREAM_PROMPT = HERE / "upstream" / "security-review.md"
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

USER_ADDENDUM = (
    "\n\n---\n\n"
    "EXECUTION NOTE: You are being invoked over a single-shot Chat "
    "Completion endpoint. Anthropic's `Task` sub-agent tool is not "
    "available in this context. Complete the entire 3-step "
    "(identify → false-positive filter → confidence-cap) analysis "
    "described in START ANALYSIS yourself, in-line, before emitting "
    "the final markdown report. The final assistant message must be "
    "the markdown report and nothing else."
)


def load_system_prompt() -> str:
    """Strip the front-matter, keep the body. The body still
    contains the `!\\`...\\`` bash placeholders — they are substituted
    per-CVE in build_user_payload()."""
    raw = UPSTREAM_PROMPT.read_text()
    # Strip leading YAML front matter (`--- ... ---`).
    if raw.startswith("---"):
        m = re.match(r"^---\n.*?\n---\n", raw, flags=re.S)
        if m:
            raw = raw[m.end():]
    return raw.strip()


def synth_git_outputs(cve: cve_loader.CVE) -> Dict[str, str]:
    """Produce the four placeholders the stock prompt requires.

    We treat the reversed-fix patch as the diff to be reviewed. The
    `git status` / `git log` / `git diff --name-only` are
    syntactically faithful but minimal — the model only really uses
    the DIFF CONTENT for its analysis.
    """
    reversed_diff = cve_loader.reverse_patch(cve.patch or "")
    files = cve.files_touched or []
    cve_id = cve.cve_id

    git_status = (
        "On branch baseline-reintroduce-vuln\n"
        "Your branch is up to date with 'origin/baseline-reintroduce-vuln'.\n\n"
        "Changes to be committed:\n"
        + "\n".join(f"\tmodified:   {f}" for f in files)
        + "\n\n"
    )
    git_diff_names = "\n".join(files)
    git_log = (
        f"commit 0000000baseline00 (HEAD -> baseline-reintroduce-vuln)\n"
        f"Author: Lace Baseline <baseline@example.invalid>\n"
        f"Date:   {datetime.now().strftime('%a %b %d %H:%M:%S %Y +0000')}\n\n"
        f"    Re-introduce vulnerability tracked by {cve_id}\n\n"
        f"    Synthetic commit produced by the Lace baseline harness:\n"
        f"    the official fix for {cve_id} was reversed so the diff\n"
        f"    represents the change that originally created the bug.\n"
    )
    return {
        "git_status": git_status.strip(),
        "git_diff_names": git_diff_names,
        "git_log": git_log,
        "git_diff": reversed_diff,
    }


def substitute_bash_placeholders(system_prompt: str,
                                 outputs: Dict[str, str]) -> str:
    """Replace `!\\`git status\\``, `!\\`git diff --name-only ...\\``,
    `!\\`git log ...\\``, `!\\`git diff --merge-base ...\\`` (each
    appears once in the upstream prompt body)."""
    return (
        system_prompt
        .replace("!`git status`", outputs["git_status"])
        .replace("!`git diff --name-only origin/HEAD...`",
                 outputs["git_diff_names"])
        .replace("!`git log --no-decorate origin/HEAD...`",
                 outputs["git_log"])
        .replace("!`git diff --merge-base origin/HEAD`",
                 outputs["git_diff"])
    )


# ──────────────────────────────────────────────────────────────────
# Markdown-finding parser.
# Upstream output schema (verbatim from security-review.md):
#
#   # Vuln 1: XSS: `foo.py:42`
#
#   * Severity: High
#   * Description: ...
#   * Exploit Scenario: ...
#   * Recommendation: ...
#
# We accept slight variations (Vuln/Vulnerability, Severity capitalization,
# colon vs no colon after the heading, etc.).
# ──────────────────────────────────────────────────────────────────

_HEADING_RE = re.compile(
    r"^#+\s*(?:Vuln(?:erability)?|Finding)\s*(\d+)?\s*[:.]?\s*(.*)$",
    flags=re.IGNORECASE,
)
_BULLET_RE = re.compile(r"^\s*[\*\-]\s*\*?\*?(\w[\w ]*?)\*?\*?\s*[:：]\s*(.*)$")
_LOC_RE = re.compile(r"`([^`:\s]+):(\d+)`")


def parse_markdown_findings(text: str) -> List[Dict[str, Any]]:
    """Best-effort split a markdown security report into a list of
    finding dicts. Tolerates the typical model deviations."""
    if not text or text.strip().lower() in ("no findings.", "no vulnerabilities found."):
        return []
    lines = text.splitlines()
    findings: List[Dict[str, Any]] = []
    cur: Optional[Dict[str, Any]] = None
    cur_body: List[str] = []

    def flush() -> None:
        nonlocal cur, cur_body
        if cur is not None:
            cur["body"] = "\n".join(cur_body).strip()
            findings.append(cur)
        cur = None
        cur_body = []

    for line in lines:
        h = _HEADING_RE.match(line.strip())
        if h:
            flush()
            cur = {
                "index": h.group(1),
                "title": (h.group(2) or "").strip(),
                "fields": {},
            }
            cur_body = []
            continue
        if cur is None:
            continue
        cur_body.append(line)
        b = _BULLET_RE.match(line)
        if b:
            k = b.group(1).strip().lower().replace(" ", "_")
            v = b.group(2).strip()
            cur["fields"][k] = v
    flush()
    return findings


_CATEGORY_HINTS = [
    ("data_race", ("race", "data race", "concurrent")),
    ("use_after_free", ("uaf", "use-after-free", "use after free")),
    ("double_free", ("double free", "double-free")),
    ("null_deref", ("null deref", "null-pointer", "null pointer")),
    ("rcu_misuse", ("rcu",)),
    ("deadlock", ("deadlock",)),
    ("memory_corruption", ("buffer overflow", "oob", "out-of-bound", "memory corruption")),
    ("integer_overflow", ("integer overflow", "underflow", "wraparound")),
    ("command_injection", ("command injection",)),
    ("path_traversal", ("path traversal",)),
    ("authn_bypass", ("authentication bypass", "auth bypass")),
    ("info_disclosure", ("info disclosure", "information disclosure")),
    ("xss", ("xss", "cross-site")),
    ("sql_injection", ("sql injection", "sqli")),
]


def infer_category(title: str, body: str) -> str:
    blob = (title + " " + body).lower()
    for cat, keys in _CATEGORY_HINTS:
        if any(k in blob for k in keys):
            return cat
    return "other_security"


def normalize_findings(
    cve: cve_loader.CVE,
    parsed: List[Dict[str, Any]],
) -> List[Finding]:
    out: List[Finding] = []
    for i, r in enumerate(parsed, 1):
        title = r.get("title") or ""
        body = r.get("body") or ""
        fields = r.get("fields") or {}

        severity = fields.get("severity") or "unknown"
        desc_bits: List[str] = []
        if title:
            desc_bits.append(f"Title: {title}")
        if "description" in fields:
            desc_bits.append(fields["description"])
        if "exploit_scenario" in fields:
            desc_bits.append("Exploit: " + fields["exploit_scenario"])
        description = "\n".join(desc_bits) or body[:600]

        # Locations: pull every `file:line` backtick-quoted token
        # from the title + body. Dedup, preserve order.
        seen: set = set()
        locs: List[Dict[str, Any]] = []
        for chunk in (title, body):
            for m in _LOC_RE.finditer(chunk):
                key = (m.group(1), m.group(2))
                if key in seen:
                    continue
                seen.add(key)
                locs.append({
                    "role": "sink" if not locs else f"site{len(locs) + 1}",
                    "code": "",
                    "file": m.group(1),
                    "line": int(m.group(2)),
                    "node_id": len(locs) + 1,
                })

        category = infer_category(title, body)
        extras: Dict[str, Any] = {}
        for k in ("recommendation", "fix", "fix_recommendation",
                  "confidence", "category", "cwe"):
            if k in fields:
                extras[k] = fields[k]

        out.append(Finding(
            hypothesis_id=f"B2_ccsr_{cve.cve_id}_{i}",
            category=category,
            description=description.strip(),
            severity=severity.lower(),
            locations=locs,
            extras=extras,
        ))
    return out


def process_cve(
    cve: cve_loader.CVE,
    client: LLMClient,
    system_prompt_template: str,
    dump_base: str,
    timestamp: str,
    dry_run: bool = False,
) -> Dict[str, Any]:
    gits = synth_git_outputs(cve)
    sys_msg = substitute_bash_placeholders(system_prompt_template, gits)
    user_msg = (
        f"Target CVE: {cve.cve_id}\n"
        f"Begin the security review per your instructions."
        + USER_ADDENDUM
    )
    record: Dict[str, Any] = {
        "cve_id": cve.cve_id,
        "sys_msg_chars": len(sys_msg),
        "diff_chars": len(gits["git_diff"]),
    }
    if dry_run:
        record["status"] = "DRY"
        return record
    t0 = time.time()
    resp = client.chat([
        {"role": "system", "content": sys_msg},
        {"role": "user", "content": user_msg},
    ])
    record["elapsed_s"] = round(resp.elapsed_s, 1)
    if resp.error:
        record["status"] = "API_ERROR"
        record["error"] = resp.error
        write_dump(
            dump_base, cve.cve_id, [],
            raw_responses=[{"error": resp.error}],
            meta={
                "baseline": "B2_ccsr",
                "model": client.model,
                "endpoint": client.endpoint,
                "diff_chars": len(gits["git_diff"]),
                "status": "API_ERROR",
            },
            timestamp=timestamp,
        )
        return record
    parsed = parse_markdown_findings(resp.text)
    findings = normalize_findings(cve, parsed)
    record["n_findings"] = len(findings)
    record["status"] = "OK"
    write_dump(
        dump_base, cve.cve_id, findings,
        raw_responses=[{
            "system_template_chars": len(system_prompt_template),
            "system_substituted_chars": len(sys_msg),
            "user": user_msg,
            "assistant_text": resp.text,
            "parsed_markdown_findings": parsed,
        }],
        meta={
            "baseline": "B2_ccsr",
            "model": client.model,
            "endpoint": client.endpoint,
            "elapsed_s": round(resp.elapsed_s, 1),
            "diff_chars": len(gits["git_diff"]),
            "n_findings_parsed": len(parsed),
            "status": "OK",
        },
        timestamp=timestamp,
    )
    return record


def main(argv: Optional[List[str]] = None) -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--cve", nargs="*", help="Restrict to these CVE IDs.")
    ap.add_argument("--limit", type=int, default=0)
    ap.add_argument("--dump-base", default=DEFAULT_DUMP_BASE)
    ap.add_argument("--dry-run", action="store_true")
    ap.add_argument("--skip-existing", action="store_true")
    args = ap.parse_args(argv)

    sys_template = load_system_prompt()
    cves = cve_loader.list_cves()
    if args.cve:
        wanted = set(args.cve)
        cves = [c for c in cves if c.cve_id in wanted]
    if args.limit > 0:
        cves = cves[: args.limit]
    dump_base = os.path.join(args.dump_base, "B2_ccsr")
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
    print(f"\n=== B2 CCSR batch (n={len(cves)}, ts={timestamp}) ===")
    summary: List[Dict[str, Any]] = []
    for i, cve in enumerate(cves, 1):
        print(f"  [{i}/{len(cves)}] {cve.cve_id} ... ", end="", flush=True)
        rec = process_cve(cve, client, sys_template, dump_base,
                          timestamp, dry_run=args.dry_run)
        if args.dry_run:
            print(f"DRY (sys={rec['sys_msg_chars']} chars, "
                  f"diff={rec['diff_chars']} chars)")
        elif rec["status"] == "OK":
            print(f"OK ({rec['n_findings']} finding(s), "
                  f"{rec['elapsed_s']}s)")
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
