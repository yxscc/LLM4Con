"""Ledger items: the recall units every task is accountable for.

Four kinds, all enumerated from activations without consulting locks:

  conflict    two steps touch the same key and at least one writes or frees.
  atomicity   one activation reads state S and later writes S (or writes a
              value computed from that read), while some activation writes S.
  guard       one activation branches on a value read from S and then touches
              other state under that branch, while some activation writes S
              (check-then-act across fields, e.g. a state flag guarding a use).
  lifetime    one activation loads a pointer from S and later uses or frees
              the object it points to, while some activation writes S or frees
              that object.

Sequence items are built around a seed read and its dependency closure: the
steps of the same activation that use the seed's value or anything derived
from it (a pointer read through it, a value written from it, a branch on it,
a callee it is passed to, which is expanded in place). Keys -- `S` a field by
DWARF name, `*S` the object a pointer read from S points to -- only find
seeds and interferers; they do not bound the sequence. Where the closure
leaves what the index can follow, a boundary is recorded instead: the value
goes to an undefined or indirect callee, is stored to memory (later reloads
are linked only by field), is accessed without a field name, or the
activation or the closure was cut by a budget. Boundaries stay open until a
review resolves them with a citation.

Interferers carry their own closure too: a writer of S brings the read of S
before it and that read's closure, so `p = o->buf; o->buf = NULL; kfree(p)`
is one connected interfering sequence.

Items are identified by code sites, so the same lines reached from several
entries are one item with several contexts. The second activation of a
context may be the same entry: an entry interferes with a concurrent
activation of itself unless it is known to run once (`reentrant == "no"`).

Nothing is pruned on program order or held locks. A lock common to two
single accesses says nothing about a sequence spanning several sections.
"""

from collections import Counter, defaultdict
from dataclasses import dataclass, field

READ_MODES = frozenset({"read", "aread"})
ATOMIC_MODES = frozenset({"aread", "awrite", "rmw"})
ACCESS_KINDS = frozenset({"read", "write", "publish", "rmw", "ref_inc", "ref_dec"})
MAX_CONTEXTS = 6
MAX_USES = 8
MAX_CLOSURE = 40
MAX_REGION = 12
MAX_B_STEPS = 12

UNSEEN_USE = frozenset({"external", "indirect", "depth", "recursion", "unresolved"})
PRIORITY = {"lifetime": 0, "atomicity": 1, "guard": 2, "conflict-free": 3, "conflict-ww": 4,
            "conflict-rw": 5}


@dataclass(frozen=True)
class Context:
    a: str                      # activation id
    a_steps: tuple              # step ids in a
    b: str                      # interfering activation id (may equal a)
    b_steps: tuple
    boundaries: tuple = ()      # boundary ids open on either side
    core: tuple = ()            # the a-side steps the item is about (default: a_steps)

    @property
    def self_interference(self):
        return self.a == self.b

    @property
    def participants(self):
        return (self.a,) if self.a == self.b else (self.a, self.b)


@dataclass
class Boundary:
    id: str
    aid: str
    kind: str                   # external | indirect | stored | unnamed | depth | recursion
                                # | unresolved | cap | truncated
    step: int | None
    detail: str
    status: str = "open"        # open | resolved (by a review, with a checked citation)
    resolution: str = ""
    citations: tuple = ()


@dataclass
class Item:
    id: str
    kind: str                   # conflict | atomicity | guard | lifetime
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
    # member paths of keys[0] over all contexts, judged side and interferer side
    paths: tuple = field(default_factory=lambda: (set(), set()))

    @property
    def boundaries(self):
        out = []
        for c in self.contexts:
            for b in c.boundaries:
                if b not in out:
                    out.append(b)
        return out


@dataclass
class ItemSet:
    items: list
    boundaries: dict            # id -> Boundary
    gaps: dict                  # static unknowns of the index, not tied to one item


def step_mode(st):
    """Mode of a field access, or None when the step does not access a field."""
    op = st.op
    if not op.key:
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
        out.append((st.op.key, m))
    k = st.op.kind
    if k in ACCESS_KINDS:
        src, pmode = st.base, "use"
    elif k == "free":
        src, pmode = st.origin, "free"
    else:
        return out
    for t in src:
        if t[0] == "load" and t[1] is not None:
            ld = steps[t[1]]
            if ld.op.kind == "read" and ld.op.key:
                out.append(("*" + ld.op.key, pmode))
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


