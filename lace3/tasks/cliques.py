"""Analysis cliques: the unit one model session reviews.

A clique is a small set of activations (participants) plus the ledger items
whose representative contexts they cover. The model sees each participant's
operation sequence around those items -- not one field, not one pair -- so a
check in one critical section and the act in the next, or a pointer taken in
one function and used two calls down, are in front of it together.

Packing is greedy in item priority order. An item joins an existing clique
when the clique already shares its key family (`S` and `*S`) or already holds
all its participants, and the result stays within the participant and item
limits; otherwise it opens a new clique. Keys only steer packing: they do not
bound what the packet shows.

Every item ends up in exactly one clique (`Item.task`). Which cliques the
model actually reviews is decided later by the run budget; the rest are
recorded as pending, never silently dropped.
"""

from dataclasses import dataclass, field

DEFAULT_MAX_PARTICIPANTS = 4
DEFAULT_MAX_ITEMS = 24


@dataclass
class Clique:
    id: str
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


RANK = {"free": 0, "write": 1, "publish": 1, "rmw": 1, "ref_dec": 1}


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
    limit = 3 if item.kind in ("lifetime", "atomicity") else 2
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
                  max_items=DEFAULT_MAX_ITEMS, acts=None):
    cliques = []
    by_family = {}

    def fits(c, parts):
        return len(set(c.participants) | parts) <= max_participants and len(c.items) < max_items

    for it in items:
        reps = representative_contexts(it, acts)
        parts = set()
        for c in reps:
            parts |= set(c.participants)
        fams = {family(k) for k in it.keys}
        cands = {id(c): c for f in fams for c in by_family.get(f, ())}
        for c in cliques[-8:]:
            if parts <= set(c.participants):
                cands[id(c)] = c
        best = None
        for c in cands.values():
            if not fits(c, parts):
                continue
            score = (len(parts & set(c.participants)), -len(c.participants), -len(c.items))
            if best is None or score > best[0]:
                best = (score, c)
        if best is None:
            target = Clique(f"T{len(cliques)}")
            cliques.append(target)
        else:
            target = best[1]
        for a in sorted(parts):
            if a not in target.participants:
                target.participants.append(a)
        target.items.append(it.id)
        target.task_contexts[it.id] = reps
        target.priority = min(target.priority, it.priority)
        for f in fams:
            if f not in target.families:
                target.families.add(f)
                by_family.setdefault(f, []).append(target)
        it.task, it.status = target.id, "assigned"
    cliques.sort(key=lambda c: (c.priority, -len(c.items), c.id))
    return cliques
