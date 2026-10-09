"""Scenario conservation and session planning, without any model requests."""

from dataclasses import replace
import json
from pathlib import Path
from types import SimpleNamespace

import pytest

from lace3 import relational, scenario_plan
from lace3.cli import main
from lace3.evidence.scenario_source import SourceCatalog
from lace3.index.ops import Op
from lace3.index.traces import Step
from lace3.tasks.relations import RelationIndex, build_relations
from lace3.tasks.scenarios import Origins, build_scenarios
from lace3.tasks.sessions import schedule
from .conftest import FIXTURES


def act(aid, keys=("obj.value",), root="parameter", once=False):
    steps = []
    for i, key in enumerate(keys):
        base = frozenset({("arg", 0)}) if root == "parameter" else frozenset({("global", root)})
        steps.append(Step(i, Op(i, "helper", "module", "write", key, None),
                          ((aid, None),), base=base))
    return SimpleNamespace(aid=aid, name=aid, steps=steps, links=[],
                           reentrant="no" if once else "unknown", truncated=False)


def pairs(ix, relation):
    cursor = None
    while True:
        page = ix.page(relation, 1000, cursor)
        yield from map(tuple, page["pairs"])
        cursor = page["next_cursor"]
        if cursor is None:
            break


def assert_exact_partition(ix, plan):
    for rid, r in ix.relations.items():
        expected = set(pairs(ix, rid))
        seen = set()
        for view in (v for v in plan.views.values() if v.relation == rid):
            owned = {(a, b) for a, b in expected if a in view.left_members}
            assert len(owned) == view.combinations
            assert not (seen & owned)
            assert view.cross_scenario_combinations == sum(plan.owner[a] != plan.owner[b] for a, b in owned)
            assert view.right_pool == r.right
            seen.update(owned)
        assert seen == expected


def test_exact_partition_of_real_fixture(conc_ll):
    ix, _ = relational.prepare([conc_ll])
    plan = build_scenarios(ix)
    assert_exact_partition(ix, plan)
    assert plan.validate()["partition_complete"]
    assert plan.stats()["cross_scenario_combinations"] > 0


def test_triangle_keeps_original_pool_order_and_nonreentrant_filter():
    aa = [act("first", ("list_head.next",) * 3),
          act("once", ("list_head.next",) * 2, once=True),
          act("last", ("list_head.next",))]
    ix = build_relations(aa)
    plan = build_scenarios(ix)
    assert len([s for s in plan.scenarios.values() if s.views]) == 3
    assert_exact_partition(ix, plan)
    assert sum(v.combinations for v in plan.views.values()) == 18  # 6*7/2 - 2*3/2


def test_shared_helper_type_does_not_collapse_unrelated_parameters():
    ix = build_relations([act("entry_a", ("list_head.next",)), act("entry_b", ("list_head.next",))])
    plan = build_scenarios(ix)
    mids = next(iter(ix.pools.values())).members
    assert len({plan.owner[m] for m in mids}) == 2
    assert plan.stats()["cross_scenario_combinations"] == 1


def test_embedded_container_roles_and_lock_bindings_retained():
    a = act("entry", ("ring.wp", "ring.wp", "ring.wp"))
    a.steps[0] = replace(a.steps[0], path="controller.command_ring.wp", held=("lock",))
    a.steps[1] = replace(a.steps[1], path="controller.event_ring.wp", held=())
    a.steps[2] = replace(a.steps[2], path="controller.command_ring.wp", held=())
    ix = build_relations([a])
    plan = build_scenarios(ix)
    mids = {m.steps[0]: m.id for m in ix.members.values()}
    assert plan.owner[mids[0]] != plan.owner[mids[1]]
    assert plan.owner[mids[0]] == plan.owner[mids[2]]
    assert a.steps[0].held and not a.steps[2].held  # same scene never merges binding facts
    assert_exact_partition(ix, plan)


def test_pointer_dependency_inherits_object_anchor():
    a = act("consume", ("obj.buf", "buf.len"))
    a.steps[0] = replace(a.steps[0], op=replace(a.steps[0].op, kind="read"))
    a.steps[1] = replace(a.steps[1], base=frozenset({("load", 0)}))
    ix = build_relations([a])
    roots = Origins(ix)
    assert roots.resolve(a.aid, 0) == roots.resolve(a.aid, 1)


def test_unresolved_alternatives_are_retained_without_fragmenting_known_root():
    a = act("entry", ("obj.x", "obj.y", "obj.z"))
    a.steps[0] = replace(a.steps[0], base=frozenset({("arg", 0), ("load", None)}))
    a.steps[1] = replace(a.steps[1], base=frozenset({("arg", 0), ("load", None)}))
    a.steps[2] = replace(a.steps[2], base=frozenset({("arg", 0), ("null",)}))
    plan = build_scenarios(build_relations([a]))
    assert plan.stats()["scenarios"] == 1
    assert plan.stats()["uncertain_anchors"] == 1
    all_roots = [r for roots in plan.possible_origins.values() for r in roots]
    assert any(r[0] == "unknown" for r in all_roots)
    assert any(r[0] == "null" for r in all_roots)
    assert_exact_partition(plan.ix, plan)


def test_null_only_roots_do_not_create_a_universal_unknown_scene():
    a = act("entry", ("obj.x", "obj.y"))
    for i in range(2):
        a.steps[i] = replace(a.steps[i], base=frozenset({("null",)}))
    plan = build_scenarios(build_relations([a]))
    assert plan.stats()["scenarios"] == 2
    assert all(roots == (("null",),) for roots in plan.possible_origins.values())


