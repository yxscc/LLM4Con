"""Offline source-reuse scheduling. A session is a plan, not a model request.

No default size cap. An optional explicit SOURCE BYTE budget is a planning
constraint, not a token estimate. Oversized scenes can split by relation view;
an indivisible oversized view remains pending, with all its members retained.
"""

from dataclasses import dataclass, field


@dataclass(frozen=True)
class Unit:
    id: str
    scene: str
    views: tuple
    files: frozenset
    priority: int


@dataclass
class Session:
    id: str
    units: list = field(default_factory=list)
    scenes: list = field(default_factory=list)
    views: list = field(default_factory=list)
    files: set = field(default_factory=set)
    source_bytes: int = 0
    missing_sources: set = field(default_factory=set)


@dataclass
class Schedule:
    sessions: list
    pending: dict
    source_byte_budget: int | None

    def validate(self, plan):
        seen = set()
        for session in self.sessions:
            ids = set(session.views)
            if len(ids) != len(session.views) or seen & ids:
                raise ValueError("duplicate scheduled view")
            seen.update(ids)
            if self.source_byte_budget is not None and session.source_bytes > self.source_byte_budget:
                raise ValueError("session exceeds source byte budget")
        if seen & self.pending.keys() or seen | self.pending.keys() != plan.views.keys():
            raise ValueError("views must be scheduled once or explicitly pending")
        scheduled = sum(plan.views[v].combinations for v in seen)
        pending = sum(plan.views[v].combinations for v in self.pending)
        if scheduled + pending != sum(r.combinations for r in plan.ix.relations.values()):
            raise ValueError("scheduled/pending coverage changed")
        return dict(scheduled_views=len(seen), pending_views=len(self.pending),
                    scheduled_combinations=scheduled, pending_combinations=pending,
                    review_complete=False, model_calls=0)


def schedule(plan, footprints, source_byte_budget=None):
    if source_byte_budget is not None and source_byte_budget <= 0:
        raise ValueError("source byte budget must be positive")
    catalog, pending, units = footprints.catalog, {}, {}
    priorities = {"lifetime": 0, "atomicity": 1, "guard": 2, "conflict": 3}
    for scene in sorted(plan.scenarios.values(), key=lambda s: s.id):
        if not scene.views:
            continue
        files = footprints.scene_files[scene.id]
        if source_byte_budget is None or catalog.cost(files) <= source_byte_budget:
            portions = [tuple(scene.views)]
        else:
            portions = [(v,) for v in scene.views]
        for number, views in enumerate(portions):
            files = frozenset().union(*(footprints.view_files[v] for v in views))
            reason = None
            if not files:
                reason = "no readable source for this view"
            elif source_byte_budget is not None and catalog.cost(files) > source_byte_budget:
                reason = "complete source footprint exceeds explicit byte budget; no truncation performed"
            if reason:
                for view in views:
                    pending[view] = reason
                continue
            uid = f"{scene.id}.{number}"
            priority = min(priorities[plan.ix.relations[plan.views[v].relation].kind] for v in views)
            units[uid] = Unit(uid, scene.id, views, files, priority)

    sessions = []
    while units:
        seed = min(units, key=lambda uid: (units[uid].priority, uid))
        session = Session(f"Q{len(sessions)}")

        def add(uid):
            unit = units.pop(uid)
            session.units.append(uid)
            if unit.scene not in session.scenes:
                session.scenes.append(unit.scene)
            session.views.extend(unit.views)
            session.files.update(unit.files)
            for vid in unit.views:
                session.missing_sources.update(footprints.view_missing[vid])
            session.source_bytes = catalog.cost(session.files)

        add(seed)
        while units:
            choices = []
            for uid, unit in units.items():
                overlap = catalog.cost(session.files & unit.files)
                if not overlap:
                    continue
                added = catalog.cost(unit.files - session.files)
                if source_byte_budget is None or session.source_bytes + added <= source_byte_budget:
                    choices.append((-overlap, added, unit.priority, uid))
            if not choices:
                break
            add(min(choices)[-1])
        sessions.append(session)
    result = Schedule(sessions, pending, source_byte_budget)
    result.validate(plan)
    return result
