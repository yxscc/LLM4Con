"""Coverage ledger: where every recall unit stands.

Four things are kept apart on purpose:

  assignment   interactions enumerated, placed in a task. Placing an item in
               a task is not analyzing it.
  review       tasks the model ran / finished / ran out of budget on / failed,
               and per item whether it got a verdict. `safe` is the model's
               judgment with a checked citation, never a static proof, and a
               model that finds nothing has not shown the code safe.
  boundaries   dependencies the index could not follow and budget cuts
               (external / indirect callees, stores, unnamed accesses, depth,
               truncation). They stay open until a review resolves them with
               a checked citation, whatever the item's verdict.
  static gaps  what the index could not see at all, not tied to an item
               (accesses without a field name, unknown bases, links).

Recall -- whether known defects are among the findings -- is never computed
here: the detector does not read ground truth. Structural coverage at any
stage is not defect coverage.
"""

from collections import Counter

ITEM_STATUSES = ("unassigned", "assigned", "pending", "reviewed", "incomplete", "error")
SAFE_MEANS = "model judgment with a checked citation; nothing is statically proven safe"


class Ledger:
    def __init__(self, items, cliques, boundaries=None, gaps=None):
        self.items = {it.id: it for it in items}
        self.cliques = {c.id: c for c in cliques}
        self.order = [c.id for c in cliques]
        self.boundaries = boundaries or {}
        self.gaps = gaps or {}

    def apply_budget(self, max_tasks):
        """Mark tasks past the budget pending (they keep their items)."""
        for k, cid in enumerate(self.order):
            c = self.cliques[cid]
            if max_tasks is not None and k >= max_tasks and c.status == "planned":
                c.status = "pending"
                c.notes.append(f"not run: task budget {max_tasks}")
                for iid in c.items:
                    self.items[iid].status = "pending"

    def runnable(self):
        return [self.cliques[c] for c in self.order if self.cliques[c].status == "planned"]

    def stats(self):
        items = list(self.items.values())
        st = Counter(it.status for it in items)
        verdicts = Counter(it.verdict for it in items if it.status == "reviewed")
        tasks = Counter(c.status for c in self.cliques.values())
        bs = list(self.boundaries.values())
        open_ = {b.id for b in bs if b.status == "open"}
        assignment = {
            "items": len(items),
            "by_kind": dict(Counter(it.kind for it in items)),
            "assigned_to_task": sum(1 for it in items if it.task),
            "unassigned": st.get("unassigned", 0),
            "contexts_seen": sum(it.n_contexts for it in items),
            "contexts_in_tasks": sum(len(c.task_contexts.get(iid, ())) for c in self.cliques.values()
                                     for iid in c.items),
            "note": "assigned is not analyzed",
        }
        review = {
            "tasks": len(self.cliques),
            "tasks_by_status": dict(tasks),
            "items_reviewed": st.get("reviewed", 0),
            "verdicts": {k: verdicts.get(k, 0) for k in ("bug", "safe", "unknown")},
            "safe_means": SAFE_MEANS,
            "safe_on_part_of_contexts": sum(1 for it in items if it.verdict == "safe"
                                            and it.contexts_reviewed < it.n_contexts),
            "safe_with_open_boundaries": sum(1 for it in items if it.verdict == "safe"
                                             and open_ & set(it.boundaries)),
            "items_incomplete": st.get("incomplete", 0),
            "items_pending": st.get("pending", 0),
            "items_error": st.get("error", 0),
            "items_not_run": st.get("assigned", 0),
        }
        boundaries = {
            "total": len(bs),
            "by_kind": dict(Counter(b.kind for b in bs)),
            "open": len(open_),
            "resolved_by_review": sum(1 for b in bs if b.status == "resolved"),
            "items_with_open_boundaries": sum(1 for it in items if open_ & set(it.boundaries)),
        }
        complete = all(st.get(s, 0) == 0 for s in
                       ("pending", "incomplete", "error", "assigned", "unassigned"))
        return {"assignment": assignment, "review": review, "boundaries": boundaries,
                "static_gaps": self.gaps, "complete": complete,
                "fully_resolved": complete and not open_,
                "recall": "not computed by the detector; score findings against ground truth "
                          "after the run, outside the detector"}
