"""Turn a model submission into ledger updates and findings.

Rules, applied mechanically:

  * every item of the task gets exactly one outcome; an item the submission
    does not mention is `incomplete`, not safe;
  * verdicts are bug | safe | unknown; anything else is unknown;
  * `safe` needs at least one citation that checks out (an `A<n>.s<k>` step
    of this run, or a `file:line` inside the source tree); otherwise it is
    recorded as unknown;
  * a `bug` without a valid citation stays a bug but is marked uncited;
  * findings that name no item of the task are kept as model-added, apart
    from the item-backed ones.
"""

from dataclasses import dataclass, field

VERDICTS = ("bug", "safe", "unknown")
CONFIDENCE = ("high", "medium", "low")


@dataclass
class Finding:
    id: str
    task: str
    title: str
    kind: str
    items: list
    interleaving: str
    consequence: str
    citations: list
    invalid_citations: list
    confidence: str
    model_added: bool = False
    notes: list = field(default_factory=list)


def submission_problems(sub):
    """Shape errors the model can fix before the submission is accepted."""
    if not isinstance(sub, dict):
        return ["submission must be a JSON object"]
    errs = []
    its = sub.get("items")
    if not isinstance(its, list):
        errs.append("`items` must be a list")
    else:
        for k, e in enumerate(its):
            if not isinstance(e, dict) or "id" not in e or "verdict" not in e:
                errs.append(f"items[{k}] needs `id` and `verdict`")
    fs = sub.get("findings", [])
    if not isinstance(fs, list):
        errs.append("`findings` must be a list")
    return errs


def apply(sub, clique, items, facts):
    """Update items in place; return (findings, notes)."""
    notes = []
    given = {}
    for e in (sub or {}).get("items", []) if isinstance(sub, dict) else []:
        if not isinstance(e, dict):
            continue
        iid = str(e.get("id", "")).strip()
        if iid not in clique.items:
            notes.append(f"verdict for {iid!r} ignored: not an item of {clique.id}")
            continue
        given[iid] = e
    for iid in clique.items:
        it = items[iid]
        e = given.get(iid)
        if e is None:
            it.status, it.verdict = "incomplete", None
            it.notes.append(f"{clique.id}: no verdict returned")
            continue
        v = str(e.get("verdict", "")).strip().lower()
        cites = [str(c) for c in e.get("citations", []) or [] if str(c).strip()]
        valid = [c for c in cites if facts.valid_citation(c)]
        if v not in VERDICTS:
            it.notes.append(f"verdict {v!r} read as unknown")
            v = "unknown"
        if v == "safe" and not valid:
            it.notes.append("safe without a valid citation: recorded as unknown")
            v = "unknown"
        if v == "bug" and not valid:
            it.notes.append("bug verdict without a valid citation")
        it.status, it.verdict = "reviewed", v
        it.contexts_reviewed = len(clique.task_contexts.get(iid, ()))
        if v == "safe" and it.contexts_reviewed < it.n_contexts:
            it.notes.append(f"safe for {it.contexts_reviewed} of {it.n_contexts} contexts")
        it.reason = str(e.get("reason", ""))[:2000]
        it.citations = tuple(valid)
        bad = [c for c in cites if c not in valid]
        if bad:
            it.notes.append("invalid citations: " + ", ".join(bad[:6]))

    findings = []
    covered = set()
    for k, f in enumerate((sub or {}).get("findings", []) or []):
        if not isinstance(f, dict):
            continue
        fit = [str(i) for i in f.get("items", []) or [] if str(i) in clique.items]
        cites = [str(c) for c in f.get("citations", []) or []]
        valid = [c for c in cites if facts.valid_citation(c)]
        conf = str(f.get("confidence", "low")).lower()
        fd = Finding(f"{clique.id}.F{k}", clique.id, str(f.get("title", ""))[:300],
                     str(f.get("kind", "other"))[:40], fit, str(f.get("interleaving", ""))[:4000],
                     str(f.get("consequence", ""))[:1000], valid,
                     [c for c in cites if c not in valid],
                     conf if conf in CONFIDENCE else "low", model_added=not fit)
        for i in fit:
            if items[i].verdict == "safe":
                fd.notes.append(f"{i} is judged safe in the same submission")
        covered.update(fit)
        findings.append(fd)
    for iid in clique.items:
        it = items[iid]
        if it.verdict == "bug" and iid not in covered:
            findings.append(Finding(f"{clique.id}.F{len(findings)}", clique.id, it.summary[:300],
                                    it.kind, [iid], "", "", list(it.citations), [], "low",
                                    notes=["derived from an item verdict without a finding"]))
    return findings, notes
