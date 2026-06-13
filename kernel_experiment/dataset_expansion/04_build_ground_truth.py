#!/usr/bin/env python3
"""Step 4: for each accepted candidate, build a ground_truth.json
mirroring the schema used by the existing 50 CVEs.

Reads:
  verdicts.json    — { "candidates": { "<bug_id>": { "status": "accept",
                                                       "two_threads": "...",
                                                       "notes": "..." } } }
  evidence/<bug>.meta.json
  candidates/<tier>_<bug>.json

Writes:
  <experiment_base>/<bug_id>/ground_truth.json
  dataset_expansion/cve_survey_additions.csv  (rows to append to cve_survey.csv)
"""
from __future__ import annotations
import argparse
import csv
import json
import subprocess
from pathlib import Path
from typing import Any, Dict, List, Optional


HERE = Path(__file__).resolve().parent
LINUX_GIT = "/mlx_devbox/users/mayunlong.39/playground/linux.git"


def git_show_message(sha: str) -> str:
    p = subprocess.run(["git", "-C", LINUX_GIT, "log", "-1",
                        "--format=%H%n%s%n%n%b", sha],
                       capture_output=True, text=True, timeout=30)
    return p.stdout if p.returncode == 0 else ""


def git_show_patch(sha: str) -> str:
    p = subprocess.run(["git", "-C", LINUX_GIT, "show", "--no-color", sha],
                       capture_output=True, text=True, timeout=60)
    return p.stdout if p.returncode == 0 else ""


def load_verdicts(path: Path) -> Dict[str, Dict[str, Any]]:
    if not path.exists():
        return {}
    d = json.load(open(path))
    return d.get("candidates", {}) if isinstance(d, dict) else {}


def find_candidate_json(cand_dir: Path, bug_id: str) -> Optional[Path]:
    for p in cand_dir.glob(f"*_{bug_id}.json"):
        return p
    return None


def build_one(bug_id: str, verdict: Dict[str, Any], cand_dir: Path,
              ev_dir: Path, out_root: Path) -> Optional[Dict[str, Any]]:
    cand_path = find_candidate_json(cand_dir, bug_id)
    if not cand_path:
        return None
    cand = json.load(open(cand_path))
    meta_path = ev_dir / f"{bug_id}.meta.json"
    if not meta_path.exists():
        return None
    ev_meta = json.load(open(meta_path))

    sha = ev_meta["mainline_sha"]
    msg = git_show_message(sha)
    patch = git_show_patch(sha)

    c_paths = ev_meta.get("c_paths", []) or []
    # Prefer files with extension .c; .h files come along too via expander.
    c_only = [p for p in c_paths if p.endswith(".c")]
    if not c_only:
        c_only = c_paths  # last resort

    gt: Dict[str, Any] = {
        "cve_id": bug_id,
        "fix_commit": sha,
        "files": c_only,
        "description": cand.get("description", "") or cand.get("title", ""),
        "cwes": cand.get("cwes", []),
        "title": cand.get("title", ""),
        "tier": cand.get("tier"),
        "source_archive": cand.get("source"),
        "thread_hint": cand.get("thread_hint"),
        "two_threads_summary": verdict.get("two_threads", ""),
        "audit_notes": verdict.get("notes", ""),
        "fix_commit_message": msg.strip()[:6000],
        "patch": patch[:8000],
        "affected_files_from_patch": c_paths,
    }

    out_dir = out_root / bug_id
    out_dir.mkdir(parents=True, exist_ok=True)
    with open(out_dir / "ground_truth.json", "w") as f:
        json.dump(gt, f, indent=2, ensure_ascii=False)
    return {
        "bug_id": bug_id,
        "has_patch": "YES",
        "files": ";".join(c_only),
        "fix_commit": sha,
    }


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--verdicts", default=str(HERE / "verdicts.json"))
    ap.add_argument("--candidates", default=str(HERE / "candidates"))
    ap.add_argument("--evidence", default=str(HERE / "evidence"))
    ap.add_argument("--out-root",
                    default="/mlx_devbox/users/mayunlong.39/playground/LLM4Con/kernel_experiment_v2_staging",
                    help="default points to v2 staging area; pass kernel_experiment "
                         "explicitly only when promoting entries.")
    ap.add_argument("--csv-out",
                    default=str(HERE / "cve_survey_additions.csv"))
    ap.add_argument("--selection",
                    default=str(HERE / "selection_core.json"),
                    help="restrict to ids listed here; '' to use all accepts")
    args = ap.parse_args()

    verdicts = load_verdicts(Path(args.verdicts))
    accepts = [(bid, v) for bid, v in verdicts.items()
               if (v.get("status") or "").lower() == "accept"]
    if args.selection:
        sel = json.load(open(args.selection))
        keep = set(sel.get("core_bug_ids", []))
        accepts = [(bid, v) for bid, v in accepts if bid in keep]
        print(f"[build_gt] filtered to {len(accepts)} bugs from {args.selection}")
    if not accepts:
        print(f"[build_gt] no accepted candidates in {args.verdicts}")
        return 1

    rows: List[Dict[str, Any]] = []
    out_root = Path(args.out_root)
    cand_dir = Path(args.candidates)
    ev_dir = Path(args.evidence)
    for bid, v in accepts:
        try:
            r = build_one(bid, v, cand_dir, ev_dir, out_root)
        except Exception as e:
            print(f"[build_gt] FAIL {bid}: {e!r}")
            continue
        if r:
            rows.append(r)
            print(f"  + {bid}  files={len(r['files'].split(';'))}  "
                  f"commit={r['fix_commit'][:12]}")
        else:
            print(f"  - {bid}  (no candidate or evidence)")

    with open(args.csv_out, "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["CVE", "HAS_PATCH", "FILES", "FIX_COMMIT"])
        for r in rows:
            w.writerow([r["bug_id"], r["has_patch"], r["files"], r["fix_commit"]])
    print(f"[build_gt] wrote {len(rows)} entries to {args.csv_out}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