def ident(st):
    """Identity of a step's code: the innermost source line, so the copies of
    an inlined helper in several callers are one site, as the helper's own
    lines are at -O0."""
    loc = st.op.loc
    return loc.via if loc is not None and loc.via else st.site


def _site(st):
    return (ident(st), st.op.kind, where(st), st.function)


# ------------------------------------------------------------- closure
@dataclass
class Closure:
    steps: list                 # step ids depending on the seed, in order
    derefs: list                # accesses / frees through the seed or derived pointers
    writes: list                # writes of a value derived from the seed
    branches: list              # branches on a derived value
    derived: list               # (sid, state) reads through the tracked pointers
    bounds: list                # (kind, sid, detail)


def closure(act, seed, limit=MAX_CLOSURE):
    steps = act.steps
    tracked = {("load", seed)}
    cut = {l.step: l for l in act.links if l.step is not None}
    c = Closure([], [], [], [], [], [])
    for st in steps[seed + 1:]:
        on_base = bool(st.base & tracked)
        on_val = bool(st.origin & tracked)
        on_arg = any(a & tracked for a in st.args)
        if not (on_base or on_val or on_arg):
            continue
        k, sid = st.op.kind, st.sid
        c.steps.append(sid)
        if k in ACCESS_KINDS and on_base:
            c.derefs.append(sid)
            if not st.op.key:
                c.bounds.append(("unnamed", sid, "access through the tracked value has no "
                                                 "field name"))
        if k == "free" and on_val:
            c.derefs.append(sid)
        if k == "read" and on_base:
            tracked.add(("load", sid))
            if st.op.key:
                c.derived.append((sid, st.op.key))
        if k in ("write", "publish", "rmw") and on_val:
            c.writes.append(sid)
            c.bounds.append(("stored", sid, f"value stored into {st.op.key or 'memory'}; "
                                            "later reloads are linked only by field"))
        if k == "branch" and on_val:
            c.branches.append(sid)
        if k == "ext" and on_arg:
            c.bounds.append(("external", sid, f"passed to undefined {st.op.callee}"))
        if k == "icall" and (on_arg or on_val):
            c.bounds.append(("indirect", sid, "flows into an indirect call"))
        if k in ("call", "ext", "icall") and on_arg:
            tracked.add(("ret", st.op.callee or "<indirect>", sid))
            l = cut.get(sid)
            if k == "call" and l is not None and l.kind in ("depth", "recursion", "unresolved"):
                c.bounds.append((l.kind, sid, f"{st.op.callee} not expanded ({l.kind})"))
        if len(c.steps) >= limit:
            c.bounds.append(("cap", sid, f"dependency closure capped at {limit} steps"))
            break
    else:
        if act.truncated:
            c.bounds.append(("truncated", None, "activation cut by the step budget; the "
                                                "closure may continue past it"))
    return c


def guarded_region(act, branch, skip_key):
    """Steps under a branch: the rest of the branching function's invocation
    (callees included), as flattened; those touching a key other than skip_key."""
    steps = act.steps
    fr = steps[branch].frames
    out = []
    for st in steps[branch + 1:]:
        if st.frames[:len(fr)] != fr:
            break
        if any(k != skip_key and k.lstrip("*") != skip_key for k, _ in step_keys(st, steps)):
            out.append(st.sid)
            if len(out) >= MAX_REGION:
                break
    return out


