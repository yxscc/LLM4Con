"""Run the v2 static front-end and the complete-module Agents SDK workflow."""
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
    parser.add_argument("--source", type=Path, required=True)
    parser.add_argument("--bitcode", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--model", default=os.getenv("LLM_MODEL"))
    parser.add_argument("--repository", type=Path, help="Read-only Git repository for source context; no checkout or full-repository compilation")
    parser.add_argument("--revision", help="Full commit SHA for all repository queries; must match the target source")
    parser.add_argument("--autoconf", type=Path, help="Generated kernel autoconf.h from the IR build; forwarded to CPG preprocessing. Its hash is recorded, but existing IR provenance is not inferred.")
    parser.add_argument("--binary", type=Path, default=Path(__file__).resolve().parents[1] / "Release-build/llm_detector")
    parser.add_argument("--max-turns", type=int, default=32, help="Per-phase execution budget; exhaustion is an incomplete error, never truncation of obligations.")
    args = parser.parse_args()
    if not args.model:
        parser.error("Set --model or LLM_MODEL")
    if args.max_turns < 0:
        parser.error("--max-turns must be nonnegative (0 disables the cap)")
    if bool(args.repository) != bool(args.revision):
        parser.error("--repository and --revision must be provided together")
    if args.autoconf and not args.autoconf.is_file():
        parser.error("--autoconf must be an existing build header")
    context = None
    if args.repository:
        context = RepositoryContext(args.repository, args.revision)
        source_check = context.verify_source(args.source)
    env = os.environ.copy()
    env.update(LACE_PYTHON=sys.executable, LLM_MODEL=args.model,
               LACE_DUMP_ROOT=str(args.output.resolve() / "artifacts"), LACE_MODULE_MAX_TURNS=str(args.max_turns))
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
    # Paths/model/budget are safe metadata. Credentials and gateway URL are not
    # copied into the manifest or process arguments.
    manifest = {"model": args.model, "harness": "openai-agents", "max_turns_per_phase": args.max_turns,
                "sdk_versions": {name: importlib.metadata.version(name) for name in ("openai", "openai-agents")},
                "model_api": env.get("LACE_MODEL_API", "chat_completions"),
                "reasoning_effort": env.get("LACE_REASONING_EFFORT", "provider default"),
                "source": str(args.source.resolve()), "bitcode": str(args.bitcode.resolve()),
                "entry_mode": "configured" if env.get("LACE_ENTRYPOINTS") or env.get("LACE_ENTRYPOINTS_FILE") else "automatic",
                "ground_truth_files_loaded": False}
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
    implementation = hashlib.sha256()
    code_files = sorted(f for folder in ('src', 'include', 'harness') for f in (repo / folder).rglob('*')
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
