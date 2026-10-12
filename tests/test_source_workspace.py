"""Real SDK shell tests on synthetic files; no model or GT analysis."""
import asyncio
import importlib.util
import json
import os
from pathlib import Path
import re
import shlex
import sys
import pytest

MODULE = Path(__file__).resolve().parents[1] / 'harness/source_workspace.py'
spec = importlib.util.spec_from_file_location('source_workspace', MODULE)
workspace_module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(workspace_module)


def test_sdk_source_shell_isolation_and_live_ledger(tmp_path, monkeypatch):
    from agents import RunContextWrapper
    source = tmp_path / 'source'; source.mkdir()
    text = 'int source_marker;\nint function(void) { return source_marker; }\n'
    (source / 'hello.c').write_text(text)
    (source / 'cpg.dot').write_text('AST_MUST_NOT_BE_EXPOSED')
    (source / 'ground_truth.json').write_text('GT_MUST_NOT_BE_EXPOSED')
    private = tmp_path / 'private.txt'; private.write_text('HOST_PRIVATE_MUST_NOT_BE_EXPOSED')
    ledger = tmp_path / 'module_contracts.json'
    state = {'source_manifest': [{'path': 'hello.c', 'lines': 2, 'bytes': len(text)}],
             'requirements': [{'id': 'req:0'}], 'guarantees': [{'id': 'gua:0', 'note': 'original'}]}
    ledger.write_text(json.dumps(state))
    monkeypatch.setenv('LLM_API_KEY', 'SECRET_MUST_NOT_BE_EXPOSED')

    async def check():
        tools, session, config = await workspace_module.source_tools({'source_root': str(source), 'contract_path': str(ledger)})
        context = RunContextWrapper(context=None)
        tool = {t.name: t for t in tools}
        async def execute(command, **kwargs):
            return await tool['exec_command'].on_invoke_tool(context, json.dumps({'cmd': command, **kwargs}))
        try:
            assert set(tool) == {'exec_command', 'write_stdin'}
            assert config['files'] == 1
            output = await execute("sed -n '1,2p' module/hello.c; find module -type f; env")
            assert 'source_marker' in output and 'hello.c' in output
            assert not any(word in output for word in ['SECRET_MUST', 'LLM_API_KEY', 'cpg.dot', 'ground_truth.json'])
            output = await execute('cat '+shlex.quote(str(private)))
            assert 'HOST_PRIVATE_MUST_NOT_BE_EXPOSED' not in output
            output = await execute('printf changed > module/hello.c')
            assert 'Read-only file system' in output
            assert (source/'hello.c').read_text() == text
            assert (Path(config['workspace'])/'module/hello.c').read_text() == text
            assert 'original' in await execute('cat contract_state.json')
            state['guarantees'][0]['note'] = 'corrected'
            temporary = ledger.with_suffix('.tmp'); temporary.write_text(json.dumps(state)); temporary.replace(ledger)
            assert 'corrected' in await execute('cat contract_state.json')
            output = await execute('sleep 0.3; printf SDK_INTERACTIVE_DONE', yield_time_ms=1)
            match = re.search(r'session ID (\d+)', output)
            assert match, output
            output = await tool['write_stdin'].on_invoke_tool(context, json.dumps({
                'session_id': int(match.group(1)), 'chars': '', 'yield_time_ms': 1000}))
            assert 'SDK_INTERACTIVE_DONE' in output
        finally:
            await session.aclose()
        events = [json.loads(line) for line in Path(config['tool_trace']).read_text().splitlines()]
        assert any(e['event'] == 'tool_result' and 'source_marker' in e.get('output', '') for e in events)
        assert any(e['name'] == 'write_stdin' for e in events)
    asyncio.run(check())


def test_source_snapshot_stays_consistent_across_phases(tmp_path):
    source = tmp_path/'source'; source.mkdir(); (source/'hello.c').write_text('int original;\n')
    ledger = tmp_path/'module_contracts.json'
    ledger.write_text(json.dumps({'source_manifest': [{'path': 'hello.c', 'lines': 1, 'bytes': 14}]}))
    workspace_module.stage_sources(source, ledger)
    workspace_module.stage_sources(source, ledger)
    (source/'hello.c').write_text('int changed;\n')
    with pytest.raises(ValueError, match='source bytes changed'):
        workspace_module.stage_sources(source, ledger)
