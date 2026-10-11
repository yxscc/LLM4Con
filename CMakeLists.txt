cmake_minimum_required(VERSION 3.16)
project(LACE VERSION 2.1 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_definitions(PROJECT_PATH="${PROJECT_SOURCE_DIR}")
find_package(LLVM REQUIRED CONFIG)
find_package(phasar REQUIRED)
include_directories(${LLVM_INCLUDE_DIRS} ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include /usr/local/include/graphviz)
add_definitions(${LLVM_DEFINITIONS})
link_directories(/usr/local/lib /usr/local/lib/graphviz)
# Keep the detector source set explicit. Adding an experimental agent must not
# silently add another detection pipeline to this executable.
file(GLOB_RECURSE STATIC_CORE CONFIGURE_DEPENDS
    src/CPG/*.cpp src/CCPG/*.cpp src/PhasarUtil/*.cpp src/Util/*.cpp)
list(FILTER STATIC_CORE EXCLUDE REGEX "/\\._")
add_library(lace_static_core OBJECT ${STATIC_CORE} src/Query/SharedFieldKey.cpp)
target_link_libraries(lace_static_core PUBLIC cgraph pthread phasar::phasar)
add_executable(llm_detector
    src/Query/ModuleFinding.cpp
    src/LLMUtil/SDKBridge.cpp src/LLMUtil/LLMClient.cpp src/LLMUtil/Conversation.cpp
    src/LLMUtil/SharedToolKit.cpp src/LLMUtil/ModuleContractAgent.cpp src/LLMUtil/ModuleRepository.cpp
    src/LLMUtil/RequirementChecker.cpp src/LLMUtil/ModuleAnalysis.cpp src/llm_main.cpp)
target_link_libraries(llm_detector PRIVATE lace_static_core)
include(CTest)
if(BUILD_TESTING)
    add_executable(module_graph_test tests/module_graph_test.cpp)
    target_link_libraries(module_graph_test PRIVATE lace_static_core)
    add_test(NAME complete_module_graph COMMAND module_graph_test)
    add_executable(requirement_checker_test tests/requirement_checker_test.cpp src/LLMUtil/RequirementChecker.cpp)
    add_test(NAME requirement_checker COMMAND requirement_checker_test)
endif()
