"""Program model over textual LLVM IR.

llvmlite supplies the structure (functions, instructions, operands, type
layouts). What it hides -- global initializers, GEP source types and debug
metadata -- is read from text. All text comes from llvmlite's own printing of
the whole module: printing a single value renumbers metadata, so the source
file, the module print and a value print each disagree on `!N` ids.

A program is a set of translation units loaded side by side rather than one
llvm-linked module: internal symbols stay per unit, and external ones resolve
across units by name, so units never have to be renamed to link.
"""

import re
from collections import defaultdict
from dataclasses import dataclass, field
from pathlib import Path

import llvmlite.binding as llvm
from llvmlite.binding import ValueKind

from .debuginfo import DebugInfo, SourceLoc, split_top

RE_DBG = re.compile(r"!dbg !(\d+)")
RE_DEFINE_NAME = re.compile(r'@("[^"]+"|[-\w.$]+)\(')
RE_DEFINE_DBG = re.compile(r"!dbg !(\d+)\s*\{\s*$")
RE_GLOBAL_LINE = re.compile(r'^@("[^"]+"|[-\w.$]+) = ')
RE_ALIAS = re.compile(r'^@([-\w.$]+) = [^\n]*?\balias\b[^\n]*?@([-\w.$]+)', re.M)
RE_SECTION = re.compile(r'section "([^"]*)"')
RE_SYMREF = re.compile(r'@("[^"]+"|[-\w.$]+)')
RE_STORAGE = re.compile(r"\s(global|constant)\s")
RE_GEP_FLAGS = re.compile(r"^(?:(?:inbounds|nuw|nusw|inrange\([^)]*\))\s+)*")
RE_STRUCT_SUFFIX = re.compile(r"(\.\d+)+$")
RE_INT_TYPE = re.compile(r"^i(\d+)$")

LOCAL_LINKAGES = {"internal", "private"}
CONSTEXPR_WRAPPERS = {"dso_local_equivalent", "no_cfi"}
MAX_GEP_CHAIN = 16


def _sym(name):
    return name[1:-1] if name.startswith('"') else name


def _is_fn(v):
    # llvmlite's is_function/is_instruction describe how a ValueRef was
    # obtained, not what it refers to; value_kind is the real kind.
    return v.value_kind == ValueKind.function


def _opcode(line):
    _, eq, rhs = line.partition(" = ")
    return (rhs if eq else line).split()[0]


def c_record_name(llvm_struct_name):
    """`%struct.demo_ops.12` -> `demo_ops`; anonymous records have no C name."""
    name = _sym(llvm_struct_name.lstrip("%"))
    for prefix in ("struct.", "union."):
        if name.startswith(prefix):
            name = name[len(prefix):]
            break
    name = RE_STRUCT_SUFFIX.sub("", name)
    return None if name == "anon" else name


def format_path(parts):
    out = ""
    for p in parts:
        out += p if p.startswith("[") or not out else "." + p
    return out


def _const_index(tok):
    return int(tok) if tok.lstrip("-").isdigit() else 0


# ------------------------------------------------------------------ records
@dataclass
class CallSite:
    caller: str
    callee: str | None          # None for an indirect call
    loc: SourceLoc | None
    fn_args: list               # [(argument index, function name)]


@dataclass
class FnStore:
    """A function's address written to memory, e.g. INIT_WORK setting work->func."""
    in_function: str
    function: str
    loc: SourceLoc | None
    dest: str                   # "field" | "global" | "stack" | "other"
    struct: str | None = None   # C record name when dest == "field"
    field: str | None = None    # member path inside `struct`
    target: str | None = None   # global written to, if any


@dataclass
class GlobalRef:
    """A function's address inside a global initializer (ops/handler tables)."""
    global_name: str
    path: str                   # e.g. "release", "[1].release", "" for a scalar
    function: str
    owner_struct: str | None    # record that declares the slot
    module: str
    section: str | None


@dataclass
class Function:
    name: str
    module: "IRModule" = field(repr=False)
    linkage: str                # "internal" | "external"
    loc: SourceLoc | None
    calls: list = field(default_factory=list)
    fn_stores: list = field(default_factory=list)
    indirect_calls: int = 0
    # Printed instruction text in program order and the register each defines,
    # kept for passes that read instruction operands (lace3.index).
    body: list = field(default_factory=list, repr=False)
    regs: dict = field(default_factory=dict, repr=False)
    nparams: int = 0
    dbg: int | None = None      # DISubprogram id, for parameter types
    section: str | None = None  # e.g. ".init.text" for __init functions

    @property
    def key(self):
        return (self.module.name, self.name)

    def __hash__(self):
        return hash(self.key)

    def __eq__(self, other):
        return isinstance(other, Function) and self.key == other.key


