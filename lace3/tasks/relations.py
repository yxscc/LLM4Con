"""Factorized candidate relations, before any agent task scheduling.

Store complete side members and a pairing predicate, never their Cartesian
product. Access modes select possible conflicts; object paths and lock facts
are retained in activation bindings, NOT used to prove disjointness/safety.

This replaces the pair-enumeration layer, not the bounded activation front
end. It deliberately exposes that boundary and makes no defect-recall claim.
"""

from bisect import bisect_right
from collections import Counter, defaultdict
from dataclasses import dataclass, field

from lace3.index.dependencies import Dependencies
from lace3.tasks.items import UNSEEN_USE, conflicting, step_keys, step_mode

WRITERS = frozenset({"write", "awrite", "rmw", "free"})


@dataclass(frozen=True)
class Member:
    id: str
    aid: str
    steps: tuple
    core: tuple
    boundaries: tuple = ()


@dataclass
class Pool:
    id: str
    members: tuple
    by_entry: dict
    member_set: frozenset = field(repr=False)


@dataclass(frozen=True)
class Relation:
    id: str
    kind: str
    keys: tuple
    left: str
    right: str
    symmetric: bool
    combinations: int
    self_combinations: int


class RelationIndex:
    def __init__(self, acts):
        self.acts = {a.aid: a for a in acts}
        self.members, self.pools, self.relations, self.boundaries = {}, {}, {}, {}
        self._member_ids, self._pool_ids, self._boundary_ids = {}, {}, {}
        self._relation_ids = {}
        self.duplicate_access_records = 0

    def member(self, aid, steps, core=None, bounds=()):
        steps = tuple(sorted(set(steps)))
        core = tuple(sorted(set(steps if core is None else core)))
        bids = []
        for kind, sid, detail in bounds:
            sig = (aid, kind, sid, detail)
            if sig not in self._boundary_ids:
                bid = f"B{len(self.boundaries)}"
                self._boundary_ids[sig] = bid
                self.boundaries[bid] = dict(id=bid, aid=aid, kind=kind, step=sid,
                                            detail=detail, status="open")
            bids.append(self._boundary_ids[sig])
        sig = (aid, steps, core, tuple(sorted(set(bids))))
        if sig not in self._member_ids:
            mid = f"M{len(self.members)}"
            self._member_ids[sig] = mid
            self.members[mid] = Member(mid, *sig)
        return self._member_ids[sig]

    def pool(self, members):
        ids = tuple(sorted(set(members), key=lambda m: int(m[1:])))
        if ids not in self._pool_ids:
            pid = f"P{len(self.pools)}"
            self._pool_ids[ids] = pid
            counts = dict(Counter(self.members[m].aid for m in ids))
            self.pools[pid] = Pool(pid, ids, counts, frozenset(ids))
        return self._pool_ids[ids]

    def add(self, kind, keys, left, right, symmetric=False):
        a, b = self.pools[left], self.pools[right]
        diagonal = symmetric and left == right
        n = len(a.members)
        total = n * (n + 1) // 2 if diagonal else n * len(b.members)
        same, excluded = 0, 0
        for aid, na in a.by_entry.items():
            nb = b.by_entry.get(aid, 0)
            count = na * (na + 1) // 2 if diagonal else na * nb
            if self.acts[aid].reentrant == "no":
                excluded += count
            else:
                same += count
        total -= excluded
        if not total:
            return None
        sig = (kind, tuple(keys), left, right, symmetric)
        if sig not in self._relation_ids:
            rid = f"R{len(self.relations)}"
            self._relation_ids[sig] = rid
            self.relations[rid] = Relation(rid, *sig, total, same)
        return self._relation_ids[sig]

    def contains(self, rid, left, right):
        r = self.relations[rid]
        a, b = self.pools[r.left], self.pools[r.right]
        inside = left in a.member_set and right in b.member_set
        if r.symmetric:
            inside |= right in a.member_set and left in b.member_set
        if not inside:
            return False
        aa, bb = self.members[left].aid, self.members[right].aid
        return aa != bb or self.acts[aa].reentrant != "no"

    def page(self, rid, limit=50, cursor=None):
        """Expand a requested page only. Cursor is an opaque pair of pool offsets.

        Page size is a transport limit: next_cursor and total always expose
        outstanding combinations. Reading a page never marks anything reviewed.
        """
        if not isinstance(limit, int) or not 1 <= limit <= 1000:
            raise ValueError("page limit must be 1..1000")
        r = self.relations[rid]
        left, right = self.pools[r.left].members, self.pools[r.right].members
        diagonal = r.symmetric and r.left == r.right
        i, j = (0, 0) if cursor is None else tuple(cursor)
        if not (isinstance(i, int) and isinstance(j, int) and 0 <= i <= len(left)
                and 0 <= j <= len(right)):
            raise ValueError("invalid cursor")
        out = []
        while i < len(left):
            if diagonal:
                j = max(i, j)
            while j < len(right):
                a, b = left[i], right[j]
                if self.contains(rid, a, b):
                    if len(out) == limit:
                        return dict(pairs=out, next_cursor=[i, j], total=r.combinations)
                    out.append([a, b])
                j += 1
            i, j = i + 1, 0
        return dict(pairs=out, next_cursor=None, total=r.combinations)

    def stats(self):
        relations = list(self.relations.values())
        gaps = Counter()
        for act in self.acts.values():
            gaps.update(f"links_{x.kind}" for x in act.links)
            gaps["truncated_activations"] += bool(act.truncated)
            for st in act.steps:
                if st.op.kind in ("read", "write", "publish", "rmw", "ref_inc", "ref_dec"):
                    gaps["unnamed_accesses"] += not bool(st.op.key)
                    gaps["unknown_base_accesses"] += not st.base or any(t[0] == "unknown" for t in st.base)
        return dict(
            activations=len(self.acts), steps=sum(len(a.steps) for a in self.acts.values()),
            members=len(self.members), pools=len(self.pools), relations=len(relations),
            duplicate_access_records_collapsed=self.duplicate_access_records,
            relations_by_kind=dict(Counter(r.kind for r in relations)),
            implicit_combinations=sum(r.combinations for r in relations),
            implicit_by_kind={k: sum(r.combinations for r in relations if r.kind == k)
                              for k in sorted({r.kind for r in relations})},
            self_combinations=sum(r.self_combinations for r in relations),
            boundaries=len(self.boundaries), static_gaps=dict(gaps),
            review=dict(individually_reviewed=0, conditionally_covered=0,
                        unreviewed=sum(r.combinations for r in relations)),
            complete=False, model_calls=0,
            counting_unit="conflict: access pair; sequence: seed/region member x interfering entry",
            scope="all members in bounded activations; not all program executions or defect recall",
        )


