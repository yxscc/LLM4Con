#!/usr/bin/env python3
"""Validate static capability coverage against flow_annotation.json files.

This is a lightweight static-front-end sanity checker. It does not judge final
recall or precision. It answers whether the static surface contains enough
grounding for the detector to reason: expected thread entries/call-chain
functions, key shared-object field tokens, and bounded entry/surface sizes.
"""

from __future__ import annotations

import argparse
import json
import re
from dataclasses import dataclass, asdict
from pathlib import Path
from typing import Any


TOKEN_RE = re.compile(r"[A-Za-z_][A-Za-z0-9_]*")
SYMBOL_RE = re.compile(r"^[A-Za-z_][A-Za-z0-9_.$]*$")
STOP_OBJECT_TOKENS = {
    "struct",
    "field",
    "object",
    "ptr",
    "data",
    "race",
    "read",
    "write",
    "use",
    "free",
    "list",
    "count",
    "state",
    "lock",
    "thread",
    "function",
}


@dataclass
class CaseExpectation:
    case_id: str
    annotation: Path
    expected_entries: list[str]
    expected_call_functions: list[str]
    object_tokens: list[str]


@dataclass
class CaseResult:
    case_id: str
    annotation: str
    surface: str | None
    status: str
    expected_entries: list[str]
    matched_entries: list[str]
    missing_entries: list[str]
    expected_call_functions: list[str]
    matched_call_functions: list[str]
    missing_call_functions: list[str]
    object_tokens: list[str]
    matched_object_tokens: list[str]
    missing_object_tokens: list[str]
    total_threads: int | None
    shared_objects: int | None
    high_risk_objects: int | None
    zero_object_threads: int | None
    low_object_threads: int | None
    top_thread_contributors: list[dict[str, Any]]
    explosion_warnings: list[str]


def read_json(path: Path) -> Any:
    with path.open("r", encoding="utf-8", errors="ignore") as f:
        return json.load(f)


def uniq(items: list[str]) -> list[str]:
    seen: set[str] = set()
    out: list[str] = []
    for item in items:
        if not item or item in seen:
            continue
        seen.add(item)
        out.append(item)
    return out


def split_entry_function_text(text: str) -> list[str]:
    """Return concrete function symbols from annotation entry text.

    Some annotations use "foo / bar" to describe equivalent entry families,
    while others use natural-language phrases such as "event removal path".
    Only keep symbol-like entries so natural-language phrases do not become
    hard MISS_ENTRY blockers.
    """
    if SYMBOL_RE.match(text):
        return [text]
    symbols = [
        tok for tok in TOKEN_RE.findall(text)
        if "_" in tok and tok not in STOP_OBJECT_TOKENS
    ]
    return uniq(symbols)


def syscall_entry_aliases(name: str) -> list[str]:
    prefixes = [
        "__x64_sys_",
        "__ia32_sys_",
        "__arm64_sys_",
        "__se_sys_",
        "__do_sys_",
        "SyS_",
        "sys_",
    ]
    suffix = None
    for prefix in prefixes:
        if name.startswith(prefix):
            suffix = name[len(prefix):]
            break
    if suffix is None:
        return [name]
    return uniq([prefix + suffix for prefix in prefixes])


def entry_present_as_thread_root(entry: str, thread_roots: list[str]) -> bool:
    thread_root_set = set(thread_roots)
    for alias in syscall_entry_aliases(entry):
        if alias in thread_root_set:
            return True
    return False


def collect_function_names(node: Any) -> list[str]:
    out: list[str] = []
    if isinstance(node, dict):
        fn = node.get("function")
        if isinstance(fn, str):
            out.append(fn)
        for value in node.values():
            out.extend(collect_function_names(value))
    elif isinstance(node, list):
        for value in node:
            out.extend(collect_function_names(value))
    return out


def collect_entry_functions(data: dict[str, Any]) -> list[str]:
    entries: list[str] = []
    ti = data.get("true_interleaving", {})
    if isinstance(ti, dict):
        for key in sorted(ti.keys()):
            thread = ti.get(key)
            if not isinstance(thread, dict):
                continue
            entry = thread.get("entry")
            if isinstance(entry, dict) and isinstance(entry.get("function"), str):
                entries.extend(split_entry_function_text(entry["function"]))
    return uniq(entries)


def collect_object_strings(data: dict[str, Any]) -> list[str]:
    strings: list[str] = []
    gt = data.get("ground_truth_access", {})
    if isinstance(gt, dict) and isinstance(gt.get("object"), str):
        strings.append(gt["object"])

    surface = data.get("vulnerability_surface", {})
    if isinstance(surface, dict):
        for key in ("primary_object", "why_relevant"):
            if isinstance(surface.get(key), str):
                strings.append(surface[key])
        aliases = surface.get("aliases", [])
        if isinstance(aliases, list):
            for alias in aliases:
                if isinstance(alias, dict) and isinstance(alias.get("name"), str):
                    strings.append(alias["name"])
    return strings


