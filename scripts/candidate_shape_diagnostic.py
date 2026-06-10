#!/usr/bin/env python3
"""Summarize candidate-graph and race-shape compression potential.

This is a read-only diagnostic for the static-compose p4x4 run. It intentionally
does not call the detector or judge recall. It consumes existing
vulnerability_surface.json dumps, enumerates raw conflicting cross-thread pairs,
and estimates how much cost could be reduced by grouping candidates into
deterministic race shapes instead of truncating by risk rank.

The counts are approximate: they are derived from the surface JSON only and do
not replay the C++ MHP, contract, or Phase-B discharge logic.
"""

from __future__ import annotations

import argparse
import csv
import json
import re
from collections import Counter, defaultdict
from dataclasses import asdict, dataclass, field
from pathlib import Path
from typing import Any


DEFAULT_ROOT = Path("/mlx_devbox/users/mayunlong.39/playground/LLM4Con")
DEFAULT_RUN = DEFAULT_ROOT / "kernel_experiment/full_staticcompose_p4x4_20260610_010518"
DEFAULT_EXCLUSIONS = DEFAULT_ROOT / "kernel_experiment/dataset_benign_exclusions.json"
DEFAULT_DUMP_BASE = DEFAULT_ROOT / "LLM_dump"
DEFAULT_EXPERIMENT_BASE = DEFAULT_ROOT / "kernel_experiment"

# Generic field/struct tokens that match almost any kernel object and so are
# useless for grounding a specific GT race. Dropped from the strong-token set.
GT_STOPWORDS = {
    "struct", "field", "object", "ptr", "data", "race", "read", "write", "use",
    "free", "list", "count", "state", "lock", "thread", "function", "flags",
    "err", "len", "used", "val", "work", "node", "head", "next", "prev",
    "size", "type", "mux", "sk", "skb", "sock", "kvm", "net", "the", "and",
    "its", "for", "with", "from", "into", "true", "false", "null", "entry",
    "atomic", "once",
}


@dataclass
class CaseStats:
    case: str
    status: str
    source_surface: str
    shared_objects: int = 0
    total_threads: int = 0
    candidate_objects: int = 0
    raw_candidates: int = 0
    common_lock_candidates: int = 0
    no_common_lock_candidates: int = 0
    object_anchor_groups: int = 0
    race_shape_classes: int = 0
    coarse_shape_classes: int = 0
    object_family_classes: int = 0
    max_raw_candidates_per_object: int = 0
    skipped_reason: str = ""
    borderline_actionable: bool = False
    top_objects: list[dict[str, Any]] = field(default_factory=list)
    top_shapes: list[dict[str, Any]] = field(default_factory=list)
    # GT coverage (approximate, surface-only)
    gt_tokens: list[str] = field(default_factory=list)
    gt_token_in_surface: bool = False
    gt_object_in_surface: bool = False
    gt_candidate_covered: bool = False
    gt_matched_objects: list[str] = field(default_factory=list)
    gt_shape_classes: int = 0
    # actual run accounting (from summary.tsv)
    actual_sessions: int = 0
    actual_calibrated: int = 0


def read_json(path: Path) -> Any:
    with path.open("r", encoding="utf-8", errors="ignore") as f:
        return json.load(f)


_IDENT_RE = re.compile(r"[A-Za-z_][A-Za-z0-9_]*")
_ARROW_RE = re.compile(r"(?:->|\.)([A-Za-z_][A-Za-z0-9_]*)")
_ONCE_RE = re.compile(r"(?:READ_ONCE|WRITE_ONCE|data_race)\(\s*([^,()]+?)\s*[,)]")
_STRUCT_RE = re.compile(r"struct\s+([A-Za-z_][A-Za-z0-9_]*)")


def _leaf_tokens(text: str) -> set[str]:
    """Leaf field identifiers from a 'a->b.c' / 'struct X.field' expression."""
    out: set[str] = set()
    for m in _ARROW_RE.findall(text or ""):
        if len(m) >= 4 and m.lower() not in GT_STOPWORDS:
            out.add(m)
    return out


