"""Independent pairing oracle, legacy coverage, serialization, and boundaries.

No model calls. Legacy pairs are enumerated ONLY in these small fixture tests.
"""

from dataclasses import replace
import json
from itertools import combinations_with_replacement, product
from types import SimpleNamespace

import pytest

from lace3 import relational
from lace3.index.dependencies import Dependencies
from lace3.index.ops import Op
from lace3.index.traces import Step
from lace3.tasks.items import build_index, closure
from lace3.tasks.relations import RelationIndex, build_relations
from .conftest import FIXTURES
from .conftest import compile_fixture


def fixture_index(ll):
    return relational.prepare([ll])[0]


def all_pairs(ix, r, limit=3):
    pairs, cursor = [], None
    while True:
        page = ix.page(r.id, limit, cursor)
        pairs.extend(map(tuple, page["pairs"]))
        cursor = page["next_cursor"]
        if cursor is None:
            return pairs


def test_factorization_matches_independent_pairing(conc_ll):
    ix = fixture_index(conc_ll)
    for r in ix.relations.values():
        left, right = ix.pools[r.left].members, ix.pools[r.right].members
        candidates = combinations_with_replacement(left, 2) if r.symmetric and r.left == r.right else product(left, right)
        expected = {(a, b) for a, b in candidates if ix.members[a].aid != ix.members[b].aid
                    or ix.acts[ix.members[a].aid].reentrant != "no"}
        got = all_pairs(ix, r)
        assert len(got) == len(set(got)) == r.combinations == len(expected)
        assert set(got) == expected
        assert r.self_combinations == sum(ix.members[a].aid == ix.members[b].aid for a, b in expected)


def test_every_legacy_context_has_complete_member_superset(conc_ll):
    ix = fixture_index(conc_ll)
    legacy = build_index(list(ix.acts.values()), max_contexts=10**9)
    for item in legacy.items:
        for ctx in item.contexts:
            found = False
            for r in ix.relations.values():
                if r.kind != item.kind or item.keys[0] not in r.keys:
                    continue
                for ma, mb in all_pairs(ix, r, 1000):
                    a, b = ix.members[ma], ix.members[mb]
                    if a.aid == ctx.a and b.aid == ctx.b and set(ctx.a_steps) <= set(a.steps) and set(ctx.b_steps) <= set(b.steps):
                        found = True
                    if r.symmetric and b.aid == ctx.a and a.aid == ctx.b and set(ctx.a_steps) <= set(b.steps) and set(ctx.b_steps) <= set(a.steps):
                        found = True
                    if found:
                        break
                if found:
                    break
            assert found, (item.sig, ctx)
    # Pair count itself is lossless for conflicts; sequence sets may now be larger.
    assert sum(r.combinations for r in ix.relations.values() if r.kind == "conflict") == sum(
        it.n_contexts for it in legacy.items if it.kind == "conflict")


def test_indexed_closure_matches_uncapped_reference(conc_ll):
    ix = fixture_index(conc_ll)
    for act in ix.acts.values():
        dep = Dependencies(act)
        for st in act.steps:
            if st.op.kind == "read":
                got, expected = dep.closure(st.sid), closure(act, st.sid, len(act.steps) + 1)
                for key in ("steps", "derefs", "writes", "branches", "derived"):
                    assert getattr(got, key) == getattr(expected, key)


def synthetic(n=1, reentrant="unknown"):
    steps = [Step(i, Op(i, "entry", "test", "write", "obj.value", None),
                  (("entry", None),), held=("lock",) if i % 2 else ()) for i in range(n)]
    return SimpleNamespace(aid="A0", steps=steps, links=[], reentrant=reentrant, truncated=False)


def test_build_never_materializes_pairs_or_drops_self(monkeypatch):
    def forbidden(*a, **kw):
        raise AssertionError("build requested a concrete pair")
    monkeypatch.setattr(RelationIndex, "page", forbidden)
    monkeypatch.setattr(RelationIndex, "contains", forbidden)
    ix = build_relations([synthetic(2000)])
    assert len(ix.relations) == 1
    assert len(ix.members) == 2000
    r = next(iter(ix.relations.values()))
    assert r.combinations == r.self_combinations == 2001000
    assert not build_relations([synthetic(2000, "no")]).relations


