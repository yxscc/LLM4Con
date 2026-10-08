"""One detection run: static preparation, then the task loop.

prepare()  IR -> entries -> activations -> ledger items -> analysis cliques.
           No model calls, no budget pruning except the per-activation step
           and depth limits, which leave `budget` / `depth` links behind.
detect()   For each runnable clique in priority order: render the evidence
           packet, run the backend with the fact tools, normalize the
           submission into item verdicts and findings, write the task
           record. Tasks past --max-tasks, or left when the token budget is
           spent, are marked pending.
"""

import json
import time
from dataclasses import asdict, dataclass, field
from pathlib import Path

from lace3.detect.facts import Facts
from lace3.detect.verdicts import apply
from lace3.entries.discover import discover_entries
from lace3.evidence.packet import PacketRenderer
from lace3.evidence.source import SourceTree
from lace3.index.ops import ProgramIndex
from lace3.index.traces import build_activations
from lace3.ir.module import load_program
from lace3.tasks.cliques import build_cliques
from lace3.tasks.items import build_index
from lace3.tasks.ledger import Ledger


@dataclass
class Limits:
    max_depth: int = 8
    max_steps: int = 4000
    max_participants: int = 6
    max_items_per_task: int = 24
    packet_chars: int = 30000
    max_tasks: int | None = None
    token_budget: int | None = None
    max_expansions: int = 3
    review: str = "two-stage"     # two-stage | direct | evidence
    evidence_packet_chars: int = 12000


@dataclass
class RunState:
    ir: list
    src: str
    limits: Limits
    prog: object = None
    px: object = None
    entries: list = field(default_factory=list)
    acts: dict = field(default_factory=dict)
    items: dict = field(default_factory=dict)
    boundaries: dict = field(default_factory=dict)
    cliques: list = field(default_factory=list)
    ledger: object = None
    tree: object = None
    renderer: object = None
    findings: list = field(default_factory=list)
    timings: dict = field(default_factory=dict)
    usage: dict = field(default_factory=lambda: {"requests": 0, "input_tokens": 0,
                                                 "output_tokens": 0, "total_tokens": 0})
    backend: str = "none"


def ir_files(paths):
    out = []
    for p in paths:
        p = Path(p)
        out += sorted(p.glob("*.ll")) if p.is_dir() else [p]
    return [str(x) for x in out]


def prepare(ir, src, limits=None):
    lim = limits or Limits()
    st = RunState(ir_files(ir), str(src), lim)
    t = time.time()
    st.prog = load_program(st.ir)
    st.px = ProgramIndex(st.prog)
    st.timings["load"] = time.time() - t
    t = time.time()
    st.entries = discover_entries(st.prog).entries
    acts = build_activations(st.entries, st.px, st.prog, max_depth=lim.max_depth,
                             max_steps=lim.max_steps)
    st.acts = {a.aid: a for a in acts}
    st.timings["activations"] = time.time() - t
    t = time.time()
    ix = build_index(acts)
    items = ix.items
    st.items = {it.id: it for it in items}
    st.boundaries = ix.boundaries
    st.cliques = build_cliques(items, lim.max_participants, lim.max_items_per_task,
                               st.acts)
    st.ledger = Ledger(items, st.cliques, ix.boundaries, ix.gaps)
    st.timings["tasks"] = time.time() - t
    st.tree = SourceTree(src)
    st.renderer = PacketRenderer(st.tree, acts, st.prog, max_chars=lim.packet_chars,
                                 boundaries=st.boundaries)
    return st


def _mark_pending(c, items, note):
    c.status = "pending"
    c.notes.append(note)
    for iid in c.items:
        if items[iid].status == "assigned":
            items[iid].status = "pending"


