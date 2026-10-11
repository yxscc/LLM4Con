#!/usr/bin/env bash
set -euo pipefail
project_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
build_type=${1:-Release}
cmake -S "$project_dir" -B "$project_dir/$build_type-build" -DCMAKE_BUILD_TYPE="$build_type"
cmake --build "$project_dir/$build_type-build" --clean-first -j "${LACE_BUILD_JOBS:-4}"
ctest --test-dir "$project_dir/$build_type-build" --output-on-failure
