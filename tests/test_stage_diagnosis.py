import sys
from pathlib import Path
import pytest
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "harness"))
from diagnose_case import diagnose


def test_model_binding_and_semantics_are_distinct():
    model = {"nodes": [{"node_id": 1, "origin": "ir", "kind": "load", "file": "/build/f.c", "line": 7}]}
    ledger = {"source_manifest": [{"path": "f.c"}], "requirements": [{"id": "r"}], "checks": {"r": {
        "bindings": [{"source_id": "s", "status": "ambiguous"}], "e_candidate": False}}, "review_disabled": True}
    gt = {"bugs": [{"gt_id": "bug", "sites": [{"role": "use", "path": "f.c", "first_line": 7, "last_line": 7}]}]}
    note = {"bug": {"requirement_ids": ["r"], "req": {"judgment": "accurate", "reason": "Exact lifetime invariant"},
                    "gua": {"judgment": "not_applicable", "reason": "Source has no synchronization at this site"}}}
    row = diagnose(model, ledger, gt, note)["cases"][0]
    assert row["static_model"]["ir_coordinates_present"]
    assert row["e"]["runtime_status"] == "partially_blocked_by_binding"
    assert [p["stage"] for p in row["problems"]] == ["binding"]
    assert not row["attribution_complete"]
    # Enum acceptance and a discharged result are not semantic correctness.
    assert diagnose(model, ledger, gt)["cases"][0]["req"]["judgment"] == "not_assessed"
    note["bug"]["req"] = {"judgment": "incorrect"}
    with pytest.raises(ValueError):
        diagnose(model, ledger, gt, note)
