"""Static concurrency index: operations on shared state, per function.

An operation is one IR access to shared state, classified as

  read | write | publish | rmw | free | alloc | ref_inc | ref_dec
  | lock | unlock | wait | wake | assert_held | call | icall | ext

Each carries the field it touches (named from DWARF by byte offset), where its
base pointer and its value came from, and its source line. `call` / `icall` /
`ext` are calls to a defined function, through a pointer, and to an undefined
non-primitive function; they are the links traces are built from.

Two kinds of fact are kept apart. `held` is a *support* fact: a linear scan in
program order, blind to branches and to locks taken by callers. It may widen
what a task includes and is shown to the model, but nothing downstream treats
it as proof that an access is protected. The same holds for two locks sharing
a name -- same field of the same struct type is not the same lock instance.

Value flow is intra-procedural. At -O0 `p = o->buf` goes through an alloca,
and slot contents are tracked flow-insensitively: every load of the slot
inherits the origins of every store to it. At -O2 the same flow is plain SSA
(phi / select / casts). Arithmetic carries origins too, so `o->count = c + 1`
records that the stored value depends on the earlier load of `o->count`.
Origins are tags:

  ("param", n)   the function's n-th argument
  ("global", g)  the address of global g
  ("load", id)   the value read by op `id` of this function
  ("fresh", id)  memory returned by allocation op `id`
  ("ret", f, id) the result of calling f at op `id`
  ("null",)      a null pointer constant

Naming: a GEP chain is walked back to the nearest named record and the summed
byte offset is named from DWARF. Where no GEP exists -- offset 0 at -O2, or a
pointer loaded from a field -- the pointee type comes from DWARF (parameter
types, local variable types, member types), so `b->len` through a pointer
loaded from `o->buf` is still `buf.len`.
"""

import re
from dataclasses import dataclass, field

from lace3.ir.debuginfo import split_top
from lace3.ir.module import RE_GEP_FLAGS, _close, _const_index, _operand_value, _sym, format_path

RE_CALLEE = re.compile(r'@("[^"]+"|[-\w.$]+)\(')
RE_REG = re.compile(r'%[-\w.$]+')
RE_DECLARE = re.compile(r'@llvm\.dbg\.declare\(metadata ptr (%[-\w.$]+), metadata !(\d+)')
RE_ASM = re.compile(r'[^"@%]*?\basm\b')

ACQUIRE_PREFIXES = ("_raw_spin_lock", "spin_lock", "raw_spin_lock", "_raw_read_lock", "read_lock",
                    "_raw_write_lock", "write_lock", "mutex_lock", "down_read", "down_write",
                    "lock_sock", "rtnl_lock", "__rcu_read_lock", "rcu_read_lock",
                    "_raw_spin_trylock", "mutex_trylock", "down_trylock", "__srcu_read_lock",
                    "srcu_read_lock", "local_bh_disable", "__local_bh_disable_ip")
RELEASE_PREFIXES = ("_raw_spin_unlock", "spin_unlock", "raw_spin_unlock", "_raw_read_unlock",
                    "read_unlock", "_raw_write_unlock", "write_unlock", "mutex_unlock", "up_read",
                    "up_write", "release_sock", "rtnl_unlock", "__rcu_read_unlock", "rcu_read_unlock",
                    "__srcu_read_unlock", "srcu_read_unlock", "local_bh_enable", "__local_bh_enable_ip")
NO_OPERAND_LOCKS = {"rtnl_lock": "rtnl", "rtnl_unlock": "rtnl", "__rcu_read_lock": "rcu",
                    "__rcu_read_unlock": "rcu", "rcu_read_lock": "rcu", "rcu_read_unlock": "rcu",
                    "local_bh_disable": "bh", "local_bh_enable": "bh",
                    "__local_bh_disable_ip": "bh", "__local_bh_enable_ip": "bh"}
SHARED_LOCK_PREFIXES = ("_raw_read_", "read_", "down_read", "up_read", "__rcu_read", "rcu_read",
                        "__srcu_read", "srcu_read", "local_bh", "__local_bh")
