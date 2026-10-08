"""Ledger items: the recall units every task is accountable for.

Three kinds, all enumerated from activations without consulting locks:

  conflict    two steps touch the same key and at least one writes or frees.
  atomicity   one activation reads state S and later writes S (or writes a
              value computed from that read), while some activation writes S.
  lifetime    one activation loads a pointer from S and later uses or frees
              the object it points to, while some activation writes S or frees
              that object.

Keys are recall hints, not context boundaries: `S` is a field by DWARF name
(`struct.field`, instance-insensitive), `*S` is the object a pointer loaded
from S points to. A pointer loaded in one function and used in a callee keeps
its origin through the activation, so take -> release -> use across functions
and fields is a single lifetime item.

Items are identified by code sites, so the same pair of lines reached from
several entries is one item with several contexts. A context is the pair of
activations it was seen in. The second activation may be the same entry: an
entry interferes with a concurrent activation of itself unless it is known to
run once (`Activation.reentrant == "no"`); when that is not known, the self
context is kept.

Nothing here is pruned on program order or held locks. Order within one
activation constrains that activation only, and `held` is a support fact.
Both are carried on the steps for the model to weigh.
"""

from collections import defaultdict
from dataclasses import dataclass, field

READ_MODES = frozenset({"read", "aread"})
ATOMIC_MODES = frozenset({"aread", "awrite", "rmw"})
MAX_CONTEXTS = 6
MAX_USES = 8

PRIORITY = {"lifetime": 0, "atomicity": 1, "conflict-free": 2, "conflict-ww": 3,
            "conflict-rw": 4}


@dataclass(frozen=True)
class Context:
    a: str                      # activation id
    a_steps: tuple              # step ids in a
    b: str                      # interfering activation id (may equal a)
    b_steps: tuple

    @property
    def self_interference(self):
        return self.a == self.b

    @property
    def participants(self):
        return (self.a,) if self.a == self.b else (self.a, self.b)


@dataclass
class Item:
    id: str
    kind: str                   # conflict | atomicity | lifetime
    keys: tuple
    sig: tuple                  # code-site identity
    summary: str
    priority: int
    contexts: list = field(default_factory=list)
    n_contexts: int = 0
    task: str | None = None     # clique id
    status: str = "unassigned"  # unassigned | assigned | pending | reviewed | incomplete | error
    verdict: str | None = None  # bug | safe | unknown (None until a task reviews it)
    reason: str = ""
    citations: tuple = ()
    contexts_reviewed: int = 0  # contexts in front of the model when it judged
    notes: list = field(default_factory=list)


def step_mode(st):
    """Mode of a field access, or None when the step does not access a field."""
    op = st.op
    if not op.state:
        return None
    if op.kind == "read":
        return "aread" if op.atomic else "read"
    if op.kind in ("write", "publish"):
        return "awrite" if op.atomic else "write"
    if op.kind in ("rmw", "ref_inc", "ref_dec"):
        return "rmw"
    return None


def step_keys(st, steps):
    """[(key, mode)] a step touches."""
    out = []
    m = step_mode(st)
    if m:
        out.append((st.op.state, m))
    k = st.op.kind
    if k in ("read", "write", "publish", "rmw", "ref_inc", "ref_dec"):
        src, pmode = st.base, "use"
    elif k == "free":
        src, pmode = st.origin, "free"
    else:
        return out
    for t in src:
        if t[0] == "load" and t[1] is not None:
            ld = steps[t[1]]
            if ld.op.kind == "read" and ld.op.state:
                out.append(("*" + ld.op.state, pmode))
        elif t[0] == "global" and k == "free":
            out.append(("*@" + t[1], "free"))
    return out


def conflicting(m1, m2):
    if "free" in (m1, m2):
        return True
    if "use" in (m1, m2):
        return False
    if m1 in READ_MODES and m2 in READ_MODES:
        return False
    if m1 in ATOMIC_MODES and m2 in ATOMIC_MODES:
        return False
    return True


def where(st):
    """Site text with the inlined body line, which -O2 code otherwise loses."""
    loc = st.op.loc
    return f"{st.site} [inlined {loc.via}]" if loc is not None and loc.via else st.site


def _site(st):
    return (where(st), st.op.kind, st.function)


