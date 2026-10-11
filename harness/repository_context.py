"""Read-only, revision-pinned source access. No checkout, history or network API.

Pagination bounds individual responses, never the searchable repository. A search
hit is navigation evidence; only an explicit read produces a citable source ID.
"""
import hashlib
from pathlib import Path, PurePosixPath
import re
import subprocess


SOURCE_SUFFIXES = {'.c', '.h', '.cc', '.cpp', '.cxx', '.hh', '.hpp', '.S', '.s', '.rs'}
READ_SUFFIXES = SOURCE_SUFFIXES | {'.rst', '.txt', '.md'}


class RepositoryContext:
    def __init__(self, repository, revision):
        self.repository = str(Path(repository).resolve())
        if not re.fullmatch(r'[0-9a-fA-F]{40}', revision):
            raise ValueError('Repository context requires a full, immutable commit SHA')
        self.revision = self.git('rev-parse', '--verify', revision + '^{commit}').decode().strip()
        if self.revision.lower() != revision.lower():
            raise ValueError('Revision did not resolve to the requested commit')

    def git(self, *args, allowed=(0,)):
        proc = subprocess.run(['git', '--no-pager', '-C', self.repository, *args],
                              capture_output=True, timeout=90)
        if proc.returncode not in allowed:
            raise ValueError('Pinned repository query failed: ' + proc.stderr.decode(errors='replace')[:500])
        return proc.stdout

    @staticmethod
    def path(raw, allow_empty=False):
        if allow_empty and not raw:
            return ''
        path = PurePosixPath(raw)
        if not raw or path.is_absolute() or '..' in path.parts or '.git' in path.parts or '\\' in raw or '\x00' in raw:
            raise ValueError('Use a repository-relative source path without traversal')
        # Do not accept Git pathspec syntax from the agent.
        if any(c in raw for c in '*?[]:') or raw.startswith('-'):
            raise ValueError('Pathspec expressions are not source paths')
        return path.as_posix()

    def blob(self, raw):
        path = self.path(raw)
        if PurePosixPath(path).suffix not in READ_SUFFIXES and PurePosixPath(path).name not in {'Makefile', 'Kbuild', 'Kconfig'}:
            raise ValueError('Only source, build descriptions and source documentation are readable')
        record = self.git('ls-tree', '-z', self.revision, '--', path).rstrip(b'\0')
        if not record or b'\0' in record:
            raise ValueError('No unique file at the pinned revision')
        meta, actual = record.split(b'\t', 1)
        mode, kind, oid = meta.split()
        if mode not in {b'100644', b'100755'} or kind != b'blob' or actual.decode() != path:
            raise ValueError('Symlinks, submodules and directories are not readable source files')
        content = self.git('cat-file', 'blob', oid.decode())
        if b'\0' in content:
            raise ValueError('Binary content is not source text')
        return path, content

    def read(self, path, start_line=1, line_count=200):
        if type(start_line) is not int or start_line < 1 or type(line_count) is not int or not 1 <= line_count <= 600:
            raise ValueError('start_line >= 1 and 1 <= line_count <= 600; use the returned next_line to continue')
        path, blob = self.blob(path)
        lines = blob.decode('utf-8', errors='replace').splitlines()
        if start_line > max(1, len(lines)):
            raise ValueError('start_line is beyond the end of the file')
        selected = lines[start_line - 1:start_line - 1 + line_count]
        # Bound huge generated lines too, without returning partial source lines.
        used, kept = 0, []
        for line in selected:
            if used + len(line.encode()) > 192 * 1024:
                break
            kept.append(line); used += len(line.encode()) + 1
        if selected and not kept:
            raise ValueError('A single source line exceeds the response byte budget')
        end = start_line + len(kept) - 1
        excerpt = '\n'.join(kept)
        key = f'{self.revision}\0{path}\0{start_line}\0{end}\0{excerpt}'.encode()
        return {'revision': self.revision, 'path': path, 'first_line': start_line,
                'last_line': end, 'total_lines': len(lines),
                'next_line': end + 1 if end < len(lines) else None,
                'source_id': 'src:' + hashlib.sha256(key).hexdigest()[:24],
                'blob_sha256': hashlib.sha256(blob).hexdigest(),
                'content': '\n'.join(f'{start_line + i}: {line}' for i, line in enumerate(kept)),
                'static_analysis': False}

    def search(self, symbol, path_prefix='', offset=0, page_size=80):
        if not re.fullmatch(r'[A-Za-z_][A-Za-z_0-9]*', symbol):
            raise ValueError('Search one C/C++ identifier at a time')
        if type(offset) is not int or offset < 0 or type(page_size) is not int or not 1 <= page_size <= 200:
            raise ValueError('offset >= 0 and 1 <= page_size <= 200')
        prefix = self.path(path_prefix, allow_empty=True)
        # -F/-w prevents regex/option injection. Revision and source globs are
        # separate arguments; no shell is involved. All matches remain pageable.
        raw = self.git('grep', '-n', '-I', '-F', '-w', '-e', symbol, self.revision,
                       '--', *['*' + ext for ext in sorted(READ_SUFFIXES)], allowed=(0, 1))
        hits = []
        for line in raw.decode(errors='replace').splitlines():
            parts = line.split(':', 3)
            if len(parts) != 4 or not parts[2].isdigit():
                continue
            _, path, number, code = parts
            if prefix and path != prefix and not path.startswith(prefix.rstrip('/') + '/'):
                continue
            hits.append({'path': path, 'line': int(number), 'code': code})
        page = hits[offset:offset + page_size]
        return {'revision': self.revision, 'symbol': symbol, 'matches': page,
                'total_matches': len(hits), 'next_offset': offset + len(page) if offset + len(page) < len(hits) else None,
                'meaning': 'Textual symbol occurrences, not a proven call graph. Read surrounding source before drawing conclusions.'}

    def verify_source(self, source):
        root = Path(source).resolve()
        files = [root] if root.is_file() else sorted(root.rglob('*'))
        verified = []
        for file in files:
            if file.is_symlink():
                raise ValueError('Target source symlinks require explicit materialization')
            if not file.is_file() or file.suffix not in SOURCE_SUFFIXES:
                continue
            relative = file.name if root.is_file() else file.relative_to(root).as_posix()
            repository_path = relative
            try:
                _, content = self.blob(relative)
            except ValueError:
                # Some input snapshots retain a flat build copy alongside the
                # canonical repository file. Accept only a unique byte-identical
                # canonical copy that itself verifies at the pinned revision.
                aliases = []
                if root.is_dir() and len(Path(relative).parts) == 1:
                    for candidate in files:
                        if candidate.is_file() and not candidate.is_symlink() and candidate != file and candidate.name == file.name and candidate.read_bytes() == file.read_bytes():
                            canonical = candidate.relative_to(root).as_posix()
                            if len(Path(canonical).parts) < 2:
                                continue
                            try:
                                _, blob = self.blob(canonical)
                            except ValueError:
                                continue
                            if blob == file.read_bytes():
                                aliases.append((canonical, blob))
                if len(aliases) != 1:
                    raise
                repository_path, content = aliases[0]
            if content != file.read_bytes():
                raise ValueError('Target source differs from pinned revision: ' + relative)
            row = {'path': relative, 'sha256': hashlib.sha256(content).hexdigest()}
            if repository_path != relative:
                row.update(repository_path=repository_path, alias_of=repository_path)
            verified.append(row)
        if not verified:
            raise ValueError('No target source files to verify')
        return {'revision': self.revision, 'verified_files': verified,
                'bitcode_revision_verified': False,
                'note': 'Source bytes verified. Bitcode provenance still depends on the existing build artifact.'}


def handle(request):
    config = request['repository']
    context = RepositoryContext(config['path'], config['revision'])
    operation = request['operation']
    args = request.get('arguments', {})
    if operation == 'read':
        return context.read(**args)
    if operation == 'search':
        return context.search(**args)
    if operation == 'verify_source':
        return context.verify_source(**args)
    raise ValueError('Unknown repository operation')