NOT_A_LOCK = ("_init", "_is_locked", "_is_contended", "_held")
FREE_FUNCS = {"kfree": 0, "kvfree": 0, "vfree": 0, "kfree_sensitive": 0, "kfree_skb": 0,
              "kfree_skb_reason": 0, "consume_skb": 0, "__kfree_skb": 0, "sk_free": 0,
              "kmem_cache_free": 1, "kvfree_call_rcu": 1, "kfree_rcu_mightsleep": 0}
DEFERRED_FREE = {"kvfree_call_rcu", "call_rcu"}
FRESH_PREFIXES = ("kmalloc", "kzalloc", "__kmalloc", "kcalloc", "kvmalloc", "kvzalloc",
                  "kmem_cache_alloc", "kmem_cache_zalloc", "alloc_skb", "__alloc_skb", "vmalloc",
                  "vzalloc", "kmalloc_trace", "kvcalloc", "kmemdup", "kstrdup")
REF_INC_PREFIXES = ("refcount_inc", "kref_get", "sock_hold", "dev_hold", "netdev_hold",
                    "get_device", "kobject_get", "skb_get", "refcount_add")
REF_DEC_PREFIXES = ("refcount_dec", "kref_put", "sock_put", "dev_put", "netdev_put", "put_device",
                    "kobject_put", "refcount_sub")
WAIT_PREFIXES = ("wait_for_completion", "flush_work", "flush_delayed_work", "cancel_work_sync",
                 "cancel_delayed_work_sync", "del_timer_sync", "timer_delete_sync",
                 "hrtimer_cancel", "synchronize_rcu", "synchronize_net", "synchronize_srcu",
                 "rcu_barrier", "kthread_stop", "tasklet_kill", "flush_workqueue",
                 "destroy_workqueue", "prepare_to_wait", "wait_event", "__wait_event",
                 "wait_woken", "schedule_timeout")
WAKE_PREFIXES = ("complete", "wake_up", "__wake_up")
ASSERT_PREFIXES = ("lock_is_held", "lockdep_is_held")
SKIP_INTRINSICS = ("llvm.dbg.", "llvm.lifetime.", "llvm.assume", "llvm.experimental.",
                   "llvm.prefetch", "llvm.stacksave", "llvm.stackrestore", "llvm.va_",
                   "llvm.objectsize", "llvm.is.constant", "llvm.expect", "llvm.trap",
                   "llvm.ubsantrap", "llvm.bswap", "llvm.ctlz", "llvm.cttz", "llvm.ctpop",
                   "llvm.fshl", "llvm.fshr", "llvm.umin", "llvm.umax", "llvm.smin", "llvm.smax",
                   "llvm.abs", "llvm.uadd", "llvm.sadd", "llvm.usub", "llvm.ssub", "llvm.umul",
                   "llvm.smul", "llvm.read_register", "llvm.frameaddress", "llvm.returnaddress")
MEM_INTRINSICS = (("llvm.memset", 0, None), ("llvm.memcpy", 0, 1), ("llvm.memmove", 0, 1))
CAST_OPS = {"bitcast", "addrspacecast", "inttoptr", "ptrtoint", "zext", "sext", "trunc", "freeze"}
MAX_GEP_CHAIN = 16

WRITE_KINDS = frozenset({"write", "publish", "rmw", "free", "ref_dec"})


def _starts(name, prefixes):
    return any(name == p or name.startswith(p) for p in prefixes)


def lock_role(name):
    if any(s in name for s in NOT_A_LOCK):
        return None
    if _starts(name, RELEASE_PREFIXES):
        return "unlock"
    if _starts(name, ACQUIRE_PREFIXES):
        return "lock"
    return None


def lock_mode(name):
    if name in NO_OPERAND_LOCKS and NO_OPERAND_LOCKS[name] in ("rcu", "bh"):
        return NO_OPERAND_LOCKS[name]
    return "shared" if _starts(name, SHARED_LOCK_PREFIXES) else "exclusive"


def free_arg(name):
    if name in FREE_FUNCS:
        return FREE_FUNCS[name]
    return None


