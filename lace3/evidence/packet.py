"""Evidence packets: what the model sees for one analysis clique.

A packet is facts, not conclusions. For every participant it shows the
entry's provenance and reentrancy, a window of its operation sequence around
the clique's items (the item steps, the loads they depend on, the bounds of
the critical sections they sit in, and the lock / free / publish / wait /
indirect-call steps in between), the links the expansion could not follow,
and the source of the functions involved with the steps marked.

Held locks and critical sections are printed as support facts. Sequence
order is the textual order of one activation and says nothing about what a
concurrent activation does in between.

When the character budget runs out, later source excerpts are dropped and
the packet is flagged `incomplete_context`; the model can still read them
through the tools.
"""

import re
from dataclasses import dataclass, field

from lace3.index.traces import lock_key, provenance, tags_text
from lace3.evidence.source import SourceError

DEFAULT_MAX_CHARS = 30000
DEFAULT_MAX_STEPS = 120
DEFAULT_MAX_FN_LINES = 160
STRUCTURAL = frozenset({"lock", "unlock", "wait", "wake", "free", "alloc", "ref_inc", "ref_dec",
                        "publish", "icall", "assert_held"})
_ABS_SITE = re.compile(r"(/[^\s:()\[\],]+):(\d+)")


@dataclass
class Packet:
    task: str
    text: str
    incomplete_context: bool = False
    omitted: list = field(default_factory=list)
    functions: list = field(default_factory=list)      # [(rel file, function, first, last)]
    steps_shown: dict = field(default_factory=dict)    # aid -> [sid]

    @property
    def chars(self):
        return len(self.text)


