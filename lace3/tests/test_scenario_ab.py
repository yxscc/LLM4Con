"""Pilot fairness, evidence paging and paid-call accounting, without a model."""

import json
from pathlib import Path
from types import SimpleNamespace

import pytest

from lace3 import relational, scenario_plan
from lace3.eval.scenario_ab import Inputs, run


@pytest.fixture
def inputs(conc_ll, tmp_path):
    src = Path(__file__).parent / "fixtures"
    ix, times = relational.prepare([conc_ll])
    index, plan = tmp_path / "index", tmp_path / "plan"
    relational.write(ix, index, src, times, dict(max_depth=8, max_steps=4000))
    scenes, footprints, sessions, timings = scenario_plan.build(ix, src)
    scenario_plan.write(scenes, footprints, sessions, plan, index, timings)
    return Inputs(src, index, plan)


def response(calls, usage=None):
    raw = dict(choices=[dict(message=dict(role="assistant", content=None, tool_calls=[
        dict(id=f"call{i}", type="function", function=dict(name=n, arguments=json.dumps(a)))
        for i, (n, a) in enumerate(calls)]), finish_reason="tool_calls")],
        usage=usage if usage is not None else dict(prompt_tokens=100, completion_tokens=20, total_tokens=120))
    return SimpleNamespace(model_dump=lambda **_: raw)


class FakeClient:
    def __init__(self, responses):
        self.responses = iter(responses)
        self.calls = []
        self.chat = SimpleNamespace(completions=SimpleNamespace(create=self.create))

    def create(self, **kwargs):
        self.calls.append(json.loads(json.dumps(kwargs)))
        value = next(self.responses)
        if isinstance(value, Exception):
            raise value
        return value


def args(out, arm="baseline", **kw):
    return SimpleNamespace(out=out, arm=arm, max_requests=kw.get("max_requests", 3),
                           token_budget=kw.get("token_budget", 100000), max_output_tokens=8192,
                           reasoning_effort="medium")


REPORT = dict(summary="No supported finding", findings=[], limitations=["Missing external implementations"])
ENCODE = lambda text: list(range(len(text) // 4))


def test_identical_source_and_complete_checklist(inputs, tmp_path):
    for arm in ("baseline", "indexed"):
        client = FakeClient([response([("submit_report", {"report": json.dumps(REPORT)})])])
        result = run(args(tmp_path / arm, arm), inputs, client, ENCODE, "test")
        assert result["status"] == "submitted" and result["usage"]["requests"] == 1
        messages = client.calls[0]["messages"]
        assert inputs.source in messages[1]["content"]
        tools = {t["function"]["name"] for t in client.calls[0]["tools"]}
        assert ("query_index" in tools) == (arm == "indexed")
        assert ("Static concurrency scenario checklist" in messages[1]["content"]) == (arm == "indexed")
        assert not result["review_complete"]
    assert {s["id"] for s in inputs.checklist()["scenarios"]} == set(inputs.scenes)


def test_query_pages_are_complete_and_only_retrieval(inputs):
    scene = max(inputs.scenes, key=lambda s: len(inputs.scenes[s]["views"]))
    cursor, found = 0, []
    while cursor is not None:
        page = inputs.query("scenario", scene, cursor, 1)
        found += [r["id"] for r in page["rows"]]
        cursor = page["next_cursor"]
        assert page["retrieval_only"] and not page["reviewed"]
    assert found == inputs.scenes[scene]["views"]
    view = inputs.query("view", found[0], 0, 1)
    assert view["total"] == len(inputs.views[found[0]]["left_members"])
    member = view["rows"][0]["id"]
    assert inputs.query("member", member, 0, 1)["total"] == len(inputs.members[member]["steps"])


def test_paging_and_location_anonymization(inputs):
    assert inputs.clean("/some/CVE-2025-12345/main.c:10") == "unavailable/main.c:10"
    assert inputs.clean("CVE-2025-12345 SYZBOT-abcdef") == "[case] [case]"
    with pytest.raises(ValueError):
        inputs.query("scenario", next(iter(inputs.scenes)), -1, 10)
    entry = next(iter(inputs.acts))
    page = inputs.query("entry", entry, 0, 1)
    assert len(page["rows"]) == 1 and page["rows"][0]["operation_info"]


def test_budget_stops_before_request(inputs, tmp_path):
    client = FakeClient([])
    result = run(args(tmp_path / "budget", token_budget=1), inputs, client, ENCODE, "test")
    assert result["status"] == "budget_pending"
    assert result["model_calls"] == 0 and client.calls == []


def test_usage_is_counted_on_failed_submission_and_no_implicit_retry(inputs, tmp_path):
    client = FakeClient([response([("submit_report", {"report": "invalid"})]),
                         response([("submit_report", {"report": json.dumps(REPORT)})])])
    result = run(args(tmp_path / "accounting", max_requests=2), inputs, client, ENCODE, "test")
    assert result["usage"]["total_tokens"] == 240
    assert result["model_calls"] == 2 and result["report"] == REPORT
    assert client.calls[-1]["tool_choice"]["function"]["name"] == "submit_report"
    with pytest.raises(ValueError, match="already started"):
        run(args(tmp_path / "accounting"), inputs, client, ENCODE, "test")


def test_errors_keep_usage_uncertainty_without_secret_text(inputs, tmp_path):
    client = FakeClient([RuntimeError("https://secret.example?ak=do-not-log")])
    result = run(args(tmp_path / "error"), inputs, client, ENCODE, "test")
    assert result["model_calls"] == 1 and not result["token_usage_complete"]
    assert result["status"] == "api_error" and len(client.calls) == 1
    assert "do-not-log" not in (tmp_path / "error/result.json").read_text()


def test_baseline_cannot_query_index_or_escape_source(inputs, tmp_path):
    client = FakeClient([response([("query_index", {"kind": "entry", "target": "A0", "cursor": 0, "limit": 1}),
                                 ("read_source", {"path": "../../ground_truth.txt", "start": 1, "end": 10})]),
                         response([("submit_report", {"report": json.dumps(REPORT)})])])
    result = run(args(tmp_path / "sandbox"), inputs, client, ENCODE, "test")
    assert result["report"] == REPORT
    answers = json.loads((tmp_path / "sandbox/tool-results-0.json").read_text())
    assert all("error" in a["result"] for a in answers)


def test_missing_usage_stops_and_preserves_raw_response(inputs, tmp_path):
    client = FakeClient([response([], usage={})])
    result = run(args(tmp_path / "missing"), inputs, client, ENCODE, "test")
    assert result["status"] == "missing_usage" and not result["token_usage_complete"]
    assert (tmp_path / "missing/response-0.json").exists()