def build_relations(acts):
    """No pair enumeration, representative selection, or lock-based pruning."""
    ix = RelationIndex(acts)
    acc = defaultdict(lambda: defaultdict(list))
    writers = defaultdict(lambda: defaultdict(set))
    deps = {}
    for aid, act in ix.acts.items():
        deps[aid] = Dependencies(act)
        for st in act.steps:
            accesses = step_keys(st, act.steps)
            ix.duplicate_access_records += len(accesses) - len(set(accesses))
            for key, mode in sorted(set(accesses)):
                acc[key][mode].append(ix.member(aid, (st.sid,)))
                if mode in WRITERS:
                    writers[key][aid].add(st.sid)
    for key in sorted(acc):
        modes = sorted(acc[key])
        pools = {m: ix.pool(acc[key][m]) for m in modes}
        for i, ma in enumerate(modes):
            for mb in modes[i:]:
                if conflicting(ma, mb):
                    ix.add("conflict", (key,), pools[ma], pools[mb], symmetric=True)

    # A sequence family references shared pools; writer-entry contexts are built
    # once per key set, never once per left seed or pair.
    families = defaultdict(list)
    for aid, act in ix.acts.items():
        dep = deps[aid]
        writes = defaultdict(list)
        for st in act.steps:
            if step_mode(st) in WRITERS:
                writes[st.op.key].append(st.sid)
        for st in act.steps:
            if st.op.kind != "read" or not st.op.key:
                continue
            key, seed = st.op.key, st.sid
            c = dep.closure(seed)
            later = writes[key][bisect_right(writes[key], seed):]
            if later or c.writes:
                core = (seed, *later, *c.writes)
                member = ix.member(aid, (*core, *c.steps), core, c.bounds)
                families[("atomicity", (key,))].append(member)
            for branch in c.branches:
                region = dep.guarded_region(branch, key)
                if region:
                    # Retain the value-dependent part too. Otherwise extending
                    # the closure could REMOVE a guard previously found beyond
                    # the legacy closure cap. Overlap with lifetime is explicit.
                    seq = (seed, branch, *region, *c.steps)
                    families[("guard", (key,))].append(ix.member(aid, seq, bounds=c.bounds))
            hidden = [sid for kind, sid, _ in c.bounds if kind in UNSEEN_USE and sid is not None]
            if c.derefs or hidden:
                keys = tuple(sorted({key, "*" + key, *("*" + d for _, d in c.derived)}))
                member = ix.member(aid, (seed, *c.steps), (seed, *c.derefs, *hidden), c.bounds)
                families[("lifetime", keys)].append(member)
    interfering = {}
    for (kind, keys), members in sorted(families.items()):
        if keys not in interfering:
            by_entry = defaultdict(set)
            for key in keys:
                for aid, sids in writers.get(key, {}).items():
                    by_entry[aid].update(sids)
            ids = []
            for aid, sids in sorted(by_entry.items()):
                seq, bounds = deps[aid].interfering(sorted(sids), keys)
                ids.append(ix.member(aid, seq, tuple(sorted(sids)), bounds))
            interfering[keys] = ix.pool(ids)
        ix.add(kind, keys, ix.pool(members), interfering[keys])
    return ix
