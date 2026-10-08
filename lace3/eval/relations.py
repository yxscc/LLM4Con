"""Offline acceptance measurements. Input is IR/source only, never bug labels.

--compare-legacy streams EVERY old context through a coverage check. Pair
enumeration here is an evaluator oracle, not part of the new index builder.
"""

import argparse
from collections import Counter, defaultdict
import hashlib
import json
from pathlib import Path
import resource
import time

from lace3 import relational
from lace3.tasks.items import _Builder


class LegacyCoverage:
    def __init__(self, ix):
        self.ix = ix
        self.conflicts = defaultdict(list)
        self.sequences = defaultdict(list)
        self.singletons = {(m.aid, m.steps[0]): m.id for m in ix.members.values()
                           if len(m.steps) == 1 and m.core == m.steps and not m.boundaries}
        for r in ix.relations.values():
            if r.kind == "conflict":
                self.conflicts[r.keys[0]].append(r)
            else:
                for mid in ix.pools[r.left].members:
                    m = ix.members[mid]
                    seed = m.steps[0]
                    key = ix.acts[m.aid].steps[seed].op.key
                    self.sequences[(r.kind, key, m.aid, seed)].append((r, m))
        self.right_by_entry = {
            p.id: {ix.members[m].aid: ix.members[m] for m in p.members}
            for p in ix.pools.values()}
        self._left_cache, self._right_cache = {}, {}
        self.counts, self.sigs, self.missing = Counter(), set(), []
        self.missing_count = 0

    def covers(self, kind, key, ctx):
        ix = self.ix
        if kind == "conflict":
            a = self.singletons.get((ctx.a, ctx.a_steps[0]))
            b = self.singletons.get((ctx.b, ctx.b_steps[0]))
            return a is not None and b is not None and any(
                ix.contains(r.id, a, b) for r in self.conflicts[key])
        sig = (kind, key, ctx.a, ctx.a_steps)
        if sig not in self._left_cache:
            self._left_cache[sig] = [r for r, m in self.sequences[(kind, key, ctx.a, ctx.a_steps[0])]
                                     if set(ctx.a_steps) <= set(m.steps)]
        for r in self._left_cache[sig]:
            bsig = (r.right, ctx.b, ctx.b_steps)
            if bsig not in self._right_cache:
                m = self.right_by_entry[r.right].get(ctx.b)
                self._right_cache[bsig] = bool(m and set(ctx.b_steps) <= set(m.steps))
            if self._right_cache[bsig] and (ctx.a != ctx.b or ix.acts[ctx.a].reentrant != "no"):
                return True
        return False

    def add(self, sig, kind, keys, summary, prio, ctx):
        self.counts[kind] += 1
        self.sigs.add(sig)
        if not self.covers(kind, keys[0], ctx):
            self.missing_count += 1
            if len(self.missing) < 10:
                self.missing.append(dict(sig=sig, context=vars(ctx)))

    def run(self):
        t = time.monotonic()
        b = _Builder(list(self.ix.acts.values()))
        b.add = self.add
        b.conflicts()
        b.sequences()
        return dict(legacy_items=len(self.sigs), legacy_contexts=sum(self.counts.values()),
                    contexts_by_kind=dict(self.counts), missing=self.missing_count,
                    missing_examples=self.missing, seconds=time.monotonic() - t)


def main(argv=None):
    ap = argparse.ArgumentParser(__doc__)
    ap.add_argument("--ir", required=True, action="append")
    ap.add_argument("--src", required=True)
    ap.add_argument("--out", required=True)
    ap.add_argument("--compare-legacy", action="store_true")
    ap.add_argument("--write-index", action="store_true")
    args = ap.parse_args(argv)
    ix, timings = relational.prepare(args.ir)
    result = dict(timings_s=timings, statistics=ix.stats())
    package = Path(__file__).resolve().parents[1]
    digest = hashlib.sha256()
    for p in sorted(package.rglob("*.py")):
        if p.name.startswith("._"):
            continue
        digest.update(str(p.relative_to(package)).encode() + b"\0" + p.read_bytes() + b"\0")
    result["code_sha256"] = digest.hexdigest()
    if args.write_index:
        t = time.monotonic()
        relational.write(ix, str(Path(args.out).with_suffix("")) + "-index", args.src,
                         timings, dict(max_depth=8, max_steps=4000))
        result["serialization_s"] = time.monotonic() - t
    if args.compare_legacy:
        result["legacy_coverage"] = LegacyCoverage(ix).run()
    result["peak_rss_kib_linux"] = resource.getrusage(resource.RUSAGE_SELF).ru_maxrss
    Path(args.out).parent.mkdir(parents=True, exist_ok=True)
    Path(args.out).write_text(json.dumps(result, indent=2) + "\n")
    print(json.dumps(result, separators=(",", ":")), flush=True)
    return int(bool(result.get("legacy_coverage", {}).get("missing")))


if __name__ == "__main__":
    raise SystemExit(main())