@dataclass
class Op:
    id: int
    function: str
    module: str
    kind: str                   # see module docstring
    state: str | None           # "struct.field" / "@global.path"; None when unnamed
    loc: object                 # SourceLoc or None
    held: tuple = ()            # lock keys held here (support fact, local scan)
    base: frozenset = frozenset()    # origins of the accessed object's pointer
    origin: frozenset = frozenset()  # value written (store) / value freed or used (free, ref)
    callee: str | None = None
    args: tuple = ()            # call-like ops: origins of each argument
    volatile: bool = False
    atomic: bool = False
    deferred: bool = False      # free deferred past an RCU grace period
    mode: str | None = None     # locks: exclusive | shared | rcu | bh
    reg: str | None = None      # register the op defines (loads, calls)
    line: int = -1              # index into Function.body

    @property
    def site(self):
        return str(self.loc) if self.loc else f"{self.function}:?"

    @property
    def is_write(self):
        return self.kind in WRITE_KINDS


@dataclass
class FunctionIndex:
    function: object
    ops: list = field(default_factory=list)
    lock_edges: list = field(default_factory=list)    # (held, acquired) pairs; deadlock unused
    unknown: list = field(default_factory=list)       # indirect-call ops

    def op(self, op_id):
        return self.ops[op_id]


def _rhs(line):
    _, eq, rhs = line.partition(" = ")
    return rhs if eq else line


def _opcode(rhs):
    toks = rhs.split()
    while toks and toks[0] in ("tail", "musttail", "notail"):
        toks = toks[1:]
    return toks[0] if toks else ""


def _strip_words(body, words):
    body = body.strip()
    changed = True
    while changed:
        changed = False
        for w in words:
            if body.startswith(w + " "):
                body = body[len(w) + 1:].lstrip()
                changed = True
    return body


def _arg_value(part):
    """Value of one call argument: `ptr noundef nonnull %5` -> `%5`."""
    part = part.strip()
    if "getelementptr" in part:
        return part[part.index("getelementptr"):]
    toks = part.split()
    for t in reversed(toks):
        if t.startswith(("%", "@")):
            return t
    return toks[-1] if toks else ""


def _paren_args(line, start):
    depth = 0
    for j in range(start, len(line)):
        if line[j] == "(":
            depth += 1
        elif line[j] == ")":
            depth -= 1
            if depth == 0:
                return [_arg_value(p) for p in split_top(line[start + 1:j])]
    return []


def _call_parts(rhs):
    """(kind, callee, args) of a call line: kind is direct | indirect | asm."""
    i = rhs.find("call ")
    if i < 0:
        return None, None, []
    rest = rhs[i + 5:]
    if RE_ASM.match(rest):
        # asm [sideeffect] [alignstack] [inteldialect] "text", "constraints"(args);
        # a quote inside either string is printed as \22, so plain " delimits.
        q = [j for j, ch in enumerate(rest) if ch == '"'][:4]
        if len(q) < 4:
            return "asm", "", []
        p = rest.find("(", q[3])
        return "asm", rest[q[0] + 1:q[1]], (_paren_args(rest, p) if p >= 0 else [])
    m = RE_CALLEE.search(rest)
    # The callee is the first `@name(` unless an indirect `%reg(` comes first.
    ind = re.search(r'(%[-\w.$]+)\(', rest)
    if m and (not ind or m.start() < ind.start()):
        return "direct", _sym(m.group(1)), _paren_args(rest, m.end() - 1)
    if ind:
        return "indirect", ind.group(1), _paren_args(rest, ind.end() - 1)
    return None, None, []


