"""End-to-end SDK/C++ plumbing test on a real static input, with NO external API.

The local scripted model explicitly records UNKNOWN for all groups. This test
checks continuation and accounting, and makes no bug-detection quality claim.
"""
import argparse
import json
import os
import re
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
    parser.add_argument("--source-anchors", action="store_true", help="Exercise source-only contracts and final report output with explicitly synthetic model judgments")
    args = parser.parse_args()
    repo = Path(__file__).resolve().parents[1]
    state = {"calls": 0, "phase": "A", "next": "start", "source_bytes_in_first_prompt": 0}

    class Handler(BaseHTTPRequestHandler):
        def log_message(self, *a):
            pass

        def do_POST(self):
            body = json.loads(self.rfile.read(int(self.headers["Content-Length"])))
            assert body.get('tool_choice') == 'required', 'Every module phase turn must use a tool, including finalization'
            tools = {t['function']['name']: t['function'] for t in body['tools']}
            assert 'record_api_summary' not in tools, 'API summaries must not be a mandatory intermediate representation'
            if 'add_requirement' in tools:
                assert set(tools['add_requirement']['parameters']['properties']['form']['enum']) == {'MustPrecede', 'MustBeAtomic', 'MustBeMediated'}
                assert set(tools['add_guarantee']['parameters']['properties']['form']['enum']) == {'Order', 'Wait', 'Exclude', 'AtomicOp'}
            state["calls"] += 1
            messages = body["messages"]
            if state["next"] == "start":
                content = messages[-1]["content"]
                overview = json.JSONDecoder().raw_decode(content.split("Static navigation overview:\n", 1)[1])[0]
                state["source_bytes_in_first_prompt"] = len(content)
                state['static_obligations'] = overview['static_obligations']
                state['source_path'] = json.loads(re.search(r'<source file=("[^\n]+")>', content).group(1))
                assert "<source file=" in content
                if overview["access_functions"]:
                    name, arguments = "record_protocol", {"functions": overview["access_functions"], "summary": "Offline plumbing test: no semantic judgment attempted.", "status": "unknown"}
                    state["next"] = "source_bad_path" if args.source_anchors else "calls" if args.repository else "finish_a"
                else:
                    if args.source_anchors:
                        name, arguments = 'get_source_operation', {'path': '../outside.c', 'first_line': 1, 'last_line': 1}
                        state['next'] = 'source_one'
                    else:
                        name, arguments = "finalize_contract", {}
                        state["next"] = "phase_c"
            elif state['next'] == 'source_bad_path':
                name, arguments = 'get_source_operation', {'path': '../outside.c', 'first_line': 1, 'last_line': 1}
                state['next'] = 'source_one'
            elif state['next'] == 'source_one':
                assert 'Unknown module snapshot path' in messages[-1]['content']
                name, arguments = 'get_source_operations', {'operations': [
                    {'path': state['source_path'], 'first_line': i, 'last_line': i+1} for i in (1,3,5)]}
                state['next'] = 'source_forged'
            elif state['next'] == 'source_forged':
                spans = json.loads(messages[-1]['content'])
                state['source_one'], state['source_two'], state['source_three'] = [s['source_id'] for s in spans]
                state['source_req'] = {'form': 'MustPrecede', 'owner_thread': -1, 'object_ids': [],
                    'resource': 'OFFLINE SYNTHETIC resource; not a defect assessment', 'a_nodes': [], 'b_nodes': [],
                    'a_sources': ['module-op:forged'], 'b_sources': [state['source_two']],
                    'note': 'Offline plumbing only: deliberately synthetic requirement. No concurrency or safety claim.'}
                name, arguments = 'add_requirement', dict(state['source_req'])
                state['next'] = 'source_valid'
            elif state['next'] == 'source_valid':
                assert 'Unknown module source operation' in messages[-1]['content']
                state['source_req']['a_sources'] = [state['source_one']]
                name, arguments = 'add_requirement', dict(state['source_req'])
                state['next'] = 'source_second_requirement'
            elif state['next'] == 'source_second_requirement':
                other = dict(state['source_req']); other['b_sources'] = [state['source_three']]
                other['resource'] += ' (independent witness)'
                name, arguments = 'add_requirement', other
                state['next'] = 'source_guarantee_rejected'
            elif state['next'] == 'source_guarantee_rejected':
                name, arguments = 'add_guarantee', {'form': 'Wait', 'a_nodes': [], 'b_nodes': [],
                    'a_sources': [state['source_one']], 'b_sources': [state['source_two']],
                    'evidence_nodes': [], 'evidence_sources': [state['source_one']], 'token': '', 'mode': '',
                    'conditions': 'Offline synthetic only', 'unconditional': True, 'note': 'Offline scope rejection test, not real synchronization.'}
                state['next'] = 'source_finish_a'
            elif state['next'] == 'source_finish_a':
                assert 'not representable by the static checker' in messages[-1]['content']
                name, arguments = 'finalize_contract', {}
                state['next'] = 'phase_c'
            elif state["next"] == "calls":
                name, arguments = "get_module_calls", {"apis": ["dst_cache_get"]}
                state["next"] = "search"
            elif state["next"] == "search":
                calls = json.loads(messages[-1]["content"])
                assert calls and calls[0]['arguments'][0]['code'] == '&ilwt->dst_cache', calls
                state['bound_nodes'] = [c['node_id'] for c in calls]
                call = next(c for c in calls if c['thread_contexts'])
                state['test_node'] = call['node_id']
                state['test_thread'] = call['thread_contexts'][0]
                name, arguments = "repository_search", {"symbol": "dst_cache_get", "path_prefix": "include", "offset": 0}
                state["next"] = "read"
            elif state["next"] == "read":
                search = json.loads(messages[-1]["content"])
                assert search['revision'] == args.revision and search['matches']
                name, arguments = "repository_read", {"path": search['matches'][0]['path'], "start_line": 1, "line_count": 100}
                state["next"] = "invalid_requirement"
            elif state["next"] == "invalid_requirement":
                source = json.loads(messages[-1]["content"])
                state['source_id'] = source['source_id']
                state['requirement'] = {'form': 'CallPrecondition', 'owner_thread': state['test_thread'],
                    'object_ids': [], 'resource': 'ilwt->dst_cache', 'a_nodes': [state['test_node']],
                    'b_nodes': [state['test_node']], 'repository_evidence_ids': [state['source_id']],
                    'note': 'Offline plumbing only: hypothetical mediation of two same-resource activations, not a source or feasibility assessment.'}
                name, arguments = 'add_requirement', dict(state['requirement'])
                state['next'] = 'forged_requirement'
            elif state['next'] == 'forged_requirement':
                assert 'Invalid requirement form' in messages[-1]['content'], messages[-1]
                state['requirement']['form'] = 'MustBeMediated'
                state['requirement']['repository_evidence_ids'] = ['src:forged']
                name, arguments = 'add_requirement', dict(state['requirement'])
                state['next'] = 'valid_requirement'
            elif state["next"] == "valid_requirement":
                assert 'Unknown source_id' in messages[-1]['content'], messages[-1]
                state['requirement']['repository_evidence_ids'] = [state['source_id']]
                name, arguments = 'add_requirement', dict(state['requirement'])
                state['next'] = 'invalid_guarantee'
            elif state['next'] == 'invalid_guarantee':
                assert 'Semantic requirements recorded' in messages[-1]['content'], messages[-1]
                name, arguments = 'add_guarantee', {'form': 'Allocation', 'a_nodes': [state['test_node']],
                    'b_nodes': [], 'token': '', 'mode': '', 'conditions': 'Offline only', 'unconditional': False,
                    'evidence_nodes': [state['test_node']], 'note': 'General API behavior is not a synchronization guarantee.'}
                state["next"] = "finish_a"
            elif state["next"] == "finish_a":
                if args.repository and not args.source_anchors:
                    assert 'Invalid guarantee form' in messages[-1]['content'], messages[-1]
                name, arguments = "finalize_contract", {}
                state["next"] = "phase_c"
            elif state["next"] == "phase_c":
                assert any("<source file=" in str(m.get("content", "")) for m in messages), "Phase C lost complete source history"
                name, arguments = "get_obligations", {"ids": []}
                state["next"] = "review"
            elif state["next"] == "review":
                rows = json.loads(messages[-1]["content"])
                if args.repository and not args.source_anchors:
                    unmodeled = [r for r in rows if r['object_id'] == -1]
                    assert len(unmodeled) == 1
                    assert unmodeled[0]['form'] == 'MustBeMediated'
                    assert unmodeled[0]['resource'] == 'ilwt->dst_cache'
                    assert unmodeled[0]['checker_hint']['status'] == 'requires_semantic_review'
                    assert 'static object mapping' in unmodeled[0]['checker_hint']['reason']
                if rows:
                    state['source_requirements'] = [r['id'] for r in rows if r.get('resource', '').startswith('OFFLINE SYNTHETIC')]
                    name, arguments = "review_obligations", {"ids": [r["id"] for r in rows], "status": "unknown",
                        "evidence_nodes": [], "witness_nodes": [], "reason": "Offline test; real model assessment intentionally not performed.", "interleaving": ""}
                    state["next"] = "source_witness" if args.source_anchors else "finish_c"
                else:
                    name, arguments = "finalize_review", {}
                    state["next"] = "done"
            elif state["next"] == "finish_c":
                name, arguments = "finalize_review", {}
                state["next"] = "done"
            elif state['next'] == 'source_witness':
                name, arguments = 'review_obligations', {'ids': [state['source_requirements'][0]], 'status': 'source_supported',
                    'evidence_nodes': [], 'witness_nodes': [],
                    'evidence_sources': [state['source_one'], state['source_two']],
                    'witness_sources': [state['source_one'], state['source_two']],
                    'reason': 'OFFLINE SYNTHETIC model judgment tests report plumbing, NOT evidence of any real bug.',
                    'interleaving': 'OFFLINE SYNTHETIC: activation A and B interleave; no actual semantic assessment.'}
                state['next'] = 'source_second_witness'
            elif state['next'] == 'source_second_witness':
                name, arguments = 'review_obligations', {'ids': [state['source_requirements'][1]], 'status': 'source_supported',
                    'evidence_nodes': [], 'witness_nodes': [],
                    'evidence_sources': [state['source_one'], state['source_three']],
                    'witness_sources': [state['source_one'], state['source_three']],
                    'reason': 'OFFLINE SYNTHETIC independent witness tests conservative report deduplication.',
                    'interleaving': 'OFFLINE SYNTHETIC: no actual semantic assessment.'}
                state['next'] = 'finish_c'
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
        if args.source_anchors:
            req = next(r for r in report['requirements'] if r.get('resource','').startswith('OFFLINE SYNTHETIC'))
            assert req['owner_thread'] == -1 and not req['a_nodes'] and not req['b_nodes']
            assert req['requires_semantic_review'] and report['checks'][req['id']]['status'] == 'requires_semantic_review'
            assert len(report['source_operations']) == 3 and not report['guarantees']
            assert len(report['requirements']) == state['static_obligations'] + 2
            bug = Path(manifest['report']).parent / 'stateful_bugs/bugs.txt'
            text = bug.read_text();assert 'OFFLINE SYNTHETIC' in text and state['source_one'] in text and state['source_two'] in text
            assert 'Source Operations (no static proof)' in text and '[node not found]' not in text
            assert text.count('========== Mechanism-Rule Violation Detected ==========') == 2, 'Independent source witnesses merged incorrectly'
        elif args.repository:
            assert 'api_summaries' not in report and 'api_resources' not in report
            assert 'unsummarized_calls_without_callee_edge' not in report['coverage']
            assert len(report['requirements']) == state['static_obligations'] + 1, 'Invalid claims mutated the ledger'
            req = next(r for r in report['requirements'] if r['origin'] == 'llm_unmodeled_resource')
            assert req['form'] == 'MustBeMediated' and req['repository_evidence_ids'] == [state['source_id']]
            assert req['a_nodes'] == req['b_nodes'] == [state['test_node']]
            assert report['coverage']['requirements_outside_static_objects'] == 1
            assert not report['guarantees'], 'Non-synchronization claims must not enter the ledger'
            assert report['repository_evidence'][state['source_id']]['revision'] == args.revision
        state.update(simulated_model=True, real_model_calls=0, coverage=report["coverage"])
        (args.output / "offline-validation.json").write_text(json.dumps(state, indent=2))
        print(json.dumps(state, indent=2))
    finally:
        server.shutdown(); server.server_close(); thread.join(timeout=3)


if __name__ == "__main__":
    main()
