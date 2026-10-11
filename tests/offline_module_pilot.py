"""End-to-end SDK/C++ plumbing test on a real static input, with NO external API.

The local scripted model explicitly records UNKNOWN for all groups. This test
checks continuation and accounting, and makes no bug-detection quality claim.
"""
import argparse
import json
import os
from pathlib import Path
import subprocess
import sys
import threading
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--source", type=Path, required=True)
    parser.add_argument("--bitcode", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--repository", type=Path)
    parser.add_argument("--revision")
    args = parser.parse_args()
    repo = Path(__file__).resolve().parents[1]
    state = {"calls": 0, "phase": "A", "next": "start", "source_bytes_in_first_prompt": 0}

    class Handler(BaseHTTPRequestHandler):
        def log_message(self, *a):
            pass

        def do_POST(self):
            body = json.loads(self.rfile.read(int(self.headers["Content-Length"])))
            state["calls"] += 1
            messages = body["messages"]
            if state["next"] == "start":
                content = messages[-1]["content"]
                overview = json.JSONDecoder().raw_decode(content.split("Static navigation overview:\n", 1)[1])[0]
                state["source_bytes_in_first_prompt"] = len(content)
                assert "<source file=" in content
                if overview["access_functions"]:
                    name, arguments = "record_protocol", {"functions": overview["access_functions"], "summary": "Offline plumbing test: no semantic judgment attempted.", "status": "unknown"}
                    state["next"] = "calls" if args.repository else "finish_a"
                else:
                    name, arguments = "finalize_contract", {}
                    state["next"] = "phase_c"
            elif state["next"] == "calls":
                name, arguments = "get_module_calls", {"apis": ["dst_cache_get"]}
                state["next"] = "search"
            elif state["next"] == "search":
                calls = json.loads(messages[-1]["content"])
                assert calls and calls[0]['arguments'][0]['code'] == '&ilwt->dst_cache', calls
                state['bound_nodes'] = [c['node_id'] for c in calls]
                name, arguments = "repository_search", {"symbol": "dst_cache_get", "path_prefix": "include", "offset": 0}
                state["next"] = "read"
            elif state["next"] == "read":
                search = json.loads(messages[-1]["content"])
                assert search['revision'] == args.revision and search['matches']
                name, arguments = "repository_read", {"path": search['matches'][0]['path'], "start_line": 1, "line_count": 100}
                state["next"] = "forged_summary"
            elif state["next"] == "forged_summary":
                source = json.loads(messages[-1]["content"])
                state['source_id'] = source['source_id']
                state['summary'] = {"api": "dst_cache_get", "effects": [{"argument_index": 0,
                    "resource_path": "pointed-to cache", "access": "ReadWrite"}],
                    "preconditions": ["Offline test predicate; not a semantic assessment"], "guarantees": [],
                    "evidence_ids": ["src:forged"], "basis": "inferred", "conditions": "Offline plumbing only",
                    "explanation": "Tests callsite binding and ledger accounting, not detection quality."}
                name, arguments = "record_api_summary", state['summary']
                state["next"] = "valid_summary"
            elif state["next"] == "valid_summary":
                assert 'Unknown source_id' in messages[-1]['content'], messages[-1]
                state['summary']['evidence_ids'] = [state['source_id']]
                name, arguments = "record_api_summary", state['summary']
                state["next"] = "finish_a"
            elif state["next"] == "finish_a":
                if args.repository:
                    applied = json.loads(messages[-1]['content'])
                    assert applied['automatic_discharge'] is False
                    assert applied['new_requirements'][0]['a_nodes'] == state['bound_nodes']
                name, arguments = "finalize_contract", {}
                state["next"] = "phase_c"
            elif state["next"] == "phase_c":
                assert any("<source file=" in str(m.get("content", "")) for m in messages), "Phase C lost complete source history"
                name, arguments = "get_obligations", {"ids": []}
                state["next"] = "review"
            elif state["next"] == "review":
                rows = json.loads(messages[-1]["content"])
                if args.repository:
                    api_rows = [r for r in rows if r['form'] == 'CallPrecondition']
                    assert len(api_rows) == 1
                    assert api_rows[0]['checker_hint']['status'] == 'requires_semantic_review'
                if rows:
                    name, arguments = "review_obligations", {"ids": [r["id"] for r in rows], "status": "unknown",
                        "evidence_nodes": [], "witness_nodes": [], "reason": "Offline test; real model assessment intentionally not performed.", "interleaving": ""}
                    state["next"] = "finish_c"
                else:
                    name, arguments = "finalize_review", {}
                    state["next"] = "done"
            elif state["next"] == "finish_c":
                name, arguments = "finalize_review", {}
                state["next"] = "done"
            else:
                raise AssertionError("Unexpected model request after finalization")
            payload = json.dumps({"id": "offline", "object": "chat.completion", "created": 1, "model": "offline-scripted",
                "choices": [{"index": 0, "message": {"role": "assistant", "content": None,
                    "tool_calls": [{"id": f"call-{state['calls']}", "type": "function", "function": {"name": name, "arguments": json.dumps(arguments)}}]},
                    "finish_reason": "tool_calls"}], "usage": {"prompt_tokens": 0, "completion_tokens": 0, "total_tokens": 0}}).encode()
            self.send_response(200); self.send_header("Content-Type", "application/json")
            self.send_header("Content-Length", str(len(payload))); self.end_headers(); self.wfile.write(payload)

    server = ThreadingHTTPServer(("127.0.0.1", 0), Handler)
    thread = threading.Thread(target=server.serve_forever, daemon=True); thread.start()
    env = os.environ.copy()
    env.update(LLM_BASE_URL=f"http://127.0.0.1:{server.server_port}/v1", LLM_API_KEY="offline-test-only",
               LLM_MODEL="offline-scripted", NO_PROXY="127.0.0.1,localhost", no_proxy="127.0.0.1,localhost")
    env.pop("LACE_MODEL_API", None)
    try:
        command = [sys.executable, str(repo / "harness/run_module.py"),
            "--source", str(args.source), "--bitcode", str(args.bitcode), "--output", str(args.output)]
        if args.repository:
            command += ['--repository', str(args.repository), '--revision', args.revision]
        code = subprocess.call(command, env=env)
        assert code == 0, f"Native workflow failed: see {args.output}/run.log"
        manifest = json.loads((args.output / "run.json").read_text())
        report = json.loads(Path(manifest["report"]).read_text())
        assert report["coverage"]["generation_finished"] and report["coverage"]["review_finished"]
        assert not report["coverage"]["complete"], "Unreviewed semantics must never become a clean result"
        assert len(report["requirements"]) == len(report["reviews"]), "Silent obligation loss"
        if args.repository:
            assert report['api_summaries']['dst_cache_get']['evidence_ids'] == [state['source_id']]
            assert report['coverage']['imported_call_resources'] == len(state['bound_nodes'])
            assert report['repository_evidence'][state['source_id']]['revision'] == args.revision
        state.update(simulated_model=True, real_model_calls=0, coverage=report["coverage"])
        (args.output / "offline-validation.json").write_text(json.dumps(state, indent=2))
        print(json.dumps(state, indent=2))
    finally:
        server.shutdown(); server.server_close(); thread.join(timeout=3)


if __name__ == "__main__":
    main()
