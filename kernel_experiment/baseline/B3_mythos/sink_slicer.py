"""Sink-guided slicing for the B3 Mythos baseline.

Reads a Mythos-style sink catalog (TAB-separated: ``CATEGORY \\t REGEX``)
and produces, for every C/H file under the CVE's src tree, a list of
hits ``{category, pattern, file, line, snippet}``. The output schema
matches the Mythos v4 sink-slice phase
(`scripts/mythos-v4.sh` + `scripts/lib/sinks/c-cpp.txt`).

Implementation note: upstream uses ripgrep through `ripgrep --json`,
which is not installed on our evaluation machine. We use Python's
`re` module instead, which is fast enough for typical CVE src trees
(≤ 50 files, ≤ 5k LoC) and removes an external runtime dependency.
"""
from __future__ import annotations

import os
import re
from dataclasses import dataclass
from typing import Dict, Iterable, List, Tuple


@dataclass
class SinkHit:
    category: str
    pattern: str
    file: str
    line: int
    snippet: str


def load_sink_catalog(path: str) -> List[Tuple[str, re.Pattern]]:
    """Parse ``c-cpp.txt`` (or any TAB-separated sink list).

    Lines starting with ``#`` or blank lines are ignored. Each
    remaining line is ``CATEGORY \\t REGEX``.
    """
    entries: List[Tuple[str, re.Pattern]] = []
    with open(path, "r", encoding="utf-8") as f:
        for raw in f:
            line = raw.rstrip("\n")
            if not line.strip() or line.lstrip().startswith("#"):
                continue
            if "\t" not in line:
                continue
            cat, pat = line.split("\t", 1)
            try:
                entries.append((cat.strip(), re.compile(pat.strip())))
            except re.error:
                continue
    return entries


def slice_file(
    path: str,
    rel_path: str,
    catalog: Iterable[Tuple[str, re.Pattern]],
) -> List[SinkHit]:
    """Scan a single source file for sink matches."""
    hits: List[SinkHit] = []
    try:
        with open(path, "r", encoding="utf-8", errors="ignore") as f:
            text = f.read()
    except OSError:
        return hits
    for i, line in enumerate(text.splitlines(), 1):
        # Skip preprocessor and comments — sinks in those are not
        # real reachable code. This matches mythos-v4's behaviour.
        s = line.strip()
        if s.startswith(("//", "*", "/*", "#define ", "#include ")):
            continue
        for cat, pat in catalog:
            if pat.search(line):
                hits.append(SinkHit(
                    category=cat,
                    pattern=pat.pattern,
                    file=rel_path,
                    line=i,
                    snippet=line.strip()[:200],
                ))
    return hits


def slice_tree(
    src_root: str,
    catalog: Iterable[Tuple[str, re.Pattern]],
    suffixes=(".c", ".h"),
) -> List[SinkHit]:
    """Walk a src tree and collect all sink hits."""
    out: List[SinkHit] = []
    cat_list = list(catalog)
    for root, _, names in os.walk(src_root):
        for n in names:
            if not n.endswith(suffixes):
                continue
            full = os.path.join(root, n)
            rel = os.path.relpath(full, src_root)
            out.extend(slice_file(full, rel, cat_list))
    return out


def density_per_file(hits: Iterable[SinkHit]) -> Dict[str, Dict[str, int]]:
    """Aggregate hit counts: file → category → count.

    Used by the file-ranker phase to bias toward high-yield files
    (mythos-v4 phase 2 — files whose matches are all SAFE_* variants
    are demoted; here we keep all categories as positive signals
    since the c-cpp.txt shipped with mythos-research does not split
    SAFE/UNSAFE for the C catalog).
    """
    out: Dict[str, Dict[str, int]] = {}
    for h in hits:
        out.setdefault(h.file, {})
        out[h.file][h.category] = out[h.file].get(h.category, 0) + 1
    return out
