# 设定CMake的最低版本要求
cmake_minimum_required(VERSION 3.23.1)

# 设置项目名称和版本
project(ConCodeQL VERSION 1.0)

# Configure top-level SVF variables (used by CMake for configuring installed SVF package)
set(SVF_INSTALL_ROOT ${CMAKE_INSTALL_PREFIX})
set(SVF_INSTALL_BIN_DIR ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR})
set(SVF_INSTALL_LIB_DIR ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR})
set(SVF_INSTALL_INCLUDE_DIR ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR})

# Set where extapi.bc is exported to in the installed CMake package
set(SVF_INSTALL_EXTAPI_DIR ${SVF_INSTALL_LIB_DIR})
set(SVF_INSTALL_EXTAPI_FILE ${SVF_INSTALL_EXTAPI_DIR}/extapi.bc)


configure_file(${PROJECT_SOURCE_DIR}/.config.in
               ${PROJECT_BINARY_DIR}/include/Util/config.h)

# Include the directory where the configuration header is exported for all targets
include_directories(${SVF_BINARY_DIR}/include)

# Install generated configuration header (see `configure_file()`) to top-level include dir of SVF
install(
  FILES ${SVF_BINARY_DIR}/include/Util/config.h
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/Util
)

# 指定C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
# add -std=gnu++14
set(CMAKE_CXX_EXTENSIONS ON)

# add_compile_options("-fno-rtti")

# Keep assertions enabled if requested
option(SVF_ENABLE_ASSERTIONS "Always enable assertions")
if(SVF_ENABLE_ASSERTIONS)
  add_compile_options("-UNDEBUG")
endif()

set(SVF_SANITIZE
    ""
    CACHE STRING "Create sanitizer build (address)")
if(SVF_SANITIZE STREQUAL "address")
  add_compile_options("-fno-omit-frame-pointer" "-fsanitize=address")
  add_link_options("-fsanitize=address")
  message(STATUS "Sanitizer build: ${SVF_SANITIZE}")
elseif(SVF_SANITIZE STREQUAL "thread")
  add_compile_options("-fsanitize=thread")
  add_link_options("-fsanitize=thread")
  message(STATUS "Sanitizer build: ${SVF_SANITIZE}")
elseif(NOT SVF_SANITIZE STREQUAL "")
  message(ERROR "Unknown sanitizer type: ${SVF_SANITIZE}")
endif()

# 寻找 LLVM 并添加定义
find_package(LLVM REQUIRED CONFIG)
message(STATUS "找到了 LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "使用的 LLVMConfig.cmake 位于: ${LLVM_DIR}")

# 添加 LLVM 包含目录
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

# 设置头文件路径
include_directories(/usr/include)

# 设置库文件路径
link_directories(/usr/lib/x86_64-linux-gnu)

# 添加头文件搜索路径
include_directories(/usr/local/include/graphviz)

# 查找 `libcgraph` 库文件
link_directories(/usr/local/lib)

link_directories(/usr/local/lib/graphviz)

# 关键修改：显式链接所有必需库
find_package(cpprestsdk REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Boost REQUIRED COMPONENTS system thread)  # 新增
find_package(phasar REQUIRED)

# Find Z3 and its include directory from the top-level include file
find_library(
  Z3_LIBRARIES
  REQUIRED
  NAMES z3
  HINTS ${Z3_DIR} ENV Z3_DIR
  PATH_SUFFIXES bin lib)
find_path(
  Z3_INCLUDES
  REQUIRED
  NAMES z3++.h
  HINTS ${Z3_DIR} ENV Z3_DIR
  PATH_SUFFIXES include z3)
message(STATUS "Z3 STATUS:
  Z3 library file:        ${Z3_LIBRARIES}
  Z3 include directory:   ${Z3_INCLUDES}")

# Add the Z3 include directory and link the Z3 library to all targets of SVF
set(CMAKE_INSTALL_RPATH ${Z3_INCLUDES})
link_libraries(${Z3_LIBRARIES})
include_directories(SYSTEM ${Z3_INCLUDES})

# 包含头文件目录
include_directories(${PROJECT_SOURCE_DIR}/include
                    ${PROJECT_SOURCE_DIR}/svf/include
                    ${PROJECT_SOURCE_DIR}/svf-llvm/include
                    ${PROJECT_BINARY_DIR}/include)
# 将SVF的include目录添加到包含路径
# add_subdirectory(svf)
# add_subdirectory(svf-llvm)

install(
DIRECTORY ${PROJECT_SOURCE_DIR}/svf/include/
          ${PROJECT_SOURCE_DIR}/svf-llvm/include/
COMPONENT devel
DESTINATION include/svf
FILES_MATCHING
PATTERN "**/*.h")

# 1. Get all source files from the src directory.
file(GLOB_RECURSE ALL_SRC "src/*.cpp" "src/Util/*.cpp")

# 2. Create a list of COMMON source files by filtering out ALL main entry points.
list(FILTER ALL_SRC EXCLUDE REGEX ".*main.cpp$")
list(FILTER ALL_SRC EXCLUDE REGEX ".*llm_main.cpp$")
list(FILTER ALL_SRC EXCLUDE REGEX ".*llm_comparison.cpp$")
set(COMMON_SRC ${ALL_SRC})

# 3. Define the source list for the 'ccql' executable.
# It includes the common files plus its own main.cpp.
set(CCQL_SRC ${COMMON_SRC} "${PROJECT_SOURCE_DIR}/src/main.cpp")

# 4. Define the source list for the 'llm_detector' executable.
# It includes the common files plus its own llm_main.cpp.
set(LLM_DETECTOR_SRC ${COMMON_SRC} "${PROJECT_SOURCE_DIR}/src/llm_main.cpp")

# Define the source list for the 'llm_conparison' executable.
set(LLM_COMPARISON_SRC ${COMMON_SRC} "${PROJECT_SOURCE_DIR}/src/llm_comparison.cpp")

# 5. Add the executables with their correct, separate source lists.
#add_executable(ccql ${CCQL_SRC})
add_executable(llm_detector ${LLM_DETECTOR_SRC})
add_executable(llm_comparison ${LLM_COMPARISON_SRC})



# link_directories(svf/lib)
# link_directories(svf-llvm/lib)
# SvfCore SvfLLVM
#target_link_libraries(ccql PUBLIC ${llvm_libs} ${Z3_LIBRARIES} curl cgraph 
#    cpprestsdk::cpprest
#    OpenSSL::SSL
#    OpenSSL::Crypto
#    Boost::system
#    Boost::thread
#    pthread
#    phasar::phasar)
target_link_libraries(llm_detector PUBLIC ${llvm_libs} ${Z3_LIBRARIES} curl cgraph 
    cpprestsdk::cpprest
    OpenSSL::SSL
    OpenSSL::Crypto
    Boost::system
    Boost::thread
    pthread
    phasar::phasar)

target_link_libraries(llm_comparison PUBLIC ${llvm_libs} ${Z3_LIBRARIES} curl cgraph 
    cpprestsdk::cpprest
    OpenSSL::SSL
    OpenSSL::Crypto
    Boost::system
    Boost::thread
    pthread
    phasar::phasar)






