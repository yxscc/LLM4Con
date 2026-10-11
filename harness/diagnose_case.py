"""Offline four-stage GT audit. This evaluator is never a detector input.

Runtime structural facts and human semantic judgments are deliberately separate.
An annotation must cite a reason; enum/schema acceptance is not semantic accuracy.
"""
import argparse
import hashlib
import json
from pathlib import Path


def read(path):
    return json.loads(path.read_text())


def diagnose(model, contracts, expectations, annotations=None):
    annotations = annotations or {}
    checks = contracts.get("checks", {})
    requirements = {r["id"]: r for r in contracts.get("requirements", [])}
    manifest = contracts.get("source_manifest", [])
    basenames = {}
    for row in manifest:
        key = Path(row["path"]).name
        basenames[key] = basenames.get(key, 0) + 1

    def locations(site):
        out = []
        for node in model["nodes"]:
            if node.get("kind") == "Method":
                continue
            locs = node.get("source_locations") or [{"file": node.get("file", ""), "line": node.get("line", 0)}]
            for loc in locs:
                file = loc["file"]
                same_file = file == site["path"] or (Path(file).name == Path(site["path"]).name and basenames.get(Path(file).name) == 1)
                if same_file and site["first_line"] <= loc["line"] <= site["last_line"]:
                    out.append({k: node.get(k) for k in ("node_id", "origin", "kind", "memory_role", "access_paths", "source_function")})
                    break
        return out

    def semantic(value, stage):
        result = dict(value or {"judgment": "not_assessed"})
        allowed = {"accurate", "missing", "incorrect", "incomplete", "unknown", "not_assessed", "not_applicable"}
        if result.get("judgment") not in allowed:
            raise ValueError("Invalid semantic judgment for " + stage)
        if result["judgment"] not in {"not_assessed", "unknown"} and not result.get("reason"):
            raise ValueError("Semantic judgments need a source-based reason: " + stage)
        result["origin"] = "offline_semantic_annotation"
        return result

    rows = []
    for gt in expectations["bugs"]:
        annotation = annotations.get(gt["gt_id"], {})
        sites = [{**s, "coordinate_nodes": locations(s)} for s in gt["sites"]]
        absent = [s["role"] for s in sites if not any(n["origin"] == "ir" for n in s["coordinate_nodes"])]
        req_judgment = semantic(annotation.get("req"), "req")
        gua_judgment = semantic(annotation.get("gua"), "gua")
        req_ids = annotation.get("requirement_ids", [])
        if any(i not in requirements for i in req_ids):
            raise ValueError("Annotated requirement ID absent in this run")
        associated = {i: checks.get(i) for i in req_ids}
        bindings = [b for e in associated.values() if e for b in e.get("bindings", [])]
        binding_issues = [b for b in bindings if b["status"] != "bound"]
        candidates = [i for i, e in associated.items() if e and e.get("e_candidate")]
        e_annotation = semantic(annotation.get("e"), "e_composition")
        problems = []
        if absent:
            problems.append({"stage": "static_model", "kind": "gt_sites_without_ir_coordinates", "roles": absent})
        if binding_issues:
            problems.append({"stage": "binding", "kind": "source_ir_binding_failed", "evidence": binding_issues})
        for stage, judgment in (("gua", gua_judgment), ("req", req_judgment), ("e", e_annotation)):
            if judgment["judgment"] in {"missing", "incorrect", "incomplete"}:
                problems.append({"stage": stage, **judgment})
        # A Gua may correctly be absent because the buggy source lacks protection.
        # Do not automatically call that an extraction failure.
        if not req_ids:
            runtime_e_status = "no_gt_requirement_association"
        elif any(e is None for e in associated.values()):
            runtime_e_status = "not_executed"
        elif binding_issues:
            runtime_e_status = "partially_blocked_by_binding"
        elif candidates:
            runtime_e_status = "candidate_emitted"
        else:
            runtime_e_status = "no_candidate_emitted"
        rows.append({"gt_id": gt["gt_id"], "static_model": {"sites": sites,
                     "ir_coordinates_present": not absent,
                     "meaning": "Coordinate coverage only; operation correspondence requires binding"},
                     "binding": {"records": bindings, "failures": binding_issues},
                     "gua": gua_judgment, "req": req_judgment,
                     "e": {"runtime_status": runtime_e_status, "associated_checks": associated,
                           "candidate_requirement_ids": candidates, "semantic_evaluation": e_annotation},
                     "gt_match": semantic(annotation.get("gt_match"), "gt_match"),
                     "problems": problems,
                     "attribution_complete": all(x["judgment"] not in {"unknown", "not_assessed"}
                                                 for x in (req_judgment, gua_judgment, e_annotation))})
    return {"schema": "lace-stage-diagnosis-v1", "evaluation_only": True,
            "review_disabled": contracts.get("review_disabled", False),
            "ground_truth_used_by_detector": False,
            "cases": rows, "note": "Multiple faults can coexist. No automatic semantic-correctness or GT-recall claim."}


def main():
    p = argparse.ArgumentParser(description=__doc__)
    for name in ("model", "contracts", "expectations", "output"):
        p.add_argument("--" + name, type=Path, required=True)
    p.add_argument("--annotations", type=Path)
    args = p.parse_args()
    result = diagnose(read(args.model), read(args.contracts), read(args.expectations),
                      read(args.annotations) if args.annotations else None)
    inputs = {k: getattr(args, k) for k in ("model", "contracts", "expectations", "annotations")}
    result["provenance"] = {k: {"path": str(v.resolve()), "sha256": hashlib.sha256(v.read_bytes()).hexdigest()}
                            for k, v in inputs.items() if v}
    args.output.write_text(json.dumps(result, indent=2, ensure_ascii=False) + "\n")


if __name__ == "__main__":
    main()
