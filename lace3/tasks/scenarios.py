"""Concurrency scenario views over complete, factorized relations.

Object provenance organizes ownership; it never proves aliasing or safety.
Each original left member is assigned exactly once. The full right pool and
the ORIGINAL triangular order are preserved, including cross-scenario pairs.
No concrete interaction pair is enumerated here.
"""

from collections import Counter, defaultdict
from dataclasses import dataclass, field, replace
import hashlib
import json


def stable_id(prefix, value):
    raw = json.dumps(value, sort_keys=True, ensure_ascii=False, separators=(",", ":"))
    return prefix + hashlib.sha256(raw.encode()).hexdigest()[:20]


@dataclass(frozen=True)
class Anchor:
    id: str
    roots: tuple
    label: str
    uncertain: bool


@dataclass
class View:
    id: str
    relation: str
    scenario: str
    left_members: tuple
    right_pool: str
    triangular: bool
    combinations: int = 0
    cross_scenario_combinations: int = 0


@dataclass
class Scenario:
    id: str
    anchor: Anchor
    views: list = field(default_factory=list)
    kinds: set = field(default_factory=set)


class Origins:
    """Trace load origins with an iterative worklist; keep alternatives apart.

    Parameters stay entry-qualified. This intentionally avoids equating two
    unrelated callers merely because both have a `list_head` parameter. Known
    globals/allocation sites can share an anchor. Secondary roots remain in
    the underlying bindings and sequences; membership is never pruned.
    """
    def __init__(self, ix):
        self.ix, self.cache = ix, {}

    def site(self, aid, sid):
        op = self.ix.acts[aid].steps[sid].op
        return (op.module, op.function, getattr(op, "local_id", getattr(op, "id", sid)))

    def describe(self, aid, sid):
        act, st = self.ix.acts[aid], self.ix.acts[aid].steps[sid]
        tags = st.origin if st.op.kind == "free" else st.base
        roots, loads = set(), []
        entry = getattr(act, "name", None) or getattr(act, "entry", aid)
        if not isinstance(entry, str):
            entry = aid
        # The containing member path distinguishes embedded objects while
        # grouping the fields of one container. It remains an organization hint.
        path = st.path or st.op.state or st.op.key or ""
        role = path.rsplit(".", 1)[0] if "." in path else ""
        for tag in sorted(tags, key=str):
            kind = tag[0]
            if kind in ("arg", "param"):
                roots.add(("parameter", act.steps[0].op.module, entry, tag[1], role))
            elif kind == "global":
                roots.add(("global", st.op.module, tag[1], role))
            elif kind == "load" and tag[1] is not None and 0 <= tag[1] < len(act.steps):
                loads.append(tag[1])
            elif kind == "fresh" and tag[1] is not None and 0 <= tag[1] < len(act.steps):
                roots.add(("allocation", *self.site(aid, tag[1])))
            elif kind == "ret" and tag[2] is not None and 0 <= tag[2] < len(act.steps):
                roots.add(("return", *self.site(aid, tag[2]), tag[1]))
            elif kind == "null":
                roots.add(("null",))
            else:
                roots.add(("unknown", *self.site(aid, sid), kind))
        if not roots and not loads:
            roots.add(("unknown", *self.site(aid, sid), "no-origin"))
        return roots, loads

    def resolve(self, aid, sid):
        if (aid, sid) in self.cache:
            return self.cache[aid, sid]
        # Explicit stack also handles long pointer chains without a recursion cap.
        stack, active = [(sid, None)], set()
        while stack:
            current, data = stack.pop()
            key = (aid, current)
            if key in self.cache:
                continue
            if data is None:
                roots, loads = self.describe(aid, current)
                active.add(current)
                stack.append((current, (roots, loads)))
                for other in reversed(loads):
                    if other not in active and (aid, other) not in self.cache:
                        stack.append((other, None))
            else:
                roots, loads = data
                for other in loads:
                    roots.update(self.cache.get((aid, other),
                        (("unknown", *self.site(aid, other), "cyclic-origin"),)))
                self.cache[key] = tuple(sorted(roots, key=str))
                active.discard(current)
        return self.cache[aid, sid]


