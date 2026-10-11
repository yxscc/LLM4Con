"""Revision isolation and source provenance tests; no model or network required."""
import importlib.util
import json
from pathlib import Path
import subprocess
import sys

import pytest

HARNESS = Path(__file__).resolve().parents[1] / 'harness'
spec = importlib.util.spec_from_file_location('repository_context', HARNESS / 'repository_context.py')
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)
RepositoryContext = module.RepositoryContext


@pytest.fixture
def context(tmp_path):
    repo = tmp_path / 'repo'
    repo.mkdir()
    def git(*args):
        return subprocess.check_output(['git', '-C', str(repo), *args], stderr=subprocess.DEVNULL).decode().strip()
    git('init')
    git('config', 'user.email', 'test@example.invalid')
    git('config', 'user.name', 'Offline test')
    (repo / 'include').mkdir()
    (repo / 'include/api.h').write_text('/* api requires caller protection */\nvoid api(void *p);\n')
    (repo / 'driver.c').write_text('void run(void *p) {\n  api(p);\n  api(p);\n}\n')
    (repo / 'link.h').symlink_to('/etc/passwd')
    git('add', '.')
    git('commit', '-m', 'source before fix')
    revision = git('rev-parse', 'HEAD')
    target = tmp_path / 'source'
    target.mkdir()
    (target / 'driver.c').write_bytes((repo / 'driver.c').read_bytes())
    (repo / 'driver.c').write_text('void run(void *p) { protect(); api(p); unprotect(); }\n')
    git('add', '.')
    git('commit', '-m', 'later fix must be invisible')
    return RepositoryContext(repo, revision), target


def test_revision_isolation_and_pagination(context):
    ctx, _ = context
    first = ctx.read('driver.c', 1, 2)
    second = ctx.read('driver.c', first['next_line'], 2)
    assert first['revision'] == ctx.revision
    assert 'protect' not in first['content'] + second['content']
    assert second['next_line'] is None
    assert first['source_id'] == ctx.read('driver.c', 1, 2)['source_id']
    assert first['source_id'] != second['source_id']
    hits, offset = [], 0
    while offset is not None:
        page = ctx.search('api', offset=offset, page_size=1)
        hits.extend(page['matches'])
        offset = page['next_offset']
    assert len(hits) == page['total_matches'] == 4
    assert len(ctx.search('api', path_prefix='include')['matches']) == 2


@pytest.mark.parametrize('path', ['../driver.c', '/etc/passwd', '.git/config', 'HEAD:driver.c', '*.c', 'link.h'])
def test_no_arbitrary_file_or_history_access(context, path):
    with pytest.raises(ValueError):
        context[0].read(path)


def test_matching_source_and_worker_without_credentials(context):
    ctx, target = context
    assert len(ctx.verify_source(target)['verified_files']) == 1
    assert ctx.verify_source(target)['bitcode_revision_verified'] is False
    request = {'action': 'repository', 'repository': {'path': ctx.repository, 'revision': ctx.revision},
               'operation': 'read', 'arguments': {'path': 'include/api.h'}}
    result = subprocess.run([sys.executable, str(HARNESS / 'sdk_worker.py')], input=json.dumps(request) + '\n',
                            text=True, capture_output=True, timeout=20)
    assert result.returncode == 0, result.stdout + result.stderr
    assert json.loads(result.stdout)['data']['revision'] == ctx.revision
    (target / 'driver.c').write_text('modified\n')
    with pytest.raises(ValueError, match='differs'):
        ctx.verify_source(target)
    with pytest.raises(ValueError, match='immutable'):
        RepositoryContext(ctx.repository, 'HEAD')