# --------------------------------------------------------- constant parsing
@dataclass
class _Agg:
    kind: str                   # "struct" | "array" | "vector"
    elems: list


@dataclass
class _Leaf:
    refs: list


def _close(s, i):
    """Index just past the bracket group opening at s[i]."""
    depth, quoted = 0, False
    for j in range(i, len(s)):
        ch = s[j]
        if quoted:
            quoted = ch != '"'
            continue
        if ch == '"':
            quoted = True
        elif ch in "({[<":
            depth += 1
        elif ch in ")}]>":
            depth -= 1
            if depth == 0:
                return j + 1
    return len(s)


def _skip_ws(s, i):
    while i < len(s) and s[i].isspace():
        i += 1
    return i


def _word(s, i):
    j = i
    while j < len(s) and (s[j].isalnum() or s[j] in "_.-+$"):
        j += 1
    return s[i:j], j


def _parse_type(s, i):
    i = _skip_ws(s, i)
    if s.startswith('%"', i):
        j = s.index('"', i + 2) + 1
        return s[i:j], j
    if i < len(s) and s[i] == "%":
        w, j = _word(s, i + 1)
        return "%" + w, j
    if i < len(s) and s[i] in "[{<":
        j = _close(s, i)
        return s[i:j], j
    w, j = _word(s, i)
    if w == "ptr" and s.startswith(" addrspace(", j):
        j = _close(s, j + 10)
    return w, j


def _parse_value(s, i=0):
    i = _skip_ws(s, i)
    if i >= len(s):
        return _Leaf([])
    if s.startswith("<{", i) or s[i] in "{[<":
        j = _close(s, i)
        packed = s.startswith("<{", i)
        inner = s[i + 2:j - 2] if packed else s[i + 1:j - 1]
        kind = "struct" if packed else {"{": "struct", "[": "array", "<": "vector"}[s[i]]
        elems = []
        for part in split_top(inner):
            _, k = _parse_type(part, 0)
            elems.append(_parse_value(part, k))
        return _Agg(kind, elems)
    if s.startswith('c"', i):
        return _Leaf([])
    if s[i] == "@":
        m = RE_SYMREF.match(s, i)
        return _Leaf([_sym(m.group(1))] if m else [])
    w, j = _word(s, i)
    if w in CONSTEXPR_WRAPPERS:
        return _parse_value(s, j)
    if "(" in s[j:]:
        # Constant expression (getelementptr, ptrtoint, ...): every symbol it
        # mentions is an address it may compute.
        return _Leaf([_sym(n) for n in RE_SYMREF.findall(s[j:])])
    return _Leaf([])


def _operand_value(text):
    """`ptr addrspace(1) %x` -> `%x`."""
    _, k = _parse_type(text, 0)
    return text[k:].strip()