def load_gt_tokens(case: str, experiment_base: Path) -> tuple[list[str], list[str]]:
    """Return (strong_leaf_tokens, struct_tokens) describing the GT object.

    Strong sources, in order: expected_contract.shared_object, flow_annotation
    object/primary_object, ground_truth patch READ_ONCE/WRITE_ONCE/data_race
    fields. Surface-only diagnostic; matching is approximate.
    """
    cdir = experiment_base / case
    leaf: set[str] = set()
    structs: set[str] = set()

    ec = cdir / "expected_contract.json"
    if ec.exists():
        try:
            so = str(read_json(ec).get("shared_object", ""))
            leaf |= _leaf_tokens(so)
            structs |= {s for s in _STRUCT_RE.findall(so) if s.lower() not in GT_STOPWORDS}
        except Exception:
            pass

    fa = cdir / "flow_annotation.json"
    if fa.exists():
        try:
            doc = read_json(fa)
            for key in ("object", "primary_object", "summary"):
                v = doc.get(key)
                if isinstance(v, str):
                    leaf |= _leaf_tokens(v)
                    structs |= {s for s in _STRUCT_RE.findall(v) if s.lower() not in GT_STOPWORDS}
            gta = doc.get("ground_truth_access")
            if isinstance(gta, dict):
                v = str(gta.get("object", ""))
                leaf |= _leaf_tokens(v)
                structs |= {s for s in _STRUCT_RE.findall(v) if s.lower() not in GT_STOPWORDS}
        except Exception:
            pass

    gt = cdir / "ground_truth.json"
    if gt.exists():
        try:
            patch = str(read_json(gt).get("patch", ""))
            for ln in patch.splitlines():
                if not ln.startswith("+") or ln.startswith("+++"):
                    continue
                for f in _ONCE_RE.findall(ln):
                    leaf |= _leaf_tokens(f)
                    bare = _IDENT_RE.findall(f)
                    for b in bare[-1:]:  # trailing bare identifier (e.g. *valp -> valp)
                        if len(b) >= 4 and b.lower() not in GT_STOPWORDS:
                            leaf.add(b)
        except Exception:
            pass

    return sorted(leaf), sorted(structs)


def load_session_actuals(run_dir: Path) -> dict[str, tuple[int, int]]:
    """case -> (sessions, calibrated_runs) parsed from summary.tsv."""
    out: dict[str, tuple[int, int]] = {}
    summ = run_dir / "summary.tsv"
    if not summ.exists():
        return out
    with summ.open("r", encoding="utf-8", errors="ignore") as f:
        for row in csv.DictReader(f, delimiter="\t"):
            case = row.get("case")
            if not case:
                continue
            try:
                sessions = int(row.get("sessions") or 0)
            except ValueError:
                sessions = 0
            cal = row.get("calibrated") or ""
            try:
                calibrated = int(cal.split("/", 1)[0]) if cal else 0
            except ValueError:
                calibrated = 0
            out[case] = (sessions, calibrated)
    return out


def object_matches_gt(name: str, leaf_tokens: set[str], struct_tokens: set[str]) -> bool:
    """Whether a surface object NAME plausibly is the GT object.

    Name-based (not access-code-based) so that GT reads folded into an unrelated
    container object (e.g. last_boosted_vcpu folded into kvm_vcpu.kvm) are NOT
    counted as covered.
    """
    if not name:
        return False
    if any(tok in name for tok in leaf_tokens):
        return True
    # struct-only match helps short leaves (e.g. wd) whose struct is specific.
    if struct_tokens and any(("struct." + s) in name or ("." + s + ".") in name for s in struct_tokens):
        return True
    return False


def normalize_code(code: str) -> str:
    code = re.sub(r"\s+", " ", code or "").strip()
    code = re.sub(r"0x[0-9a-fA-F]+", "0xADDR", code)
    code = re.sub(r"\b\d+\b", "N", code)
    return code[:180]


def field_family(name: str) -> str:
    """Return an object-family label that can merge sibling field instances."""
    if not name:
        return "<anon>"
    m = re.match(r"field:(struct\.[^.@+]+)", name)
    if m:
        return "field:" + m.group(1) + ".*"
    if name.startswith("global:"):
        return "global:*"
    if name.startswith("obj:"):
        return "obj:*"
    return name.split("@", 1)[0]


def lock_tokens(lock: str) -> frozenset[str]:
    if not lock:
        return frozenset()
    # Keep lock expressions coarse but stable. This is diagnostic only.
    toks = re.findall(r"[A-Za-z_][A-Za-z0-9_]*(?:->[A-Za-z_][A-Za-z0-9_]*)*", lock)
    ignore = {
        "mutex_lock", "mutex_unlock", "spin_lock", "spin_unlock",
        "spin_lock_bh", "spin_unlock_bh", "read_lock", "write_lock",
        "raw_spin_lock", "raw_spin_unlock", "rcu_read_lock", "rcu_read_unlock",
    }
    return frozenset(t for t in toks if t not in ignore)