def detect(st, backend, out_dir, log=print):
    """Run the task loop. With backend=None only the packets are written and
    every task stays planned (no model calls)."""
    lim = st.limits
    out = Path(out_dir)
    (out / "packets").mkdir(parents=True, exist_ok=True)
    (out / "tasks").mkdir(parents=True, exist_ok=True)
    st.ledger.apply_budget(lim.max_tasks)
    st.backend = getattr(backend, "name", "none") if backend else "none"
    runnable = st.ledger.runnable()
    for k, c in enumerate(runnable):
        packet = st.renderer.render(c, st.items)
        (out / "packets" / f"{c.id}.txt").write_text(packet.text)
        if backend is None:
            continue
        if lim.token_budget is not None and st.usage["total_tokens"] >= lim.token_budget:
            for rest in runnable[k:]:
                _mark_pending(rest, st.items, f"not run: token budget {lim.token_budget} spent")
            log(f"[lace3] token budget spent; {len(runnable) - k} tasks pending")
            break
        facts = Facts(st, c, lim.max_expansions)
        log(f"[lace3] {c.id}: {len(c.participants)} participants, {len(c.items)} items, "
            f"packet {packet.chars} chars")
        reviews, findings, notes = [], [], []
        stages = ["direct", "evidence"] if lim.review == "two-stage" else [lim.review]
        prior = ""
        for stage in stages:
            if stage == "evidence" and len(stages) == 2:
                focus = [i for i in c.items if st.items[i].status != "reviewed"
                         or st.items[i].verdict in ("bug", "unknown")]
                if not focus:
                    break
                if lim.token_budget is not None and st.usage["total_tokens"] >= lim.token_budget:
                    notes.append("evidence pass not run: token budget spent")
                    for i in focus:
                        st.items[i].notes.append("evidence pass not run: token budget")
                    break
            else:
                focus = list(c.items)
            pk = packet
            if stage == "evidence" and len(stages) == 2:
                pk = st.renderer.render(c, st.items, only=set(focus),
                                        max_chars=lim.evidence_packet_chars)
                (out / "packets" / f"{c.id}.evidence.txt").write_text(pk.text)
            rv = backend.review(pk, facts, c, stage, focus, prior)
            for key in st.usage:
                st.usage[key] += rv.usage.get(key, 0) or 0
            reviews.append((stage, focus, rv))
            if not isinstance(rv.submission, dict):
                rv.submission = None
            if rv.submission is not None:
                fs, ns = apply(rv.submission, c, st.items, facts, st.boundaries,
                               only=focus if stage == "evidence" and len(stages) == 2 else None,
                               stage=stage)
                if stage == "evidence" and len(stages) == 2:
                    # first-pass findings about rechecked items are superseded
                    rechecked = set(focus)
                    findings = [f for f in findings if not f.items or not set(f.items) <= rechecked]
                findings += fs
                notes += ns
            elif stage == "evidence" and len(stages) == 2:
                for i in focus:
                    if st.items[i].status == "reviewed":
                        st.items[i].notes.append(f"evidence pass {rv.status}: first-pass verdict "
                                                 "kept, not rechecked")
                    else:
                        st.items[i].status = "error" if rv.status == "error" else "incomplete"
                notes.append(f"evidence pass {rv.status}: {rv.error[:200]}")
            else:
                for iid in c.items:
                    it = st.items[iid]
                    it.status = "error" if rv.status == "error" else "incomplete"
                    it.notes.append(f"{c.id} {stage}: {rv.status} {rv.error[:200]}")
                break
            prior = "\n".join(f"  {i}: {st.items[i].verdict or 'none'} -- {st.items[i].reason[:200]}"
                               for i in c.items if i in focus or stage == "direct")
        for f in findings:
            if f.id.split(".")[-1].startswith("d") and len(stages) == 2:
                f.notes.append("first pass only (no fact tools)")
        unjudged = [i for i in c.items if st.items[i].status != "reviewed"]
        statuses = [rv.status for _, _, rv in reviews]
        if reviews and reviews[0][2].status == "error" and reviews[0][2].submission is None:
            c.status = "error"
        elif unjudged or any(x != "submitted" for x in statuses):
            c.status = "incomplete"
        else:
            c.status = "done"
        if packet.incomplete_context:
            c.notes.append("packet cut at the character budget: " + "; ".join(packet.omitted[:5]))
        c.notes += notes
        st.findings += findings
        usage = {}
        for _, _, rv in reviews:
            for key, v in rv.usage.items():
                usage[key] = usage.get(key, 0) + (v or 0)
        rec = {"task": c.id, "status": c.status, "participants": c.participants,
               "items": c.items, "unjudged": unjudged,
               "packet": {"chars": packet.chars, "incomplete_context": packet.incomplete_context,
                          "omitted": packet.omitted, "functions": packet.functions},
               "tools": facts.log, "expansions": c.expansions,
               "reviews": [{"stage": stg, "items": fc, "status": rv.status, "error": rv.error,
                            "usage": rv.usage, "seconds": round(rv.seconds, 1),
                            "attempts": rv.attempts, "submission": rv.submission}
                           for stg, fc, rv in reviews],
               "findings": [asdict(f) for f in findings], "notes": c.notes}
        (out / "tasks" / f"{c.id}.json").write_text(json.dumps(rec, indent=1))
        log(f"[lace3] {c.id}: {c.status}, {len(findings)} findings, "
            + ", ".join(f"{stg} {len(fc)} items {rv.status} {rv.usage.get('total_tokens', '?')} tok"
                        for stg, fc, rv in reviews) + f", {len(facts.log)} tool calls")
    return st
