"""Real source/IR end-to-end plumbing test using a local scripted model.

Synthetic contracts/judgments exercise transport, anchoring, catalog extension,
E and final reports. This is NOT a concurrency detection quality experiment.
"""
import argparse
import json
import os
from pathlib import Path
import re
import subprocess
import sys
import threading
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source', type=Path, required=True)
    parser.add_argument('--bitcode', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--no-review', action='store_true')
    parser.add_argument('--catalog', type=Path)
    parser.add_argument('--source-anchors', action='store_true', help='Exercise synthetic source-only report output')
    parser.add_argument('--recover-lock-binding', action='store_true', help='OA TC6 only: exercise inspected macro-call binding, reread, withdrawal and E persistence; no real model')
    parser.add_argument('--exhaust-after-contract', action='store_true', help='Verify budget exhaustion still runs E on accepted partial contracts')
    parser.add_argument('--fail-after-contract', choices=['context_error', 'api_error', 'disconnect'],
                        help='Fail the localhost API after accepting Req/Gua; verify partial E and incomplete status')
    args = parser.parse_args()
    if args.exhaust_after_contract and args.fail_after_contract:
        parser.error('Choose one extraction failure mode')
    repo = Path(__file__).resolve().parents[1]
    state = {'calls': 0, 'step': 'start', 'phase': 'A'}
    errors = []

    class Handler(BaseHTTPRequestHandler):
        def log_message(self, *_):
            pass

        def do_POST(self):
            try:
                body = json.loads(self.rfile.read(int(self.headers['Content-Length'])))
                assert body['tool_choice'] == 'required'
                tools = {t['function']['name']: t['function'] for t in body['tools']}
                assert 'get_object' not in tools and 'record_api_summary' not in tools
                assert {'exec_command', 'write_stdin'} <= set(tools)
                assert not {'read_file', 'grep_source', 'list_dir'} & set(tools)
                messages = body['messages']
                initial = next(m['content'] for m in messages if m.get('role') == 'user')
                if 'review_obligations' in tools and state['phase'] == 'A':
                    state.update(phase='C', step='obligations')
                    assert 'Phase A.' in initial and 'NO precomputed requirements' in initial, 'History or extraction setup lost'
                step = state['step']
                if step == 'fail':
                    return self.do_failure()
                if step == 'start':
                    assert tools['add_requirement']['parameters']['properties']['form']['enum'] == ['MustPrecede', 'MustBeAtomic', 'MustBeMediated']
                    assert 'owner_thread' not in tools['add_requirement']['parameters']['properties']
                    catalog = json.loads(initial.split('\nCatalog: ', 1)[1].split('\nRepository context: ', 1)[0])
                    state['patterns'] = [p['id'] for p in catalog['patterns']]
                    assert '<source file=' not in initial, 'Source bodies were injected'
                    state['source_bytes_in_first_prompt'] = len(initial.encode())
                    command = "python3 - <<'PY'\nimport json\nfrom pathlib import Path\nfiles=json.loads(Path('module_files.json').read_text())\nfor row in files:\n p=Path(row['path'])\n if p.suffix not in {'.c','.cpp','.cc','.cxx'}:continue\n lines=(Path('module')/p).read_text().splitlines()\n selected=[i+1 for i,line in enumerate(lines) if line.strip()][:2]\n if len(selected)<2:continue\n print('OFFLINE_SOURCE_SAMPLE='+json.dumps({'path':str(p),'lines':selected,'last_line':len(lines)}))\n break\nPY"
                    name, arguments = 'exec_command', {'cmd': command}
                    state['step'] = 'source_index'
                elif step == 'source_index':
                    sample = re.search(r'OFFLINE_SOURCE_SAMPLE=(\{.*\})', messages[-1]['content'])
                    assert sample, messages[-1]['content']
                    state.update(json.loads(sample.group(1)))
                    name, arguments = 'get_module_functions', {'name': '', 'origin': 'ir', 'offset': 0, 'limit': 1}
                    state['step'] = 'function_page'
                elif step == 'function_page':
                    page = json.loads(messages[-1]['content'])
                    assert page['rows'] and page['total'] >= 1
                    state['ir_function_id'] = page['rows'][0]['function_id']
                    state['ir_function_total'] = page['total']
                    name, arguments = 'get_operations', {'function_id': state['ir_function_id'], 'path': '', 'first_line': 0, 'last_line': 0, 'origin': 'ir', 'offset': 0, 'limit': 1}
                    state['step'] = 'operation_page'
                elif step == 'operation_page':
                    page = json.loads(messages[-1]['content'])
                    assert page['rows'] and page['next_offset'] is not None, 'Entire IR function/operations not retained'
                    name, arguments = 'get_operations', {'function_id': state['ir_function_id'], 'path': '', 'first_line': 0, 'last_line': 0, 'origin': 'ir', 'offset': page['next_offset'], 'limit': 1}
                    state['step'] = 'anchors'
                elif step == 'anchors':
                    assert json.loads(messages[-1]['content'])['rows']
                    name, arguments = 'get_operations', {'function_id': -1, 'path': '', 'first_line': 0, 'last_line': 0, 'origin': 'source', 'offset': 0, 'limit': 1000}
                    state['step'] = 'ast_rejected'
                elif step == 'ast_rejected':
                    assert 'IR evidence only' in messages[-1]['content'], 'AST query was not rejected'
                    name, arguments = 'get_source_operations', {'operations': [{'path': state['path'], 'first_line': n, 'last_line': n} for n in state['lines']] + [{'path': state['path'], 'first_line': 1, 'last_line': state['last_line']}]}
                    state['step'] = 'compact_anchors'
                elif step == 'compact_anchors':
                    anchors = json.loads(messages[-1]['content'])
                    assert len(anchors) == 3 and all(a['operation_equivalence_proved'] is False for a in anchors)
                    assert all('graph_candidates' not in a and 'nodes' not in a and 'rows' not in a for a in anchors), 'Graph dump leaked into source evidence'
                    broad=anchors[-1]
                    assert broad['ir_candidate_count'] > 1 and broad['binding'] in {'operation_not_resolved','ambiguous'}
                    state['source_anchor_response_bytes']=len(messages[-1]['content'].encode())
                    state['anchors'] = [a['source_id'] for a in anchors[:2]]
                    name, arguments = 'get_operations', {'function_id': -1, 'path': '', 'first_line': 0, 'last_line': 0, 'source_id': broad['source_id'], 'origin': 'ir', 'offset': 0, 'limit': 1}
                    state['step'] = 'ir_anchor_query'
                elif step == 'ir_anchor_query':
                    page=json.loads(messages[-1]['content'])
                    assert len(page['rows'])==1 and page['next_offset'] is not None
                    assert page['rows'][0]['origin']=='ir' and page['rows'][0]['kind']!='Method'
                    name, arguments = 'get_module_calls', {'apis': [], 'offset': 0, 'limit': 1}
                    state['step'] = 'compact_calls'
                elif step == 'compact_calls':
                    page=json.loads(messages[-1]['content'])
                    assert len(page['rows'])<=1
                    assert all(set(row)=={'api','path','line','containing_function'} for row in page['rows']), 'AST call records leaked'
                    name, arguments = 'get_module_calls', {'apis': [], 'offset': page['next_offset'] or page['total'], 'limit': 1}
                    state['step'] = 'invalid_type'
                elif step == 'invalid_type':
                    assert len(json.loads(messages[-1]['content'])['rows'])<=1
                    if args.recover_lock_binding and not state.get('binding_recovery_done'):
                        state['lock_anchor']={'path':'drivers/net/ethernet/oa_tc6.c','first_line':333,'last_line':333,
                            'operation_kind':'call','symbol':'mutex_lock','function':'oa_tc6_read_registers'}
                        name, arguments='get_source_operation',state['lock_anchor']
                        state['step']='recovery_uninspected'
                    else:
                        name, arguments=self.invalid_requirement()
                elif step == 'recovery_uninspected':
                    op=json.loads(messages[-1]['content'])
                    assert op['binding']=='operation_not_resolved' and op['ir_candidate_count']>0
                    state['recovered_source']=op['source_id']
                    name, arguments='bind_source_operation',{'source_id':op['source_id'],'node_id':2147483647,'reason':'OFFLINE probe of uninspected-node rejection'}
                    state['step']='recovery_query'
                elif step == 'recovery_query':
                    assert 'Inspect the selected IR instruction' in messages[-1]['content']
                    name, arguments='get_operations',{'function_id':-1,'path':'','first_line':0,'last_line':0,
                        'source_id':state['recovered_source'],'origin':'ir','offset':0,'limit':1000}
                    state['step']='recovery_select'
                elif step == 'recovery_select':
                    page=json.loads(messages[-1]['content'])
                    matches=[n for n in page['rows'] if n.get('api')=='mutex_lock_nested']
                    assert len(matches)==1 and page['source_binding']['status']=='operation_not_resolved'
                    state['recovered_node']=matches[0]['node_id']
                    name, arguments=self.recover_binding()
                    state['step']='recovery_reregister'
                elif step == 'recovery_reregister':
                    result=json.loads(messages[-1]['content'])
                    assert result['recorded'] and result['binding']['binding_origin']=='agent_selected'
                    assert result['binding']['node_id']==state['recovered_node'] and not result['synchronization_established']
                    name, arguments='get_source_operation',state['lock_anchor']
                    state['step']='recovery_withdraw'
                elif step == 'recovery_withdraw':
                    op=json.loads(messages[-1]['content'])
                    assert op['binding']=='bound' and op['agent_binding']['node_id']==state['recovered_node']
                    name, arguments='bind_source_operation',{'source_id':state['recovered_source'],'node_id':-1,
                        'reason':'OFFLINE withdrawal exercises restoration of unknown correspondence'}
                    state['step']='recovery_reselect'
                elif step == 'recovery_reselect':
                    result=json.loads(messages[-1]['content'])
                    assert result['binding']['status']=='operation_not_resolved'
                    name, arguments=self.recover_binding()
                    state['step']='recovery_done'
                elif step == 'recovery_done':
                    assert json.loads(messages[-1]['content'])['binding']['status']=='bound'
                    state['binding_recovery_done']=True
                    state['anchors'][0]=state['recovered_source']
                    name, arguments=self.invalid_requirement()
                elif step == 'invalid_node':
                    assert 'Invalid contract form' in messages[-1]['content']
                    state['req']['form'] = 'MustPrecede'
                    name, arguments = 'add_requirement', {**state['req'], 'a_sources': [], 'a_nodes': [2147483647]}
                    state['step'] = 'add_valid'
                elif step == 'add_valid':
                    assert 'Unknown operation ID' in messages[-1]['content']
                    name, arguments = 'add_requirement', state['req']
                    state['step'] = 'early_finish'
                elif step == 'early_finish':
                    assert json.loads(messages[-1]['content'])['id'] == 'req:0', 'Invalid claims mutated the ledger'
                    if args.recover_lock_binding:
                        recovered=json.loads(messages[-1]['content'])['diagnostics']['bindings'][0]
                        assert recovered['binding_origin']=='agent_selected' and recovered['selected_node_id']==state['recovered_node']
                    if args.fail_after_contract:
                        name, arguments = 'add_guarantee', {
                            'form': 'Order', 'resource': state['req']['resource'], 'scope': state['req']['scope'],
                            'a_nodes': [], 'b_nodes': [], 'a_sources': [state['anchors'][0]], 'b_sources': [state['anchors'][1]],
                            'token': '', 'mode': '', 'conditions': 'OFFLINE synthetic synchronization is unresolved',
                            'unconditional': False, 'evidence_nodes': [], 'evidence_sources': [state['anchors'][0]],
                            'regions': [], 'source_regions': [], 'repository_evidence_ids': [],
                            'note': 'OFFLINE SYNTHETIC Gua tests persistence across an extraction error; no synchronization claim.'}
                        state['step'] = 'guarantee_accepted'
                    else:
                        name, arguments = 'finalize_contract', {}
                        state['step'] = 'pattern'
                elif step == 'guarantee_accepted':
                    assert json.loads(messages[-1]['content'])['id'] == 'gua:0'
                    state['step'] = 'fail'
                    return self.do_failure()
                elif step == 'pattern':
                    assert 'Unaccounted catalog patterns' in messages[-1]['content']
                    state['pattern_index'] = 0
                    name, arguments = self.pattern()
                elif step == 'pattern_next':
                    state['pattern_index'] += 1
                    if state['pattern_index'] < len(state['patterns']):
                        name, arguments = self.pattern()
                    else:
                        name, arguments = 'finalize_contract', {}
                        state['step'] = 'await_c'
                elif step == 'obligations':
                    name, arguments = 'get_obligations', {'ids': []}
                    state['step'] = 'judge'
                elif step == 'judge':
                    rows = json.loads(messages[-1]['content'])
                    assert len(rows) == 1 and rows[0]['id'] == 'req:0'
                    assert rows[0]['checker_evidence']['status'] == 'requires_semantic_review'
                    assert not rows[0]['checker_evidence']['proof'] and not rows[0]['checker_evidence']['counterexample_constructed']
                    assert any(gap.get('snapshot_present') is True for gap in rows[0]['checker_evidence']['gaps']), 'Real source snapshot not passed to E'
                    name, arguments = 'review_obligations', {'ids': ['req:0'], 'status': 'source_supported' if args.source_anchors else 'unknown',
                        'evidence_nodes': [], 'witness_nodes': [], 'evidence_sources': state['anchors'],
                        'witness_sources': state['anchors'] if args.source_anchors else [], 'repository_evidence_ids': [],
                        'reason': 'OFFLINE SYNTHETIC judgment tests pipeline; no actual code feasibility or harm analysis.',
                        'interleaving': 'OFFLINE SYNTHETIC A/B interleaving, no actual semantic assessment.' if args.source_anchors else ''}
                    state['step'] = 'finish_c'
                elif step == 'finish_c':
                    name, arguments = 'finalize_review', {}
                    state['step'] = 'done'
                else:
                    raise AssertionError(f'Unexpected request: {state}')
                state['calls'] += 1
                payload = json.dumps({'id': 'offline', 'object': 'chat.completion', 'created': 1, 'model': 'offline-scripted',
                    'choices': [{'index': 0, 'message': {'role': 'assistant', 'content': None, 'tool_calls': [{'id': f"call-{state['calls']}", 'type': 'function',
                        'function': {'name': name, 'arguments': json.dumps(arguments)}}]}, 'finish_reason': 'tool_calls'}],
                    'usage': {'prompt_tokens': 0, 'completion_tokens': 0, 'total_tokens': 0}}).encode()
                self.send_response(200); self.send_header('Content-Type', 'application/json'); self.send_header('Content-Length', str(len(payload)))
                self.end_headers(); self.wfile.write(payload)
            except Exception as exc:
                errors.append(repr(exc))
                self.send_error(500, 'Scripted test assertion failed')

        def pattern(self):
            state['step'] = 'pattern_next'
            return 'record_pattern_review', {'pattern_id': state['patterns'][state['pattern_index']], 'status': 'unknown', 'reason': 'Offline test; model semantic audit intentionally not performed.'}

        def invalid_requirement(self):
            state['req'] = {'form': 'CallPrecondition', 'resource': 'OFFLINE SYNTHETIC protocol resource', 'scope': 'synthetic-one-instance',
                'actors': ['synthetic activation A', 'synthetic activation B'], 'a_nodes': [], 'b_nodes': [],
                'a_sources': [state['anchors'][0]], 'b_sources': [state['anchors'][1]], 'interference_nodes': [],
                'interference_sources': [], 'assumptions': ['Offline plumbing; no semantic or feasibility assessment'], 'repository_evidence_ids': [],
                'note': 'OFFLINE SYNTHETIC Req exercises source-only preservation; it is NOT a discovered concurrency defect.'}
            state['step']='invalid_node'
            return 'add_requirement',state['req']

        def recover_binding(self):
            return 'bind_source_operation',{'source_id':state['recovered_source'],'node_id':state['recovered_node'],
                'reason':'OFFLINE scripted correspondence: source mutex_lock macro compiles to inspected mutex_lock_nested with subclass 0; no synchronization judgment'}

        def do_failure(self):
            state['calls'] += 1
            if args.fail_after_contract == 'disconnect':
                self.close_connection = True
                self.connection.shutdown(2)
                return
            context_error = args.fail_after_contract == 'context_error'
            payload = json.dumps({'error': {
                'message': 'OFFLINE synthetic context limit' if context_error else 'OFFLINE synthetic API unavailable',
                'type': 'invalid_request_error' if context_error else 'server_error',
                'code': 'context_length_exceeded' if context_error else 'offline_unavailable'}}).encode()
            self.send_response(400 if context_error else 503)
            self.send_header('Content-Type', 'application/json')
            self.send_header('Content-Length', str(len(payload)))
            self.end_headers(); self.wfile.write(payload)

    server = ThreadingHTTPServer(('127.0.0.1', 0), Handler)
    thread = threading.Thread(target=server.serve_forever, daemon=True); thread.start()
    env = os.environ.copy()
    env.update(LLM_BASE_URL=f'http://127.0.0.1:{server.server_port}/v1', LLM_API_KEY='offline-test-only', LLM_MODEL='offline-scripted',
               NO_PROXY='127.0.0.1,localhost', no_proxy='127.0.0.1,localhost', LACE_PROTOCOL_CATALOG=str(args.catalog.resolve()) if args.catalog else str(repo/'config/protocol_catalog.json'))
    env.pop('LACE_MODEL_API', None)
    try:
        code = subprocess.call([sys.executable, str(repo/'harness/run_module.py'), '--source', str(args.source), '--bitcode', str(args.bitcode), '--output', str(args.output)] + (['--no-review'] if args.no_review else []) + (['--max-turns', '13'] if args.exhaust_after_contract else []), env=env,
                               stdout=subprocess.DEVNULL)
        assert not errors, errors
        assert code == (2 if args.exhaust_after_contract or args.fail_after_contract else 0), f'Unexpected native exit; see {args.output}/run.log'
        manifest = json.loads((args.output/'run.json').read_text())
        report_path = Path(manifest['report']); report = json.loads(report_path.read_text())
        model = json.loads((report_path.parent/'module_model.json').read_text())
        if args.exhaust_after_contract or args.fail_after_contract:
            assert state['phase'] == 'A'
            if args.exhaust_after_contract:
                assert state['calls'] == 13
            else:
                assert len(report['guarantees']) == 1 and report['guarantees'][0]['id'] == 'gua:0'
            assert len(report['requirements']) == 1 and 'req:0' in report['checks']
            assert report['coverage']['e_finished'] and not report['coverage']['generation_finished']
            assert not report['coverage']['workflow_complete'] and report['partial_extraction']
            assert report['review_disabled'] and not report['reviews']
            expected_error = {'context_error': 'BadRequestError', 'api_error': 'InternalServerError',
                              'disconnect': 'APIConnectionError'}.get(args.fail_after_contract, 'MaxTurnsExceeded')
            assert expected_error in report['error'] and manifest['exit_code'] == 2
            assert (report_path.parent/'stateful_bugs/reports.json').is_file()
            assert (report_path.parent/'sdk_source_tools.jsonl').is_file()
            print(json.dumps({'budget_exhaustion_regression': args.exhaust_after_contract,
                              'extraction_failure_regression': args.fail_after_contract or 'turn_limit', 'real_model_calls': 0,
                              'scripted_calls': state['calls'], 'coverage': report['coverage']}))
            return
        assert model['parallel_entries_inferred'] is False
        source_events = [json.loads(line) for line in (report_path.parent/'sdk_source_tools.jsonl').read_text().splitlines()]
        assert any(row['name'] == 'exec_command' and row['event'] == 'tool_result' for row in source_events)
        assert model['coverage']['ir_functions'] == state['ir_function_total']
        assert report['coverage']['workflow_complete']
        if args.recover_lock_binding:
            assert state['binding_recovery_done'] and len(report['source_binding_revisions'])==3
            recovered=report['checks']['req:0']['bindings'][0]
            assert recovered['binding_origin']=='agent_selected' and recovered['node_id']==state['recovered_node']
            assert report['checks']['req:0']['resolved_operands']['a_nodes']==[state['recovered_node']]
        assert all('graph_candidates' not in op for op in report['source_operations'].values())
        assert len(report['requirements']) == 1 and len(report['reviews']) == (0 if args.no_review else 1) and not report['guarantees']
        if args.no_review:
            assert state['phase'] == 'A' and report['review_disabled'] and report['coverage']['e_finished']
        assert 'owner_thread' not in report['requirements'][0] and 'object_id' not in report['requirements'][0]
        assert len(report['pattern_reviews']) == len(state['patterns'])
        assert not (report_path.parent/'vulnerability_surface.json').exists()
        final = json.loads((report_path.parent/'stateful_bugs/reports.json').read_text())
        assert len(final) == (0 if args.no_review else int(args.source_anchors))
        if final:
            assert not final[0]['reproduced'] and 'OFFLINE SYNTHETIC' in final[0]['evidence'][0]['description']
            assert len(final[0]['evidence'][0]['source_operations']) == 2
        summary = {'simulated_model': True, 'real_model_calls': 0, 'scripted_calls': state['calls'], 'graph': model['coverage'],
            'catalog_patterns': state['patterns'], 'coverage': report['coverage'], 'synthetic_reports': len(final),
            'source_anchor_response_bytes':state['source_anchor_response_bytes'], 'ast_node_observations':0,
            'ir_evidence_is_on_demand':True}
        summary['source_binding_recovery']=args.recover_lock_binding
        (args.output/'offline-validation.json').write_text(json.dumps(summary, indent=2))
        print(json.dumps(summary, indent=2))
    finally:
        server.shutdown(); server.server_close(); thread.join(timeout=3)


if __name__ == '__main__':
    main()