def access_kind(access: dict[str, Any]) -> str:
    typ = str(access.get("access_type", "")).lower()
    code = str(access.get("code", "")).lower()
    if "free" in typ or re.search(r"\b(kfree|vfree|free_|consume_skb|kfree_skb|call_rcu)\b", code):
        return "free"
    if "write" in typ or "store" in code or re.search(r"(^|[^=!<>])=([^=]|$)", code):
        return "write"
    if "read" in typ or "load" in code:
        return "read"
    if "call" in typ:
        return "call"
    return "touch"


def use_context(accesses: list[dict[str, Any]]) -> str:
    text = " ".join(str(a.get("code", "")) for a in accesses).lower()
    if re.search(r"\[[^\]]+\]", text):
        return "index"
    if re.search(r"->|\\*", text) and re.search(r"\b(if|while|return|!|==|!=|<|>)\b", text):
        return "guarded_ptr_or_field"
    if re.search(r"\b(kfree|vfree|free_|consume_skb|kfree_skb|call_rcu)\b", text):
        return "lifetime"
    if re.search(r"\b(list_add|list_del|hlist_|rb_|xarray|idr_)\b", text):
        return "container"
    if re.search(r"\b(if|while|switch|==|!=|<|>)\b", text):
        return "branch"
    if re.search(r"\bmemcpy|memmove|copy_(to|from)_user\b", text):
        return "bulk_copy"
    return "plain"


def hazard_kind(obj: dict[str, Any]) -> str:
    if obj.get("has_free_operation"):
        return "lifetime"
    if obj.get("has_list_mutation"):
        return "container"
    if obj.get("is_self_race"):
        return "self_race"
    if obj.get("has_scalar_torn_access") or obj.get("has_missing_atomic_annotation"):
        return "scalar_rw"
    if obj.get("has_cross_thread_rw"):
        return "rw"
    return "touch"


def representative_access(accesses: list[dict[str, Any]], prefer_mutating: bool) -> dict[str, Any]:
    if not accesses:
        return {}
    if prefer_mutating:
        for a in accesses:
            if access_kind(a) in {"free", "write"}:
                return a
    return accesses[0]


