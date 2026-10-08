"""Analysis cliques: the unit one model session reviews.

A clique is anchored on one activation's operation sequence: a segment of it
(the seed steps and dependency closures of its items) plus the interfering
activations of those items. Items are placed by their anchor -- the first
side of their strongest representative context -- so everything one
execution does with a value taken from shared state (the take, the unlock,
the uses two calls down, the branch it decides) is judged in one session,
whichever fields the individual steps touch. Keys play no part in packing.

Items whose anchor spans overlap or lie within GAP steps share a clique as
long as the participant and item limits hold; otherwise the anchor gets
another clique. Every item ends up in exactly one clique (`Item.task`).
Which cliques are reviewed is decided by the run budget; the rest are
recorded pending, never dropped.
"""

from dataclasses import dataclass, field

DEFAULT_MAX_PARTICIPANTS = 6
DEFAULT_MAX_ITEMS = 24
GAP = 24
RANK = {"free": 0, "write": 1, "publish": 1, "rmw": 1, "ref_dec": 1}


@dataclass
class Clique:
    id: str
    anchor: str = ""
    lo: int = 0
    hi: int = 0
    participants: list = field(default_factory=list)
    items: list = field(default_factory=list)          # item ids
    families: set = field(default_factory=set)
    priority: int = 99
    status: str = "planned"     # planned | pending | done | incomplete | error
    expansions: list = field(default_factory=list)     # model-requested additions
    notes: list = field(default_factory=list)
    task_contexts: dict = field(default_factory=dict)  # item id -> [Context] shown


def family(key):
    return key.lstrip("*")


def strength(ctx, acts):
    """How hard the interfering side hits: frees, then writes, then the rest."""
    if acts is None:
        return 2
    steps = acts[ctx.b].steps
    return min((RANK.get(steps[s].op.kind, 2) for s in ctx.b_steps), default=2)


def representative_contexts(item, acts=None):
    """The strongest cross-entry context, the strongest self-interference
    context, and for sequence items a second cross context with another
    interfering entry."""
    limit = 3 if item.kind in ("lifetime", "atomicity", "guard") else 2
    ranked = sorted(item.contexts, key=lambda c: strength(c, acts))
    out = []
    cross = [c for c in ranked if not c.self_interference]
    selfc = [c for c in ranked if c.self_interference]
    if cross:
        out.append(cross[0])
    if selfc:
        out.append(selfc[0])
    for c in cross[1:]:
        if len(out) >= limit:
            break
        if all(c.b != o.b for o in out):
            out.append(c)
    return out[:limit]


def build_cliques(items, max_participants=DEFAULT_MAX_PARTICIPANTS,
                  max_items=DEFAULT_MAX_ITEMS, acts=None, gap=GAP):
    placed = []
    for it in items:
        reps = representative_contexts(it, acts)
        c0 = reps[0] if reps else it.contexts[0]
        parts = set()
        for c in reps:
            parts |= set(c.participants)
        placed.append((c0.a, min(c0.a_steps), max(c0.a_steps), it, reps, parts))
    placed.sort(key=lambda p: (p[0], p[1], p[3].priority))

    cliques, by_anchor = [], {}
    for aid, lo, hi, it, reps, parts in placed:
        best = None
        for c in by_anchor.get(aid, [])[-4:]:
            if lo > c.hi + gap:
                continue
            if len(set(c.participants) | parts) > max_participants or len(c.items) >= max_items:
                continue
            score = (len(parts & set(c.participants)), -len(c.items))
            if best is None or score > best[0]:
                best = (score, c)
        if best is None:
            target = Clique(f"T{len(cliques)}", anchor=aid, lo=lo, hi=hi, participants=[aid])
            cliques.append(target)
            by_anchor.setdefault(aid, []).append(target)
        else:
            target = best[1]
        target.lo, target.hi = min(target.lo, lo), max(target.hi, hi)
        for a in sorted(parts):
            if a not in target.participants:
                target.participants.append(a)
        target.items.append(it.id)
        target.task_contexts[it.id] = reps
        target.priority = min(target.priority, it.priority)
        target.families |= {family(k) for k in it.keys}
        it.task, it.status = target.id, "assigned"
    cliques.sort(key=lambda c: (c.priority, -len(c.items), c.id))
    return cliques