def object_tokens_from_strings(strings: list[str]) -> list[str]:
    tokens: list[str] = []
    for text in strings:
        for tok in TOKEN_RE.findall(text):
            if tok in STOP_OBJECT_TOKENS:
                continue
            if len(tok) < 3:
                continue
            # Keep field-ish and struct-ish tokens. This intentionally stays
            # token-based because annotations often say "foo / bar" while the
            # surface says "field:struct.x.foo@offset".
            tokens.append(tok)
    return uniq(tokens)


def load_expectation(annotation: Path) -> CaseExpectation:
    data = read_json(annotation)
    case_id = data.get("bug_id") or annotation.parent.name
    entries = collect_entry_functions(data)
    call_functions = collect_function_names(data.get("true_interleaving", {}))
    object_tokens = object_tokens_from_strings(collect_object_strings(data))
    return CaseExpectation(
        case_id=case_id,
        annotation=annotation,
        expected_entries=entries,
        expected_call_functions=uniq(call_functions),
        object_tokens=object_tokens,
    )


def latest_surface(case_id: str, dump_dir: Path) -> Path | None:
    candidates = sorted(
        dump_dir.glob(f"{case_id}_*/vulnerability_surface.json"),
        key=lambda p: p.stat().st_mtime,
        reverse=True,
    )
    return candidates[0] if candidates else None


def surface_text(surface: dict[str, Any]) -> str:
    pieces: list[str] = []
    for thread in surface.get("threads", []):
        if isinstance(thread, dict):
            pieces.append(str(thread.get("entry_function", "")))
    for obj in surface.get("shared_objects", []):
        if not isinstance(obj, dict):
            continue
        pieces.append(str(obj.get("name", "")))
        for access in obj.get("accesses", []):
            if isinstance(access, dict):
                pieces.append(str(access.get("function", "")))
                pieces.append(str(access.get("containing_function", "")))
                pieces.append(str(access.get("code", "")))
    return "\n".join(pieces)


