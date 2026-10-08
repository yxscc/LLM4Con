"""Offline relational-index command and normalized, inspectable artifacts.

The existing `run` command is intentionally unchanged. These relation counts
are not agent-task counts and this command does not report safety verdicts.
"""

from dataclasses import asdict
import json
from pathlib import Path
from types import SimpleNamespace
import time

from lace3.entries.discover import discover_entries
from lace3.evidence.source import SourceTree
from lace3.index.ops import ProgramIndex
from lace3.index.traces import build_activations, provenance
from lace3.ir.module import load_program
from lace3.tasks.relations import Member, Pool, Relation, RelationIndex, build_relations

SCHEMA = "lace3.relations/1"


def prepare(ir, max_depth=8, max_steps=4000):
    paths = []
    for p in map(Path, ir):
        paths.extend(sorted(p.glob("*.ll")) if p.is_dir() else [p])
    if not paths:
        raise ValueError("no IR files")
    if len({p.stem for p in paths}) != len(paths):
        raise ValueError("IR files must have distinct stems: the current frontend keys modules by stem")
    t = time.monotonic()
    prog = load_program(paths)
    px = ProgramIndex(prog)
    entries = discover_entries(prog).entries
    acts = build_activations(entries, px, prog, max_depth=max_depth, max_steps=max_steps)
    front = time.monotonic() - t
    t = time.monotonic()
    ix = build_relations(acts)
    return ix, {"front": front, "relations": time.monotonic() - t}


def _jsonl(path, records):
    with path.open("w") as f:
        for record in records:
            f.write(json.dumps(record, ensure_ascii=False, separators=(",", ":")) + "\n")


def write(ix, out_dir, src, timings, limits):
    tree = SourceTree(src)
    out = Path(out_dir)
    out.mkdir(parents=True, exist_ok=True)
    operations, op_ids, frames, frame_ids = {}, {}, {}, {}

    def bindings():
        for aid, act in ix.acts.items():
            for st in act.steps:
                op = st.op
                sig = (op.module, op.function, op.id)
                if sig not in op_ids:
                    oid = f"O{len(op_ids)}"
                    op_ids[sig] = oid
                    operations[oid] = dict(id=oid, module=op.module, function=op.function,
                        local_id=op.id, kind=op.kind, key=op.key, state=op.state,
                        location=asdict(op.loc) if op.loc else None,
                        source=tree.resolve(op.loc.file) if op.loc else None,
                        callee=op.callee, atomic=op.atomic, volatile=op.volatile,
                        deferred=op.deferred, mode=op.mode)
                if st.frames not in frame_ids:
                    fid = f"F{len(frame_ids)}"
                    frame_ids[st.frames] = fid
                    frames[fid] = dict(id=fid, calls=st.frames)
                yield dict(id=f"{aid}.s{st.sid}", aid=aid, sid=st.sid,
                    operation=op_ids[sig], frames=frame_ids[st.frames], path=st.path,
                    base=sorted(st.base, key=str), origin=sorted(st.origin, key=str),
                    args=[sorted(a, key=str) for a in st.args], held=st.held, sections=st.sections)

    _jsonl(out / "bindings.jsonl", bindings())
    _jsonl(out / "operations.jsonl", operations.values())
    _jsonl(out / "frames.jsonl", frames.values())
    _jsonl(out / "members.jsonl", (asdict(m) for m in ix.members.values()))
    _jsonl(out / "pools.jsonl", (dict(id=p.id, members=p.members, by_entry=p.by_entry)
                                for p in ix.pools.values()))
    _jsonl(out / "relations.jsonl", (asdict(r) for r in ix.relations.values()))
    _jsonl(out / "boundaries.jsonl", ix.boundaries.values())
    _jsonl(out / "activations.jsonl", (
        dict(aid=a.aid, entry=a.name, provenance=provenance(a.entry),
             reentrant=a.reentrant, reentrant_why=a.reentrant_why,
             truncated=a.truncated, links=[asdict(x) for x in a.links],
             sections=[asdict(x) for x in a.sections]) for a in ix.acts.values()))
    stats = ix.stats()
    stats["unique_operations"] = len(operations)
    manifest = dict(schema=SCHEMA, source=str(tree.root), limits=limits,
                    timings_s=timings, statistics=stats,
                    pairing="Cartesian product except non-reentrant self; symmetric same-pool relations use upper triangle including diagonal",
                    facts="locks, order, container paths are support facts, not safety/disjointness proofs",
                    status="offline candidate index; agent scheduling and review not performed")
    (out / "index.json").write_text(json.dumps(manifest, indent=2, ensure_ascii=False) + "\n")
    return manifest


def load(out_dir):
    """Read a saved index for paging; source/IR need not be loaded again."""
    out = Path(out_dir)
    manifest = json.loads((out / "index.json").read_text())
    if manifest.get("schema") != SCHEMA:
        raise ValueError("unsupported relational index schema")

    def records(name):
        with (out / name).open() as f:
            yield from (json.loads(line) for line in f if line.strip())

    acts = [SimpleNamespace(**a) for a in records("activations.jsonl")]
    ix = RelationIndex(acts)
    for m in records("members.jsonl"):
        ix.members[m["id"]] = Member(m["id"], m["aid"], tuple(m["steps"]),
                                     tuple(m["core"]), tuple(m["boundaries"]))
    for p in records("pools.jsonl"):
        ix.pools[p["id"]] = Pool(p["id"], tuple(p["members"]), p["by_entry"], frozenset(p["members"]))
    for r in records("relations.jsonl"):
        r["keys"] = tuple(r["keys"])
        ix.relations[r["id"]] = Relation(**r)
    ix.boundaries = {b["id"]: b for b in records("boundaries.jsonl")}
    return ix


def main(args):
    if args.cmd == "pairs":
        ix = load(args.index)
        page = ix.page(args.relation, args.limit, args.cursor)
        page["members"] = {m: asdict(ix.members[m]) for pair in page["pairs"] for m in pair}
        print(json.dumps(page, indent=2, ensure_ascii=False))
        return 0
    SourceTree(args.src)  # fail early for a missing source root
    ix, timings = prepare(args.ir, args.max_depth, args.max_steps)
    manifest = write(ix, args.out, args.src, timings,
                     dict(max_depth=args.max_depth, max_steps=args.max_steps))
    print(json.dumps(manifest, indent=2, ensure_ascii=False))
    return 0
