"""Complete source-file footprints for offline scenario/session planning.

File granularity is deliberate in this first prototype: no guessed function
end, excerpt cap or representative context. Byte costs are measured from the
actual rendered source, NOT presented as model tokens or a context limit.
"""

from pathlib import Path

from lace3.evidence.source import SourceError, SourceTree


class SourceCatalog:
    def __init__(self, root):
        self.tree = SourceTree(root)
        self.text, self.byte_sizes, self.line_counts = {}, {}, {}
        self.unavailable = {}

    def add(self, path):
        rel = self.tree.resolve(path)
        if rel is None:
            self.unavailable[str(path)] = "source not resolved inside the source sandbox"
            return None
        if rel not in self.text:
            try:
                lines = self.tree.lines(rel)
            except (OSError, SourceError) as exc:
                self.unavailable[str(path)] = str(exc)
                return None
            text = f"===== Source: {rel} =====\n" + "".join(
                f"{n}: {line}\n" for n, line in enumerate(lines, 1)) + "\n"
            self.text[rel] = text
            self.byte_sizes[rel] = len(text.encode("utf-8"))
            self.line_counts[rel] = len(lines)
        return rel

    def cost(self, files):
        return sum(self.byte_sizes[f] for f in set(files))

    def render(self, files):
        return "".join(self.text[f] for f in sorted(set(files)))

    def whole_input(self):
        files = set()
        for rel in self.tree.files():
            if Path(rel).suffix in (".c", ".h", ".S", ".s", ".rs"):
                if self.add(rel):
                    files.add(rel)
        return files


class Footprints:
    def __init__(self, plan, src):
        self.catalog = SourceCatalog(src)
        self.binding_files, self.member_files, self.pool_files = {}, {}, {}
        self.view_files, self.scene_files = {}, {}
        self.missing_locations = set()
        self.member_missing, self.pool_missing, self.view_missing, self.scene_missing = {}, {}, {}, {}
        ix = plan.ix
        binding_missing = {}
        for aid, act in ix.acts.items():
            for st in act.steps:
                paths, missing = set(), set()
                loc = st.op.loc
                if loc:
                    paths.add(loc.file)
                    if loc.via:
                        path, _, line = loc.via.rpartition(":")
                        if line.isdigit():
                            paths.add(path)
                else:
                    missing.add(f"{st.op.module}/{st.op.function}: no source location")
                for _, site in st.frames:
                    if site:
                        path, _, line = site.rpartition(":")
                        if line.isdigit():
                            paths.add(path)
                files = set()
                for path in sorted(paths):
                    rel = self.catalog.add(path)
                    if rel:
                        files.add(rel)
                    else:
                        missing.add(path)
                self.binding_files[aid, st.sid] = frozenset(files)
                binding_missing[aid, st.sid] = frozenset(missing)
                self.missing_locations.update(missing)
        for mid in plan.owner:
            member = ix.members[mid]
            files, missing = set(), set()
            for sid in member.steps:
                files.update(self.binding_files[member.aid, sid])
                missing.update(binding_missing[member.aid, sid])
            self.member_files[mid] = frozenset(files)
            self.member_missing[mid] = frozenset(missing)
        for view in plan.views.values():
            pid = view.right_pool
            if pid not in self.pool_files:
                self.pool_files[pid] = frozenset().union(
                    *(self.member_files[m] for m in ix.pools[pid].members))
                self.pool_missing[pid] = frozenset().union(
                    *(self.member_missing[m] for m in ix.pools[pid].members))
            self.view_files[view.id] = self.pool_files[pid].union(
                *(self.member_files[m] for m in view.left_members))
            self.view_missing[view.id] = self.pool_missing[pid].union(
                *(self.member_missing[m] for m in view.left_members))
        for scene in plan.scenarios.values():
            if scene.views:
                self.scene_files[scene.id] = frozenset().union(*(self.view_files[v] for v in scene.views))
                self.scene_missing[scene.id] = frozenset().union(*(self.view_missing[v] for v in scene.views))