def summarize_surface(
    surface: dict[str, Any],
    case: str,
    status: str,
    source: Path,
    borderline: bool,
    gt_leaf: list[str],
    gt_structs: list[str],
) -> CaseStats:
    stats = CaseStats(
        case=case,
        status=status,
        source_surface=str(source),
        shared_objects=len(surface.get("shared_objects", [])),
        total_threads=int(surface.get("total_threads") or 0),
        borderline_actionable=borderline,
        gt_tokens=sorted(set(gt_leaf) | set(gt_structs)),
    )
    leaf_set = set(gt_leaf)
    struct_set = set(gt_structs)
    gt_matched_objs: set[str] = set()
    gt_shape_keys: set[tuple[Any, ...]] = set()

    object_anchor_keys: set[tuple[Any, ...]] = set()
    race_shape_keys: set[tuple[Any, ...]] = set()
    coarse_shape_keys: set[tuple[Any, ...]] = set()
    family_keys: set[str] = set()
    object_counts: Counter[str] = Counter()
    shape_counts: Counter[tuple[Any, ...]] = Counter()

    for obj_idx, obj in enumerate(surface.get("shared_objects", [])):
        obj_name = str(obj.get("name", ""))
        obj_is_gt_name = bool((leaf_set or struct_set) and object_matches_gt(obj_name, leaf_set, struct_set))
        if not stats.gt_token_in_surface and (leaf_set or struct_set):
            blob = obj_name + " " + " ".join(str(a.get("code", "")) for a in obj.get("accesses", []))
            if any(tok in blob for tok in leaf_set) or any(("struct." + s) in blob for s in struct_set):
                stats.gt_token_in_surface = True
        if obj_is_gt_name:
            stats.gt_object_in_surface = True
        by_thread: dict[int, list[dict[str, Any]]] = defaultdict(list)
        for access in obj.get("accesses", []):
            tid = access.get("thread_id")
            if tid is None:
                continue
            try:
                tid = int(tid)
            except (TypeError, ValueError):
                continue
            by_thread[tid].append(access)

        tids = sorted(by_thread)
        object_raw = 0
        object_had_candidate = False
        hz = hazard_kind(obj)
        family = field_family(str(obj.get("name", "")))

        for i, t1 in enumerate(tids):
            for t2 in tids[i + 1:]:
                a1 = by_thread[t1]
                a2 = by_thread[t2]
                kinds1 = {access_kind(a) for a in a1}
                kinds2 = {access_kind(a) for a in a2}
                mut1 = bool(kinds1 & {"write", "free"})
                mut2 = bool(kinds2 & {"write", "free"})
                if not (mut1 or mut2):
                    continue

                object_had_candidate = True
                object_raw += 1
                stats.raw_candidates += 1

                locks1 = set().union(*(lock_tokens(str(a.get("lock", ""))) for a in a1))
                locks2 = set().union(*(lock_tokens(str(a.get("lock", ""))) for a in a2))
                common_lock = bool(locks1 and locks2 and locks1.intersection(locks2))
                if common_lock:
                    stats.common_lock_candidates += 1
                    lock_relation = "common_lock"
                else:
                    stats.no_common_lock_candidates += 1
                    lock_relation = "no_common_lock"

                mutating_accesses = a1 if mut1 else a2
                reading_accesses = a2 if mut1 else a1
                mut = representative_access(mutating_accesses, prefer_mutating=True)
                read = representative_access(reading_accesses, prefer_mutating=False)
                mut_fn = str(mut.get("containing_function") or mut.get("function") or "")
                read_fn = str(read.get("containing_function") or read.get("function") or "")
                mut_code = normalize_code(str(mut.get("code", "")))
                mut_loc = str(mut.get("location", "")).rsplit("/", 1)[-1]
                ctx = use_context(mutating_accesses + reading_accesses)

                object_anchor_keys.add((obj_idx, hz, mut_fn, mut_loc, mut_code))
                race_shape = (
                    hz,
                    family,
                    mut_fn,
                    read_fn,
                    tuple(sorted(kinds1)),
                    tuple(sorted(kinds2)),
                    lock_relation,
                    ctx,
                )
                coarse_shape = (hz, mut_fn, read_fn, lock_relation, ctx)
                race_shape_keys.add(race_shape)
                coarse_shape_keys.add(coarse_shape)
                shape_counts[race_shape] += 1
                if obj_is_gt_name:
                    gt_shape_keys.add(race_shape)

        if object_had_candidate:
            stats.candidate_objects += 1
            family_keys.add(family)
            object_counts[str(obj.get("name", f"obj#{obj_idx}"))] = object_raw
            stats.max_raw_candidates_per_object = max(stats.max_raw_candidates_per_object, object_raw)
            if obj_is_gt_name:
                stats.gt_candidate_covered = True
                gt_matched_objs.add(obj_name or f"obj#{obj_idx}")

    stats.object_anchor_groups = len(object_anchor_keys)
    stats.race_shape_classes = len(race_shape_keys)
    stats.coarse_shape_classes = len(coarse_shape_keys)
    stats.object_family_classes = len(family_keys)
    stats.gt_matched_objects = sorted(gt_matched_objs)[:8]
    stats.gt_shape_classes = len(gt_shape_keys)
    stats.top_objects = [
        {"object": name, "raw_candidates": count}
        for name, count in object_counts.most_common(5)
    ]
    stats.top_shapes = [
        {"shape": "|".join(map(str, key)), "raw_candidates": count}
        for key, count in shape_counts.most_common(5)
    ]
    return stats


def resolve_surface(row: dict[str, Any], dump_base: Path) -> Path | None:
    src = row.get("source_dump_dir") or ""
    if src:
        p = Path(src) / "vulnerability_surface.json"
        if p.exists():
            return p
    case = row["case"]
    candidates = sorted(
        dump_base.glob(f"{case}_*/vulnerability_surface.json"),
        key=lambda p: p.stat().st_mtime,
        reverse=True,
    )
    return candidates[0] if candidates else None


def load_exclusions(path: Path) -> tuple[set[str], set[str]]:
    if not path.exists():
        return set(), set()
    doc = read_json(path)
    benign = {x["bug_id"] for x in doc.get("benign_exclude", [])}
    borderline = {x["bug_id"] for x in doc.get("keep_borderline_actionable", [])}
    return benign, borderline


def ratio(n: int, d: int) -> float:
    return round((float(n) / float(d)), 4) if d else 0.0


