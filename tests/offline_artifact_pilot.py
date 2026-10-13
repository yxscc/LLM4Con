"""Source contract files -> native binding/E via the actual SDK, localhost only.

All contracts and judgments are synthetic plumbing fixtures, not detected bugs.
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
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--fail-after-import', action='store_true')
    parser.add_argument('--review', action='store_true')
    args = parser.parse_args()
    repo = Path(__file__).resolve().parents[1]
    args.output.mkdir(parents=True, exist_ok=False)
    source = args.output/'inputs'; source.mkdir()
    (source/'fixture.c').write_text('extern void enter(void);\nextern void leave(void);\nint shared;\nvoid updater(void) {\n enter();\n shared=1;\n leave();\n}\nint reader(void) {\n return shared;\n}\n')
    (source/'extra.c').write_text('extern int shared;\nvoid uncompiled(void) {\n shared=2;\n}\n')
    bitcode = args.output/'fixture.ll'
    subprocess.run(['/usr/bin/clang-16', '-g', '-O0', '-fno-discard-value-names', '-S', '-emit-llvm',
                    str(source/'fixture.c'), '-o', str(bitcode)], check=True)
    operation = lambda path, line, kind, expression: {'path': path, 'line': line, 'kind': kind, 'expression': expression}
    write = operation('fixture.c', 6, 'write', 'shared')
    read = operation('fixture.c', 10, 'read', 'shared')
    missing = operation('extra.c', 3, 'write', 'shared')
    enter = operation('fixture.c', 5, 'call', 'enter')
    leave = operation('fixture.c', 7, 'call', 'leave')
    req = {'kind': 'requirement', 'form': 'MustBeMediated', 'resource': 'shared', 'scope': 'same shared',
           'a': [write], 'b': [read], 'actors': ['updater', 'reader'],
           'assumptions': ['OFFLINE fixture does not establish concurrency or harm'], 'note': 'OFFLINE synthetic original'}
    gua = {'kind': 'guarantee', 'form': 'Exclude', 'resource': 'shared', 'scope': 'same shared', 'a': [write],
           'evidence': [enter, leave], 'regions': [{'start': enter, 'end': leave}], 'token': 'fixture lock',
           'mode': 'exclusive', 'conditions': 'OFFLINE fixture supplies no actual synchronization semantics',
           'unconditional': False, 'note': 'OFFLINE synthetic guarantee'}
    unknown = {**req, 'a': [missing], 'note': 'OFFLINE source-only uncompiled operation'}
    state = {'step': 0, 'requests': 0, 'real_model_calls': 0}
    errors = []

    def files_command(files, remove=()):
        return "python3 - <<'PY'\nimport json\nfrom pathlib import Path\nfiles=json.loads("+repr(json.dumps(files))+" )\nfor name,data in files.items():\n p=Path('contracts')/name\n p.parent.mkdir(parents=True,exist_ok=True)\n temporary=p.with_suffix('.tmp')\n temporary.write_text(json.dumps(data))\n temporary.replace(p)\nfor name in "+repr(list(remove))+":\n (Path('contracts')/name).unlink()\nPY"

    def feedback(content):
        return json.loads(content.split('[contract artifacts] ', 1)[1])

    class Handler(BaseHTTPRequestHandler):
        def log_message(self, *_):
            pass

        def do_POST(self):
            try:
                body = json.loads(self.rfile.read(int(self.headers['Content-Length'])))
                state['requests'] += 1
                tools = {t['function']['name']: t['function'] for t in body['tools']}
                assert {'exec_command', 'write_stdin'} <= set(tools)
                if 'finalize_contract' in tools:
                    assert not {'add_requirement', 'add_guarantee', 'get_source_operation', 'get_source_operations', 'record_gap'} & set(tools)
                messages = body['messages']; last = messages[-1].get('content', '')
                step = state['step']
                if step == 0:
                    initial = next(m['content'] for m in messages if m.get('role') == 'user')
                    instructions = '\n'.join(m.get('content', '') for m in messages if m.get('role') in {'system', 'developer'})
                    assert '<source file=' not in initial and 'shared=1' not in initial
                    assert 'add_requirement' not in instructions and 'record_pattern_review' not in instructions
                    assert 'source_id' not in tools['get_operations']['parameters']['properties']
                    (args.output/'initial-request.json').write_text(json.dumps(body, indent=2))
                    name, arguments = 'exec_command', {'cmd': 'cat contract_format.json; nl -ba module/fixture.c; nl -ba module/extra.c'}
                elif step == 1:
                    assert 'lace-source-contract-v1' in last and 'shared=1' in last
                    name, arguments = 'exec_command', {'cmd': files_command({'relation.json': req, 'protection.json': gua})}
                elif step == 2:
                    result = feedback(last)
                    assert not result['errors'] and {r['id'] for r in result['accepted']} == {'req:0', 'gua:0'}
                    if args.fail_after_import:
                        return self.respond({'error': {'message': 'OFFLINE API failure after accepted files',
                            'type': 'invalid_request_error', 'code': 'context_length_exceeded'}}, 400)
                    name, arguments = 'exec_command', {'cmd': files_command({
                        'relation.json': {**req, 'note': 'OFFLINE revision two'}, 'unknown.json': unknown})}
                elif step == 3:
                    result = feedback(last)
                    assert not result['errors'] and any(r['id'] == 'req:0' and r['revision'] == 2 for r in result['accepted'])
                    assert any(r['id'] == 'req:1' and r['unknown_operations'] for r in result['accepted'])
                    name, arguments = 'exec_command', {'cmd': files_command({'relation.json': {**req, 'b': []}})}
                elif step == 4:
                    result = feedback(last)
                    assert 'relation.json' in result['errors']
                    name, arguments = 'exec_command', {'cmd': 'cat contract_status.json; cat contract_state.json'}
                elif step == 5:
                    assert 'OFFLINE revision two' in last and 'Operand B required' in last
                    name, arguments = 'finalize_contract', {}
                elif step == 6:
                    assert 'Invalid contract files remain' in last
                    name, arguments = 'exec_command', {'cmd': files_command({
                        'relation.json': {**req, 'note': 'OFFLINE repaired revision three'},
                        'nested/unknown.json': unknown}, ['unknown.json'])}
                elif step == 7:
                    result = feedback(last)
                    assert not result['errors']
                    assert any(r['id'] == 'req:2' for r in result['accepted'])
                    assert any(r['id'] == 'req:1' and r.get('deleted') for r in result['accepted'])
                    name, arguments = 'get_operations', {'operation': write, 'offset': 0, 'limit': 10}
                elif step == 8:
                    result = json.loads(last)
                    assert result['rows'] and result['source_binding']['status'] == 'bound'
                    name, arguments = 'finalize_contract', {}
                elif step == 9:
                    assert 'SOURCE REQUIREMENT CROSSCHECK' in last
                    assert any(m.get('role') == 'tool' and 'req:0' in m.get('content', '') for m in messages)
                    name, arguments = 'exec_command', {'cmd': files_command({'protection.json': {**gua, 'note': 'OFFLINE crosscheck revised Gua'}})}
                elif step == 10:
                    result = feedback(last)
                    assert result['accepted'][0]['id'] == 'gua:0' and result['accepted'][0]['revision'] == 2
                    name, arguments = 'finalize_contract', {}
                elif step == 11:
                    assert args.review and 'review_obligations' in tools
                    name, arguments = 'exec_command', {'cmd': 'printf changed > contracts/relation.json'}
                elif step == 12:
                    assert 'Read-only file system' in last
                    name, arguments = 'get_obligations', {'ids': []}
                elif step == 13:
                    assert {r['id'] for r in json.loads(last)} == {'req:0', 'req:2'}
                    name, arguments = 'review_obligations', {'ids': ['req:0', 'req:2'], 'status': 'unknown',
                        'evidence_nodes': [], 'witness_nodes': [], 'evidence_sources': [], 'witness_sources': [],
                        'repository_evidence_ids': [], 'reason': 'OFFLINE synthetic unknown judgment', 'interleaving': ''}
                elif step == 14:
                    name, arguments = 'finalize_review', {}
                else:
                    raise AssertionError(f'Unexpected request step {step}')
                state['step'] += 1
                self.respond({'id': 'offline', 'object': 'chat.completion', 'created': 1, 'model': 'offline-scripted',
                    'choices': [{'index': 0, 'finish_reason': 'tool_calls', 'message': {'role': 'assistant', 'content': None,
                        'tool_calls': [{'id': f'call_{step}', 'type': 'function', 'function': {'name': name, 'arguments': json.dumps(arguments)}}]}}],
                    'usage': {'prompt_tokens': 1, 'completion_tokens': 1, 'total_tokens': 2}})
            except Exception as error:
                errors.append({'step': state['step'], 'error': repr(error)})
                (args.output/'failed-request.json').write_text(json.dumps(body, indent=2))
                self.respond({'error': {'message': str(error), 'type': 'invalid_request_error'}}, 400)

        def respond(self, value, status=200):
            payload = json.dumps(value).encode()
            self.send_response(status); self.send_header('Content-Type', 'application/json')
            self.send_header('Content-Length', str(len(payload))); self.end_headers(); self.wfile.write(payload)

    server = ThreadingHTTPServer(('127.0.0.1', 0), Handler)
    thread = threading.Thread(target=server.serve_forever, daemon=True); thread.start()
    env = os.environ.copy()
    env.update(LLM_BASE_URL=f'http://127.0.0.1:{server.server_port}/v1', LLM_API_KEY='offline-test-only',
               LLM_MODEL='offline-scripted', NO_PROXY='127.0.0.1,localhost', no_proxy='127.0.0.1,localhost')
    env.pop('LACE_MODEL_API', None)
    try:
        run = args.output/'native'
        code = subprocess.call([sys.executable, str(repo/'harness/run_module.py'), '--contract-input', 'artifacts',
            '--source', str(source), '--bitcode', str(bitcode), '--output', str(run),
            '--requirement-sweep', 'on'] + ([] if args.review else ['--no-review']), env=env, stdout=subprocess.DEVNULL)
        assert not errors, errors
        assert code == (2 if args.fail_after_import else 0), f'Exit {code}; see {run}/run.log'
        manifest = json.loads((run/'run.json').read_text())
        contracts = json.loads(Path(manifest['report']).read_text())
        assert contracts['contract_input'] == 'artifacts' and contracts['coverage']['e_finished']
        assert 'req:0' in contracts['checks']
        if args.fail_after_import:
            assert contracts['partial_extraction'] and not contracts['coverage']['workflow_complete']
            assert len(contracts['requirements']) == len(contracts['guarantees']) == 1
            assert contracts['review_disabled'] and 'BadRequestError' in contracts['error']
        else:
            assert contracts['coverage']['workflow_complete'] and not contracts['artifact_errors']
            assert {r['id'] for r in contracts['requirements']} == {'req:0', 'req:2'}
            assert contracts['contract_artifacts']['relation.json']['revision'] == 3
            assert contracts['contract_artifacts']['protection.json']['revision'] == 2
            assert contracts['contract_artifacts']['nested/unknown.json']['id'] == 'req:2'
            assert any(r['event'] == 'deleted' for r in contracts['artifact_revisions'])
            assert not contracts['pattern_reviews'] and len(contracts['extraction_passes']) == 2
            assert contracts['checks']['req:2']['resolved_operands']['a_nodes'] == []
            assert len(contracts['reviews']) == (2 if args.review else 0)
        result = {**state, 'native_exit': code, 'review': args.review, 'failure': args.fail_after_import,
                  'workflow_complete': contracts['coverage']['workflow_complete'], 'E_finished': True}
        (args.output/'validation.json').write_text(json.dumps(result, indent=2))
        print(json.dumps(result))
    finally:
        server.shutdown(); server.server_close(); thread.join(timeout=3)


if __name__ == '__main__':
    main()