class _Builder:
    def __init__(self, acts, max_contexts=MAX_CONTEXTS):
        self.acts = {a.aid: a for a in acts}
        self.order = [a.aid for a in acts]
        self.max_contexts = max_contexts
        self.items = {}
        self.boundaries = {}
        self._bkey = {}
        self._closures = {}

    def can_pair(self, a, b):
        return a != b or self.acts[a].reentrant != "no"

    def closure(self, aid, seed):
        key = (aid, seed)
        if key not in self._closures:
            self._closures[key] = closure(self.acts[aid], seed)
        return self._closures[key]

    def boundary_ids(self, aid, bounds):
        out = []
        for kind, sid, detail in bounds:
            k = (aid, kind, sid)
            if k not in self._bkey:
                b = Boundary(f"B{len(self.boundaries)}", aid, kind, sid, detail)
                self.boundaries[b.id] = b
                self._bkey[k] = b.id
            out.append(self._bkey[k])
        return tuple(out)

    def add(self, sig, kind, keys, summary, prio, ctx):
        it = self.items.get(sig)
        if it is None:
            it = self.items[sig] = Item(f"I{len(self.items)}", kind, tuple(keys), sig, summary, prio)
        it.n_contexts += 1
        key = it.keys[0].lstrip("*")
        for side, aid, sids in ((0, ctx.a, ctx.a_steps), (1, ctx.b, ctx.b_steps)):
            steps = self.acts[aid].steps
            it.paths[side].update(steps[x].path or key for x in sids if steps[x].op.key == key)
        if len(it.contexts) < self.max_contexts:
            it.contexts.append(ctx)
        elif not ctx.self_interference and all(c.self_interference for c in it.contexts):
            it.contexts[-1] = ctx
        elif ctx.self_interference and not any(c.self_interference for c in it.contexts):
            it.contexts[-1] = ctx

    # --------------------------------------------------------- conflicts
    def conflicts(self):
        acc = defaultdict(lambda: defaultdict(list))     # key -> site -> [(aid, sid)]
        for aid in self.order:
            act = self.acts[aid]
            for st in act.steps:
                for key, mode in step_keys(st, act.steps):
                    acc[key][_site(st) + (mode,)].append((aid, st.sid))
        for key, sites in acc.items():
            # the display text and function differ between inlined copies: identity is
            # (innermost line, kind, mode); the first copy seen names the site
            merged = {}
            for k, v in sites.items():
                merged.setdefault((k[0], k[1], k[4]), [k, []])[1].extend(v)
            sites = {m[0]: m[1] for m in merged.values()}
            sk = sorted(sites)
            for i, sa in enumerate(sk):
                for sb in sk[i:]:
                    ma, mb = sa[4], sb[4]
                    if not conflicting(ma, mb):
                        continue
                    if "free" in (ma, mb):
                        kind = "conflict-free"
                    elif ma not in READ_MODES and mb not in READ_MODES:
                        kind = "conflict-ww"
                    else:
                        kind = "conflict-rw"
                    sig = ("conflict", key, sa[0], sa[1], sa[4], sb[0], sb[1], sb[4])
                    summary = (f"{key}: {sa[1]} at {sa[2]} ({sa[3]}) vs "
                               f"{sb[1]} at {sb[2]} ({sb[3]})")
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

    def _interfering_sequence(self, bid, sids, keys):
        """The interferer's writing steps plus what they connect to: the read
        of the same field before a write (take-then-clear), the load a freed
        value came from, and those reads' closures."""
        act = self.acts[bid]
        steps = act.steps
        out, bounds = set(sids), []
        for x in sids:
            st = steps[x]
            seeds = []
            if st.op.kind == "free":
                seeds = [t[1] for t in st.origin if t[0] == "load" and t[1] is not None]
            elif st.op.key in keys:
                prior = [s.sid for s in steps[:x] if s.op.kind == "read"
                         and s.op.key == st.op.key and s.function == st.function]
                seeds = prior[-1:]
            for r in seeds:
                out.add(r)
                c = self.closure(bid, r)
                out.update(c.steps[:MAX_B_STEPS])
                bounds += c.bounds
        return tuple(sorted(out)[:MAX_B_STEPS * 2]), bounds

    def _interferers(self, writers, keys, owner):
        out = defaultdict(list)
        for key in keys:
            for aid, sid in writers.get(key, ()):
                if self.can_pair(owner, aid):
                    out[aid].append(sid)
        return out

    def _contexts(self, writers, keys, aid, a_steps, a_bounds, core):
        for bid, bsteps in self._interferers(writers, keys, aid).items():
            bsteps = sorted(set(bsteps))[:MAX_USES]
            seq, b_bounds = self._interfering_sequence(bid, bsteps, set(keys))
            ids = self.boundary_ids(aid, a_bounds) + self.boundary_ids(bid, b_bounds)
            yield Context(aid, a_steps, bid, seq, tuple(dict.fromkeys(ids)), tuple(core))

    # ------------------------------------------------------ sequences
    def sequences(self):
        writers = self._writers()
        for aid in self.order:
            act = self.acts[aid]
            steps = act.steps
            field_writes = defaultdict(list)
            for st in steps:
                if step_mode(st) in ("write", "awrite", "rmw"):
                    field_writes[st.op.key].append(st.sid)
            for st in steps:
                if st.op.kind != "read" or not st.op.key:
                    continue
                s = st.op.key
                c = self.closure(aid, st.sid)
                # atomicity: read S, later write S or a write of a value derived from it
                later = [w for w in field_writes.get(s, ()) if w > st.sid]
                if later or c.writes:
                    y = (later or c.writes)[0]
                    seq = tuple(sorted({st.sid, y, *c.steps[:MAX_USES]}))
                    sig = ("atomicity", s, ident(st), ident(steps[y]))
                    summary = (f"{s}: read at {where(st)} ({st.function}) then "
                               f"{steps[y].op.kind} {steps[y].op.key} at {where(steps[y])} "
                               f"({steps[y].function})")
                    core = sorted({st.sid, y} | ({y} if y in c.writes else set()))
                    for ctx in self._contexts(writers, [s], aid, seq, c.bounds, core):
                        self.add(sig, "atomicity", (s,), summary, PRIORITY["atomicity"], ctx)
                # guard: a branch on the value, then other state under it
                dep = set(c.steps)
                for b in c.branches[:1]:
                    # only state not reached through the value itself: that is lifetime's part
                    region = [x for x in guarded_region(act, b, s) if x not in dep]
                    if not region:
                        continue
                    rkeys = []
                    for x in region:
                        for k, _ in step_keys(steps[x], steps):
                            if k != s and k not in rkeys:
                                rkeys.append(k)
                    seq = (st.sid, b) + tuple(region)
                    sig = ("guard", s, ident(st), ident(steps[b]))
                    summary = (f"{s}: read at {where(st)} ({st.function}) decides a branch at "
                               f"{where(steps[b])}; under it: {', '.join(rkeys[:4])}")
                    for ctx in self._contexts(writers, [s], aid, seq, c.bounds, seq):
                        self.add(sig, "guard", (s, *rkeys[:4]), summary, PRIORITY["guard"], ctx)
                # lifetime: pointer read from S, its object (or one reached through it) used
                # a value handed to code the index cannot see may be used there
                hidden = [x for k, x, _ in c.bounds if k in UNSEEN_USE]
                if c.derefs or hidden:
                    seq = (st.sid,) + tuple(c.steps[:MAX_CLOSURE])
                    keys = [s, "*" + s] + ["*" + d for _, d in c.derived[:4]]
                    used = sorted({where(steps[x]) for x in c.derefs})
                    sig = ("lifetime", s, ident(st))
                    summary = (f"{s}: pointer loaded at {where(st)} ({st.function}), its object "
                               + (f"used at {', '.join(used[:4])}"
                                  + (f" (+{len(used) - 4})" if len(used) > 4 else "")
                                  if used else "")
                               + (f"{'; ' if used else ''}passed where the index cannot follow "
                                  f"at {', '.join(sorted({where(steps[x]) for x in hidden})[:3])}"
                                  if hidden else ""))
                    core = (st.sid,) + tuple(sorted(set(c.derefs + hidden))[:MAX_CLOSURE])
                    for ctx in self._contexts(writers, keys, aid, seq, c.bounds, core):
                        self.add(sig, "lifetime", (s, "*" + s), summary, PRIORITY["lifetime"],
                                 ctx)

    # ------------------------------------------------------------ gaps
    def gaps(self):
        """What the index itself could not see, counted per kind."""
        in_seq = {(c.a, s) for it in self.items.values() for c in it.contexts for s in c.a_steps}
        g = Counter()
        for aid in self.order:
            act = self.acts[aid]
            for st in act.steps:
                if st.op.kind in ACCESS_KINDS:
                    g["accesses"] += 1
                    if not st.op.key:
                        g["unnamed_accesses"] += 1
                        if (aid, st.sid) in in_seq:
                            g["unnamed_in_sequences"] += 1
                    if not st.base or any(t[0] == "unknown" for t in st.base):
                        g["unknown_base_accesses"] += 1
            for l in act.links:
                g[f"links_{l.kind}"] += 1
            g["truncated_activations"] += act.truncated
            g["reentrancy_unknown"] += act.reentrant == "unknown"
        return dict(g)


def build_index(acts, max_contexts=MAX_CONTEXTS):
    b = _Builder(acts, max_contexts)
    b.conflicts()
    b.sequences()
    items = list(b.items.values())
    items.sort(key=lambda it: (it.priority, it.sig))
    for k, it in enumerate(items):
        it.id = f"I{k}"
    return ItemSet(items, b.boundaries, b.gaps())


def build_items(acts, max_contexts=MAX_CONTEXTS):
    return build_index(acts, max_contexts).items
