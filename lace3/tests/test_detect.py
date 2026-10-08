"""v3 detection pipeline on the concurrency fixture, at -O0 and -O2.

The model is replaced by ScriptedBackend; packets, fact tools, verdict
normalization, the ledger and the report are the real ones.
"""

import json

import pytest

from lace3 import report
from lace3.detect.backend import ScriptedBackend
from lace3.detect.runner import Limits, detect, prepare
from lace3.evidence.source import SourceError, SourceTree
from lace3.tasks.items import build_items
from .conftest import FIXTURES, line_of

SRC = "conc_patterns.c"


def L(snippet, nth=1):
    return line_of(snippet, nth, SRC)


def run_state(ll, **kw):
    return prepare([ll], FIXTURES, Limits(**kw))


def aid_of(st, name):
    return next(a.aid for a in st.acts.values() if a.name == name)


def contexts(st, item):
    return {(st.acts[c.a].name, st.acts[c.b].name) for c in item.contexts}


def steps_at(st, aid, kind, line):
    return [s for s in st.acts[aid].steps
            if s.op.kind == kind and s.op.loc is not None and s.op.loc.line == line]


# ------------------------------------------------------------ recall shapes
def test_cross_function_take_release_use(conc_ll):
    """Pointer read from obj.buf in obj_consume, used in use_buf; obj_close
    clears obj.buf and frees the object in release_buf. Every field access is
    locked; the use and the free are in callees."""
    st = run_state(conc_ll)
    take = L("p = o->buf;", 1)
    life = [it for it in st.items.values()
            if it.kind == "lifetime" and it.keys == ("obj.buf", "*obj.buf")
            and f":{take}" in it.summary]
    assert len(life) == 1
    it = life[0]
    assert ("obj_consume", "obj_close") in contexts(st, it)
    a = aid_of(st, "obj_consume")
    ctx = next(c for c in it.contexts if c.a == a and st.acts[c.b].name == "obj_close")
    used = [st.acts[a].steps[s] for s in ctx.a_steps[1:]]
    use = L("return b->len;")
    # at -O2 use_buf is inlined: the step sits on the call line, the body line is `via`
    assert any(s.op.loc.line == use or (s.op.loc.via or "").endswith(f":{use}") for s in used)
    close = st.acts[ctx.b]
    kinds = {(close.steps[s].op.kind, close.steps[s].op.loc.line) for s in ctx.b_steps}
    assert ("free", L("kfree(b);")) in kinds or ("write", L("o->buf = 0;")) in kinds
    # the free-vs-use conflict on the pointee is its own item
    assert any(it.kind == "conflict" and it.keys == ("*obj.buf",)
               and f":{L('return b->len;')}" in it.summary and f":{L('kfree(b);')}" in it.summary
               for it in st.items.values())
    # all accesses of obj_consume on obj.buf hold the lock: no pruning happened
    assert all(st.acts[a].steps[s].held for s in ctx.a_steps[:1])
    # the context shown to the model is the one whose other side frees
    task = next(c for c in st.cliques if it.id in c.items)
    assert any(st.acts[c.b].name == "obj_close" for c in task.task_contexts[it.id])


def test_locked_accesses_unsafe_sequence(conc_ll):
    """obj_bump reads count in one critical section and writes it in another."""
    st = run_state(conc_ll)
    bump = aid_of(st, "obj_bump")
    at = [it for it in st.items.values() if it.kind == "atomicity" and it.keys == ("obj.count",)
          and f":{L('c = o->count;')}" in it.summary]
    assert len(at) == 1
    it = at[0]
    c = next(c for c in it.contexts if c.a == bump)
    seq = [st.acts[bump].steps[s] for s in c.a_steps]
    r = seq[0]
    w = next(s for s in seq if s.op.kind == "write")
    assert r.op.kind == "read" and r.held and w.held
    assert any(s.op.kind == "branch" for s in seq)      # `if (c < 8)` is part of the sequence
    assert set(r.sections).isdisjoint(w.sections)
    assert ("obj_bump", "obj_reset") in contexts(st, it)
    assert ("obj_bump", "obj_bump") in contexts(st, it)


