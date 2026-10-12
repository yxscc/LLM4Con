"""Expose SDK shell tools over an isolated, read-only source workspace.

The SDK owns command execution, PTYs, output formatting and the agent loop.
This adapter only stages allowed inputs, confines the local backend, and audits
its observations. It does not implement an alternative shell or tool harness.
"""
import hashlib
import json
import os
from pathlib import Path
import shutil
import sys


def stage_sources(source_root, contract_path):
    source = Path(source_root).resolve(strict=True)
    ledger = Path(contract_path).resolve(strict=True)
    state = json.loads(ledger.read_text())
    workspace = ledger.parent / 'agent_workspace'
    module = workspace / 'module'
    module.mkdir(parents=True, exist_ok=True)
    index = workspace / 'module_files.json'
    previous = {row['path']: row for row in json.loads(index.read_text())} if index.exists() else None
    current_paths = {row['path'] for row in state['source_manifest']}
    if previous is not None and set(previous) != current_paths:
        raise ValueError('Module source inventory changed during the conversation')
    inventory = []
    for row in state['source_manifest']:
        relative = Path(row['path'])
        if relative.is_absolute() or '..' in relative.parts:
            raise ValueError('Source manifest contains a non-module path')
        original = source / relative
        if original.is_symlink() or not original.resolve(strict=True).is_relative_to(source):
            raise ValueError('Source manifest leaves the supplied module')
        contents = original.read_bytes()
        target = module / relative
        target.parent.mkdir(parents=True, exist_ok=True)
        if target.is_symlink():
            raise ValueError('Source workspace contains a symlink')
        digest = hashlib.sha256(contents).hexdigest()
        if previous is not None:
            if previous[row['path']]['sha256'] != digest or not target.is_file() or hashlib.sha256(target.read_bytes()).hexdigest() != digest:
                raise ValueError('Module source bytes changed during the conversation: '+row['path'])
        else:
            target.write_bytes(contents)
        inventory.append({**row, 'sha256': digest})
    index.write_text(json.dumps(inventory, indent=2))
    # The live ledger is mounted over this placeholder for each command. Atomic
    # backend rewrites therefore remain visible on the next query, including C.
    (workspace / 'contract_state.json').write_text('{}\n')
    return workspace, ledger, inventory


async def source_tools(request):
    from agents.sandbox import Manifest
    from agents.sandbox.sandboxes.unix_local import UnixLocalSandboxClient, UnixLocalSandboxSession
    from agents.sandbox.capabilities.tools.shell_tool import ExecCommandTool, WriteStdinTool

    if sys.platform != 'linux':
        raise RuntimeError('The source workspace currently requires Linux with bubblewrap')
    bwrap = os.getenv('LACE_SOURCE_BWRAP') or shutil.which('bwrap')
    if not bwrap:
        raise RuntimeError('Install bubblewrap or set LACE_SOURCE_BWRAP for SDK source tools')
    workspace, ledger, inventory = stage_sources(request['source_root'], request['contract_path'])

    class SourceSession(UnixLocalSandboxSession):
        def _confined_exec_command(self, *, command_parts, workspace_root, env):
            # Unix-local does not confine Linux shell commands itself. Expose
            # runtime binaries plus these inputs, with no host home, /proc,
            # network, credentials, GT, AST or unrelated project files.
            command = [bwrap, '--die-with-parent', '--new-session', '--unshare-user',
                       '--unshare-pid', '--unshare-net', '--unshare-ipc', '--unshare-uts', '--clearenv',
                       '--setenv', 'PATH', '/usr/bin:/bin', '--setenv', 'HOME', str(workspace_root),
                       '--setenv', 'LANG', 'C.UTF-8']
            for path in ('/usr', '/bin', '/lib', '/lib64'):
                if Path(path).exists():
                    command += ['--ro-bind', path, path]
            command += ['--dir', '/etc', '--dir', '/proc', '--dir', '/dev',
                        '--dev-bind', '/dev/null', '/dev/null', '--tmpfs', '/tmp']
            if Path('/etc/ld.so.cache').exists():
                command += ['--ro-bind', '/etc/ld.so.cache', '/etc/ld.so.cache']
            command += ['--ro-bind', str(workspace_root), str(workspace_root),
                        '--ro-bind', str(ledger), str(workspace_root / 'contract_state.json'),
                        '--chdir', str(workspace_root), '--', *command_parts]
            return command

    client = UnixLocalSandboxClient(inherit_host_environment=False)
    original = await client.create(manifest=Manifest(root=str(workspace)))
    session = SourceSession(state=original.state)
    session._host_environment_allowlist = frozenset()
    await session.start()
    try:
        # Fail before calling a model if the host cannot enforce this boundary.
        probe = await session.exec('/bin/true')
        if probe.exit_code != 0:
            raise RuntimeError('SDK source sandbox confinement is unavailable: '+probe.stderr[:300])
    except Exception:
        await session.aclose()
        raise

    events = ledger.parent / 'sdk_source_tools.jsonl'
    tools = [ExecCommandTool(session=session), WriteStdinTool(session=session)]
    for tool in tools:
        invoke = tool.on_invoke_tool
        name = tool.name

        async def audit(context, arguments, *, invoke=invoke, name=name):
            def record(event):
                with events.open('a') as stream:
                    stream.write(json.dumps(event, ensure_ascii=False)+'\n')
            record({'event': 'tool_call', 'name': name, 'arguments': json.loads(arguments)})
            try:
                result = await invoke(context, arguments)
            except Exception as exc:
                record({'event': 'tool_error', 'name': name, 'error_type': type(exc).__name__, 'message': str(exc)})
                raise
            record({'event': 'tool_result', 'name': name, 'output': result})
            return result
        tool.on_invoke_tool = audit
    return tools, session, {'implementation': 'agents.sandbox.ExecCommandTool/WriteStdinTool',
                           'isolation': 'bubblewrap_read_only_source', 'files': len(inventory),
                           'workspace': str(workspace), 'tool_trace': str(events)}
