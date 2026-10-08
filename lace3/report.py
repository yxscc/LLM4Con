"""Run outputs.

  results.json   schema lace3.results/1: run metadata, completeness, findings,
                 every item with its outcome, every task with its status
  ledger.json    the coverage ledger counts alone
  report.md      the same for people
  packets/       the evidence packet of each task that was rendered
  tasks/         per task: tool calls, expansions, usage, raw submission

Out of scope and stated in every output: deadlocks and lock ordering.
"""

import json
from dataclasses import asdict
from pathlib import Path

from lace3.index.traces import provenance

SCHEMA = "lace3.results/1"
SCOPE = {"analyzed": ["data race", "atomicity violation", "lifetime (use/free after "
                      "revoke, double free, publish before init)"],
         "not_analyzed": ["deadlock / lock ordering"],
         "unsupported": ["deadlock / lock ordering"]}


def _item(it, r):
    return {"id": it.id, "kind": it.kind, "keys": list(it.keys), "summary": r.shorten(it.summary),
            "priority": it.priority, "contexts_seen": it.n_contexts,
            "contexts": [asdict(c) for c in it.contexts], "boundaries": it.boundaries,
            "task": it.task, "status": it.status,
            "verdict": it.verdict, "contexts_reviewed": it.contexts_reviewed,
            "reason": it.reason, "citations": list(it.citations),
            "notes": it.notes}


def _act(a, r):
    return {"aid": a.aid, "entry": a.name, "at": r.site(a.function.loc),
            "provenance": [r.shorten(p) for p in provenance(a.entry)],
            "reentrant": a.reentrant, "steps": len(a.steps), "sections": len(a.sections),
            "links": dict(_count(l.kind for l in a.links)), "truncated": a.truncated}


def _count(xs):
    out = {}
    for x in xs:
        out[x] = out.get(x, 0) + 1
    return out


def write(st, out_dir, model_note):
    out = Path(out_dir)
    out.mkdir(parents=True, exist_ok=True)
    r = st.renderer
    stats = st.ledger.stats()
    truncated = [a.aid for a in st.acts.values() if a.truncated]
    run = {"ir": st.ir, "src": st.src, "limits": asdict(st.limits), "backend": st.backend,
           "model_calls": model_note, "usage": st.usage,
           "timings_s": {k: round(v, 2) for k, v in st.timings.items()},
           "entries": len(st.acts), "truncated_activations": truncated}
    findings = [asdict(f) for f in st.findings]
    res = {"schema": SCHEMA, "scope": SCOPE, "run": run, "completeness": stats,
           "findings": findings,
           "items": [_item(it, r) for it in st.items.values()],
           "tasks": [{"id": c.id, "status": c.status, "participants": c.participants,
                      "items": c.items, "priority": c.priority, "expansions": c.expansions,
                      "notes": c.notes} for c in st.cliques],
           "boundaries": [asdict(b) for b in st.boundaries.values()],
           "activations": [_act(a, r) for a in st.acts.values()]}
    (out / "results.json").write_text(json.dumps(res, indent=1))
    (out / "ledger.json").write_text(json.dumps(stats, indent=1))
    (out / "report.md").write_text(markdown(res))
    return res


def markdown(res):
    c, run = res["completeness"], res["run"]
    a, rv = c["assignment"], c["review"]
    L = ["# LACE v3 report", ""]
    L.append(f"- IR: {len(run['ir'])} module(s); source root `{run['src']}`")
    L.append(f"- Backend: {run['backend']}; model calls: {run['model_calls']}")
    L.append(f"- Usage: {run['usage']}")
    L.append("- Unsupported (not analyzed): deadlock / lock ordering")
    L.append(f"- Complete: **{c['complete']}**"
             + ("" if c["complete"] else " (pending / incomplete / not-run items below)"))
    L.append("")
    L.append("## Coverage ledger")
    L.append("Structural counts; none of them is defect coverage.")
    L.append("")
    L.append(f"- Assignment: {a['items']} items {a['by_kind']}, {a['assigned_to_task']} in tasks, "
             f"{a['contexts_seen']} contexts seen, {a['contexts_in_tasks']} shown in tasks")
    L.append(f"- Review: tasks {rv['tasks_by_status']}; items reviewed {rv['items_reviewed']} "
             f"{rv['verdicts']} (safe on part of the contexts only: "
             f"{rv['safe_on_part_of_contexts']}), incomplete {rv['items_incomplete']}, pending "
             f"{rv['items_pending']}, error {rv['items_error']}, not run {rv['items_not_run']}")
    b = c["boundaries"]
    L.append(f"- Boundaries (unfollowed dependencies, budget cuts): {b['total']} {b['by_kind']}; "
             f"open {b['open']}, resolved by review {b['resolved_by_review']}; items with open "
             f"boundaries {b['items_with_open_boundaries']}; safe items with open boundaries "
             f"{rv['safe_with_open_boundaries']}")
    L.append(f"- Static gaps (not tied to an item): {c['static_gaps']}")
    L.append(f"- Safe means: {rv['safe_means']}")
    L.append(f"- Fully resolved (complete and no open boundary): **{c['fully_resolved']}**")
    L.append(f"- Recall: {c['recall']}")
    if run["truncated_activations"]:
        L.append(f"- Truncated activations (step/depth budget): {run['truncated_activations']}")
    L.append("")
    L.append(f"## Findings ({len(res['findings'])})")
    for f in res["findings"]:
        tag = " (model-added)" if f["model_added"] else ""
        L.append(f"### {f['id']} [{f['kind']}, {f['confidence']}]{tag} {f['title']}")
        if f["items"]:
            L.append(f"Items: {', '.join(f['items'])}")
        if f["interleaving"]:
            L.append(f"Interleaving: {f['interleaving']}")
        if f["consequence"]:
            L.append(f"Consequence: {f['consequence']}")
        if f["citations"]:
            L.append(f"Citations: {', '.join(f['citations'])}")
        for n in f["notes"]:
            L.append(f"Note: {n}")
        L.append("")
    open_ = [i for i in res["items"] if i["status"] != "reviewed" or i["verdict"] == "unknown"]
    L.append(f"## Open items ({len(open_)}: unknown, incomplete, pending or not run)")
    for i in open_[:200]:
        L.append(f"- {i['id']} {i['kind']} [{i['status']}"
                 + (f"/{i['verdict']}" if i["verdict"] else "") + f"] {i['summary']}")
    if len(open_) > 200:
        L.append(f"- ... {len(open_) - 200} more in results.json")
    return "\n".join(L) + "\n"