def test_same_entry_self_interference(conc_ll):
    st = run_state(conc_ll)
    mark = aid_of(st, "obj_mark")
    w = L("o->state = 2;")
    self_items = [it for it in st.items.values() if any(
        c.a == c.b == mark for c in it.contexts)]
    assert any(it.kind == "conflict" and f":{w}" in it.summary for it in self_items)
    assert st.acts[mark].reentrant == "unknown"


def test_run_once_entry_has_no_self_context(conc_ll):
    st = run_state(conc_ll)
    setup = aid_of(st, "obj_setup")
    assert st.acts[setup].reentrant == "no"
    for it in st.items.values():
        assert not any(c.a == c.b == setup for c in it.contexts)
    # still paired with the other writers of count
    assert any(c.a == setup or c.b == setup for it in st.items.values() for c in it.contexts)


def test_reentrancy_unknown_keeps_self_context(conc_ll):
    st = run_state(conc_ll)
    acts = list(st.acts.values())
    for a in acts:
        a.reentrant = "no"
    none_self = build_items(acts)
    assert not any(c.self_interference for it in none_self for c in it.contexts)
    for a in acts:
        a.reentrant = "unknown"
    assert any(c.self_interference for it in build_items(acts) for c in it.contexts)


def lifetime_of(st, fn):
    return next(it for it in st.items.values()
                if it.kind == "lifetime" and f"({fn})," in it.summary)


def test_derived_pointer_stays_in_sequence(conc_ll):
    """p = o->buf; unlock; q = p->next; q->ref: the second-level use is part
    of the same lifetime sequence, and close's take/clear/free is connected."""
    st = run_state(conc_ll)
    it = lifetime_of(st, "obj_consume_deep")
    a = aid_of(st, "obj_consume_deep")
    ctx = next(c for c in it.contexts if c.a == a and st.acts[c.b].name == "obj_close")
    lines = {st.acts[a].steps[s].op.loc.line for s in ctx.core}
    assert L("q = p->next;") in lines and L("return q->ref;") in lines
    close = st.acts[ctx.b]
    kinds = {close.steps[s].op.kind for s in ctx.b_steps}
    assert {"read", "write", "free"} <= kinds


def test_unfollowed_dependency_is_open_boundary(conc_ll, tmp_path):
    st = run_state(conc_ll, max_tasks=None)
    it = lifetime_of(st, "obj_hand_off")
    bs = [st.boundaries[b] for b in it.boundaries]
    assert any(b.kind == "external" and "ext_consume" in b.detail for b in bs)
    ext = next(b for b in bs if b.kind == "external")

    def review(packet, facts, clique):
        sub = safe_all(packet, facts, clique)
        if it.id in clique.items:
            assert ext.id in packet.text and "Open boundaries" in packet.text
            sub["boundaries"] = [{"id": ext.id, "status": "resolved", "how": "no cite",
                                  "citations": []}]
        return sub

    detect(st, ScriptedBackend(review), tmp_path, log=lambda *_: None)
    s = st.ledger.stats()
    assert ext.status == "open"                     # no citation: stays open
    assert s["complete"] is True and s["fully_resolved"] is False
    assert s["review"]["safe_with_open_boundaries"] >= 1
    assert s["boundaries"]["open"] >= 1


def test_boundary_resolved_with_citation(conc_ll, tmp_path):
    st = run_state(conc_ll, max_tasks=None)
    it = lifetime_of(st, "obj_hand_off")
    ext = next(st.boundaries[b] for b in it.boundaries if st.boundaries[b].kind == "external")

    def review(packet, facts, clique):
        sub = safe_all(packet, facts, clique)
        if it.id in clique.items:
            sub["boundaries"] = [{"id": ext.id, "status": "resolved", "how": "read callee",
                                  "citations": [f"{SRC}:{L('ext_consume(p);')}"]}]
        return sub

    detect(st, ScriptedBackend(review), tmp_path, log=lambda *_: None)
    assert ext.status == "resolved" and ext.citations