class _Builder:
    def __init__(self, acts, max_contexts=MAX_CONTEXTS):
        self.acts = {a.aid: a for a in acts}
        self.order = [a.aid for a in acts]
        self.max_contexts = max_contexts
        self.items = {}

    def can_pair(self, a, b):
        return a != b or self.acts[a].reentrant != "no"

    def add(self, sig, kind, keys, summary, prio, ctx):
        it = self.items.get(sig)
        if it is None:
            it = self.items[sig] = Item(f"I{len(self.items)}", kind, tuple(keys), sig, summary, prio)
        it.n_contexts += 1
        if len(it.contexts) < self.max_contexts:
            it.contexts.append(ctx)
        elif not ctx.self_interference and all(c.self_interference for c in it.contexts):
            it.contexts[-1] = ctx
        elif ctx.self_interference and not any(c.self_interference for c in it.contexts):
            it.contexts[-1] = ctx

    # --------------------------------------------------------- conflicts
    def conflicts(self):
        acc = defaultdict(lambda: defaultdict(list))     # key -> site -> [(aid, sid, mode)]
        for aid in self.order:
            act = self.acts[aid]
            for st in act.steps:
                for key, mode in step_keys(st, act.steps):
                    acc[key][_site(st) + (mode,)].append((aid, st.sid))
        for key, sites in acc.items():
            sk = sorted(sites)
            for i, sa in enumerate(sk):
                for sb in sk[i:]:
                    ma, mb = sa[3], sb[3]
                    if not conflicting(ma, mb):
                        continue
                    if "free" in (ma, mb):
                        kind = "conflict-free"
                    elif ma not in READ_MODES and mb not in READ_MODES:
                        kind = "conflict-ww"
                    else:
                        kind = "conflict-rw"
                    sig = ("conflict", key, sa, sb)
                    summary = (f"{key}: {sa[1]} at {sa[0]} ({sa[2]}) vs "
                               f"{sb[1]} at {sb[0]} ({sb[2]})")
                    for aa, xa in sites[sa]:
                        for ab, xb in sites[sb]:
                            if sa == sb and (aa, xa) > (ab, xb):
                                continue
                            if not self.can_pair(aa, ab):
                                continue
                            self.add(sig, "conflict", (key,), summary, PRIORITY[kind],
                                     Context(aa, (xa,), ab, (xb,)))

    # ------------------------------------------------------- interferers
    def _writers(self):
        """key -> [(aid, sid)] of steps that write a field key or free a pointee key."""
        w = defaultdict(list)
        for aid in self.order:
            act = self.acts[aid]
            for st in act.steps:
                for key, mode in step_keys(st, act.steps):
                    if mode in ("write", "awrite", "rmw", "free"):
                        w[key].append((aid, st.sid))
        return w

    def _interferers(self, writers, keys, owner):
        """Per interfering activation, its writing steps on `keys`. The owner
        itself counts when it may run concurrently with itself: its own write
        then interferes from the second activation."""
        out = defaultdict(list)
        for key in keys:
            for aid, sid in writers.get(key, ()):
                if self.can_pair(owner, aid):
                    out[aid].append(sid)
        return out

    # ------------------------------------------------------ sequences
    def sequences(self):
        writers = self._writers()
        for aid in self.order:
            act = self.acts[aid]
            steps = act.steps
            field_writes = defaultdict(list)       # state -> [sid]
            dep_writes = defaultdict(list)         # load sid -> [sid] writes of a value from it
            uses = defaultdict(list)               # load sid -> [sid] uses/frees of its pointee
            for st in steps:
                m = step_mode(st)
                if m in ("write", "awrite", "rmw"):
                    field_writes[st.op.state].append(st.sid)
                    for t in st.origin:
                        if t[0] == "load" and t[1] is not None:
                            dep_writes[t[1]].append(st.sid)
                for key, mode in step_keys(st, steps):
                    if key.startswith("*") and mode in ("use", "free"):
                        src = st.origin if mode == "free" else st.base
                        for t in src:
                            if t[0] == "load" and t[1] is not None:
                                uses[t[1]].append(st.sid)
            for st in steps:
                if st.op.kind != "read" or not st.op.state:
                    continue
                s = st.op.state
                # atomicity: read S, later write S or write a value derived from the read
                later = [w for w in field_writes.get(s, ()) if w > st.sid]
                derived = [w for w in dep_writes.get(st.sid, ()) if w > st.sid]
                if later or derived:
                    y = (later or derived)[0]
                    seq = (st.sid, y)
                    inter = self._interferers(writers, [s], aid)
                    for bid, bsteps in inter.items():
                        sig = ("atomicity", s, where(st), where(steps[y]))
                        summary = (f"{s}: read at {where(st)} ({st.function}) then "
                                   f"{steps[y].op.kind} {steps[y].op.state} at {where(steps[y])} "
                                   f"({steps[y].function})")
                        self.add(sig, "atomicity", (s,), summary, PRIORITY["atomicity"],
                                 Context(aid, seq, bid, tuple(sorted(set(bsteps)))[:MAX_USES]))
                # lifetime: pointer loaded from S, its object used or freed later
                u = sorted(set(uses.get(st.sid, ())))
                if u:
                    seq = (st.sid,) + tuple(u[:MAX_USES])
                    inter = self._interferers(writers, [s, "*" + s], aid)
                    for bid, bsteps in inter.items():
                        sig = ("lifetime", s, where(st))
                        summary = (f"{s}: pointer loaded at {where(st)} ({st.function}), its object "
                                   f"used at {', '.join(sorted({where(steps[x]) for x in u})[:4])}")
                        self.add(sig, "lifetime", (s, "*" + s), summary, PRIORITY["lifetime"],
                                 Context(aid, seq, bid, tuple(sorted(set(bsteps)))[:MAX_USES]))


def build_items(acts, max_contexts=MAX_CONTEXTS):
    b = _Builder(acts, max_contexts)
    b.conflicts()
    b.sequences()
    items = list(b.items.values())
    items.sort(key=lambda it: (it.priority, it.sig))
    for k, it in enumerate(items):
        it.id = f"I{k}"
    return items
