"""Reproducible offline scenario/session measurements on IR and source only."""

import argparse
import hashlib
import json
from pathlib import Path
import resource
import sys
import time

from lace3 import relational, scenario_plan
from lace3.tasks.sessions import schedule


def main(argv=None):
    ap = argparse.ArgumentParser(__doc__)
    ap.add_argument("--ir", required=True)
    ap.add_argument("--src", required=True)
    ap.add_argument("--out", required=True)
    ap.add_argument("--budget-fractions", default="",
                    help="optional sensitivity budgets as fractions of unique relevant source bytes; not model limits")
    args = ap.parse_args(argv)
    out = Path(args.out)
    out.mkdir(parents=True, exist_ok=True)
    start = time.monotonic()
    ix, front_times = relational.prepare([args.ir])
    index_dir = out / "index"
    relational.write(ix, index_dir, args.src, front_times, dict(max_depth=8, max_steps=4000))
    plan, footprints, sessions, timings = scenario_plan.build(ix, args.src)
    unrestricted = scenario_plan.write(plan, footprints, sessions, out / "unrestricted", index_dir, timings)
    sensitivity = []
    base = unrestricted["source_metrics"]["unique_relevant_source_bytes"]
    for fraction in args.budget_fractions.split(","):
        if not fraction:
            continue
        f = float(fraction)
        if not 0 < f <= 1:
            raise ValueError("budget fractions must be in (0, 1]")
        budget = max(1, int(base * f))
        t = time.monotonic()
        result = schedule(plan, footprints, budget)
        record = scenario_plan.write(plan, footprints, result, out / f"budget-{budget}", index_dir,
                                     dict(scheduling=time.monotonic() - t))
        sensitivity.append(dict(fraction_of_unique_source=f, **record))
    package = Path(__file__).resolve().parents[1]
    digest = hashlib.sha256()
    for p in sorted(package.rglob("*.py")):
        if not p.name.startswith("._"):
            digest.update(str(p.relative_to(package)).encode() + b"\0" + p.read_bytes() + b"\0")
    rss = resource.getrusage(resource.RUSAGE_SELF).ru_maxrss
    record = dict(input_ir=str(Path(args.ir).resolve()), input_ir_sha256=hashlib.sha256(Path(args.ir).read_bytes()).hexdigest(),
                  source_root=str(Path(args.src).resolve()), code_sha256=digest.hexdigest(),
                  platform=sys.platform, frontend_timings=front_times, frontend_statistics=ix.stats(),
                  unrestricted=unrestricted, sensitivity=sensitivity, model_calls=0,
                  elapsed_s=time.monotonic() - start, peak_rss_kib=rss / 1024 if sys.platform == "darwin" else rss)
    (out / "measurement.json").write_text(json.dumps(record, indent=2, ensure_ascii=False) + "\n")
    print(json.dumps(dict(out=str(out), statistics=plan.stats(),
                         unrestricted=unrestricted["source_metrics"], sensitivity=[
                             dict(budget=r["source_byte_budget"], metrics=r["source_metrics"],
                                  scheduling=r["scheduling"]) for r in sensitivity],
                         elapsed_s=record["elapsed_s"]), ensure_ascii=False), flush=True)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
