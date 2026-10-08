"""Activations: the operation sequence one execution of an entry performs.

An activation is the entry's function with its defined callees expanded in
place, in program order. Origins are rewritten from per-function tags into
trace tags as callees are expanded, so a pointer loaded in the entry and used
three calls down is still `("load", step)` of the entry's load:

  ("arg", n)        the entry's n-th argument
  ("global", g)     the address of global g
  ("load", s)       the value read by step s of this activation
  ("fresh", s)      memory allocated at step s
  ("ret", f, s)     the result of the call to f at step s
  ("null",)         a null pointer
  ("unknown",)      a callee parameter the caller did not supply

The sequence is flattened in textual program order: branches are not split,
and a loop body appears once. That order is a *display and recall* order,
never a claim that one step happens before another in every execution, and
never a claim about what another activation can do in between.

Whatever the expansion could not follow is kept as a link instead of being
dropped: calls through pointers, calls past the depth or step budget,
recursion, and undefined functions that receive shared pointers. Critical
sections are recovered by pairing lock and unlock steps in this order; like
`Op.held` they are support facts only.
"""

from dataclasses import dataclass, field

from lace3.index.ops import NO_OPERAND_LOCKS

DEFAULT_MAX_DEPTH = 8
DEFAULT_MAX_STEPS = 4000


@dataclass
class Step:
    sid: int
    op: object                  # lace3.index.ops.Op
    frames: tuple               # ((function, call site or None), ...) outermost first
    base: frozenset = frozenset()
    origin: frozenset = frozenset()
    args: tuple = ()
    held: tuple = ()            # support: lock keys open at this step
    sections: tuple = ()        # support: ids of open critical sections

    @property
    def function(self):
        return self.op.function

    @property
    def site(self):
        return self.op.site

    @property
    def depth(self):
        return len(self.frames) - 1


@dataclass
class Section:
    sec: int
    key: str
    mode: str | None
    start: int                  # step id of the lock
    end: int | None             # step id of the matching unlock, None if never seen
    function: str


@dataclass
class Link:
    kind: str                   # indirect | depth | budget | recursion | external | unresolved
    step: int | None
    function: str
    callee: str | None
    site: str
    detail: str = ""


@dataclass
class Activation:
    aid: str
    entry: object               # lace3.entries.discover.Entry
    steps: list = field(default_factory=list)
    sections: list = field(default_factory=list)
    links: list = field(default_factory=list)
    reentrant: str = "unknown"  # unknown | no
    reentrant_why: str = ""
    truncated: bool = False

    @property
    def function(self):
        return self.entry.function

    @property
    def name(self):
        return self.entry.function.name


def tag_text(tag, steps=None):
    k = tag[0]
    if k == "arg":
        return f"arg{tag[1]}"
    if k == "global":
        return f"@{tag[1]}"
    if k in ("load", "fresh"):
        if tag[1] is None:
            return f"{k}?"
        if k == "load" and steps is not None and steps[tag[1]].op.state:
            return f"s{tag[1]}({steps[tag[1]].op.state})"
        return f"{k}:s{tag[1]}"
    if k == "ret":
        return f"ret:{tag[1]}@s{tag[2]}"
    return k


def tags_text(tags, steps=None):
    return "|".join(sorted(tag_text(t, steps) for t in tags)) or "?"


# Members inside the kernel's lock types that GEPs to a lock's first word name
# (`chan->mutex` reaches `mutex.owner.counter`); the lock is the member above.
LOCK_INTERNALS = frozenset({"owner", "counter", "rlock", "raw_lock", "raw", "val", "locked",
                            "wait_lock", "count", "lock_count", "osq", "tail", "pending",
                            "locked_pending", "rwbase", "rtmutex", "wait_list"})


def lock_name(state):
    if not state:
        return "*"
    parts = state.split(".")
    while len(parts) > 1 and parts[-1] in LOCK_INTERNALS:
        parts.pop()
    return ".".join(parts)


def lock_key(step, steps):
    op = step.op
    if op.callee in NO_OPERAND_LOCKS:
        return NO_OPERAND_LOCKS[op.callee]
    root = tags_text(step.base) if step.base else "?"
    return f"{lock_name(op.state)}@{root}"


