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
add_executable(llm_detector ${STATIC_CORE}
    src/Query/SharedFieldKey.cpp src/Query/VulnerabilitySurfaceGenerator.cpp src/Query/ModuleFinding.cpp
    src/LLMUtil/SDKBridge.cpp src/LLMUtil/LLMClient.cpp src/LLMUtil/Conversation.cpp
    src/LLMUtil/SharedToolKit.cpp src/LLMUtil/ModuleContractAgent.cpp src/LLMUtil/ModuleRepository.cpp
    src/LLMUtil/RequirementChecker.cpp src/LLMUtil/ModuleAnalysis.cpp src/llm_main.cpp)
target_link_libraries(llm_detector PRIVATE cgraph pthread phasar::phasar)
include(CTest)
if(BUILD_TESTING)
    add_executable(module_requirements_test tests/module_requirements_test.cpp)
    add_test(NAME module_requirements COMMAND module_requirements_test)
    add_executable(requirement_checker_test tests/requirement_checker_test.cpp src/LLMUtil/RequirementChecker.cpp)
    add_test(NAME requirement_checker COMMAND requirement_checker_test)
endif()