def write_tsv(path: Path, rows: list[CaseStats]) -> None:
    fields = [
        "case", "status", "borderline_actionable", "shared_objects", "total_threads",
        "candidate_objects", "raw_candidates", "common_lock_candidates",
        "no_common_lock_candidates", "object_anchor_groups", "race_shape_classes",
        "coarse_shape_classes", "object_family_classes",
        "anchor_per_raw", "shape_per_raw", "coarse_per_raw",
        "gt_token_in_surface", "gt_object_in_surface", "gt_candidate_covered",
        "gt_shape_classes", "actual_sessions", "actual_calibrated",
        "source_surface", "skipped_reason",
    ]
    with path.open("w", encoding="utf-8", newline="") as f:
        w = csv.DictWriter(f, delimiter="\t", fieldnames=fields)
        w.writeheader()
        for r in rows:
            d = asdict(r)
            d["anchor_per_raw"] = ratio(r.object_anchor_groups, r.raw_candidates)
            d["shape_per_raw"] = ratio(r.race_shape_classes, r.raw_candidates)
            d["coarse_per_raw"] = ratio(r.coarse_shape_classes, r.raw_candidates)
            d = {k: d.get(k, "") for k in fields}
            w.writerow(d)


def aggregate(rows: list[CaseStats]) -> dict[str, Any]:
    usable = [r for r in rows if not r.skipped_reason]
    totals = {
        "cases_total": len(rows),
        "cases_with_surface": len(usable),
        "shared_objects": sum(r.shared_objects for r in usable),
        "candidate_objects": sum(r.candidate_objects for r in usable),
        "raw_candidates": sum(r.raw_candidates for r in usable),
        "common_lock_candidates": sum(r.common_lock_candidates for r in usable),
        "no_common_lock_candidates": sum(r.no_common_lock_candidates for r in usable),
        "object_anchor_groups": sum(r.object_anchor_groups for r in usable),
        "race_shape_classes": sum(r.race_shape_classes for r in usable),
        "coarse_shape_classes": sum(r.coarse_shape_classes for r in usable),
        "object_family_classes": sum(r.object_family_classes for r in usable),
    }
    totals["anchor_per_raw"] = ratio(totals["object_anchor_groups"], totals["raw_candidates"])
    totals["shape_per_raw"] = ratio(totals["race_shape_classes"], totals["raw_candidates"])
    totals["coarse_per_raw"] = ratio(totals["coarse_shape_classes"], totals["raw_candidates"])
    totals["common_lock_fraction"] = ratio(totals["common_lock_candidates"], totals["raw_candidates"])

    # --- GT coverage (cases that have GT tokens at all) ---
    with_gt = [r for r in usable if r.gt_tokens]
    covered = [r for r in with_gt if r.gt_candidate_covered]
    token_present = [r for r in with_gt if r.gt_token_in_surface]
    totals["gt_cases_with_tokens"] = len(with_gt)
    totals["gt_token_in_surface"] = len(token_present)
    totals["gt_candidate_covered"] = len(covered)
    totals["gt_coverage_fraction"] = ratio(len(covered), len(with_gt))

    # --- LLM session accounting ---
    totals["actual_sessions_sum"] = sum(r.actual_sessions for r in usable)
    totals["actual_calibrated_sum"] = sum(r.actual_calibrated for r in usable)
    totals["sessions_if_anchor_per_class"] = totals["object_anchor_groups"]
    totals["sessions_if_shape_per_class"] = totals["race_shape_classes"]
    totals["sessions_if_coarse_per_class"] = totals["coarse_shape_classes"]
    totals["gt_not_covered_cases"] = [
        {"case": r.case, "gt_tokens": r.gt_tokens, "token_in_surface": r.gt_token_in_surface,
         "borderline": r.borderline_actionable}
        for r in with_gt if not r.gt_candidate_covered
    ]
    totals["top_raw_cases"] = [
        {
            "case": r.case,
            "raw_candidates": r.raw_candidates,
            "race_shape_classes": r.race_shape_classes,
            "shape_per_raw": ratio(r.race_shape_classes, r.raw_candidates),
            "shared_objects": r.shared_objects,
        }
        for r in sorted(usable, key=lambda x: x.raw_candidates, reverse=True)[:15]
    ]
    totals["borderline_actionable"] = aggregate([r for r in usable if r.borderline_actionable]) if False else {}
    return totals


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--run-dir", type=Path, default=DEFAULT_RUN)
    ap.add_argument("--dump-base", type=Path, default=DEFAULT_DUMP_BASE)
    ap.add_argument("--experiment-base", type=Path, default=DEFAULT_EXPERIMENT_BASE)
    ap.add_argument("--exclusions", type=Path, default=DEFAULT_EXCLUSIONS)
    ap.add_argument("--out-prefix", type=Path, default=None)
    ap.add_argument("--include-benign", action="store_true")
    args = ap.parse_args()

    manifest_path = args.run_dir / "eval_dump_manifest_20260610.json"
    manifest = read_json(manifest_path)
    benign, borderline = load_exclusions(args.exclusions)
    session_actuals = load_session_actuals(args.run_dir)

    rows: list[CaseStats] = []
    for row in manifest:
        case = row["case"]
        if not args.include_benign and case in benign:
            continue
        surface_path = resolve_surface(row, args.dump_base)
        if not surface_path or not surface_path.exists():
            rows.append(CaseStats(
                case=case,
                status=row.get("status", ""),
                source_surface="",
                skipped_reason="missing_surface",
                borderline_actionable=case in borderline,
            ))
            continue
        try:
            surface = read_json(surface_path)
            gt_leaf, gt_structs = load_gt_tokens(case, args.experiment_base)
            cs = summarize_surface(
                surface, case, row.get("status", ""), surface_path,
                case in borderline, gt_leaf, gt_structs,
            )
            sess = session_actuals.get(case)
            if sess:
                cs.actual_sessions, cs.actual_calibrated = sess
            rows.append(cs)
        except Exception as exc:  # keep cohort accounting robust
            rows.append(CaseStats(
                case=case,
                status=row.get("status", ""),
                source_surface=str(surface_path),
                skipped_reason=f"surface_parse_error:{exc}",
                borderline_actionable=case in borderline,
            ))

    out_prefix = args.out_prefix or (args.run_dir / "candidate_shape_diagnostic")
    out_prefix.parent.mkdir(parents=True, exist_ok=True)
    json_path = out_prefix.with_suffix(".json")
    tsv_path = out_prefix.with_suffix(".tsv")

    usable = [r for r in rows if not r.skipped_reason]
    border_rows = [r for r in usable if r.borderline_actionable]
    doc = {
        "run_dir": str(args.run_dir),
        "manifest": str(manifest_path),
        "exclusions": str(args.exclusions),
        "include_benign": args.include_benign,
        "notes": [
            "Counts are surface-only diagnostics; they do not replay C++ MHP, contracts, or Phase-B discharge.",
            "raw_candidates counts conflicting thread pairs per shared object.",
            "object_anchor_groups approximate per-object mutating-anchor dedup.",
            "race_shape_classes merge by hazard, object family, mutating/reading functions, access kinds, lock relation, and use context.",
            "coarse_shape_classes additionally drop object family and access-kind details.",
        ],
        "summary": aggregate(rows),
        "borderline_summary": aggregate(border_rows),
        "cases": [asdict(r) for r in rows],
    }
    with json_path.open("w", encoding="utf-8") as f:
        json.dump(doc, f, indent=2, ensure_ascii=False)
    write_tsv(tsv_path, rows)

    s = doc["summary"]
    b = doc["borderline_summary"]
    print(f"wrote {json_path}")
    print(f"wrote {tsv_path}")
    print(
        "clean cases with surface={cases_with_surface}/{cases_total} "
        "raw={raw_candidates} anchor_groups={object_anchor_groups} "
        "shape_classes={race_shape_classes} coarse={coarse_shape_classes} "
        "shape/raw={shape_per_raw} coarse/raw={coarse_per_raw}".format(**s)
    )
    print(
        "LLM sessions: actual_total={actual_sessions_sum} actual_calibrated={actual_calibrated_sum} "
        "| if 1-call-per anchor={sessions_if_anchor_per_class} "
        "shape={sessions_if_shape_per_class} coarse={sessions_if_coarse_per_class}".format(**s)
    )
    print(
        "GT coverage: cases_with_GT={gt_cases_with_tokens} token_in_surface={gt_token_in_surface} "
        "candidate_covered={gt_candidate_covered} coverage={gt_coverage_fraction}".format(**s)
    )
    print(
        "borderline with surface={cases_with_surface}/{cases_total} "
        "raw={raw_candidates} shape_classes={race_shape_classes} "
        "shape/raw={shape_per_raw} gt_covered={gt_candidate_covered}/{gt_cases_with_tokens}".format(**b)
    )
    notc = s.get("gt_not_covered_cases", [])
    if notc:
        print(f"GT NOT covered ({len(notc)}):")
        for c in notc:
            print("  -", c["case"], "border=" + str(c["borderline"]),
                  "token_in_surface=" + str(c["token_in_surface"]), "tokens=", c["gt_tokens"][:6])
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
