"""Uncapped dependency closures over a bounded activation.

The activation's textual order is used only to collect candidate sequences.
This is neither a path-feasibility nor a happens-before analysis. A reverse
use index avoids rescanning the entire activation for every seed read.
"""

from bisect import bisect_right
from collections import defaultdict
import heapq

from lace3.tasks.items import ACCESS_KINDS, Closure, step_keys


class Dependencies:
    def __init__(self, act):
        self.act = act
        self.users = defaultdict(list)
        self.reads = defaultdict(list)
        self.cuts = {x.step: x for x in act.links if x.step is not None}
        self.cache = {}
        for st in act.steps:
            for tag in st.base | st.origin | frozenset(t for a in st.args for t in a):
                self.users[tag].append(st.sid)
            if st.op.kind == "read":
                self.reads[(st.op.key, st.function)].append(st.sid)

    def closure(self, seed):
        if seed in self.cache:
            return self.cache[seed]
        tracked, queued, heap = set(), set(), []

        def track(tag, after):
            tracked.add(tag)
            users = self.users.get(tag, ())
            for sid in users[bisect_right(users, after):]:
                if sid not in queued:
                    queued.add(sid)
                    heapq.heappush(heap, sid)

        track(("load", seed), seed)
        c = Closure([], [], [], [], [], [])
        while heap:
            sid = heapq.heappop(heap)
            st = self.act.steps[sid]
            base, val = bool(st.base & tracked), bool(st.origin & tracked)
            arg = any(a & tracked for a in st.args)
            k = st.op.kind
            c.steps.append(sid)
            if k in ACCESS_KINDS and base:
                c.derefs.append(sid)
                if not st.op.key:
                    c.bounds.append(("unnamed", sid, "access through a tracked value has no field name"))
            if k == "free" and val:
                c.derefs.append(sid)
            if k == "read" and base:
                track(("load", sid), sid)
                if st.op.key:
                    c.derived.append((sid, st.op.key))
            if k in ("write", "publish", "rmw") and val:
                c.writes.append(sid)
                c.bounds.append(("stored", sid, f"value stored into {st.op.key or 'memory'}; reload aliasing unresolved"))
            if k == "branch" and val:
                c.branches.append(sid)
            if k == "ext" and arg:
                c.bounds.append(("external", sid, f"passed to undefined {st.op.callee}"))
            if k == "icall" and (arg or val):
                c.bounds.append(("indirect", sid, "flows into an indirect call"))
            if k in ("call", "ext", "icall") and arg:
                track(("ret", st.op.callee or "<indirect>", sid), sid)
                cut = self.cuts.get(sid)
                if k == "call" and cut and cut.kind in ("depth", "recursion", "unresolved"):
                    c.bounds.append((cut.kind, sid, f"{st.op.callee} not expanded ({cut.kind})"))
        if self.act.truncated:
            c.bounds.append(("truncated", None, "bounded activation; dependency may continue outside it"))
        self.cache[seed] = c
        return c

    def guarded_region(self, branch, key):
        frames = self.act.steps[branch].frames
        out = []
        for st in self.act.steps[branch + 1:]:
            if st.frames[:len(frames)] != frames:
                break
            if any(k.lstrip("*") != key for k, _ in step_keys(st, self.act.steps)):
                out.append(st.sid)
        return out

    def interfering(self, sids, keys):
        """All writers and their take/clear/free dependencies, without sampling."""
        out, bounds = set(sids), set()
        for sid in sids:
            st = self.act.steps[sid]
            seeds = []
            if st.op.kind == "free":
                seeds = [t[1] for t in st.origin if t[0] == "load" and t[1] is not None]
            elif st.op.key in keys:
                reads = self.reads.get((st.op.key, st.function), ())
                at = bisect_right(reads, sid)
                seeds = reads[at - 1:at] if at else []
            for seed in seeds:
                c = self.closure(seed)
                out.add(seed)
                out.update(c.steps)
                bounds.update(c.bounds)
        return tuple(sorted(out)), tuple(sorted(bounds, key=str))