class _Indexer:
    def __init__(self, fn, prog):
        self.fn, self.prog, self.mod = fn, prog, fn.module
        self.di, self.regs = fn.module.di, fn.regs
        self.slots = {r for r, l in self.regs.items() if _opcode(_rhs(l)) == "alloca"}
        self.slot_var = {}
        for line in fn.body:
            if "llvm.dbg.declare" in line:
                m = RE_DECLARE.search(line)
                if m:
                    self.slot_var[m.group(1)] = int(m.group(2))
        self._ptype, self._name = {}, {}

    # ------------------------------------------------------------ naming
    def is_stack(self, v):
        root, _, _ = self.gep_root(v)
        return root in self.slots

    def gep_body(self, v):
        """Operand text of the GEP that computes v, or None."""
        if v.startswith("getelementptr"):
            start = v.index("(")
            return v[start + 1:_close(v, start) - 1]
        if v in self.regs:
            rhs = _rhs(self.regs[v])
            if _opcode(rhs) == "getelementptr":
                return rhs.split("getelementptr", 1)[1]
        return None

    def gep_root(self, v):
        """(root base, byte offset from it, any variable index) through casts and GEPs."""
        total, variable = 0, False
        for _ in range(MAX_GEP_CHAIN):
            if v in self.regs and _opcode(_rhs(self.regs[v])) in ("bitcast", "addrspacecast"):
                v = _operand_value(_rhs(self.regs[v]).split(None, 1)[1].split(" to ")[0])
                continue
            body = self.gep_body(v)
            if body is None:
                return v, total, variable
            parts = [p for p in split_top(body) if not p.startswith("!")]
            if len(parts) < 2:
                return v, total, variable
            src = RE_GEP_FLAGS.sub("", parts[0].strip())
            off, var, _ = self.mod._gep_offset(src, [p.split()[-1] for p in parts[2:]])
            if off is None:
                return v, total, True
            total, variable = total + off, variable or var
            v = _operand_value(parts[1])
        return v, total, variable

    def name(self, v):
        """State name of the memory at address v, or None."""
        if v in self._name:
            return self._name[v]
        self._name[v] = None
        out = self._name_uncached(v)
        self._name[v] = out
        return out

    def _name_uncached(self, v):
        total, variable, cur = 0, False, v
        for _ in range(MAX_GEP_CHAIN):
            if cur in self.regs and _opcode(_rhs(self.regs[cur])) in ("bitcast", "addrspacecast"):
                cur = _operand_value(_rhs(self.regs[cur]).split(None, 1)[1].split(" to ")[0])
                continue
            body = self.gep_body(cur)
            if body is None:
                break
            parts = [p for p in split_top(body) if not p.startswith("!")]
            if len(parts) < 2:
                return None
            src = RE_GEP_FLAGS.sub("", parts[0].strip())
            off, var, cname = self.mod._gep_offset(src, [p.split()[-1] for p in parts[2:]])
            if off is None:
                return None
            total, variable = total + off, variable or var
            if cname:
                rec = self.di.record(cname, self.mod._size(src))
                if rec is not None:
                    return self._format(cname, self.di.path_at(rec, total)[0], variable)
            cur = _operand_value(parts[1])
        if cur.startswith("@"):
            g = _sym(cur[1:])
            gt = self.mod.global_types.get(g)
            if gt is not None and self.di.is_record(gt):
                return self._format(self.di.record_name(gt) or f"@{g}",
                                    self.di.path_at(gt, total)[0], variable)
            if gt is not None and self.di.is_array(gt):
                return self._format(f"@{g}", self.di.path_at(gt, total)[0], True)
            return f"@{g}"
        if cur in self.slots:
            return None
        t = self.ptype(cur)
        if t is not None and self.di.is_record(t):
            name = self.di.record_name(t)
            if name:
                return self._format(name, self.di.path_at(t, total)[0], variable)
        return None

    @staticmethod
    def _format(record, parts, variable):
        if variable:
            parts = [("[*]" if p.startswith("[") else p) for p in parts]
        path = format_path(parts)
        return f"{record}.{path}" if path else record

    def objtype(self, addr):
        """DWARF type of the object stored at address `addr`."""
        root, off, _ = self.gep_root(addr)
        if root in self.slots:
            var = self.slot_var.get(root)
            t = self.di.variable_type(var) if var is not None else None
            return self.di.leaf_at(t, off) if t is not None and off else t
        if root.startswith("@"):
            gt = self.mod.global_types.get(_sym(root[1:]))
            return self.di.leaf_at(gt, off) if gt is not None else None
        base = self.ptype(root)
        if base is None:
            return None
        return self.di.leaf_at(base, off) if (off or self.di.is_record(base)) else base

    def ptype(self, v, depth=0):
        """DWARF type id of what pointer value v points to, or None."""
        if v in self._ptype:
            return self._ptype[v]
        self._ptype[v] = None
        out = None
        if depth > 16:
            return None
        if v.startswith("@"):
            out = self.mod.global_types.get(_sym(v[1:]))
        elif v.startswith("%") and v not in self.regs:
            n = v[1:]
            if n.isdigit() and int(n) < self.fn.nparams:
                out = self.di.pointee(self.di.param_type(self.fn.dbg, int(n)))
        elif v in self.slots:
            var = self.slot_var.get(v)
            out = self.di.variable_type(var) if var is not None else None
        elif v in self.regs:
            rhs = _rhs(self.regs[v])
            op = _opcode(rhs)
            if op == "load":
                out = self.di.pointee(self.objtype(self._load_addr(rhs)))
            elif op == "getelementptr":
                out = self.objtype(v)
            elif op in CAST_OPS:
                out = self.ptype(_operand_value(rhs.split(None, 1)[1].split(" to ")[0]), depth + 1)
            elif op in ("phi", "select"):
                for r in RE_REG.findall(rhs.split(None, 1)[1]):
                    out = self.ptype(r, depth + 1)
                    if out is not None:
                        break
            elif op == "call":
                kind, callee, _ = _call_parts(rhs)
                if kind == "direct":
                    tgt = self.prog.resolve(self.mod, callee)
                    if tgt is not None and tgt.dbg is not None:
                        out = self.di.pointee(tgt.module.di.return_type(tgt.dbg)) \
                            if tgt.module is self.mod else None
        self._ptype[v] = out
        return out

    @staticmethod
    def _load_addr(rhs):
        body = _strip_words(rhs.split(None, 1)[1], ("atomic", "volatile"))
        parts = split_top(body)
        return _operand_value(parts[1]) if len(parts) > 1 else ""

    # ------------------------------------------------------------- scan
    def run(self):
        fi = FunctionIndex(self.fn)
        ops = fi.ops
        load_op, fresh_op, ret_op = {}, {}, {}
        pending = []                                     # (op, raw base, raw value, raw args)

        def add(kind, state, line_no, line, **kw):
            raw = kw.pop("raw", (None, None, ()))
            op = Op(len(ops), self.fn.name, self.mod.name, kind, state,
                    self.mod._loc(line), line=line_no, **kw)
            ops.append(op)
            pending.append((op, *raw))
            return op

        for i, line in enumerate(self.fn.body):
            reg, eq, _ = line.partition(" = ")
            reg = reg.strip() if eq and reg.strip().startswith("%") else None
            rhs = _rhs(line)
            op = _opcode(rhs)
            if op == "load":
                body = rhs.split(None, 1)[1]
                vol, atom = " volatile " in f" {body[:24]} ", body.startswith("atomic")
                addr = self._load_addr(rhs)
                if not addr or self.is_stack(addr):
                    continue
                root, _, _ = self.gep_root(addr)
                o = add("read", self.name(addr), i, line, volatile=vol, atomic=atom, reg=reg,
                        raw=(root, None, ()))
                if reg:
                    load_op[reg] = o.id
            elif op == "store":
                body = rhs.split(None, 1)[1]
                vol, atom = body.startswith("volatile") or " volatile " in body[:24], \
                    body.startswith("atomic")
                parts = split_top(_strip_words(body, ("atomic", "volatile")))
                if len(parts) < 2:
                    continue
                val, addr = _operand_value(parts[0]), _operand_value(parts[1])
                if self.is_stack(addr):
                    continue
                root, _, _ = self.gep_root(addr)
                add("write", self.name(addr), i, line, volatile=vol, atomic=atom,
                    raw=(root, val, ()))
            elif op in ("atomicrmw", "cmpxchg"):
                body = _strip_words(rhs.split(None, 1)[1], ("volatile", "weak"))
                parts = split_top(body)
                addr = parts[0].split()[-1] if parts else ""
                if not addr or self.is_stack(addr):
                    continue
                root, _, _ = self.gep_root(addr)
                o = add("rmw", self.name(addr), i, line, atomic=True, reg=reg,
                        raw=(root, None, ()))
                if reg:
                    load_op[reg] = o.id
            elif op in ("call", "invoke", "callbr"):
                self._call(i, line, rhs, reg, add, fresh_op, ret_op, fi)

        flow = _Flow(self, load_op, fresh_op, ret_op)
        for op, raw_base, raw_val, raw_args in pending:
            if raw_base is not None:
                op.base = frozenset(flow.origin(raw_base))
            if raw_val is not None:
                op.origin = frozenset(flow.origin(raw_val))
            if raw_args:
                op.args = tuple(frozenset(flow.origin(a)) for a in raw_args)
            if op.kind == "write" and any(t[0] == "fresh" for t in op.origin):
                op.kind = "publish"
            if op.kind == "free" and op.args:
                k = free_arg(op.callee)
                if k is not None and k < len(op.args):
                    op.origin = op.args[k]
            if op.kind in ("ref_inc", "ref_dec") and op.args:
                op.origin = op.args[0]

        held = []
        for op in ops:
            if op.kind == "lock":
                key = self.lock_key(op)
                for h in held:
                    fi.lock_edges.append((h, key))
                held.append(key)
            elif op.kind == "unlock":
                key = self.lock_key(op)
                if key in held:
                    held.reverse()
                    held.remove(key)
                    held.reverse()
            op.held = tuple(held)
        return fi

    def lock_key(self, op):
        if op.callee in NO_OPERAND_LOCKS:
            return NO_OPERAND_LOCKS[op.callee]
        return op.state or "*" + describe(op.base)

    def _call(self, i, line, rhs, reg, add, fresh_op, ret_op, fi):
        kind, callee, args = _call_parts(rhs)
        if kind is None:
            return
        if kind == "asm":
            ptrs = [a for a in args if a.startswith(("%", "@", "getelementptr"))
                    and not self.is_stack(a)]
            for a in ptrs:
                root, _, _ = self.gep_root(a)
                add("rmw", self.name(a), i, line, atomic=True, callee="asm:" + callee[:40],
                    raw=(root, None, ()))
            return
        if kind == "indirect":
            o = add("icall", None, i, line, callee=None, reg=reg, raw=(None, callee, tuple(args)))
            fi.unknown.append(o)
            if reg:
                ret_op[reg] = o.id
            return
        name = self.prog.aliases.get(callee, callee)
        if name.startswith(SKIP_INTRINSICS):
            return
        for prefix, dst, src in MEM_INTRINSICS:
            if name.startswith(prefix):
                for k, kind2 in ((dst, "write"), (src, "read")):
                    if k is None or k >= len(args) or self.is_stack(args[k]):
                        continue
                    root, _, _ = self.gep_root(args[k])
                    add(kind2, self.name(args[k]), i, line, callee=name, raw=(root, None, ()))
                return
        if name.startswith("llvm."):
            return
        role = lock_role(name)
        if role:
            a0 = args[0] if args and name not in NO_OPERAND_LOCKS else None
            root = self.gep_root(a0)[0] if a0 else None
            add(role, self.name(a0) if a0 else None, i, line, callee=name, mode=lock_mode(name),
                raw=(root, None, tuple(args)))
            return
        if free_arg(name) is not None:
            add("free", None, i, line, callee=name, deferred=name in DEFERRED_FREE,
                raw=(None, None, tuple(args)))
            return
        if _starts(name, FRESH_PREFIXES):
            o = add("alloc", None, i, line, callee=name, reg=reg, raw=(None, None, tuple(args)))
            if reg:
                fresh_op[reg] = o.id
            return
        for prefixes, k in ((REF_INC_PREFIXES, "ref_inc"), (REF_DEC_PREFIXES, "ref_dec"),
                            (WAIT_PREFIXES, "wait"), (WAKE_PREFIXES, "wake"),
                            (ASSERT_PREFIXES, "assert_held")):
            if _starts(name, prefixes):
                a0 = args[0] if args else None
                root = self.gep_root(a0)[0] if a0 and a0.startswith(("%", "@", "getel")) else None
                add(k, self.name(a0) if root else None, i, line, callee=name,
                    raw=(root, None, tuple(args)))
                return
        target = self.prog.resolve(self.mod, name)
        o = add("call" if target is not None else "ext", None, i, line, callee=name, reg=reg,
                raw=(None, None, tuple(args)))
        if reg:
            ret_op[reg] = o.id


