"""Targeted fact queries for one task.

These are the model's tools, and they are plain functions so offline tests
and scripted backends call the same code. Every answer comes from the IR
index of the analyzed program or from the sandboxed source tree; nothing
reads outside them. Each call is logged on the task.

`expand` is the clique-expansion step: it adds another activation to the
clique, attaches the known contexts that involve it, and returns its
sequence window, so a writer or freer the packet did not include can be
examined in the same session. Expansions are bounded per task.
"""

import re

from lace3.evidence.source import SourceError
from lace3.index.traces import provenance, tags_text
from lace3.tasks.items import step_keys

MAX_OUT = 5000                  # every tool result is resent on each later turn
_STEP_REF = re.compile(r"^(A\d+)\.s(\d+)$")
_SITE_REF = re.compile(r"^(.+?):(\d+)(?:-(\d+))?$")


def _cap(text):
    if len(text) <= MAX_OUT:
        return text
    return text[:MAX_OUT] + f"\n[output cut at {MAX_OUT} chars]"


class Facts:
    def __init__(self, run, clique, max_expansions=3):
        self.run = run                  # lace3.detect.runner.RunState
        self.clique = clique
        self.max_expansions = max_expansions
        self.log = []
        self.stage = "evidence"
        self.focus = list(clique.items)

    # --------------------------------------------------------------- helpers
    def _record(self, name, args, out):
        sent = _cap(out)
        self.log.append({"tool": name, "args": args, "chars": len(out), "sent": len(sent)})
        return sent

    def _act(self, ref):
        acts = self.run.acts
        if ref in acts:
            return acts[ref]
        for a in acts.values():
            if a.name == ref:
                return a
        return None

    # ----------------------------------------------------------------- source
    def read_source(self, path, start_line, end_line):
        try:
            rel, ls = self.run.tree.read(path, start_line, end_line, max_lines=120)
            out = f"{rel}:{start_line}-{end_line}\n" + "\n".join(f"{n:6} {l}" for n, l in ls)
        except SourceError as e:
            out = f"error: {e}"
        return self._record("read_source", [path, start_line, end_line], out)

    def grep_source(self, pattern, path_filter=""):
        try:
            hits, more = self.run.tree.grep(pattern, path_filter or None)
            out = "\n".join(f"{r}:{n}: {t[:200]}" for r, n, t in hits) or "no matches"
            if more:
                out += "\n[more matches not shown; narrow the pattern or path]"
        except SourceError as e:
            out = f"error: {e}"
        return self._record("grep_source", [pattern, path_filter], out)

    # --------------------------------------------------------------- IR facts
    def function_ops(self, function):
        fns = self.run.prog.functions_named(function)
        if not fns:
            return self._record("function_ops", [function], f"no function {function} in the IR")
        out = []
        r = self.run.renderer
        for fn in fns[:2]:
            fi = self.run.px.of(fn)
            out.append(f"{fn.name} ({r.site(fn.loc)}), {len(fi.ops)} ops:")
            for op in fi.ops[:200]:
                extra = op.state or op.callee or ""
                held = f" held=[{', '.join(op.held)}]" if op.held else ""
                out.append(f"  #{op.id} {op.kind} {extra} @{r.site(op.loc)}{held}")
            if len(fi.ops) > 200:
                out.append(f"  ... {len(fi.ops) - 200} more")
        return self._record("function_ops", [function], "\n".join(out))

    def callers(self, function):
        prog, r = self.run.prog, self.run.renderer
        out = []
        for fn in prog.functions:
            for c in fn.calls:
                if c.callee == function:
                    out.append(f"direct: {fn.name} at {r.site(c.loc)}")
                elif any(name == function for _, name in c.fn_args):
                    out.append(f"passed as argument to {c.callee or 'indirect call'} in "
                               f"{fn.name} at {r.site(c.loc)}")
            for s in fn.fn_stores:
                if getattr(s, "function", None) == function:
                    where = f"{s.struct}.{s.field}" if s.struct else s.dest
                    out.append(f"stored: into {where} in {fn.name} at {r.site(s.loc)}")
        for g in prog.global_refs():
            if g.function == function:
                out.append(f"table: {g.global_name}.{g.path} ({g.owner_struct or '?'})")
        for a in self.run.acts.values():
            if a.name == function:
                out.append(f"entry {a.aid}: " + "; ".join(provenance(a.entry)))
        return self._record("callers", [function], "\n".join(out[:80]) or "no callers in the IR")

    def accesses(self, key, limit=60):
        """Every step on a field key (or `*key` pointee) across all activations."""
        out = []
        r = self.run.renderer
        for a in self.run.acts.values():
            for st in a.steps:
                for k, mode in step_keys(st, a.steps):
                    if k == key:
                        out.append(f"{a.aid}.s{st.sid} {a.name}: {mode} {st.op.kind} "
                                   f"base={tags_text(st.base, a.steps)} @{r.site(st.op.loc)}"
                                   + (f" held=[{', '.join(st.held)}]" if st.held else ""))
        more = len(out) - limit
        text = "\n".join(out[:limit]) or f"no step touches {key}"
        if more > 0:
            text += f"\n... {more} more"
        return self._record("accesses", [key], text)

    def entry_info(self, entry):
        a = self._act(entry)
        if a is None:
            return self._record("entry_info", [entry], f"no activation {entry}")
        r = self.run.renderer
        out = [f"{a.aid} {a.name} ({r.site(a.function.loc)}) reentrant={a.reentrant} "
               f"{a.reentrant_why}", f"{len(a.steps)} steps, {len(a.sections)} critical sections"
               + (", TRUNCATED" if a.truncated else "")]
        out += [r.shorten(p) for p in provenance(a.entry)]
        for l in a.links[:30]:
            out.append(f"link {l.kind}: {l.function} -> {l.callee or '?'} at {r.shorten(l.site)}"
                       + (f" ({l.detail})" if l.detail else ""))
        return self._record("entry_info", [entry], "\n".join(out))

    def steps(self, activation, first, last):
        a = self._act(activation)
        if a is None:
            return self._record("steps", [activation, first, last], f"no activation {activation}")
        first, last = max(0, int(first)), min(len(a.steps) - 1, int(last), int(first) + 80)
        out = [self.run.renderer.step_text(a, a.steps[s]) for s in range(first, last + 1)]
        return self._record("steps", [activation, first, last], "\n".join(out) or "empty range")

    def item_contexts(self, item):
        it = self.run.items.get(item)
        if it is None:
            return self._record("item_contexts", [item], f"no item {item}")
        out = [f"{it.id} {it.kind}: {self.run.renderer.shorten(it.summary)}",
               f"{it.n_contexts} contexts seen, {len(it.contexts)} recorded:"]
        for c in it.contexts:
            out.append(f"  {c.a} ({self.run.acts[c.a].name}) "
                       f"{','.join('s%d' % s for s in c.a_steps)} vs {c.b} "
                       f"({self.run.acts[c.b].name}) {','.join('s%d' % s for s in c.b_steps)}"
                       + ("  self" if c.self_interference else ""))
        return self._record("item_contexts", [item], "\n".join(out))

    # -------------------------------------------------------------- expansion
    def expand(self, activation, reason=""):
        a = self._act(activation)
        if a is None:
            return self._record("expand", [activation, reason], f"no activation {activation}")
        c = self.clique
        if a.aid in c.participants:
            return self._record("expand", [activation, reason], f"{a.aid} is already a participant")
        used = sum(1 for e in c.expansions if e.get("kind") == "activation")
        if used >= self.max_expansions:
            return self._record("expand", [activation, reason],
                                f"expansion budget ({self.max_expansions}) used up for this task")
        c.participants.append(a.aid)
        focus, added = set(), 0
        for iid in c.items:
            it = self.run.items[iid]
            shown = c.task_contexts.setdefault(iid, [])
            for ctx in it.contexts:
                if a.aid in ctx.participants and ctx not in shown:
                    shown.append(ctx)
                    added += 1
                    focus.update(ctx.a_steps if ctx.a == a.aid else ())
                    focus.update(ctx.b_steps if ctx.b == a.aid else ())
        if not focus:
            fams = c.families
            for st in a.steps:
                if any(k.lstrip("*") in fams for k, _ in step_keys(st, a.steps)):
                    focus.add(st.sid)
        c.expansions.append({"kind": "activation", "aid": a.aid, "reason": reason,
                             "contexts_added": added})
        r = self.run.renderer
        sids, elided = r.window(a, focus) if focus else (list(range(min(30, len(a.steps)))), 0)
        out = [f"added {a.aid} {a.name} reentrant={a.reentrant}; {added} item contexts attached"]
        out += [r.shorten(p) for p in provenance(a.entry)[:3]]
        out += [r.step_text(a, a.steps[s]) for s in sids]
        if elided:
            out.append(f"({elided} steps elided)")
        return self._record("expand", [activation, reason], "\n".join(out))

    # ------------------------------------------------------------- citations
    def valid_citation(self, ref):
        ref = ref.strip()
        m = _STEP_REF.match(ref)
        if m:
            a = self.run.acts.get(m.group(1))
            return a is not None and int(m.group(2)) < len(a.steps)
        m = _SITE_REF.match(ref)
        if m:
            rel = self.run.tree.resolve(m.group(1))
            if rel is None:
                return False
            try:
                return int(m.group(2)) <= len(self.run.tree.lines(rel))
            except SourceError:
                return False
        return False
