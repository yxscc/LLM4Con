"""DWARF metadata read from the textual IR.

llvmlite exposes no metadata API, so the `!N = ...` lines are parsed directly.
Only what the pipeline needs is modelled: source locations, subprograms, and
struct/union layouts for naming fields by byte offset.
"""

import os
import re
from dataclasses import dataclass

RE_MD = re.compile(r"^!(\d+) = (?:distinct )?(.*)$", re.M)
RE_NODE = re.compile(r"^!(DI\w+)\((.*)\)$", re.S)
RE_REF = re.compile(r"^!(\d+)$")
RE_ESC = re.compile(r"\\([0-9A-Fa-f]{2})")

QUALIFIER_TAGS = {"DW_TAG_typedef", "DW_TAG_const_type", "DW_TAG_volatile_type",
                  "DW_TAG_restrict_type", "DW_TAG_atomic_type"}
RECORD_TAGS = {"DW_TAG_structure_type", "DW_TAG_union_type"}


@dataclass(frozen=True)
class SourceLoc:
    file: str
    line: int
    col: int = 0
    # Innermost "file:line" when the instruction comes from an inlined body;
    # the kernel's __always_inline helpers are inlined even at -O0.
    via: str | None = None

    def __str__(self):
        return f"{self.file}:{self.line}"


@dataclass(frozen=True)
class Member:
    name: str        # "" for an anonymous struct/union member
    offset: int      # bytes from the start of the enclosing record
    size: int        # bytes
    type: int        # metadata id of the member type, or -1


def split_top(s):
    """Split on commas that are outside brackets and quoted strings."""
    out, cur, depth, quoted = [], [], 0, False
    for ch in s:
        if quoted:
            cur.append(ch)
            if ch == '"':
                quoted = False
            continue
        if ch == '"':
            quoted = True
        elif ch in "({[<":
            depth += 1
        elif ch in ")}]>":
            depth -= 1
        if ch == "," and depth == 0:
            out.append("".join(cur).strip())
            cur = []
        else:
            cur.append(ch)
    tail = "".join(cur).strip()
    if tail:
        out.append(tail)
    return out


def ref(value):
    m = RE_REF.match(value or "")
    return int(m.group(1)) if m else None


def unquote(value):
    if not value or value[0] != '"':
        return value
    return RE_ESC.sub(lambda m: chr(int(m.group(1), 16)), value[1:-1])


