"""Command line: python -m lace3 run --ir X.ll --src SRC --out OUT [...]

Entries are always discovered from the IR; there is no option to supply
them. Model calls need an explicit task or token budget, so a run never
turns into a full paid scan by accident.
"""

import argparse
import sys

from lace3 import report
from lace3.detect.runner import Limits, detect, prepare


def _args(argv):
    ap = argparse.ArgumentParser(prog="python -m lace3")
    sub = ap.add_subparsers(dest="cmd", required=True)
    r = sub.add_parser("run", help="detect concurrency defects in kernel IR")
    r.add_argument("--ir", action="append", required=True,
                   help=".ll file or directory of .ll files (repeatable)")
    r.add_argument("--src", required=True, help="source root the IR was compiled from")
    r.add_argument("--out", required=True, help="output directory")
    r.add_argument("--offline", action="store_true",
                   help="static part and packets only, no model calls")
    r.add_argument("--max-tasks", type=int, default=None,
                   help="review at most N tasks; the rest are recorded pending")
    r.add_argument("--token-budget", type=int, default=None,
                   help="stop starting tasks once this many tokens are used")
    r.add_argument("--max-turns", type=int, default=12, help="model turns per task")
    r.add_argument("--max-expansions", type=int, default=3, help="clique expansions per task")
    r.add_argument("--max-depth", type=int, default=8, help="call expansion depth")
    r.add_argument("--max-steps", type=int, default=4000, help="steps per activation")
    r.add_argument("--max-participants", type=int, default=4)
    r.add_argument("--max-items-per-task", type=int, default=24)
    r.add_argument("--packet-chars", type=int, default=30000)
    return ap.parse_args(argv)


def main(argv=None):
    a = _args(argv)
    lim = Limits(max_depth=a.max_depth, max_steps=a.max_steps,
                 max_participants=a.max_participants, max_items_per_task=a.max_items_per_task,
                 packet_chars=a.packet_chars, max_tasks=a.max_tasks,
                 token_budget=a.token_budget, max_expansions=a.max_expansions)
    backend, note = None, "not made (--offline)"
    if not a.offline:
        if a.max_tasks is None and a.token_budget is None:
            print("model runs need --max-tasks or --token-budget (or use --offline)",
                  file=sys.stderr)
            return 2
        from lace3.config import llm_settings
        try:
            settings = llm_settings()
        except RuntimeError as e:
            print(f"[lace3] {e}; running offline, model calls unverified", file=sys.stderr)
            note = f"not made: {e}"
        else:
            from lace3.detect.backend import AgentsBackend
            backend = AgentsBackend(settings, max_turns=a.max_turns)
            note = f"{settings.model} via {settings.azure_endpoint}"
    st = prepare(a.ir, a.src, lim)
    n_items = len(st.items)
    print(f"[lace3] {len(st.acts)} activations, {n_items} items, {len(st.cliques)} tasks "
          f"({', '.join(f'{k} {v:.1f}s' for k, v in st.timings.items())})")
    detect(st, backend, a.out)
    res = report.write(st, a.out, note)
    c = res["completeness"]
    print(f"[lace3] findings {len(res['findings'])}; review {c['review']['tasks_by_status']}; "
          f"complete={c['complete']}; wrote {a.out}/report.md")
    return 0
