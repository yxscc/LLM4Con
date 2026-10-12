"""Offline transport tests use a local scripted API; these are NOT model evaluations."""
import importlib.util
import json
import os
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
@pytest.mark.parametrize("turn_cap", [0, 5])
def test_official_runner_tool_cycle_and_continuation(route, turn_cap):
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
        "max_turns": turn_cap}
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


def test_native_compaction_configuration(monkeypatch):
    monkeypatch.delenv('LACE_CONTEXT_MANAGEMENT', raising=False)
    monkeypatch.delenv('LACE_COMPACT_THRESHOLD', raising=False)
    capability, config = worker.context_management('chat_completions', 'offline-test')
    assert capability is None and config['reason'] == 'responses_api_required'
    monkeypatch.setenv('LACE_CONTEXT_MANAGEMENT', 'native')
    with pytest.raises(ValueError, match='Responses-compatible endpoint'):
        worker.context_management('chat_completions', 'offline-test')
    monkeypatch.setenv('LACE_COMPACT_THRESHOLD', '1000')
    capability, config = worker.context_management('responses', 'offline-test')
    assert config['context_management'] == [{'type': 'compaction', 'compact_threshold': 1000}]
    assert capability.process_context([{'role': 'user', 'content': 'older'},
        {'type': 'compaction', 'encrypted_content': 'opaque'}, {'role': 'user', 'content': 'new'}]) == [
        {'type': 'compaction', 'encrypted_content': 'opaque'}, {'role': 'user', 'content': 'new'}]


def test_official_runner_native_compaction_and_continuation():
    requests = []

    class Handler(BaseHTTPRequestHandler):
        def log_message(self, *_):
            pass

        def do_POST(self):
            body = json.loads(self.rfile.read(int(self.headers['Content-Length'])))
            requests.append(body)
            if len(requests) == 1:
                output = [{'type': 'compaction', 'id': 'cmp_test', 'encrypted_content': 'opaque-offline-compaction'},
                          {'type': 'function_call', 'id': 'fc_test', 'call_id': 'call-1',
                           'name': 'read_evidence', 'arguments': '{"node":7}', 'status': 'completed'}]
            else:
                output = [{'type': 'message', 'id': 'msg_test', 'role': 'assistant', 'status': 'completed',
                           'content': [{'type': 'output_text', 'text': 'evidence reviewed', 'annotations': []}]}]
            payload = json.dumps({'id': f'resp_{len(requests)}', 'object': 'response', 'created_at': 1,
                'status': 'completed', 'model': 'offline-test', 'output': output,
                'usage': {'input_tokens': 20, 'output_tokens': 5, 'total_tokens': 25,
                          'input_tokens_details': {'cached_tokens': 0}, 'output_tokens_details': {'reasoning_tokens': 0}}}).encode()
            self.send_response(200); self.send_header('Content-Type', 'application/json')
            self.send_header('Content-Length', str(len(payload))); self.end_headers(); self.wfile.write(payload)

    server = ThreadingHTTPServer(('127.0.0.1', 0), Handler)
    thread = threading.Thread(target=server.serve_forever, daemon=True); thread.start()
    env = {**os.environ, 'LACE_MODEL_API': 'responses', 'LACE_CONTEXT_MANAGEMENT': 'native',
           'LACE_COMPACT_THRESHOLD': '1000'}
    request = {'action': 'agent', 'api_key': 'offline-only', 'base_url': f'http://127.0.0.1:{server.server_port}/v1',
        'model': 'offline-test', 'instructions': 'Inspect evidence', 'input': 'PUBLIC_OLD_SOURCE_BEFORE_COMPACTION',
        'history': [], 'tools': [{'name': 'read_evidence', 'description': 'Read a node',
            'parameters': {'type': 'object', 'properties': {'node': {'type': 'integer'}}, 'required': ['node']}}],
        'max_turns': 0}
    try:
        proc = subprocess.Popen([sys.executable, str(WORKER)], stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                                stderr=subprocess.PIPE, text=True, env=env)
        proc.stdin.write(json.dumps(request)+'\n'); proc.stdin.flush()
        assert json.loads(proc.stdout.readline())['event'] == 'tool_call'
        proc.stdin.write(json.dumps({'output': 'node 7 is a use'})+'\n'); proc.stdin.flush()
        result = json.loads(proc.stdout.readline())
        assert proc.wait(timeout=20) == 0, proc.stderr.read()
        assert result['event'] == 'result' and result['context_management']['enabled']
        assert requests[0]['context_management'] == [{'type': 'compaction', 'compact_threshold': 1000}]
        assert requests[0]['store'] is False
        assert 'PUBLIC_OLD_SOURCE_BEFORE_COMPACTION' not in json.dumps(requests[1]['input'])
        assert any(item.get('type') == 'compaction' for item in requests[1]['input'])
        assert any(item.get('type') == 'function_call_output' for item in requests[1]['input'])
        # Audit history stays complete even though the model sees the SDK's
        # compacted context. Continue Phase C without losing the compaction item.
        assert 'PUBLIC_OLD_SOURCE_BEFORE_COMPACTION' in json.dumps(result['history'])
        request.update(history=result['history'], input='Continue Phase C')
        second = subprocess.run([sys.executable, str(WORKER)], input=json.dumps(request)+'\n',
                                capture_output=True, text=True, timeout=20, env=env)
        assert second.returncode == 0, second.stdout+second.stderr
        assert 'PUBLIC_OLD_SOURCE_BEFORE_COMPACTION' not in json.dumps(requests[2]['input'])
        assert 'Continue Phase C' in json.dumps(requests[2]['input'])
    finally:
        server.shutdown(); server.server_close(); thread.join(timeout=3)