# ------------------------------------------------------------------- module
class IRModule:
    def __init__(self, path):
        self.path = Path(path)
        self.name = self.path.stem
        try:
            self.llmod = llvm.parse_assembly(self.path.read_text(errors="ignore"))
        except RuntimeError as e:
            raise RuntimeError(f"{self.path}: {e}") from None
        self.td = llvm.create_target_data(self.llmod.data_layout)
        text = str(self.llmod)
        self.di = DebugInfo(text)
        self.aliases = dict(RE_ALIAS.findall(text))

        bodies, dbg_of, section_of, self._global_lines = {}, {}, {}, {}
        cur, pending = None, None
        for line in text.splitlines():
            if cur is not None:
                s = line.strip()
                if pending is not None:
                    # switch/indirectbr print their case table over several
                    # lines; the instruction (and its !dbg) ends at the "]".
                    pending.append(s)
                    if s.startswith("]"):
                        cur.append(" ".join(pending))
                        pending = None
                elif line.startswith("}"):
                    cur = None
                elif s.startswith(("to label", "unwind ")) and cur:
                    # callbr (asm goto) and invoke print their successors,
                    # and the !dbg after them, on a continuation line.
                    cur[-1] += " " + s
                elif line.startswith("  ") and s and not s.startswith(("#dbg_", ";")):
                    if s.endswith("["):
                        pending = [s]
                    else:
                        cur.append(s)
                continue
            if line.startswith("define"):
                n, d = RE_DEFINE_NAME.search(line), RE_DEFINE_DBG.search(line)
                if n:
                    cur = bodies.setdefault(_sym(n.group(1)), [])
                    if d:
                        dbg_of[_sym(n.group(1))] = int(d.group(1))
                    sec = RE_SECTION.search(line)
                    if sec:
                        section_of[_sym(n.group(1))] = sec.group(1)
            elif line.startswith("@"):
                m = RE_GLOBAL_LINE.match(line)
                if m:
                    self._global_lines[_sym(m.group(1))] = line

        self.global_types = {}
        for name, line in self._global_lines.items():
            d = RE_DBG.search(line)
            if d:
                self.global_types[name] = self.di.global_var_type(int(d.group(1)))

        self.functions = {}
        self.declared = set()
        for f in self.llmod.functions:
            if f.is_declaration:
                self.declared.add(f.name)
                continue
            fn = Function(
                name=f.name, module=self,
                linkage="internal" if f.linkage.name in LOCAL_LINKAGES else "external",
                loc=self.di.subprogram_loc(dbg_of.get(f.name)),
                dbg=dbg_of.get(f.name),
                section=section_of.get(f.name),
            )
            self._scan_body(f, fn, bodies.get(f.name, []))
            self.functions[f.name] = fn

        self.global_refs = []
        for g in self.llmod.global_variables:
            if not g.is_declaration:
                self._scan_global(g)

    # ---------------------------------------------------------- functions
    def _loc(self, line):
        m = RE_DBG.search(line)
        return self.di.location(int(m.group(1))) if m else None

    def _scan_body(self, f, fn, body):
        instrs = [ins for block in f.blocks for ins in block.instructions]
        if len(instrs) != len(body):
            raise RuntimeError(f"{self.path}: {f.name}: {len(instrs)} instructions "
                               f"but {len(body)} printed lines")
        regs = {}
        for line in body:
            reg, eq, _ = line.partition(" = ")
            if eq and reg.startswith("%"):
                regs[reg] = line
        fn.body, fn.regs, fn.nparams = body, regs, len(list(f.arguments))
        for ins, line in zip(instrs, body):
            op = ins.opcode
            if op in ("call", "invoke", "callbr"):
                ops = list(ins.operands)
                callee = ops[-1]
                if _is_fn(callee):
                    if callee.name.startswith("llvm.dbg."):
                        continue
                    name = callee.name
                elif callee.value_kind == ValueKind.inline_asm:
                    continue
                else:
                    name = None
                    fn.indirect_calls += 1
                args = [(k, a.name) for k, a in enumerate(ops[:-1]) if _is_fn(a)]
                fn.calls.append(CallSite(fn.name, name, self._loc(line), args))
            elif op == "store":
                if _is_fn(list(ins.operands)[0]):
                    fn.fn_stores.append(self._describe_store(
                        fn.name, list(ins.operands)[0].name, line, regs))

    def _describe_store(self, in_fn, target_fn, line, regs):
        st = FnStore(in_fn, target_fn, self._loc(line), "other")
        body = line.split(None, 1)[1]
        while body.startswith(("volatile ", "atomic ")):
            body = body.split(None, 1)[1]
        parts = split_top(body)
        dst = _operand_value(parts[1]) if len(parts) > 1 else ""
        named = None
        if dst.startswith("@"):
            st.dest, st.target = "global", _sym(dst[1:])
        elif dst.startswith("%"):
            defline = regs.get(dst, "")
            op = _opcode(defline) if defline else None
            if op == "alloca":
                st.dest = "stack"
            elif op == "getelementptr":
                named = self._gep_field(defline.split("getelementptr", 1)[1], regs)
        elif "getelementptr" in dst:
            start = dst.index("(")
            named = self._gep_field(dst[start + 1:_close(dst, start) - 1], regs)
        if named:
            st.dest = "field"
            st.struct, st.field, tgt = named
            st.target = tgt or st.target
        return st

    def _struct(self, tstr):
        try:
            return self.llmod.get_struct_type(_sym(tstr[1:]))
        except NameError:
            return None

    def _size(self, tstr):
        m = RE_INT_TYPE.match(tstr)
        if m:
            return max(int(m.group(1)) // 8, 1)
        if tstr == "ptr":
            return 8
        if tstr.startswith("%"):
            ty = self._struct(tstr)
            return self.td.get_abi_size(ty) if ty is not None else None
        if tstr.startswith("[") and " x " in tstr:
            n, _, elem = tstr[1:-1].partition(" x ")
            es = self._size(elem.strip())
            return int(n) * es if es is not None else None
        return None

    def _gep_offset(self, src, indices):
        """Byte offset a GEP adds, whether any index was not a constant, and
        the C name of the source record (None unless a named struct)."""
        size = self._size(src)
        if size is None or not indices:
            return None, False, None
        variable = not indices[0].lstrip("-").isdigit()
        off = _const_index(indices[0]) * size
        tstr, ty = src, None
        for idx in indices[1:]:
            variable |= not idx.lstrip("-").isdigit()
            k = _const_index(idx)
            if ty is None and tstr.startswith("["):
                tstr = tstr[1:-1].partition(" x ")[2].strip()
                off += k * self._size(tstr)
                continue
            if ty is None and tstr.startswith("%"):
                ty = self._struct(tstr)
            if ty is None:
                return None, variable, None
            if ty.is_struct:
                off += self.td.get_element_offset(ty, k)
                ty = list(ty.elements)[k]
            elif ty.is_array:
                ty = next(iter(ty.elements))
                off += k * self.td.get_abi_size(ty)
            else:
                break
        return off, variable, (c_record_name(src) if src.startswith("%") else None)

    def _gep_field(self, gep_body, regs):
        """(record, member path, global) of the address a GEP chain computes.

        The chain is walked back to the innermost named record and the summed
        byte offset is named from DWARF, so anonymous unions and opaque-pointer
        reinterpretation between GEPs do not break the naming."""
        total, variable, body = 0, False, gep_body
        for _ in range(MAX_GEP_CHAIN):
            parts = [p for p in split_top(body) if not p.startswith("!")]
            if len(parts) < 2:
                return None
            src = RE_GEP_FLAGS.sub("", parts[0].strip())
            base = _operand_value(parts[1])
            off, var, cname = self._gep_offset(src, [p.split()[-1] for p in parts[2:]])
            if off is None:
                return None
            total, variable = total + off, variable or var
            target = _sym(base[1:]) if base.startswith("@") else None
            named = None
            if cname:
                record = self.di.record(cname, self._size(src))
                if record is None:
                    return None
                named = (cname, self.di.path_at(record, total)[0])
            elif target and target in self.global_types:
                path, owner = self.di.path_at(self.global_types[target], total)
                named = (owner, path)
            if named:
                path = named[1]
                if variable:
                    path = [("[*]" if p.startswith("[") else p) for p in path]
                return named[0], format_path(path), target
            defline = regs.get(base, "")
            if not defline or _opcode(defline) != "getelementptr":
                return None
            body = defline.split("getelementptr", 1)[1]
        return None

    # ------------------------------------------------------------ globals
    def _scan_global(self, g):
        line = self._global_lines.get(g.name)
        if not line:
            return
        sec = RE_SECTION.search(line)
        section = sec.group(1) if sec else None
        m = RE_STORAGE.search(line, line.find(" = "))
        if not m:
            return
        rest = line[m.end():]
        _, k = _parse_type(rest, 0)
        tail = rest[k:]
        node = _parse_value(split_top(tail)[0] if tail.strip() else "", 0)
        leaves = []
        self._leaf_offsets(node, g.global_value_type, 0, leaves)
        gtype = self.global_types.get(g.name)
        for off, refs in leaves:
            for r in refs:
                if r not in self.functions and r not in self.declared:
                    continue
                if gtype is not None:
                    parts, owner = self.di.path_at(gtype, off)
                    path = format_path(parts)
                else:
                    path, owner = f"+{off}", None
                self.global_refs.append(GlobalRef(g.name, path, r, owner, self.name, section))

    def _leaf_offsets(self, node, ty, off, out):
        if isinstance(node, _Leaf):
            if node.refs:
                out.append((off, node.refs))
            return
        if node.kind == "struct" and ty is not None and ty.is_struct:
            etys = list(ty.elements)
            for i, en in enumerate(node.elems[:len(etys)]):
                self._leaf_offsets(en, etys[i], off + self.td.get_element_offset(ty, i), out)
        elif node.kind == "array" and ty is not None and ty.is_array:
            ety = next(iter(ty.elements))
            size = self.td.get_abi_size(ety)
            for i, en in enumerate(node.elems):
                self._leaf_offsets(en, ety, off + i * size, out)
        else:
            for en in node.elems:
                self._leaf_offsets(en, None, off, out)


# ------------------------------------------------------------------ program
class Program:
    def __init__(self, modules):
        self.modules = list(modules)
        self._by_name = defaultdict(list)
        self._external = {}
        self.aliases = {}
        for m in self.modules:
            self.aliases.update(m.aliases)
            for f in m.functions.values():
                self._by_name[f.name].append(f)
                if f.linkage == "external":
                    self._external.setdefault(f.name, f)

    @property
    def functions(self):
        for m in self.modules:
            yield from m.functions.values()

    def functions_named(self, name):
        return list(self._by_name.get(name, []))

    def function(self, name):
        fs = self._by_name.get(name, [])
        if len(fs) != 1:
            raise LookupError(f"{name}: {len(fs)} definitions")
        return fs[0]

    def is_defined(self, name):
        return name in self._by_name

    def resolve(self, module, name):
        """Definition a reference to `name` from `module` binds to, or None."""
        return module.functions.get(name) or self._external.get(name)

    def callees(self, fn):
        out = set()
        for c in fn.calls:
            if c.callee:
                tgt = self.resolve(fn.module, c.callee)
                if tgt is not None:
                    out.add(tgt)
        return out

    def global_refs(self):
        return [r for m in self.modules for r in m.global_refs]


def load_program(paths):
    return Program(IRModule(p) for p in paths)