def build_activation(aid, entry, px, prog, max_depth=DEFAULT_MAX_DEPTH,
                     max_steps=DEFAULT_MAX_STEPS):
    act = Activation(aid, entry)
    sec = entry.function.section or ""
    if sec.startswith((".init", ".exit")):
        act.reentrant, act.reentrant_why = "no", f"defined in {sec}: runs once"
    steps, links = act.steps, act.links

    def expand(fn, argmap, frames, stack):
        fi = px.of(fn)
        local = {}
        created = []

        def conv(tags):
            out = set()
            for t in tags:
                if t[0] == "param":
                    out |= argmap.get(t[1], {("unknown",)})
                elif t[0] in ("load", "fresh"):
                    out.add((t[0], local.get(t[1])))
                elif t[0] == "ret":
                    out.add(("ret", t[1], local.get(t[2])))
                else:
                    out.add(t)
            return frozenset(out)

        for op in fi.ops:
            if len(steps) >= max_steps:
                links.append(Link("budget", None, fn.name, None, op.site,
                                  f"step budget {max_steps} reached"))
                act.truncated = True
                return False
            st = Step(len(steps), op, frames, conv(op.base), conv(op.origin),
                      tuple(conv(a) for a in op.args))
            steps.append(st)
            local[op.id] = st.sid
            created.append(st)
            if op.kind == "call":
                tgt = prog.resolve(fn.module, op.callee)
                if tgt is None:
                    links.append(Link("unresolved", st.sid, fn.name, op.callee, op.site))
                elif tgt in stack:
                    links.append(Link("recursion", st.sid, fn.name, op.callee, op.site))
                elif len(frames) > max_depth:
                    links.append(Link("depth", st.sid, fn.name, op.callee, op.site,
                                      f"call depth {max_depth} reached"))
                    act.truncated = True
                else:
                    am = {k: a for k, a in enumerate(st.args)}
                    if not expand(tgt, am, frames + ((tgt.name, op.site),), stack | {tgt}):
                        return False
            elif op.kind == "icall":
                links.append(Link("indirect", st.sid, fn.name, None, op.site,
                                  "function pointer from " + tags_text(st.origin, steps)))
            elif op.kind == "ext" and any(t[0] in ("arg", "load", "global") for a in st.args
                                          for t in a):
                links.append(Link("external", st.sid, fn.name, op.callee, op.site,
                                  "undefined callee receives shared pointers"))
        for st in created:
            # Loop-carried origins name ops later in the body; resolve them now.
            st.base, st.origin = conv(st.op.base), conv(st.op.origin)
            st.args = tuple(conv(a) for a in st.op.args)
        return True

    fn = entry.function
    expand(fn, {k: {("arg", k)} for k in range(fn.nparams)}, ((fn.name, None),), {fn})
    _sections(act)
    return act


def _sections(act):
    """Pair lock and unlock steps. Optimized code repeats an unlock on each
    path out of a critical section; a second unlock of the same key before
    the key is locked again extends the section to it rather than leaving the
    steps between the two unlocks outside."""
    open_, last_closed = [], {}
    for st in act.steps:
        k = st.op.kind
        if k == "lock":
            key = lock_key(st, act.steps)
            s = Section(len(act.sections), key, st.op.mode, st.sid, None, st.function)
            act.sections.append(s)
            open_.append(s)
            last_closed.pop(key, None)
        elif k == "unlock":
            key = lock_key(st, act.steps)
            for s in reversed(open_):
                if s.key == key:
                    s.end = st.sid
                    open_.remove(s)
                    last_closed[key] = s
                    break
            else:
                if key in last_closed:
                    last_closed[key].end = st.sid
    n = len(act.steps)
    for st in act.steps:
        inside = [s for s in act.sections
                  if s.start <= st.sid < (s.end if s.end is not None else n)]
        st.held = tuple(s.key for s in inside)
        st.sections = tuple(s.sec for s in inside)


def build_activations(entries, px, prog, **kw):
    return [build_activation(f"A{i}", e, px, prog, **kw) for i, e in enumerate(entries)]


def provenance(entry):
    """One line per escape fact: how this entry can be invoked."""
    out = []
    for f in entry.facts:
        d = f.detail
        if f.kind == "A":
            out.append(f"A: address in initializer of {d.get('global')}"
                       f" slot {d.get('struct') or '?'}.{d.get('path') or ''}")
        elif f.kind == "E":
            out.append(f"E: exported ({d.get('symbol')})")
        elif f.kind == "B":
            out.append(f"B: passed to undefined {d.get('callee')} (arg {d.get('arg')}) "
                       f"in {d.get('caller')} at {d.get('file')}:{d.get('line')}")
        elif f.kind == "S":
            where = f"{d.get('struct')}.{d.get('field')}" if d.get("struct") else d.get("dest")
            out.append(f"S: stored into {where} in {d.get('in_function')} "
                       f"at {d.get('file')}:{d.get('line')}")
        elif f.kind == "C":
            out.append("C: syscall wrapper")
        elif f.kind == "D":
            out.append("D: external linkage, no caller in the analyzed program")
    return out