def evaluate_case(
    exp: CaseExpectation,
    surface_path: Path | None,
    max_threads: int,
    max_shared_objects: int,
) -> CaseResult:
    if surface_path is None or not surface_path.exists():
        return CaseResult(
            case_id=exp.case_id,
            annotation=str(exp.annotation),
            surface=None,
            status="NO_SURFACE",
            expected_entries=exp.expected_entries,
            matched_entries=[],
            missing_entries=exp.expected_entries,
            expected_call_functions=exp.expected_call_functions,
            matched_call_functions=[],
            missing_call_functions=exp.expected_call_functions,
            object_tokens=exp.object_tokens,
            matched_object_tokens=[],
            missing_object_tokens=exp.object_tokens,
            total_threads=None,
            shared_objects=None,
            high_risk_objects=None,
            zero_object_threads=None,
            low_object_threads=None,
            top_thread_contributors=[],
            explosion_warnings=[],
        )

    data = read_json(surface_path)
    threads = [
        str(t.get("entry_function", ""))
        for t in data.get("threads", [])
        if isinstance(t, dict)
    ]
    text = surface_text(data)

    matched_entries = [
        fn for fn in exp.expected_entries
        if entry_present_as_thread_root(fn, threads)
    ]
    missing_entries = [
        fn for fn in exp.expected_entries
        if not entry_present_as_thread_root(fn, threads)
    ]
    matched_call_functions = [
        fn for fn in exp.expected_call_functions if fn in text
    ]
    missing_call_functions = [
        fn for fn in exp.expected_call_functions if fn not in text
    ]
    matched_tokens = [tok for tok in exp.object_tokens if tok in text]
    missing_tokens = [tok for tok in exp.object_tokens if tok not in text]

    total_threads = data.get("total_threads")
    if not isinstance(total_threads, int):
        total_threads = len(threads)
    shared_objects = data.get("shared_objects", [])
    shared_count = len(shared_objects) if isinstance(shared_objects, list) else None
    high_risk = 0
    object_ids_by_thread: dict[int, set[int]] = {}
    access_count_by_thread: dict[int, int] = {}
    if isinstance(shared_objects, list):
        for obj_idx, obj in enumerate(shared_objects):
            if not isinstance(obj, dict):
                continue
            if int(obj.get("risk_score", 0)) >= 100:
                high_risk += 1
            for access in obj.get("accesses", []):
                if not isinstance(access, dict):
                    continue
                tid = access.get("thread_id")
                if not isinstance(tid, int):
                    continue
                object_ids_by_thread.setdefault(tid, set()).add(obj_idx)
                access_count_by_thread[tid] = access_count_by_thread.get(tid, 0) + 1

    zero_object_threads = 0
    low_object_threads = 0
    contributors: list[dict[str, Any]] = []
    for tid, entry in enumerate(threads):
        obj_count = len(object_ids_by_thread.get(tid, set()))
        acc_count = access_count_by_thread.get(tid, 0)
        if obj_count == 0:
            zero_object_threads += 1
        if obj_count <= 2:
            low_object_threads += 1
        contributors.append(
            {
                "thread_id": tid,
                "entry": entry,
                "object_count": obj_count,
                "access_count": acc_count,
                "expected": entry in exp.expected_call_functions,
            }
        )
    contributors.sort(key=lambda x: (x["object_count"], x["access_count"]), reverse=True)
    top_contributors = contributors[:10]

    warnings: list[str] = []
    if total_threads is not None and total_threads > max_threads:
        warnings.append(f"threads>{max_threads}")
    if shared_count is not None and shared_count > max_shared_objects:
        warnings.append(f"shared_objects>{max_shared_objects}")
    if total_threads and zero_object_threads > max(10, total_threads // 10):
        warnings.append("many_zero_object_threads")
    if total_threads and low_object_threads > max(20, total_threads // 4):
        warnings.append("many_low_object_threads")

    if missing_entries:
        status = "MISS_ENTRY"
    elif exp.object_tokens and len(matched_tokens) == 0:
        status = "MISS_OBJECT"
    elif missing_call_functions:
        status = "PARTIAL_CALLCHAIN"
    else:
        status = "PASS"
    if warnings:
        status = "PASS_WITH_EXPLOSION_RISK" if status == "PASS" else status + "_EXPLOSION_RISK"

    return CaseResult(
        case_id=exp.case_id,
        annotation=str(exp.annotation),
        surface=str(surface_path),
        status=status,
        expected_entries=exp.expected_entries,
        matched_entries=matched_entries,
        missing_entries=missing_entries,
        expected_call_functions=exp.expected_call_functions,
        matched_call_functions=matched_call_functions,
        missing_call_functions=missing_call_functions,
        object_tokens=exp.object_tokens,
        matched_object_tokens=matched_tokens,
        missing_object_tokens=missing_tokens,
        total_threads=total_threads,
        shared_objects=shared_count,
        high_risk_objects=high_risk,
        zero_object_threads=zero_object_threads,
        low_object_threads=low_object_threads,
        top_thread_contributors=top_contributors,
        explosion_warnings=warnings,
    )


def summarize(results: list[CaseResult]) -> dict[str, Any]:
    by_status: dict[str, int] = {}
    for r in results:
        by_status[r.status] = by_status.get(r.status, 0) + 1
    with_surface = [r for r in results if r.surface]
    return {
        "total_cases": len(results),
        "cases_with_surface": len(with_surface),
        "by_status": dict(sorted(by_status.items())),
        "max_threads": max((r.total_threads or 0 for r in with_surface), default=0),
        "max_shared_objects": max((r.shared_objects or 0 for r in with_surface), default=0),
        "explosion_risk_cases": sum(1 for r in results if r.explosion_warnings),
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--kernel-experiment", type=Path, default=Path("kernel_experiment"))
    parser.add_argument("--dump-dir", type=Path, default=Path("LLM_dump"))
    parser.add_argument("--case", action="append", default=[])
    parser.add_argument("--max-threads", type=int, default=100)
    parser.add_argument("--max-shared-objects", type=int, default=500)
    parser.add_argument("--json-out", type=Path)
    parser.add_argument("--summary-only", action="store_true")
    args = parser.parse_args()

    annotations = sorted(args.kernel_experiment.glob("*/flow_annotation.json"))
    if args.case:
        wanted = set(args.case)
        annotations = [p for p in annotations if p.parent.name in wanted]

    results: list[CaseResult] = []
    for ann in annotations:
        exp = load_expectation(ann)
        surface = latest_surface(exp.case_id, args.dump_dir)
        results.append(
            evaluate_case(
                exp,
                surface,
                max_threads=args.max_threads,
                max_shared_objects=args.max_shared_objects,
            )
        )

    payload = {
        "summary": summarize(results),
        "results": [asdict(r) for r in results],
    }
    if args.json_out:
        args.json_out.parent.mkdir(parents=True, exist_ok=True)
        args.json_out.write_text(json.dumps(payload, indent=2), encoding="utf-8")

    print(json.dumps(payload["summary"], indent=2))
    if not args.summary_only:
        for r in results:
            missing = []
            if r.missing_entries:
                missing.append("entries=" + ",".join(r.missing_entries))
            if r.object_tokens and not r.matched_object_tokens:
                missing.append("objects=" + ",".join(r.object_tokens[:8]))
            if r.explosion_warnings:
                missing.append("risk=" + ",".join(r.explosion_warnings))
            detail = (" " + " ".join(missing)) if missing else ""
            print(
                f"{r.status}\t{r.case_id}\tthreads={r.total_threads}"
                f"\tobjects={r.shared_objects}{detail}"
            )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