def test_cross_field_guard(conc_ll):
    st = run_state(conc_ll)
    g = [it for it in st.items.values() if it.kind == "guard" and it.keys[0] == "obj.state"
         and "(obj_check_state)" in it.summary]
    assert len(g) == 1 and "obj.count" in g[0].keys
    writers = {st.acts[c.b].name for c in g[0].contexts}
    assert {"obj_reset", "obj_mark"} <= writers


def test_embedded_member_meets_on_innermost_record(conc_ll):
    """`e->ring.wp` and `r->wp` (r = &c->ring) are one key; the packet names
    the two containers as a fact, not a prune."""
    st = run_state(conc_ll)
    pair = [it for it in st.items.values() if it.kind == "conflict" and it.keys == ("ring.wp",)
            and "(evq_recycle)" in it.summary
            and ("(ring_add)" in it.summary or "(cmdq_send)" in it.summary)]
    assert pair
    paths = st.renderer.object_paths(pair[0].contexts, "ring.wp")
    assert {"evq.ring.wp", "cmdq.ring.wp"} <= set(paths)
    c = next(c for c in st.cliques if pair[0].id in c.items)
    assert "ring.wp by container" in st.renderer.render(c, st.items).text


def test_section_fact_is_about_the_sequence(conc_ll):
    st = run_state(conc_ll)
    r = st.renderer
    use = lifetime_of(st, "obj_consume")
    c = use.contexts[0]
    assert r.section_fact(c.a, c.core).startswith("sequence NOT inside")
    at = next(it for it in st.items.values() if it.kind == "atomicity"
              and "(obj_close) then write obj.buf" in it.summary)
    c = at.contexts[0]
    assert r.section_fact(c.a, c.core).startswith("sequence inside one critical section")


def test_unlock_on_two_paths_is_one_section(conc_ll):
    st = run_state(conc_ll)
    act = st.acts[aid_of(st, "obj_len_locked")]
    use = [s for s in act.steps if s.op.kind == "read" and s.op.state == "buf.len"]
    assert use and all(s.held for s in use)
    assert len(act.sections) == 1


# -------------------------------------------------------------- truncation
def test_step_budget_leaves_links(conc_ll):
    st = run_state(conc_ll, max_steps=3)
    cut = [a for a in st.acts.values() if a.truncated]
    assert cut
    assert all(any(l.kind == "budget" for l in a.links) for a in cut)


def safe_all(packet, facts, clique):
    return {"items": [{"id": i, "verdict": "safe", "reason": "scripted",
                       "citations": [f"{SRC}:{L('_raw_spin_lock(&o->lock);')}"]}
                      for i in clique.items], "findings": []}


def test_task_budget_marks_pending(conc_ll, tmp_path):
    st = run_state(conc_ll, max_tasks=1)
    detect(st, ScriptedBackend(safe_all), tmp_path, log=lambda *_: None)
    s = st.ledger.stats()
    assert s["review"]["tasks_by_status"].get("done") == 1
    assert s["review"]["tasks_by_status"].get("pending", 0) == len(st.cliques) - 1
    assert s["review"]["items_pending"] > 0
    assert s["complete"] is False
    res = report.write(st, tmp_path, "scripted")
    pend = [i for i in res["items"] if i["status"] == "pending"]
    assert pend and all(i["verdict"] is None for i in pend)


def test_token_budget_marks_pending(conc_ll, tmp_path):
    class Costly(ScriptedBackend):
        def review(self, *a, **kw):
            rv = super().review(*a, **kw)
            rv.usage = {"total_tokens": 1000}
            return rv

    st = run_state(conc_ll, max_tasks=None, token_budget=500)
    detect(st, Costly(safe_all), tmp_path, log=lambda *_: None)
    by = st.ledger.stats()["review"]["tasks_by_status"]
    assert by.get("done") == 1 and by.get("pending") == len(st.cliques) - 1