class ScenarioIndex:
    def __init__(self, ix):
        self.ix = ix
        self.scenarios, self.views, self.owner = {}, {}, {}
        self.possible_origins = {}

    def validate(self):
        """Structural proof of partition conservation, with no pair expansion."""
        by_relation = defaultdict(list)
        for view in self.views.values():
            by_relation[view.relation].append(view)
        for rid, relation in self.ix.relations.items():
            expected = self.ix.pools[relation.left].member_set
            seen = set()
            for v in by_relation[rid]:
                members = set(v.left_members)
                if len(members) != len(v.left_members) or seen & members:
                    raise ValueError(f"{rid}: duplicate left ownership")
                if v.right_pool != relation.right or v.triangular != (
                        relation.symmetric and relation.left == relation.right):
                    raise ValueError(f"{rid}: original pairing predicate changed")
                if any(self.owner[m] != v.scenario for m in members):
                    raise ValueError(f"{rid}: incorrect owner")
                seen.update(members)
            if seen != expected:
                raise ValueError(f"{rid}: missing or extra left members")
            if sum(v.combinations for v in by_relation[rid]) != relation.combinations:
                raise ValueError(f"{rid}: combination count changed")
        return dict(relations_checked=len(by_relation), views_checked=len(self.views),
                    partition_complete=True, pair_enumeration=False,
                    implicit_combinations=sum(v.combinations for v in self.views.values()))

    def stats(self):
        active = [s for s in self.scenarios.values() if s.views]
        return dict(scenarios=len(active), views=len(self.views),
                    uncertain_anchors=sum(s.anchor.uncertain for s in active),
                    implicit_combinations=sum(v.combinations for v in self.views.values()),
                    cross_scenario_combinations=sum(v.cross_scenario_combinations for v in self.views.values()),
                    members_with_owner=len(self.owner), reviewed_combinations=0)


def build_scenarios(ix):
    out, origins = ScenarioIndex(ix), Origins(ix)
    relevant = set()
    for r in ix.relations.values():
        relevant.update(ix.pools[r.left].members)
        relevant.update(ix.pools[r.right].members)
    for mid in sorted(relevant):
        m = ix.members[mid]
        seed = (m.core or m.steps)[0]
        roots = origins.resolve(m.aid, seed)
        out.possible_origins[mid] = roots
        # Ownership does not require proving a unique object identity. Retain
        # every alternative separately, but don't fragment a known object role
        # into a new scene for each unresolved load or nullable SSA value.
        known = tuple(r for r in roots if r[0] not in ("unknown", "null"))
        home = known or (("unknown", *origins.site(m.aid, seed), "no-known-object"),)
        anchor_id = stable_id("A", home)
        scene_id = stable_id("S", home)
        uncertain = len(roots) != 1 or any(r[0] in ("unknown", "return", "parameter", "null") for r in roots)
        if scene_id not in out.scenarios:
            label = " + ".join(":".join(map(str, r)) for r in home)
            out.scenarios[scene_id] = Scenario(scene_id, Anchor(anchor_id, home, label, uncertain))
        elif uncertain and not out.scenarios[scene_id].anchor.uncertain:
            out.scenarios[scene_id].anchor = replace(out.scenarios[scene_id].anchor, uncertain=True)
        out.owner[mid] = scene_id

    # Right-pool summaries are shared. Neither counting nor ownership builds pairs.
    pool_counts = {}
    for relation in ix.relations.values():
        right = ix.pools[relation.right]
        if right.id not in pool_counts:
            aids, scenes, both = Counter(), Counter(), Counter()
            for mid in right.members:
                aid, scene = ix.members[mid].aid, out.owner[mid]
                aids[aid] += 1
                scenes[scene] += 1
                both[scene, aid] += 1
            pool_counts[right.id] = (aids, scenes, both)
        aids, scenes, both = (c.copy() for c in pool_counts[right.id])
        remaining = len(right.members)
        triangular = relation.symmetric and relation.left == relation.right
        grouped = {}
        for mid in ix.pools[relation.left].members:
            scene, aid = out.owner[mid], ix.members[mid].aid
            if scene not in grouped:
                grouped[scene] = [[], 0, 0]
            group = grouped[scene]
            group[0].append(mid)
            eligible, same_scene = remaining, scenes[scene]
            if ix.acts[aid].reentrant == "no":
                eligible -= aids[aid]
                same_scene -= both[scene, aid]
            group[1] += eligible
            group[2] += eligible - same_scene
            if triangular:
                remaining -= 1
                aids[aid] -= 1
                scenes[scene] -= 1
                both[scene, aid] -= 1
        for scene, (members, count, cross) in sorted(grouped.items()):
            vid = stable_id("V", (relation.id, scene))
            view = View(vid, relation.id, scene, tuple(members), right.id, triangular, count, cross)
            out.views[vid] = view
            out.scenarios[scene].views.append(vid)
            out.scenarios[scene].kinds.add(relation.kind)
    out.validate()
    return out
