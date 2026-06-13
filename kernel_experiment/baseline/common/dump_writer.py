"""Emit baseline findings in the bugs.txt / confirmed_hypotheses.log
format that scripts/evaluate_recall.py reads, so the judge can score
every baseline with zero modifications.

evaluate_recall.py splits on the literal banner:
    ========== Hypothesis-Based Violation Detected ==========
and treats each chunk as one bug report. Each chunk must contain
free-text identifying:
  * Hypothesis: <id>       -- arbitrary string, opaque to the judge
  * Category: <bug_kind>   -- consistent label (data_race / uaf /
                              double_free / null_deref / atomicity /
                              order_violation / use_before_init / ...)
  * Description: <text>    -- the human-readable bug description; the
                              judge keys off this for root-cause
                              matching against the CVE patch
  * --- Involved Nodes --- -- at least one '<role> (node N): <code>
                              [<file>:<line>]' tuple is highly
                              recommended (the judge inspects
                              file/line overlap with the patch)

We follow the layout LLM_dump/<CVE>_<TIMESTAMP>/ that Lace itself
uses, so swapping DUMP_BASE in evaluate_recall.py picks the baseline
up directly.
"""
from __future__ import annotations

import json
import os
from dataclasses import asdict, dataclass, field
from datetime import datetime
from typing import Any, Dict, List, Optional, Sequence


_BANNER = "========== Hypothesis-Based Violation Detected =========="


@dataclass
class Finding:
    """A single tool finding, normalized across all baselines."""

    hypothesis_id: str
    category: str
    description: str
    severity: str = "unknown"
    # Optional location tuples ([role, code_snippet, file, line]).
    locations: List[Dict[str, Any]] = field(default_factory=list)
    # Provider-specific extras (CWE, confidence, exploit sketch, ...).
    extras: Dict[str, Any] = field(default_factory=dict)

    def to_bugs_chunk(self) -> str:
        lines: List[str] = [
            _BANNER,
            f"Hypothesis: {self.hypothesis_id}",
            f"Category: {self.category} (severity: {self.severity})",
            f"Description: {self.description}",
            "",
            "--- Involved Nodes ---",
        ]
        if not self.locations:
            lines.append("  (none reported)")
        for loc in self.locations:
            role = loc.get("role", "site")
            code = loc.get("code") or ""
            f = loc.get("file") or ""
            ln = loc.get("line")
            ln_str = f":{ln}" if ln else ""
            code_str = f": {code}" if code else ""
            file_part = f"  [{f}{ln_str}]" if f else ""
            node_id = loc.get("node_id", "?")
            lines.append(f"  {role} (node {node_id}){code_str}{file_part}")
        lines.append("")
        lines.append("--- Verified Constraints ---")
        if self.extras:
            for k, v in self.extras.items():
                lines.append(f"  [meta] {k}: {v}")
        else:
            lines.append("  (baseline does not produce DSL constraints)")
        lines.append("=" * 58)
        return "\n".join(lines)


def write_dump(
    dump_base: str,
    cve_id: str,
    findings: Sequence[Finding],
    *,
    raw_responses: Optional[Sequence[Dict[str, Any]]] = None,
    meta: Optional[Dict[str, Any]] = None,
    timestamp: Optional[str] = None,
) -> str:
    """Persist findings + provenance for one CVE in Lace-compatible layout.

    Returns the dump directory path."""
    ts = timestamp or datetime.now().strftime("%Y-%m-%d_%H-%M-%S")
    out_dir = os.path.join(dump_base, f"{cve_id}_{ts}")
    os.makedirs(os.path.join(out_dir, "stateful_bugs"), exist_ok=True)

    chunks: List[str] = []
    for i, fnd in enumerate(findings, 1):
        chunks.append(fnd.to_bugs_chunk())
        chunks.append(f"\ncount  {i} {'-' * 40}\n")
    bugs_text = "\n".join(chunks) if chunks else ""

    with open(os.path.join(out_dir, "stateful_bugs", "bugs.txt"), "w") as f:
        f.write(bugs_text)
    with open(os.path.join(out_dir, "confirmed_hypotheses.log"), "w") as f:
        f.write("========= Open-Hypothesis Detection Results =========\n\n")
        for fnd in findings:
            f.write(
                json.dumps(
                    {
                        "bug_category": fnd.category,
                        "hypothesis_id": fnd.hypothesis_id,
                        "description": fnd.description,
                        "severity": fnd.severity,
                        "nodes": {
                            loc.get("role", f"site{i}"): {
                                "code": loc.get("code", ""),
                                "id": loc.get("node_id"),
                                "file": loc.get("file", ""),
                                "line": loc.get("line"),
                            }
                            for i, loc in enumerate(fnd.locations)
                        },
                        "constraints": [],
                        "extras": fnd.extras,
                    },
                    indent=2,
                    ensure_ascii=False,
                )
            )
            f.write("\n\n")

    if raw_responses is not None:
        with open(os.path.join(out_dir, "raw_llm_responses.json"), "w") as f:
            json.dump(list(raw_responses), f, indent=2, ensure_ascii=False)
    if meta is not None:
        with open(os.path.join(out_dir, "baseline_meta.json"), "w") as f:
            json.dump(meta, f, indent=2, ensure_ascii=False)
    return out_dir
