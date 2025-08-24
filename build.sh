jobs=8
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
ProjectPath="${SCRIPT_DIR}"
sysOS=$(uname -s)
arch=$(uname -m)
SVF_SANITIZER='thread'

if [[ $1 =~ ^[Dd]ebug$ ]]; then
    BUILD_TYPE='Debug'
else
    BUILD_TYPE='Release'
fi
BUILD_DIR="./${BUILD_TYPE}-build"
rm -rf "${BUILD_DIR}"
mkdir "${BUILD_DIR}"
# If you need shared libs, turn BUILD_SHARED_LIBS on     #    -DSVF_ENABLE_ASSERTIONS:BOOL=true            \
export CC=clang
export CXX=clang++
cmake -D CMAKE_BUILD_TYPE:STRING="${BUILD_TYPE}" \
    -D CMAKE_CXX_FLAGS_DEBUG="-g -fstandalone-debug" \
    -DBUILD_SHARED_LIBS=off                      \
    -S "${ProjectPath}" -B "${BUILD_DIR}"
cmake --build "${BUILD_DIR}" -j ${jobs}