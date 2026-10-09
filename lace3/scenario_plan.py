"""Build, validate and render an offline concurrency scenario/session plan."""

from dataclasses import asdict
import json
from pathlib import Path
import re
import time

from lace3.evidence.scenario_source import Footprints
from lace3.relational import _jsonl, load
from lace3.tasks.scenarios import build_scenarios
from lace3.tasks.sessions import schedule


def build(ix, src, source_byte_budget=None):
    start = time.monotonic()
    plan = build_scenarios(ix)
    scenes_s = time.monotonic() - start
    start = time.monotonic()
    footprints = Footprints(plan, src)
    source_s = time.monotonic() - start
    start = time.monotonic()
    sessions = schedule(plan, footprints, source_byte_budget)
    return plan, footprints, sessions, dict(scenarios=scenes_s, source=source_s,
                                          scheduling=time.monotonic() - start)


def write(plan, footprints, sessions, out_dir, index_dir, timings):
    out = Path(out_dir)
    out.mkdir(parents=True, exist_ok=True)
    old_sessions = set()
    previous = out / "sessions.jsonl"
    if previous.exists():
        for line in previous.read_text().splitlines():
            record = json.loads(line)
            if re.fullmatch(r"Q[0-9]+", record.get("id", "")):
                old_sessions.add(record["id"])
    catalog = footprints.catalog
    upstream = json.loads((Path(index_dir) / "index.json").read_text())
    _jsonl(out / "scenarios.jsonl", (
        dict(id=s.id, anchor=asdict(s.anchor), views=s.views, kinds=sorted(s.kinds),
             source_files=sorted(footprints.scene_files.get(s.id, ())),
             missing_sources=sorted(footprints.scene_missing.get(s.id, ())),
             status="planned" if s.views else "reference_only")
        for s in plan.scenarios.values()))
    _jsonl(out / "views.jsonl", (asdict(v) for v in plan.views.values()))
    _jsonl(out / "ownership.jsonl", (dict(member=m, scenario=s, possible_origins=plan.possible_origins[m])
                                     for m, s in sorted(plan.owner.items())))
    _jsonl(out / "sessions.jsonl", (
        dict(id=s.id, scenarios=s.scenes, views=s.views, source_files=sorted(s.files),
             source_bytes=s.source_bytes, missing_sources=sorted(s.missing_sources),
             status="planned", reviewed=False) for s in sessions.sessions))
    (out / "pending.json").write_text(json.dumps(sessions.pending, indent=2) + "\n")
    packs = out / "source"
    packs.mkdir(exist_ok=True)
    for old in old_sessions - {s.id for s in sessions.sessions}:
        (packs / f"{old}.txt").unlink(missing_ok=True)
    for session in sessions.sessions:
        text = catalog.render(session.files)
        if len(text.encode("utf-8")) != session.source_bytes:
            raise ValueError("rendered source cost differs from planned cost")
        (packs / f"{session.id}.txt").write_text(text)

    scene_files = footprints.scene_files.values()
    union = frozenset().union(*scene_files)
    whole_input = catalog.whole_input()
    assigned_views = {v for s in sessions.sessions for v in s.views}
    assigned_scenes = {s for session in sessions.sessions for s in session.scenes}
    metrics = dict(
        source_granularity="complete files, including inline/caller source when resolvable",
        cost_unit="UTF-8 bytes of rendered, line-numbered source; NOT model tokens",
        unique_relevant_source_bytes=catalog.cost(union),
        unique_relevant_source_lines=sum(catalog.line_counts[f] for f in union),
        independent_scene_source_bytes=sum(catalog.cost(fs) for fs in scene_files),
        independent_view_source_bytes=sum(catalog.cost(fs) for fs in footprints.view_files.values()),
        planned_source_bytes=sum(s.source_bytes for s in sessions.sessions),
        planned_views_independent_source_bytes=sum(catalog.cost(footprints.view_files[v]) for v in assigned_views),
        planned_scenes_independent_source_bytes=sum(catalog.cost(footprints.scene_files[s]) for s in assigned_scenes),
        whole_input_source_bytes=catalog.cost(whole_input),
        whole_input_fits_source_budget=sessions.source_byte_budget is None or catalog.cost(whole_input) <= sessions.source_byte_budget,
        max_session_source_bytes=max((s.source_bytes for s in sessions.sessions), default=0),
        sessions=len(sessions.sessions), sessions_with_source_gaps=sum(bool(s.missing_sources) for s in sessions.sessions),
        unresolved_source_locations=len(footprints.missing_locations),
    )
    manifest = dict(schema="lace3.scenario-plan/1", index_directory=str(Path(index_dir).resolve()),
        source_root=str(catalog.tree.root), source_byte_budget=sessions.source_byte_budget,
        upstream_index=dict(schema=upstream["schema"], limits=upstream["limits"], statistics=upstream["statistics"]),
        algorithm="origin-anchored left ownership, complete right pools, greedy shared-source scheduling",
        statistics=plan.stats(), coverage=plan.validate(), scheduling=sessions.validate(plan),
        source_metrics=metrics, timings_s=timings,
        limitations=["bounded P0 frontend retained", "object anchors are organizational, not alias proofs",
                     "parameter anchors remain entry-qualified", "source gaps remain explicit",
                     "oversized indivisible views remain pending", "source budget excludes future prompts/tool history/output",
                     "unbounded source sessions are not validated model-context fits",
                     "no agent review or measured model-token savings"])
    (out / "plan.json").write_text(json.dumps(manifest, indent=2, ensure_ascii=False) + "\n")
    return manifest


def main(args):
    t = time.monotonic()
    ix = load(args.index, with_bindings=True)
    loaded = time.monotonic() - t
    meta = json.loads((Path(args.index) / "index.json").read_text())
    src = args.src or meta["source"]
    plan, footprints, sessions, timings = build(ix, src, args.source_byte_budget)
    timings["load_index"] = loaded
    result = write(plan, footprints, sessions, args.out, args.index, timings)
    print(json.dumps(result, indent=2, ensure_ascii=False))
    return 0
