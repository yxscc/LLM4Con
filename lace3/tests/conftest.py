import subprocess
from pathlib import Path

import pytest

from lace3 import config

FIXTURES = Path(__file__).resolve().parent / "fixtures"


def line_of(snippet, nth=1, name="mini_ops.c"):
    """1-based line of the nth occurrence of `snippet` in a fixture."""
    hits = [i for i, line in enumerate((FIXTURES / name).read_text().splitlines(), 1)
            if snippet in line]
    return hits[nth - 1]


def compile_fixture(name, out_dir, opt="-O0"):
    src = FIXTURES / name
    ll = Path(out_dir) / f"{src.stem}{opt}.ll"
    subprocess.run(
        [config.clang(), "-S", "-emit-llvm", "-g", opt, "-Wno-everything",
         str(src), "-o", str(ll)],
        check=True, cwd=FIXTURES,
    )
    return ll


@pytest.fixture(scope="session")
def mini_ll(tmp_path_factory):
    return compile_fixture("mini_ops.c", tmp_path_factory.mktemp("ir"))


@pytest.fixture(scope="session", params=["-O0", "-O2"])
def conc_ll(request, tmp_path_factory):
    return compile_fixture("conc_patterns.c", tmp_path_factory.mktemp("ir"), request.param)
