"""Offline transport tests use a local scripted API; these are NOT model evaluations."""
import importlib.util
import json
from pathlib import Path
import subprocess
import sys
import threading
import pytest
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer

WORKER = Path(__file__).resolve().parents[1] / "harness/sdk_worker.py"
spec = importlib.util.spec_from_file_location("sdk_worker", WORKER)
worker = importlib.util.module_from_spec(spec)
spec.loader.exec_module(worker)


def test_endpoint_normalization():
    assert worker.base_url("https://example.invalid/v1/chat/completions?route=one") == "https://example.invalid/v1/"
    assert worker.endpoint_query("https://example.invalid/v1/chat/completions?route=one&ak=test") == {"route": "one", "ak": "test"}
    assert worker.base_url("https://example.invalid/v1/responses") == "https://example.invalid/v1/"
    assert worker.base_url("https://example.invalid/v1/") == "https://example.invalid/v1/"


@pytest.mark.parametrize("route", ["/v1/chat/completions", "/gateway/custom/completion"])
def test_official_runner_tool_cycle_and_continuation(route):
    requests = []

    class Handler(BaseHTTPRequestHandler):
        def log_message(self, *args):
            pass

        def do_POST(self):
            assert self.path == route + "?ak=offline-only", self.path
            body = json.loads(self.rfile.read(int(self.headers["Content-Length"])))
            requests.append(body)
            if len(requests) == 1 or body["messages"][-1].get("content") in ("test turn limit", "test finish tool"):
                message = {"role": "assistant", "content": None, "tool_calls": [
                    {"id": "call-1", "type": "function", "function": {"name": "read_evidence", "arguments": '{"node":7}'}}]}
                reason = "tool_calls"
            else:
                message = {"role": "assistant", "content": "evidence reviewed"}
                reason = "stop"
            payload = json.dumps({"id": "offline", "object": "chat.completion", "created": 1,
                "model": "offline-test", "choices": [{"index": 0, "message": message, "finish_reason": reason}],
                "usage": {"prompt_tokens": 20, "completion_tokens": 5, "total_tokens": 25}}).encode()
            self.send_response(200)
            self.send_header("Content-Type", "application/json")
            self.send_header("Content-Length", str(len(payload)))
            self.end_headers()
            self.wfile.write(payload)

    server = ThreadingHTTPServer(("127.0.0.1", 0), Handler)
    thread = threading.Thread(target=server.serve_forever, daemon=True)
    thread.start()
    request = {"action": "agent", "api_key": "offline-test-only", "base_url": f"http://127.0.0.1:{server.server_port}{route}?ak=offline-only",
        "model": "offline-test", "instructions": "Inspect evidence", "input": "module source " + "x" * 70000,
        "history": [], "tools": [{"name": "read_evidence", "description": "Read a node",
            "parameters": {"type": "object", "properties": {"node": {"type": "integer"}}, "required": ["node"]}}],
        "max_turns": 5}
    try:
        proc = subprocess.Popen([sys.executable, str(WORKER)], stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                                stderr=subprocess.PIPE, text=True)
        proc.stdin.write(json.dumps(request) + "\n"); proc.stdin.flush()
        tool = json.loads(proc.stdout.readline())
        assert tool == {"event": "tool_call", "name": "read_evidence", "arguments": {"node": 7}}
        proc.stdin.write(json.dumps({"output": "node 7 is a use"}) + "\n"); proc.stdin.flush()
        result = json.loads(proc.stdout.readline())
        assert proc.wait(timeout=20) == 0, proc.stderr.read()
        assert result["event"] == "result", result
        assert result["usage"]["requests"] == 2
        assert any(m.get("role") == "tool" and m["content"] == "node 7 is a use" for m in requests[1]["messages"])
        assert any(len(str(m.get("content", ""))) > 70000 for m in requests[1]["messages"])
        request.update(history=result["history"], input="Continue Phase C")
        second = subprocess.run([sys.executable, str(WORKER)], input=json.dumps(request) + "\n",
                                capture_output=True, text=True, timeout=20)
        assert second.returncode == 0, second.stdout + second.stderr
        assert json.loads(second.stdout)["event"] == "result"
        assert any(m.get("role") == "tool" for m in requests[2]["messages"])
        assert requests[2]["messages"][-1]["content"] == "Continue Phase C"
        for prompt, output, expected in (("test turn limit", "evidence", "error"),
                                          ("test finish tool", "finish", "result")):
            request.update(history=[], input=prompt, max_turns=1)
            limited = subprocess.Popen([sys.executable, str(WORKER)], stdin=subprocess.PIPE,
                                       stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
            limited.stdin.write(json.dumps(request) + "\n"); limited.stdin.flush()
            assert json.loads(limited.stdout.readline())["event"] == "tool_call"
            limited.stdin.write(json.dumps({"output": output}) + "\n"); limited.stdin.flush()
            outcome = json.loads(limited.stdout.readline())
            code = limited.wait(timeout=20)
            assert outcome["event"] == expected, outcome
            if expected == "error":
                assert code != 0 and outcome["error_type"] == "MaxTurnsExceeded"
            else:
                assert code == 0 and outcome["tool_finished"] is True
    finally:
        server.shutdown(); server.server_close(); thread.join(timeout=3)


def test_invalid_request_fails_and_redacts_secret():
    request = {"action": "agent", "api_key": "secret-not-a-real-key", "base_url": "http://127.0.0.1:1/v1", "tools": []}
    response = subprocess.run([sys.executable, str(WORKER)], input=json.dumps(request) + "\n",
                              capture_output=True, text=True, timeout=20)
    assert response.returncode != 0
    error = json.loads(response.stdout)
    assert error["event"] == "error"
    assert "secret-not-a-real-key" not in response.stdout + response.stderr