def test_more_than_legacy_context_and_closure_caps_retained():
    act = synthetic(100)
    act.steps[0] = replace(act.steps[0], op=replace(act.steps[0].op, kind="read"))
    for i in range(1, 100):
        act.steps[i] = replace(act.steps[i], origin=frozenset({("load", 0)}))
    ix = build_relations([act])
    r = next(r for r in ix.relations.values() if r.kind == "atomicity")
    left = ix.members[ix.pools[r.left].members[0]]
    assert left.steps == tuple(range(100))
    right = ix.members[ix.pools[r.right].members[0]]
    assert set(range(100)) <= set(right.steps)
    assert not any(b["kind"] == "cap" for b in ix.boundaries.values())


def test_container_and_lock_differences_do_not_remove_relations():
    a = synthetic()
    b = synthetic()
    b.aid = "A1"
    a.steps[0] = replace(a.steps[0], path="owner_a.field", held=("lock_a",))
    b.steps[0] = replace(b.steps[0], path="owner_b.field", held=())
    ix = build_relations([a, b])
    r = next(iter(ix.relations.values()))
    assert r.combinations == 3
    assert a.steps[0].held == ("lock_a",) and b.steps[0].held == ()


def test_serialized_index_roundtrip_keeps_full_members(conc_ll, tmp_path):
    ix = fixture_index(conc_ll)
    meta = relational.write(ix, tmp_path, FIXTURES, {}, {})
    saved = relational.load(tmp_path)
    assert saved.members == ix.members and saved.relations == ix.relations
    for r in ix.relations.values():
        assert all_pairs(ix, r, 7) == all_pairs(saved, r, 13)
    assert meta["statistics"]["review"]["individually_reviewed"] == 0
    assert meta["statistics"]["complete"] is False


def test_frontend_truncation_remains_explicit(conc_ll):
    ix, _ = relational.prepare([conc_ll], max_steps=3)
    assert ix.stats()["static_gaps"]["truncated_activations"] > 0
    assert ix.stats()["review"]["unreviewed"] == ix.stats()["implicit_combinations"]


def test_page_validates_limits_and_cursor():
    ix = build_relations([synthetic(4)])
    rid = next(iter(ix.relations))
    for limit in (0, -1, 1001):
        with pytest.raises(ValueError):
            ix.page(rid, limit)
    with pytest.raises(ValueError):
        ix.page(rid, cursor=(-1, 0))


def test_duplicate_provenance_does_not_duplicate_same_access():
    act = synthetic(3)
    for i in (0, 1):
        act.steps[i] = replace(act.steps[i], op=replace(act.steps[i].op, kind="read"))
    act.steps[2] = replace(act.steps[2], op=replace(act.steps[2].op, kind="free", state=None),
                           origin=frozenset({("load", 0), ("load", 1)}))
    ix = build_relations([act])
    conflicts = [r for r in ix.relations.values() if r.kind == "conflict"]
    assert len(conflicts) == 1 and conflicts[0].combinations == 1
    assert ix.stats()["duplicate_access_records_collapsed"] == 1
    from lace3.eval.relations import LegacyCoverage
    audit = LegacyCoverage(ix).run()
    assert audit["missing"] == 0
    assert audit["contexts_by_kind"]["conflict"] > 1


def test_helper_called_with_and_without_lock_keeps_separate_bindings(tmp_path):
    ll = compile_fixture("relation_bindings.c", tmp_path)
    ix = fixture_index(ll)
    meta = relational.write(ix, tmp_path / "index", FIXTURES, {}, {})
    root = tmp_path / "index"
    ops = {o["id"]: o for o in map(json.loads, (root / "operations.jsonl").read_text().splitlines())}
    records = list(map(json.loads, (root / "bindings.jsonl").read_text().splitlines()))
    helpers = [b for b in records if ops[b["operation"]]["function"] == "helper"
               and ops[b["operation"]]["kind"] == "write"]
    assert len(helpers) == 2 and len({b["operation"] for b in helpers}) == 1
    assert {bool(b["held"]) for b in helpers} == {True, False}
    assert len({b["aid"] for b in helpers}) == 2
    assert meta["statistics"]["review"]["individually_reviewed"] == 0


def test_ambiguous_module_identity_is_rejected(tmp_path):
    with pytest.raises(ValueError, match="distinct stems"):
        relational.prepare([tmp_path / "a" / "main.ll", tmp_path / "b" / "main.ll"])