class PacketRenderer:
    def __init__(self, tree, acts, prog, max_chars=DEFAULT_MAX_CHARS,
                 max_steps=DEFAULT_MAX_STEPS, max_fn_lines=DEFAULT_MAX_FN_LINES):
        self.tree = tree
        self.acts = {a.aid: a for a in acts}
        self.prog = prog
        self.max_chars, self.max_steps, self.max_fn_lines = max_chars, max_steps, max_fn_lines
        self._basenames = None
        self.fns = {}
        for f in prog.functions:
            self.fns.setdefault(f.name, []).append(f)

    # ------------------------------------------------------------ text helpers
    def rel(self, path):
        r = self.tree.resolve(path)
        return r if r else path

    def short(self, path):
        """Basename when it is unique in the source tree, else the relative path."""
        rel = self.tree.resolve(path)
        if rel is None:
            return path.rsplit("/", 1)[-1]
        if self._basenames is None:
            self._basenames = {}
            for f in self.tree.files():
                b = f.rsplit("/", 1)[-1]
                self._basenames[b] = self._basenames.get(b, 0) + 1
        b = rel.rsplit("/", 1)[-1]
        return b if self._basenames.get(b) == 1 else rel

    def site(self, loc):
        if loc is None:
            return "?"
        s = f"{self.short(loc.file)}:{loc.line}"
        if loc.via:
            f, _, ln = loc.via.rpartition(":")
            s += f" [inlined {self.short(f)}:{ln}]"
        return s

    def shorten(self, text):
        return _ABS_SITE.sub(lambda m: f"{self.short(m.group(1))}:{m.group(2)}", text)

    def function(self, name, module=None):
        cands = self.fns.get(name, [])
        for f in cands:
            if module is None or f.module.name == module:
                return f
        return cands[0] if cands else None

    def step_text(self, act, st, marks=(), show_fn=True):
        op, steps = st.op, act.steps
        k = op.kind
        if k in ("lock", "unlock"):
            what = f"{lock_key(st, steps)}" + (f" ({op.mode})" if op.mode and k == "lock" else "")
        elif k in ("read", "write", "publish", "rmw", "ref_inc", "ref_dec"):
            what = f"{op.state or '<unnamed>'} of {tags_text(st.base, steps)}"
            if k in ("write", "publish") and st.origin:
                what += f" := {tags_text(st.origin, steps)}"
            if op.atomic:
                what += " (atomic)"
            if op.volatile:
                what += " (volatile)"
        elif k == "free":
            what = f"{op.callee}({tags_text(st.origin, steps)})"
        elif k in ("call", "ext"):
            what = f"{op.callee}(" + ", ".join(tags_text(a, steps) for a in st.args) + ")"
        elif k == "icall":
            what = f"*({tags_text(st.origin, steps)})(" + ", ".join(
                tags_text(a, steps) for a in st.args) + ")"
        elif k == "alloc":
            what = f"{op.callee}() -> fresh:s{st.sid}"
        else:
            what = op.callee or op.state or ""
        mk = f"  <{','.join(marks)}>" if marks else ""
        if show_fn:
            held = f"  held=[{', '.join(st.held)}]" if st.held else ""
            return f"  s{st.sid:<4} {st.function}: {k} {what}  @{self.site(op.loc)}{held}{mk}"
        cs = f"  [{','.join('cs%d' % c for c in st.sections)}]" if st.sections else ""
        ind = "  " * st.depth
        return f"  s{st.sid:<4} {ind}{k} {what}  @{self.site(op.loc)}{cs}{mk}"

    # ---------------------------------------------------------- step windows
    def window(self, act, focus, max_steps=None, dep_levels=2):
        steps = act.steps
        if not focus:
            return [], 0
        keep = set(focus)
        frontier = set(focus)
        for _ in range(dep_levels):
            nxt = set()
            for sid in frontier:
                st = steps[sid]
                for tags in (st.base, st.origin, *st.args):
                    for t in tags:
                        if t[0] in ("load", "fresh", "ret") and t[-1] is not None:
                            nxt.add(t[-1])
            nxt -= keep
            keep |= nxt
            frontier = nxt
        for sid in list(keep):
            for sec in steps[sid].sections:
                s = act.sections[sec]
                keep.add(s.start)
                if s.end is not None:
                    keep.add(s.end)
        lo, hi = min(keep), max(keep)
        extra = []
        for st in steps[lo:hi + 1]:
            if st.sid in keep:
                continue
            if st.op.kind in STRUCTURAL or (st.op.kind == "call" and st.sid + 1 < len(steps)
                                             and steps[st.sid + 1].depth > st.depth):
                extra.append(st.sid)
        must = sorted(keep)
        budget = (max_steps or self.max_steps) - len(must)
        if budget < len(extra):
            fl = sorted(focus)
            extra.sort(key=lambda s: min(abs(s - f) for f in fl))
            elided = len(extra) - max(budget, 0)
            extra = extra[:max(budget, 0)]
        else:
            elided = 0
        return sorted(set(must) | set(extra)), elided

    # ---------------------------------------------------------------- render
    def render(self, clique, items):
        """Render within the character budget: when the sequences alone do not
        fit, the per-participant step windows are halved (at most three times)
        before anything is cut."""
        scale = 1.0
        for _ in range(4):
            meta = self._render(clique, items, scale)
            if not meta.omitted or "packet text cut" not in meta.omitted:
                return meta
            scale /= 2
        return meta

    def _render(self, clique, items, scale):
        out, meta = [], Packet(clique.id, "")
        marks = {}                         # (aid, sid) -> [item ids]
        focus = {a: set() for a in clique.participants}
        for iid in clique.items:
            for c in clique.task_contexts.get(iid, ()):
                for aid, sids in ((c.a, c.a_steps), (c.b, c.b_steps)):
                    focus.setdefault(aid, set()).update(sids)
                    for s in sids:
                        m = marks.setdefault((aid, s), [])
                        if iid not in m:
                            m.append(iid)
        for e in clique.expansions:
            if e.get("kind") == "activation":
                focus.setdefault(e["aid"], set())
        out.append(f"# Task {clique.id}")
        out.append("Participants (each may run concurrently with the others; an entry may also run "
                   "concurrently with itself unless marked reentrant=no):")
        for aid in focus:
            act = self.acts[aid]
            loc = self.site(act.function.loc)
            out.append(f"- {aid}: {act.name} ({loc}) reentrant={act.reentrant}"
                       + (f" ({act.reentrant_why})" if act.reentrant_why else "")
                       + (" TRUNCATED" if act.truncated else ""))
            for p in provenance(act.entry)[:4]:
                out.append(f"    {self.shorten(p)}")
        out.append("")
        out.append("## Items to judge")
        for iid in clique.items:
            it = items[iid]
            out.append(f"[{iid}] {it.kind} keys={','.join(it.keys)}")
            out.append(f"    {self.shorten(it.summary)}")
            for c in clique.task_contexts.get(iid, ()):
                tag = "  (self-interference: two concurrent activations of the same entry)" \
                    if c.self_interference else ""
                out.append(f"    context: {c.a} steps {','.join('s%d' % s for s in c.a_steps)}"
                           f"  vs  {c.b} steps {','.join('s%d' % s for s in c.b_steps)}{tag}")
            if it.n_contexts > len(clique.task_contexts.get(iid, ())):
                out.append(f"    (seen in {it.n_contexts} contexts in total; others not shown)")
        out.append("")
        out.append("## Operation sequences (one activation each, textual program order; "
                   "held=[...] is a local lock scan, support only)")
        src_marks = {}                     # (rel, fn) -> {line: [labels]}
        for aid, fs in focus.items():
            act = self.acts[aid]
            share = max(12, int(self.max_steps * scale) // max(1, len(focus)))
            sids, elided = (self.window(act, fs, share, 2 if scale >= 0.5 else 1) if fs
                            else (list(range(min(len(act.steps), 30))), 0))
            meta.steps_shown[aid] = sids
            out.append(f"### {aid} {act.name} ({len(act.steps)} steps, "
                       f"{len(act.sections)} critical sections)")
            secs = sorted({c for sid in sids for c in act.steps[sid].sections})
            if secs:
                out.append("  critical sections (lock/unlock paired in this order; support only):")
                for c in secs:
                    s_ = act.sections[c]
                    end = f"s{s_.end}" if s_.end is not None else "no unlock seen"
                    out.append(f"    cs{c} = {s_.key} ({s_.mode}) s{s_.start}..{end}")
            prev, frame = None, None
            for sid in sids:
                if prev is not None and sid > prev + 1:
                    out.append(f"  ...  ({sid - prev - 1} steps)")
                st = act.steps[sid]
                if st.frames != frame:
                    call = st.frames[-1][1]
                    out.append(f"  in {st.function}" + (f" (called at {self.shorten(call)})"
                                                         if call else ""))
                    frame = st.frames
                out.append(self.step_text(act, st, marks.get((aid, sid), ()), show_fn=False))
                prev = sid
                if st.op.loc is not None:
                    rel = self.tree.resolve(st.op.loc.file)
                    if rel:
                        src_marks.setdefault((rel, st.function, st.op.module), {}).setdefault(
                            st.op.loc.line, []).append(f"{aid}.s{sid} {st.op.kind}")
            if prev is not None and prev < len(act.steps) - 1:
                out.append(f"  ...  ({len(act.steps) - 1 - prev} more steps)")
            if elided:
                out.append(f"  ({elided} structural steps elided by the window budget)")
                meta.omitted.append(f"{aid}: {elided} steps")
            lk = [l for l in act.links if not sids or l.step is None
                  or min(sids) <= l.step <= max(sids)]
            if lk:
                out.append("  unresolved / cut links:")
                for l in lk[:12]:
                    out.append(f"    {l.kind}: {l.function} -> {l.callee or '?'} at "
                               f"{self.shorten(l.site)}" + (f" ({l.detail})" if l.detail else "")
                               + (f" [s{l.step}]" if l.step is not None else ""))
                if len(lk) > 12:
                    out.append(f"    ... {len(lk) - 12} more")
            out.append("")
        body = "\n".join(out)
        src = self.sources(src_marks, meta, self.max_chars - len(body))
        meta.text = body + "\n## Source\n" + src if src else body
        if not src and src_marks:
            meta.incomplete_context = True
        if len(meta.text) > self.max_chars:
            meta.text = meta.text[:self.max_chars] + "\n[packet cut at the character budget]"
            meta.incomplete_context = True
            meta.omitted.append("packet text cut")
        return meta

    def sources(self, src_marks, meta, budget):
        parts = []
        order = sorted(src_marks.items(), key=lambda kv: -len(kv[1]))
        for (rel, fname, module), lines in order:
            fn = self.function(fname, module)
            if fn is None or fn.loc is None:
                continue
            try:
                span = self.tree.function_span(fn.loc.file, fn.loc.line)
                if span is None:
                    continue
                first, last = span
                _, ls = self.tree.read(rel, first, last, max_lines=10 ** 6)
            except SourceError as e:
                meta.omitted.append(f"{fname}: {e}")
                continue
            keep = set()
            if last - first + 1 <= self.max_fn_lines:
                keep = {n for n, _ in ls}
            else:
                keep |= set(range(first, first + 3))
                for ln in lines:
                    keep |= set(range(ln - 8, ln + 9))
            txt = [f"--- {rel}:{first}-{last}  {fname}"]
            prev = None
            for n, line in ls:
                if n not in keep:
                    continue
                if prev is not None and n > prev + 1:
                    txt.append("        ...")
                mk = f"   // <{'; '.join(lines[n][:4])}>" if n in lines else ""
                txt.append(f"{n:6} {line}{mk}")
                prev = n
            chunk = "\n".join(txt) + "\n"
            if len(chunk) > budget:
                meta.incomplete_context = True
                meta.omitted.append(f"source of {fname} ({rel}:{first}) not shown: budget")
                continue
            budget -= len(chunk)
            parts.append(chunk)
            meta.functions.append((rel, fname, first, last))
        return "\n".join(parts)
