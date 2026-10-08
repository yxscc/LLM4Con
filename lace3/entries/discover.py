"""Thread-entry discovery from name-free escape facts (design step S2).

A function is an entry when its address escapes the code that calls it
directly, or when nothing in the program calls it at all. Six facts, none of
which needs a per-subsystem name table:

  A  the address sits in a global initializer (ops / handler tables)
  E  an EXPORT_SYMBOL-style marker global names it
  B  the address is passed to a callee the program does not define
     (call_rcu, timer_init_key, request_irq, ...)
  S  the address is stored into memory, as INIT_WORK does for work->func
  C  the name follows the kernel-wide syscall wrapper convention
  D  external linkage and no direct caller anywhere in the program

Every fact keeps where it came from, so later steps can reason about which
entries may run concurrently. Nothing is pruned here; `reachable_from` lists
the other entries whose call graph already contains an entry, for consumers
that want to trade independent roots for cost.
"""

from collections import deque
from dataclasses import dataclass, field

BOOKKEEPING_PREFIXES = ("llvm.", "__kstrtab", "____versions", "__func__", ".str")
DEAD_SECTIONS = (".discard", ".debug", ".note", ".modinfo", "llvm.metadata")
EXPORT_PREFIXES = ("__ksymtab", "__addressable_", "__UNIQUE_ID___addressable_",
                   "__UNIQUE_ID_addressable_", "__export_symbol_")
SYSCALL_PREFIXES = ("__x64_sys_", "__ia32_sys_", "__arm64_sys_", "__se_sys_",
                    "__do_sys_", "__sys_", "SyS_", "sys_")
FACT_KINDS = "AEBSCD"


def syscall_core(name):
    for p in SYSCALL_PREFIXES:
        if name.startswith(p) and len(name) > len(p):
            return name[len(p):]
    return None


@dataclass
class Fact:
    kind: str
    detail: dict = field(default_factory=dict)


@dataclass
class Entry:
    function: object            # lace3.ir.module.Function
    facts: list = field(default_factory=list)
    reachable_from: list = field(default_factory=list)

    @property
    def kinds(self):
        return "".join(k for k in FACT_KINDS if any(f.kind == k for f in self.facts))


@dataclass
class EntrySet:
    entries: list

    def by_kinds(self, kinds):
        return [e for e in self.entries if any(f.kind in kinds for f in e.facts)]


def _site(loc):
    if loc is None:
        return {"file": None, "line": None}
    out = {"file": loc.file, "line": loc.line}
    if loc.via:
        out["via"] = loc.via
    return out


def discover_entries(prog, kinds=FACT_KINDS):
    found = {}

    def add(fn, kind, **detail):
        if fn is None or kind not in kinds:
            return
        found.setdefault(fn, Entry(fn)).facts.append(Fact(kind, detail))

    def target(module, name):
        return prog.resolve(module, prog.aliases.get(name, name))

    for m in prog.modules:
        for r in m.global_refs:
            g = r.global_name
            if g.startswith(BOOKKEEPING_PREFIXES):
                continue
            if g.startswith(EXPORT_PREFIXES):
                add(target(m, r.function), "E", symbol=g, module=m.name)
                continue
            if r.section and r.section.startswith(DEAD_SECTIONS):
                continue
            add(target(m, r.function), "A", **{
                "global": g, "path": r.path, "struct": r.owner_struct,
                "section": r.section, "module": m.name})

    called = set()
    for fn in prog.functions:
        for c in fn.calls:
            if c.callee:
                called.add(c.callee)
            if not c.callee or c.callee.startswith("llvm.") or prog.is_defined(c.callee):
                continue
            for arg, name in c.fn_args:
                add(target(fn.module, name), "B", callee=c.callee, arg=arg,
                    caller=fn.name, **_site(c.loc))
        for st in fn.fn_stores:
            add(target(fn.module, st.function), "S", in_function=fn.name, dest=st.dest,
                struct=st.struct, field=st.field, target=st.target, **_site(st.loc))

    for fn in prog.functions:
        if syscall_core(fn.name):
            add(fn, "C")
        if fn.linkage == "external" and fn.name not in called:
            add(fn, "D")

    roots = set(found)
    for r in roots:
        for fn in _reachable(prog, r) & roots:
            if fn != r:
                found[fn].reachable_from.append(r.name)
    entries = sorted(found.values(), key=lambda e: (e.function.module.name, e.function.name))
    for e in entries:
        e.reachable_from.sort()
    return EntrySet(entries)


def _reachable(prog, root):
    seen = {root}
    todo = deque([root])
    while todo:
        for c in prog.callees(todo.popleft()):
            if c not in seen:
                seen.add(c)
                todo.append(c)
    return seen