# ------------------------------------------------------- verdict handling
def test_missing_and_uncited_verdicts(conc_ll, tmp_path):
    def partial(packet, facts, clique):
        its = clique.items
        return {"items": [{"id": its[0], "verdict": "safe", "reason": "no cite", "citations": []},
                          {"id": its[1], "verdict": "safe", "reason": "bad cite",
                           "citations": ["../../etc/passwd:1", "A99.s1"]}],
                "findings": [{"title": "extra", "kind": "race", "items": [],
                              "interleaving": "x", "citations": []}]}

    st = run_state(conc_ll, max_tasks=1)
    detect(st, ScriptedBackend(partial), tmp_path, log=lambda *_: None)
    c = st.cliques[0]
    assert c.status == "incomplete"
    first, second = (st.items[i] for i in c.items[:2])
    assert first.verdict == "unknown" and second.verdict == "unknown"
    assert all(st.items[i].status == "incomplete" for i in c.items[2:])
    assert any(f.model_added for f in st.findings)


def test_bug_finding_and_expansion(conc_ll, tmp_path):
    """The script expands the task with an activation the packet lacked and
    reports the take/free/use item as a bug."""
    seen = {}

    def script(packet, facts, clique):
        seen["packet"] = packet.text
        outside = [a for a in facts.run.acts if a not in clique.participants]
        if outside:
            seen["expand"] = facts.expand(outside[0], "check another writer")
        seen["acc"] = facts.accesses("*obj.buf")
        seen["src"] = facts.read_source(SRC, 1, 5)
        seen["deny"] = facts.read_source("../conftest.py", 1, 5)
        life = next(i for i in clique.items if facts.run.items[i].kind == "lifetime")
        a = next(c.a for c in clique.task_contexts[life])
        return {"items": [{"id": i, "verdict": "bug" if i == life else "unknown",
                           "reason": "scripted", "citations": [f"{a}.s1"]}
                          for i in clique.items],
                "findings": [{"title": "use after free of obj.buf", "kind": "use-after-free",
                              "items": [life], "interleaving": "consume takes p; close frees it",
                              "citations": [f"{a}.s1", f"{SRC}:{L('kfree(b);')}"],
                              "confidence": "high"}]}

    st = run_state(conc_ll, max_tasks=1)
    detect(st, ScriptedBackend(script), tmp_path, log=lambda *_: None)
    assert "Items to judge" in seen["packet"] and "## Source" in seen["packet"]
    assert "reentrant=" in seen["packet"]
    assert "kfree" in seen["acc"] or "free" in seen["acc"]
    assert seen["src"].startswith(SRC)
    assert seen["deny"].startswith("error")
    if "expand" in seen:
        assert st.cliques[0].expansions
    bugs = [f for f in st.findings if not f.model_added]
    assert len(bugs) == 1 and bugs[0].kind == "use-after-free" and len(bugs[0].citations) == 2
    rec = json.loads((tmp_path / "tasks" / f"{st.cliques[0].id}.json").read_text())
    assert rec["status"] == "done" and rec["tools"]
    res = report.write(st, tmp_path, "scripted")
    assert res["scope"]["unsupported"] == ["deadlock / lock ordering"]
    assert (tmp_path / "report.md").read_text().count("use after free of obj.buf") == 1


# ----------------------------------------------------------------- sandbox
def test_source_sandbox(tmp_path):
    (tmp_path / "src").mkdir()
    (tmp_path / "src" / "a.c").write_text("int x;\n")
    (tmp_path / "src" / "ground_truth.json").write_text("{}")
    (tmp_path / "src" / "fix.patch").write_text("---")
    (tmp_path / "secret.c").write_text("int y;\n")
    t = SourceTree(tmp_path / "src")
    assert t.files() == ["a.c"]
    assert t.resolve("/build/linux/drivers/a.c") == "a.c"
    assert t.resolve("ground_truth.json") is None
    assert t.resolve("../secret.c") is None
    with pytest.raises(SourceError):
        t.read("../secret.c")
    with pytest.raises(SourceError):
        t.lines("fix.patch")
    assert t.grep("int")[0] == [("a.c", 1, "int x;")]


