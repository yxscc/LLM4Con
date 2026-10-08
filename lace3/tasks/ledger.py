"""Coverage ledger: where every recall unit stands.

Three numbers are kept apart on purpose:

  assignment   items enumerated, items placed in a task. Structural only.
  review       tasks the model ran / finished / ran out of budget on / failed,
               and per item whether it got a verdict. A finished task does
               not mean its items are understood: `unknown` is a verdict.
  recall       whether known defects are among the findings. Never computed
               here -- the detector does not read ground truth; it is scored
               after the run, outside the detector.

Structural coverage at any stage is not defect coverage. A run with pending
or incomplete tasks says so in `complete: false` and in the counts.
"""

from collections import Counter

ITEM_STATUSES = ("unassigned", "assigned", "pending", "reviewed", "incomplete", "error")


class Ledger:
    def __init__(self, items, cliques):
        self.items = {it.id: it for it in items}
        self.cliques = {c.id: c for c in cliques}
        self.order = [c.id for c in cliques]

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
        assignment = {
            "items": len(items),
            "by_kind": dict(Counter(it.kind for it in items)),
            "assigned_to_task": sum(1 for it in items if it.task),
            "unassigned": st.get("unassigned", 0),
            "contexts_seen": sum(it.n_contexts for it in items),
            "contexts_in_tasks": sum(len(c.task_contexts.get(iid, ())) for c in self.cliques.values()
                                     for iid in c.items),
        }
        review = {
            "tasks": len(self.cliques),
            "tasks_by_status": dict(tasks),
            "items_reviewed": st.get("reviewed", 0),
            "verdicts": {k: verdicts.get(k, 0) for k in ("bug", "safe", "unknown")},
            "safe_on_part_of_contexts": sum(1 for it in items if it.verdict == "safe"
                                            and it.contexts_reviewed < it.n_contexts),
            "items_incomplete": st.get("incomplete", 0),
            "items_pending": st.get("pending", 0),
            "items_error": st.get("error", 0),
            "items_not_run": st.get("assigned", 0),
        }
        complete = (st.get("pending", 0) == 0 and st.get("incomplete", 0) == 0
                    and st.get("error", 0) == 0 and st.get("assigned", 0) == 0
                    and st.get("unassigned", 0) == 0)
        return {"assignment": assignment, "review": review, "complete": complete,
                "recall": "not computed by the detector; score findings against ground truth "
                          "after the run, outside the detector"}