def test_origin_cycles_and_long_chains_are_explicit_and_finite():
    a = act("entry", ("obj.next",) * 1500)
    for i in range(1, len(a.steps)):
        a.steps[i] = replace(a.steps[i], base=frozenset({("load", i - 1)}))
    ix = RelationIndex([a])
    roots = Origins(ix)
    assert roots.resolve(a.aid, 1499) == roots.resolve(a.aid, 0)
    a.steps[0] = replace(a.steps[0], base=frozenset({("load", 1499)}))
    cyclic = Origins(ix).resolve(a.aid, 0)
    assert any(r[0] == "unknown" and r[-1] == "cyclic-origin" for r in cyclic)


def test_build_does_not_expand_pairs(monkeypatch):
    ix = build_relations([act("a", ("obj.value",) * 2000)])
    def forbidden(*args, **kwargs):
        raise AssertionError("pair enumeration is forbidden while planning")
    monkeypatch.setattr(ix, "page", forbidden)
    monkeypatch.setattr(ix, "contains", forbidden)
    plan = build_scenarios(ix)
    assert plan.validate()["implicit_combinations"] == 2001000


def test_partition_validation_rejects_missing_members():
    plan = build_scenarios(build_relations([act("a", ("obj.value",) * 3)]))
    view = next(iter(plan.views.values()))
    view.left_members = view.left_members[:-1]
    with pytest.raises(ValueError, match="missing or extra"):
        plan.validate()


def fake_footprints(tmp_path, plan, file_sets):
    catalog = SourceCatalog(tmp_path)
    for name in set().union(*file_sets):
        (tmp_path / name).write_text("int example;\n")
        catalog.add(name)
    vfiles = {v: frozenset(fs) for v, fs in zip(plan.views, file_sets)}
    sfiles = {s.id: frozenset().union(*(vfiles[v] for v in s.views))
              for s in plan.scenarios.values() if s.views}
    return SimpleNamespace(catalog=catalog, view_files=vfiles, scene_files=sfiles,
                           view_missing={v: frozenset() for v in plan.views})


def test_shared_source_is_scheduled_once(tmp_path):
    ix = build_relations([act("a", ("first.value",)), act("b", ("second.value",))])
    plan = build_scenarios(ix)
    fp = fake_footprints(tmp_path, plan, [{"common.c", "a.c"}, {"common.c", "b.c"}])
    result = schedule(plan, fp)
    assert len(result.sessions) == 1
    assert result.sessions[0].source_bytes == fp.catalog.cost({"common.c", "a.c", "b.c"})
    assert result.validate(plan)["pending_combinations"] == 0
    cap = max(fp.catalog.cost(fs) for fs in fp.scene_files.values())
    limited = schedule(plan, fp, cap)
    assert len(limited.sessions) == 2 and not limited.pending
    assert all(s.source_bytes <= cap for s in limited.sessions)


def test_one_scene_can_span_sessions_without_losing_views(tmp_path):
    plan = build_scenarios(build_relations([act("a", ("obj.x", "obj.y"))]))
    assert plan.stats()["scenarios"] == 1
    fp = fake_footprints(tmp_path, plan, [{"x.c"}, {"y.c"}])
    cap = max(fp.catalog.byte_sizes.values())
    result = schedule(plan, fp, cap)
    assert len(result.sessions) == 2 and not result.pending
    assert result.sessions[0].scenes == result.sessions[1].scenes
    small = schedule(plan, fp, 1)
    assert not small.sessions and len(small.pending) == len(plan.views)
    assert small.validate(plan)["pending_combinations"] == plan.stats()["implicit_combinations"]


def test_archive_cli_and_complete_source_roundtrip(conc_ll, tmp_path, capsys):
    ix, _ = relational.prepare([conc_ll])
    root = tmp_path / "index"
    relational.write(ix, root, FIXTURES, {}, {})
    archive = relational.load(root, with_bindings=True)
    assert build_scenarios(ix).owner == build_scenarios(archive).owner
    out = tmp_path / "plan"
    assert main(["plan", "--index", str(root), "--out", str(out)]) == 0
    manifest = json.loads((out / "plan.json").read_text())
    assert manifest["coverage"]["partition_complete"]
    assert manifest["scheduling"]["model_calls"] == 0
    assert not manifest["scheduling"]["review_complete"]
    source = "".join(p.read_text() for p in (out / "source").glob("*.txt"))
    original = (FIXTURES / "conc_patterns.c").read_text().splitlines()
    assert f"{len(original)}: {original[-1]}\n" in source
    sessions = [json.loads(line) for line in (out / "sessions.jsonl").read_text().splitlines()]
    for s in sessions:
        assert len((out / "source" / f"{s['id']}.txt").read_bytes()) == s["source_bytes"]
    # Replanning with an impossible explicit budget must not leave an old
    # source packet looking like runnable work in the current output directory.
    assert main(["plan", "--index", str(root), "--out", str(out), "--source-byte-budget", "1"]) == 0
    assert not list((out / "source").glob("Q*.txt"))
    pending = json.loads((out / "pending.json").read_text())
    assert pending


def test_source_sandbox_and_gaps_stay_explicit(tmp_path):
    (tmp_path / "ground_truth.c").write_text("do not read")
    catalog = SourceCatalog(tmp_path)
    assert catalog.add("ground_truth.c") is None
    assert catalog.add("missing.c") is None
    assert not catalog.text and len(catalog.unavailable) == 2