# ------------------------------------------------------------- two stages
def _life(facts, clique):
    return next(i for i in clique.items if facts.run.items[i].kind == "lifetime")


def _first_pass(packet, facts, clique, life):
    a = next(c.a for c in clique.task_contexts[life])
    return {"items": [{"id": i, "verdict": "bug" if i == life else
                       "unknown" if k == 1 else "safe",
                       "reason": "first", "citations": [f"{a}.s1"]}
                      for k, i in enumerate(clique.items)],
            "findings": [{"title": "first-pass uaf", "kind": "use-after-free", "items": [life],
                          "interleaving": "x", "citations": [f"{a}.s1"]}]}


def test_two_stage_evidence_overturns_first_pass(conc_ll, tmp_path):
    """Only bug/unknown items go to the evidence pass; its verdict replaces
    the first one and supersedes the first-pass finding."""
    calls = []

    def script(packet, facts, clique):
        life = _life(facts, clique)
        calls.append((facts.stage, list(facts.focus)))
        if facts.stage == "direct":
            return _first_pass(packet, facts, clique, life)
        a = next(c.a for c in clique.task_contexts[life])
        return {"items": [{"id": i, "verdict": "safe", "reason": "second",
                           "citations": [f"{a}.s1"]} for i in facts.focus]}

    st = run_state(conc_ll, max_tasks=1)
    detect(st, ScriptedBackend(script), tmp_path, log=lambda *_: None)
    c = st.cliques[0]
    assert [s for s, _ in calls] == ["direct", "evidence"]
    assert calls[0][1] == c.items and len(calls[1][1]) < len(c.items)
    assert all(st.items[i].verdict == "safe" for i in c.items)
    assert any("(was bug)" in n for i in calls[1][1] for n in st.items[i].notes)
    assert not st.findings and c.status == "done"
    rec = json.loads((tmp_path / "tasks" / f"{c.id}.json").read_text())
    assert [r["stage"] for r in rec["reviews"]] == ["direct", "evidence"]


def test_two_stage_evidence_failure_keeps_first_pass(conc_ll, tmp_path):
    """A failed evidence pass leaves the first-pass verdicts, marked as not
    rechecked, and the task incomplete."""
    def script(packet, facts, clique):
        if facts.stage == "direct":
            return _first_pass(packet, facts, clique, _life(facts, clique))
        raise RuntimeError("gateway down")

    st = run_state(conc_ll, max_tasks=1)
    detect(st, ScriptedBackend(script), tmp_path, log=lambda *_: None)
    c = st.cliques[0]
    bug = [i for i in c.items if st.items[i].verdict == "bug"]
    assert len(bug) == 1
    assert any("not rechecked" in n for n in st.items[bug[0]].notes)
    assert c.status == "incomplete"
    assert len(st.findings) == 1 and any("first pass only" in n for n in st.findings[0].notes)


def test_evidence_tool_budget(conc_ll, tmp_path):
    """Past the budget, fact tools answer with a request to submit."""
    seen = []

    def script(packet, facts, clique):
        life = _life(facts, clique)
        if facts.stage == "direct":
            return _first_pass(packet, facts, clique, life)
        seen.extend(facts.read_source(SRC, 1, 3) for _ in range(4))
        a = next(c.a for c in clique.task_contexts[life])
        return {"items": [{"id": i, "verdict": "unknown", "reason": "budget",
                           "citations": [f"{a}.s1"]} for i in facts.focus]}

    st = run_state(conc_ll, max_tasks=1, evidence_tool_calls=2)
    detect(st, ScriptedBackend(script), tmp_path, log=lambda *_: None)
    assert seen[0].startswith(SRC) and seen[1].startswith(SRC)
    assert seen[2].startswith("tool budget") and seen[3].startswith("tool budget")
