"""Run a frozen case split through the existing module harness.

This only schedules isolated run_module processes. Model calls and tool loops
remain in the official SDK harness. Labels and fixes are never loaded here.
"""
import argparse
from concurrent.futures import ThreadPoolExecutor, as_completed
from collections import Counter
import datetime
import hashlib
import json
import os
from pathlib import Path
import re
import subprocess
import sys
import time

from repository_context import RepositoryContext


def sha256(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def git(project, *args):
    return subprocess.check_output(["git", "-C", str(project), *args], text=True).strip()


def summarize(output):
    manifest_path = output / "run.json"
    if not manifest_path.exists():
        return {"run_manifest": False, "usage_complete": False}
    manifest = json.loads(manifest_path.read_text())
    summary = {"exit_code": manifest.get("exit_code"), "coverage": manifest.get("coverage"),
               "model": manifest["model"], "implementation_commit": manifest.get("revision"),
               "sdk_versions": manifest["sdk_versions"], "usage_complete": False}
    # Primary detection output is the final, deduplicated bug report. Contract
    # counts below are diagnostics and must not be interpreted as bug counts.
    bug_reports = sorted((output / "artifacts").glob("**/stateful_bugs/bugs.txt"))
    if len(bug_reports) > 1:
        raise ValueError("Multiple final reports in an isolated module run")
    if bug_reports:
        final_path = bug_reports[0]
        content = final_path.read_text()
        structured = final_path.parent / "reports.json"
        count = len(json.loads(structured.read_text())) if structured.exists() else len(re.findall(r"^count  \d+ -+$", content, re.MULTILINE))
        summary["final_bug_report"] = {"path": str(final_path), "sha256": sha256(final_path), "count": count,
                                       "structured_path": str(structured) if structured.exists() else None}
    else:
        log_path = output / "run.log"
        explicit_zero = manifest.get("exit_code") == 0 and log_path.exists() and "No bugs detected." in log_path.read_text()
        summary["final_bug_report"] = {"path": None, "count": 0 if explicit_zero else None,
                                       "explicit_zero_in_native_log": explicit_zero}
    if not manifest.get("report"):
        return summary
    report_path = Path(manifest["report"])
    report = json.loads(report_path.read_text())
    completed, tools = [], Counter()
    trace_path = report_path.parent / "llm_simplified_trace.log"
    if trace_path.exists():
        with trace_path.open() as trace:
            for line in trace:
                if line.startswith("[sdk] Completed: "):
                    completed.append(json.loads(line.split(": ", 1)[1]))
                elif line.startswith("[sdk] Tool: "):
                    tools[line.split(" ", 3)[2]] += 1
    rejected = []
    if completed:
        for item in completed[-1]["history"]:
            if item.get("type") != "function_call_output":
                continue
            try:
                value = json.loads(item.get("output", ""))
            except (ValueError, TypeError):
                continue
            if isinstance(value, dict) and "error" in value:
                rejected.append(value["error"])
    summary.update(
        requirement_forms=dict(Counter(r["form"] for r in report["requirements"])),
        requirement_origins=dict(Counter(r["origin"] for r in report["requirements"])),
        guarantee_forms=dict(Counter(g["form"] for g in report["guarantees"])),
        review_statuses=dict(Counter(r["status"] for r in report["reviews"].values())),
        source_supported_ids=[i for i, r in report["reviews"].items() if r["status"] == "source_supported"],
        completed_phase_usage=[r["usage"] for r in completed],
        usage_complete=bool(manifest.get("exit_code") == 0 and len(completed) == (1 if manifest.get("review_disabled") else 2)),
        tool_calls=dict(tools), rejected_calls_in_completed_history=rejected,
        repository_files_read=sorted({e["path"] for e in report["repository_evidence"].values()}),
        gaps=report["gaps"], error=report.get("error"),
        agent_instructions_sha256=hashlib.sha256(report["agent_instructions"].encode()).hexdigest(),
    )
    # Failed phases may have made requests without a Completed usage record.
    if summary["usage_complete"]:
        summary["total_usage"] = {k: sum(r["usage"].get(k, 0) for r in completed)
                                  for k in ["requests", "input_tokens", "output_tokens"]}
    return summary


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--plan", type=Path, required=True)
    parser.add_argument("--split", choices=["development", "validation"], default="development")
    parser.add_argument("--dataset", type=Path, required=True)
    parser.add_argument("--repository", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--no-review", action="store_true", help="Disable final Agent review in every case")
    parser.add_argument("--jobs", type=int, default=2)
    parser.add_argument("--max-turns", type=int, default=96)
    args = parser.parse_args()
    if args.jobs < 1 or args.max_turns < 0:
        parser.error("jobs >= 1 and max-turns >= 0")
    project = Path(__file__).resolve().parents[1]
    if git(project, "status", "--porcelain"):
        parser.error("Commit the implementation before running a frozen batch")
    plan = json.loads(args.plan.read_text())
    selected = [r for r in plan["cases"] if r["split"] == args.split]
    if not selected or len({r["case"] for r in selected}) != len(selected):
        parser.error("Split must contain unique cases")
    prepared = []
    # Verify all inputs before starting any model requests.
    for row in selected:
        case = args.dataset.resolve() / row["case"]
        source, bitcode = case / "src", case / row["ir"]
        if not bitcode.is_file():
            parser.error("Missing IR: " + str(bitcode))
        checked = RepositoryContext(args.repository, row["revision"]).verify_source(source)
        config = case / row["autoconf"] if row.get("autoconf") else None
        if config and not config.is_file():
            parser.error("Missing build configuration: " + str(config))
        source_lines = sum(len((source / f["path"]).read_bytes().splitlines()) for f in checked["verified_files"])
        prepared.append({**row, "source": str(source), "bitcode": str(bitcode),
                         "source_lines": source_lines, "source_check": checked, "bitcode_sha256": sha256(bitcode),
                         "autoconf_path": str(config) if config else None, "autoconf_sha256": sha256(config) if config else None})
    args.output.mkdir(parents=True, exist_ok=False)
    snapshot = {"plan": plan, "selected_split": args.split, "cases": prepared,
                "implementation_commit": git(project, "rev-parse", "HEAD"),
                "binary_sha256": sha256(project / "Release-build/llm_detector"),
                "model": os.environ.get("LLM_MODEL"), "max_turns_per_phase": args.max_turns,
                "jobs": args.jobs, "model_api": os.environ.get("LACE_MODEL_API", "chat_completions"),
                "reasoning_effort": os.environ.get("LACE_REASONING_EFFORT", "provider default"),
                "started_at_utc": datetime.datetime.now(datetime.timezone.utc).isoformat(),
                "ground_truth_files_loaded": False, "review_disabled": args.no_review, "results": {}}
    (args.output / "batch.json").write_text(json.dumps(snapshot, indent=2))

    def run(row):
        output = args.output / row["case"]
        command = [sys.executable, str(project / "harness/run_module.py"),
                   "--source", row["source"], "--bitcode", row["bitcode"], "--output", str(output),
                   "--repository", str(args.repository.resolve()), "--revision", row["revision"],
                   "--max-turns", str(args.max_turns)]
        if args.no_review:
            command.append("--no-review")
        if row.get("autoconf_path"):
            command += ["--autoconf", row["autoconf_path"]]
        print(json.dumps({"started": row["case"], "lines": row["source_lines"]}), flush=True)
        start = time.monotonic()
        # run_module isolates cwd, CPG artifacts, native tools and SDK history.
        proc = subprocess.run(command, cwd=project, capture_output=True, text=True)
        (args.output / (row["case"] + ".runner.log")).write_text(proc.stdout + proc.stderr)
        result = summarize(output)
        result.update(process_exit_code=proc.returncode, elapsed_seconds=round(time.monotonic() - start, 3),
                      implementation_unchanged=git(project, "rev-parse", "HEAD") == snapshot["implementation_commit"]
                      and not git(project, "status", "--porcelain"))
        if output.exists():
            (output / "pilot-summary.json").write_text(json.dumps(result, indent=2))
        return row["case"], result

    with ThreadPoolExecutor(max_workers=args.jobs) as pool:
        futures = {pool.submit(run, row): row["case"] for row in prepared}
        for future in as_completed(futures):
            name = futures[future]
            try:
                name, result = future.result()
            except Exception as error:
                result = {"runner_error": str(error)}
            snapshot["results"][name] = result
            (args.output / "batch.json").write_text(json.dumps(snapshot, indent=2))
            print(json.dumps({"finished": name, "exit_code": result.get("process_exit_code"),
                              "final_reports": result.get("final_bug_report", {}).get("count"),
                              "coverage": result.get("coverage"), "error": result.get("error", result.get("runner_error"))}), flush=True)
    snapshot["finished_at_utc"] = datetime.datetime.now(datetime.timezone.utc).isoformat()
    (args.output / "batch.json").write_text(json.dumps(snapshot, indent=2))
    return 0 if all(r.get("process_exit_code") == 0 for r in snapshot["results"].values()) else 1


if __name__ == "__main__":
    raise SystemExit(main())