class _Flow:
    """Origins of SSA values and stack slots inside one function."""

    def __init__(self, ix, load_op, fresh_op, ret_op):
        self.ix, self.regs = ix, ix.regs
        self.load_op, self.fresh_op, self.ret_op = load_op, fresh_op, ret_op
        self.slot_origin = {s: set() for s in ix.slots}
        stores = []
        for line in ix.fn.body:
            rhs = _rhs(line)
            if _opcode(rhs) == "store":
                parts = split_top(_strip_words(rhs.split(None, 1)[1], ("atomic", "volatile")))
                if len(parts) >= 2:
                    stores.append((_operand_value(parts[0]), _operand_value(parts[1])))
        for _ in range(4):
            changed = False
            for val, dst in stores:
                root = ix.gep_root(dst)[0]
                if root in self.slot_origin:
                    self._memo = {}
                    new = self.origin(val)
                    if not new <= self.slot_origin[root]:
                        self.slot_origin[root] |= new
                        changed = True
            if not changed:
                break
        self._memo = {}

    def origin(self, v, depth=0):
        if not v or depth > 24:
            return set()
        if v in self._memo:
            return self._memo[v]
        self._memo[v] = set()
        out = set()
        if v == "null":
            out.add(("null",))
        elif v.startswith("@"):
            out.add(("global", _sym(v[1:])))
        elif v.startswith("getelementptr"):
            root = self.ix.gep_root(v)[0]
            out |= self.origin(root, depth + 1) if root != v else set()
        elif v.startswith("%") and v not in self.regs:
            n = v[1:]
            if n.isdigit() and int(n) < self.ix.fn.nparams:
                out.add(("param", int(n)))
        elif v in self.load_op:
            out.add(("load", self.load_op[v]))
        elif v in self.fresh_op:
            out.add(("fresh", self.fresh_op[v]))
        elif v in self.ret_op:
            o = self.ix_ops_callee(v)
            out.add(("ret", o, self.ret_op[v]))
        elif v in self.regs:
            rhs = _rhs(self.regs[v])
            op = _opcode(rhs)
            if op == "load":
                root = self.ix.gep_root(self.ix._load_addr(rhs))[0]
                out |= self.slot_origin.get(root, set())
            elif op == "alloca":
                pass
            elif op == "getelementptr":
                out |= self.origin(self.ix.gep_root(v)[0], depth + 1)
            elif op == "call":
                pass
            else:
                body = rhs.split(None, 1)[1] if " " in rhs else ""
                for r in RE_REG.findall(body):
                    out |= self.origin(r, depth + 1)
        self._memo[v] = out
        return out

    def ix_ops_callee(self, v):
        kind, callee, _ = _call_parts(_rhs(self.regs[v]))
        return callee if kind == "direct" else "<indirect>"


def describe(tags):
    """Short text for a set of origin tags: `arg0`, `load#3`, `@g`, ..."""
    out = []
    for t in sorted(tags, key=str):
        if t[0] == "param":
            out.append(f"arg{t[1]}")
        elif t[0] == "global":
            out.append(f"@{t[1]}")
        elif t[0] in ("load", "fresh"):
            out.append(f"{t[0]}#{t[1]}")
        elif t[0] == "ret":
            out.append(f"ret:{t[1]}#{t[2]}")
        elif t[0] == "null":
            out.append("null")
        else:
            out.append(t[0])
    return "|".join(out) or "?"


def index_function(fn, prog):
    return _Indexer(fn, prog).run()


class ProgramIndex:
    """Lazily built FunctionIndex per function."""

    def __init__(self, prog):
        self.prog = prog
        self._by_key = {}

    def of(self, fn):
        fi = self._by_key.get(fn.key)
        if fi is None:
            fi = self._by_key[fn.key] = index_function(fn, self.prog)
        return fi