class DebugInfo:
    def __init__(self, text):
        self.nodes = {}    # id -> (kind, {field: raw value})
        self.tuples = {}   # id -> [id or None]
        for m in RE_MD.finditer(text):
            nid, body = int(m.group(1)), m.group(2).strip()
            if body.startswith("!{"):
                self.tuples[nid] = [ref(x) for x in split_top(body[2:-1])]
                continue
            n = RE_NODE.match(body)
            if not n:
                continue
            fields = {}
            for part in split_top(n.group(2)):
                key, _, val = part.partition(":")
                fields[key.strip()] = val.strip()
            self.nodes[nid] = (n.group(1), fields)
        self._records = {}
        for nid, (kind, f) in self.nodes.items():
            if kind == "DICompositeType" and f.get("tag") in RECORD_TAGS \
                    and "elements" in f and "name" in f:
                self._records.setdefault(unquote(f["name"]), []).append(nid)

    def _field(self, nid, key):
        node = self.nodes.get(nid)
        return node[1].get(key) if node else None

    def kind(self, nid):
        node = self.nodes.get(nid)
        return node[0] if node else None

    # -------------------------------------------------------------- locations
    def file_of_scope(self, nid):
        seen = set()
        while nid is not None and nid not in seen:
            seen.add(nid)
            kind = self.kind(nid)
            if kind == "DIFile":
                name = unquote(self._field(nid, "filename"))
                d = unquote(self._field(nid, "directory") or '""')
                if d and not os.path.isabs(name):
                    name = os.path.join(d, name)
                return os.path.normpath(name)
            nxt = ref(self._field(nid, "file"))
            nid = nxt if nxt is not None else ref(self._field(nid, "scope"))
        return "?"

    def _plain_location(self, nid):
        return SourceLoc(self.file_of_scope(ref(self._field(nid, "scope"))),
                         int(self._field(nid, "line") or 0),
                         int(self._field(nid, "column") or 0))

    def location(self, nid):
        """Source position in the function the instruction was written in,
        i.e. the outermost frame of an inlining chain."""
        if self.kind(nid) != "DILocation":
            return None
        inner = self._plain_location(nid)
        outer, seen = nid, {nid}
        while True:
            nxt = ref(self._field(outer, "inlinedAt"))
            if nxt is None or nxt in seen or self.kind(nxt) != "DILocation":
                break
            seen.add(nxt)
            outer = nxt
        if outer == nid:
            return inner
        o = self._plain_location(outer)
        return SourceLoc(o.file, o.line, o.col, via=str(inner))

    def subprogram_loc(self, nid):
        if self.kind(nid) != "DISubprogram":
            return None
        return SourceLoc(self.file_of_scope(nid), int(self._field(nid, "line") or 0))

    def global_var_type(self, expr_id):
        """Type of the variable behind a global's `!dbg` attachment."""
        var = ref(self._field(expr_id, "var")) if self.kind(expr_id) == \
            "DIGlobalVariableExpression" else expr_id
        return ref(self._field(var, "type"))

    # ------------------------------------------------------------------ types
    def strip(self, nid):
        seen = set()
        while nid is not None and nid not in seen:
            seen.add(nid)
            if self.kind(nid) == "DIDerivedType" and self._field(nid, "tag") in QUALIFIER_TAGS:
                nid = ref(self._field(nid, "baseType"))
                continue
            return nid
        return nid

    def is_record(self, nid):
        return self.kind(nid) == "DICompositeType" and self._field(nid, "tag") in RECORD_TAGS

    def is_array(self, nid):
        return self.kind(nid) == "DICompositeType" and \
            self._field(nid, "tag") == "DW_TAG_array_type"

    def array_element(self, nid):
        return self.strip(ref(self._field(nid, "baseType")))

    def record_name(self, nid):
        name = self._field(nid, "name")
        return unquote(name) if name else None

    def record(self, name, size_bytes=None):
        cands = self._records.get(name, [])
        if size_bytes is not None:
            sized = [c for c in cands if int(self._field(c, "size") or 0) == size_bytes * 8]
            cands = sized or cands
        return cands[0] if cands else None

    def members(self, nid):
        out = []
        for mid in self.tuples.get(ref(self._field(nid, "elements")), []):
            if mid is None or self._field(mid, "tag") != "DW_TAG_member":
                continue
            st = self.strip(ref(self._field(mid, "baseType")))
            out.append(Member(
                name=unquote(self._field(mid, "name")) if self._field(mid, "name") else "",
                offset=int(self._field(mid, "offset") or 0) // 8,
                size=(int(self._field(mid, "size") or 0) + 7) // 8,
                type=st if st is not None else -1,
            ))
        return out

    def size_of(self, nid):
        nid = self.strip(nid)
        node = self.nodes.get(nid)
        if not node:
            return 0
        kind, f = node
        if kind == "DIDerivedType" and f.get("tag") == "DW_TAG_pointer_type":
            return int(f.get("size") or 64) // 8
        return int(f.get("size") or 0) // 8

    def path_at(self, type_id, offset):
        """Member path to the scalar at byte `offset` inside `type_id`.

        Unions and anonymous members make several paths cover one offset; the
        one ending in a pointer that starts exactly at `offset` wins, then
        declaration order. Returns (parts, owner): parts like ["[1]", "release"]
        and owner the innermost named record enclosing the last named member.
        """
        _, parts, owner, _ = self._path_at(self.strip(type_id), offset, 0)
        return parts, owner

    def leaf_at(self, type_id, offset):
        """Type id of the scalar `path_at` selects, or None."""
        return self._path_at(self.strip(type_id), offset, 0)[3]

    def pointee(self, type_id):
        t = self.strip(type_id)
        if self.kind(t) == "DIDerivedType" and self._field(t, "tag") == "DW_TAG_pointer_type":
            return self.strip(ref(self._field(t, "baseType")))
        return None

    def param_type(self, subprogram, n):
        """Declared type of parameter `n` (0-based) of a DISubprogram."""
        types = self._signature(subprogram)
        return self.strip(types[n + 1]) if n + 1 < len(types) and types[n + 1] is not None \
            else None

    def return_type(self, subprogram):
        types = self._signature(subprogram)
        return self.strip(types[0]) if types and types[0] is not None else None

    def _signature(self, subprogram):
        if self.kind(subprogram) != "DISubprogram":
            return []
        sig = ref(self._field(subprogram, "type"))
        return self.tuples.get(ref(self._field(sig, "types")), [])

    def variable_type(self, var):
        """Declared type of a DILocalVariable / DIGlobalVariable."""
        if self.kind(var) not in ("DILocalVariable", "DIGlobalVariable"):
            return None
        return self.strip(ref(self._field(var, "type")))

    def _path_at(self, t, off, depth):
        if t is None or depth > 32:
            return 0, [], None, None
        if self.is_record(t):
            best = None
            for m in self.members(t):
                if not m.offset <= off < m.offset + max(m.size, 1):
                    continue
                score, parts, owner, leaf = self._path_at(
                    m.type if m.type != -1 else None, off - m.offset, depth + 1)
                if m.name:
                    parts = [m.name] + parts
                if best is None or score > best[0]:
                    best = (score, parts, owner, leaf)
            if best is None:
                return 0, [], None, None
            score, parts, owner, leaf = best
            if owner is None and parts and self.record_name(t):
                owner = self.record_name(t)
            return score, parts, owner, leaf
        if self.is_array(t):
            elem = self.array_element(t)
            size = self.size_of(elem)
            if size <= 0:
                return 0, [], None, None
            score, parts, owner, leaf = self._path_at(elem, off % size, depth + 1)
            return score, [f"[{off // size}]"] + parts, owner, leaf
        if off != 0:
            return 0, [], None, None
        return (2 if self._field(t, "tag") == "DW_TAG_pointer_type" else 1), [], None, t
