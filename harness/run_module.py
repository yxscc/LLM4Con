"""Run the complete input graph and source-led module Agents SDK workflow."""
import argparse
import importlib.metadata
import json
import hashlib
import os
from pathlib import Path
import subprocess
import sys
from repository_context import RepositoryContext


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--no-review", action="store_true", help="Run extraction plus E only; final Agent review disabled")
    parser.add_argument("--static-only", action="store_true", help="Build complete input graph without model calls")
    parser.add_argument("--catalog", type=Path, help="Versioned protocol catalog; default config/protocol_catalog.json")
    parser.add_argument("--source", type=Path, required=True)
    parser.add_argument("--bitcode", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--model", default=os.getenv("LLM_MODEL"))
    parser.add_argument("--repository", type=Path, help="Read-only Git repository for source context; no checkout or full-repository compilation")
    parser.add_argument("--revision", help="Full commit SHA for all repository queries; must match the target source")
    parser.add_argument("--autoconf", type=Path, help="Generated kernel autoconf.h from the IR build; forwarded to CPG preprocessing. Its hash is recorded, but existing IR provenance is not inferred.")
    parser.add_argument("--binary", type=Path, default=Path(__file__).resolve().parents[1] / "Release-build/llm_detector")
    parser.add_argument("--max-turns", type=int, default=0, help="Optional cap per SDK invocation (0: no cap). Each extraction pass has an invocation; exhaustion remains incomplete.")
    parser.add_argument("--requirement-sweep", choices=["on","off"], default=os.getenv("LACE_REQUIREMENT_SWEEP","on"),
                        help="Further source-led Req discovery within Phase A; off supports controlled ablation.")
    args = parser.parse_args()
    if not args.source.is_dir() or not args.bitcode.is_file():
        parser.error("--source must be an existing directory and --bitcode an existing matching IR file")
    if not args.model and not args.static_only:
        parser.error("Set --model or LLM_MODEL")
    if args.max_turns < 0:
        parser.error("--max-turns must be nonnegative (0 disables the cap)")
    if bool(args.repository) != bool(args.revision):
        parser.error("--repository and --revision must be provided together")
    if args.autoconf and not args.autoconf.is_file():
        parser.error("--autoconf must be an existing build header")
    if args.catalog and not args.catalog.is_file():
        parser.error("--catalog must be an existing JSON catalog")
    context = None
    if args.repository:
        context = RepositoryContext(args.repository, args.revision)
        source_check = context.verify_source(args.source)
    env = os.environ.copy()
    env.update(LACE_PYTHON=sys.executable, LLM_MODEL=args.model or "",
               LACE_DUMP_ROOT=str(args.output.resolve() / "artifacts"), LACE_MODULE_MAX_TURNS=str(args.max_turns),
               LACE_REQUIREMENT_SWEEP=args.requirement_sweep)
    if args.catalog:
        env["LACE_PROTOCOL_CATALOG"] = str(args.catalog.resolve())
    env.pop('LACE_CPG_AUTOCONF', None)
    if args.autoconf:
        env['LACE_CPG_AUTOCONF'] = str(args.autoconf.resolve())
    for key in ('LACE_CONTEXT_REPOSITORY', 'LACE_CONTEXT_REVISION'):
        env.pop(key, None)
    if context:
        env.update(LACE_CONTEXT_REPOSITORY=context.repository, LACE_CONTEXT_REVISION=context.revision)
    args.output.mkdir(parents=True, exist_ok=False)
    command = [str(args.binary.resolve()), "--input-src", str(args.source.resolve()),
               "--input-bc", str(args.bitcode.resolve())]
    if args.static_only:
        command.append("--static-only")
    if args.no_review:
        command.append("--no-review")
    # Paths/model/budget are safe metadata. Credentials and gateway URL are not
    # copied into the manifest or process arguments.
    manifest = {"model": args.model, "harness": "openai-agents", "max_turns_per_phase": args.max_turns,
                "max_turns_per_sdk_invocation": args.max_turns, "requirement_sweep_requested": args.requirement_sweep,
                "sdk_versions": {name: importlib.metadata.version(name) for name in ("openai", "openai-agents")},
                "model_api": env.get("LACE_MODEL_API", "chat_completions"),
                "context_management_requested": env.get("LACE_CONTEXT_MANAGEMENT", "auto"),
                "compaction_threshold": env.get("LACE_COMPACT_THRESHOLD", "sdk_default"),
                "reasoning_effort": env.get("LACE_REASONING_EFFORT", "provider default"),
                "source": str(args.source.resolve()), "bitcode": str(args.bitcode.resolve()),
                "entry_mode": "not_inferred", "graph_scope": "complete_input_module",
                "agent_observation_policy": "source_first_ir_on_demand_no_ast",
                "source_access": "sdk_exec_command_write_stdin_read_only_workspace",
                "ground_truth_files_loaded": False, "review_disabled": args.no_review}
    manifest['cpg_build_config'] = {'mode': 'explicit_autoconf' if args.autoconf else 'legacy_compile_log_discovery',
        'path': str(args.autoconf.resolve()) if args.autoconf else None,
        'sha256': hashlib.sha256(args.autoconf.read_bytes()).hexdigest() if args.autoconf else None,
        'matches_existing_ir_build': 'unverified',
        'note': 'Supply the actual header saved from the IR build; no case-based CONFIG guesses. Missing config/source mappings remain explicit gaps.'}
    if context:
        manifest['repository_context'] = {'path': context.repository, **source_check}
    repo = Path(__file__).resolve().parents[1]
    revision = subprocess.run(["git", "-C", str(repo), "rev-parse", "HEAD"], text=True, capture_output=True)
    if revision.returncode == 0:
        manifest["revision"] = revision.stdout.strip()
        dirty = subprocess.run(["git", "-C", str(repo), "status", "--porcelain", "--untracked-files=no"], text=True, capture_output=True, check=True)
        manifest["implementation_dirty"] = bool(dirty.stdout.strip())
    catalog_path = Path(env.get("LACE_PROTOCOL_CATALOG", repo / "config/protocol_catalog.json")).resolve()
    manifest["protocol_catalog"] = {"path": str(catalog_path), "sha256": hashlib.sha256(catalog_path.read_bytes()).hexdigest(), "content": json.loads(catalog_path.read_text())}
    implementation = hashlib.sha256()
    code_files = sorted(f for folder in ('src', 'include', 'harness', 'config') for f in (repo / folder).rglob('*')
                        if f.is_file() and '__pycache__' not in f.parts)
    code_files += [repo / name for name in ('CMakeLists.txt', 'requirements-harness.txt')]
    for path in code_files:
        implementation.update(path.relative_to(repo).as_posix().encode() + b'\0' + path.read_bytes() + b'\0')
    manifest['implementation_sha256'] = implementation.hexdigest()
    manifest['bitcode_sha256'] = hashlib.sha256(args.bitcode.read_bytes()).hexdigest()
    manifest['source_snapshot'] = [
        {'path': f.relative_to(args.source).as_posix(), 'sha256': hashlib.sha256(f.read_bytes()).hexdigest()}
        for f in sorted(args.source.rglob('*')) if f.is_file() and not f.is_symlink()
        and f.suffix in {'.c', '.h', '.cpp', '.hpp', '.hh', '.cc', '.cxx'}]
    (args.output / "run.json").write_text(json.dumps(manifest, indent=2))
    with (args.output / "run.log").open("w") as log:
        code = subprocess.call(command, env=env, cwd=args.output, stdout=log, stderr=subprocess.STDOUT)
    manifest["exit_code"] = code
    reports = list((args.output / "artifacts").glob("**/module_contracts.json"))
    if reports:
        report = json.loads(reports[-1].read_text())
        manifest["coverage"] = report["coverage"]
        manifest["report"] = str(reports[-1])
    (args.output / "run.json").write_text(json.dumps(manifest, indent=2))
    print(json.dumps(manifest, indent=2))
    return code


if __name__ == "__main__":
    sys.exit(main())
