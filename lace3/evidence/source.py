"""Sandboxed access to the source tree the IR was compiled from.

Everything the detector shows or lets the model read comes through here.
Paths resolve inside the given root only, and file names that carry answers
rather than code -- patches, ground truth, CVE write-ups, hand-written entry
lists -- are refused even when they sit inside the root.

DWARF records absolute paths from the build machine; they are mapped onto the
root by the longest path suffix that exists under it.
"""

import os
import re
from pathlib import Path

DENY = re.compile(r"(\.patch$|\.diff$|\.orig$|\.rej$|flow_annotation|ground_truth|expected_contract|"
                  r"dataset_entrypoints|expansion_report|(^|/)cve-|syzbot|judge|bugs\.txt$)", re.I)
TEXT_SUFFIXES = (".c", ".h", ".S", ".s", ".rs", ".lds", ".txt", ".rst", "Kconfig", "Makefile",
                 "Kbuild")
MAX_FILE_BYTES = 4 << 20


class SourceError(Exception):
    pass


class SourceTree:
    def __init__(self, root):
        self.root = Path(root).resolve()
        if not self.root.is_dir():
            raise SourceError(f"source root {root} is not a directory")
        self._files = None
        self._resolved = {}
        self._lines = {}

    def files(self):
        if self._files is None:
            out = []
            for dirpath, dirnames, filenames in os.walk(self.root):
                dirnames[:] = [d for d in dirnames if not d.startswith(".")]
                for f in filenames:
                    rel = os.path.relpath(os.path.join(dirpath, f), self.root)
                    if not DENY.search(rel) and rel.endswith(TEXT_SUFFIXES):
                        out.append(rel)
            self._files = sorted(out)
        return self._files

    def _by_suffix(self):
        if not hasattr(self, "_suffix_index"):
            idx = {}
            for rel in self.files():
                parts = rel.split(os.sep)
                for k in range(len(parts)):
                    idx.setdefault(os.sep.join(parts[k:]), []).append(rel)
            self._suffix_index = idx
        return self._suffix_index

    def resolve(self, path):
        """Relative path under the root for a DWARF or user path, or None."""
        if path is None:
            return None
        if path in self._resolved:
            return self._resolved[path]
        out = None
        p = str(path)
        cand = (self.root / p).resolve() if not os.path.isabs(p) else Path(p).resolve()
        if str(cand).startswith(str(self.root) + os.sep) and cand.is_file():
            rel = os.path.relpath(cand, self.root)
            out = None if DENY.search(rel) else rel
        if out is None:
            parts = Path(p).parts
            idx = self._by_suffix()
            for k in range(len(parts)):
                hits = idx.get(os.sep.join(parts[k:]))
                if hits:
                    out = hits[0] if len(hits) == 1 or k < len(parts) - 1 else None
                    break
        self._resolved[path] = out
        return out

    def lines(self, rel):
        if rel not in self._lines:
            full = (self.root / rel).resolve()
            if not str(full).startswith(str(self.root) + os.sep) or DENY.search(rel):
                raise SourceError(f"{rel}: outside the source sandbox")
            if full.stat().st_size > MAX_FILE_BYTES:
                raise SourceError(f"{rel}: too large")
            self._lines[rel] = full.read_text(errors="replace").splitlines()
        return self._lines[rel]

    def read(self, path, start=1, end=None, max_lines=400):
        rel = self.resolve(path)
        if rel is None:
            raise SourceError(f"{path}: no such file in the source tree (or not readable)")
        ls = self.lines(rel)
        start = max(1, int(start))
        end = len(ls) if end is None else min(len(ls), int(end))
        end = min(end, start + max_lines - 1)
        return rel, [(n, ls[n - 1]) for n in range(start, end + 1)]

    def grep(self, pattern, path_glob=None, max_hits=60):
        try:
            rx = re.compile(pattern)
        except re.error as e:
            raise SourceError(f"bad regex: {e}") from None
        hits, scanned = [], 0
        for rel in self.files():
            if path_glob and not Path(rel).match(path_glob) and path_glob not in rel:
                continue
            scanned += 1
            try:
                ls = self.lines(rel)
            except SourceError:
                continue
            for n, line in enumerate(ls, 1):
                if rx.search(line):
                    hits.append((rel, n, line.strip()))
                    if len(hits) >= max_hits:
                        return hits, True
        return hits, False

    def function_span(self, path, start_line, max_lines=200):
        """(first, last) line of the function whose definition starts at start_line,
        found by brace matching; last is capped."""
        rel = self.resolve(path)
        if rel is None:
            return None
        ls = self.lines(rel)
        depth, opened = 0, False
        for n in range(start_line, min(len(ls), start_line + 2000) + 1):
            text = _strip_comments_and_strings(ls[n - 1])
            for ch in text:
                if ch == "{":
                    depth += 1
                    opened = True
                elif ch == "}":
                    depth -= 1
            if opened and depth <= 0:
                return start_line, n
        return start_line, min(len(ls), start_line + max_lines)


_STR = re.compile(r'"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'')


def _strip_comments_and_strings(line):
    line = _STR.sub('""', line)
    return line.split("//", 1)[0]
