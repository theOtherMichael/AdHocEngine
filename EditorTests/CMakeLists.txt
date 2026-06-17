find_package(GTest CONFIG REQUIRED)
include(GoogleTest)

# EditorTests only exists for the shared (Editor DLL) build.
# Editor is never built as a static library.
set(EDITORTESTS_SOURCES_COMMON
    # Add test files here as they are created.
    # Mirror the Editor source tree: tests for Editor/include/A/B.h → EditorTests/src/A/BTests.cpp
)

set(EDITORTESTS_SOURCES_WINDOWS
    src/_platform/Windows/MimallocNewDeleteOverride.cpp
)

if(WIN32)
    set(EDITORTESTS_SOURCES_PLATFORM ${EDITORTESTS_SOURCES_WINDOWS})
else()
    set(EDITORTESTS_SOURCES_PLATFORM)
endif()

list(LENGTH EDITORTESTS_SOURCES_COMMON _num_common_sources)
if(_num_common_sources EQUAL 0 AND NOT WIN32)
    # No test sources yet on this platform — skip creating an empty test target.
    return()
endif()

add_executable(EditorTests
    ${EDITORTESTS_SOURCES_COMMON}
    ${EDITORTESTS_SOURCES_PLATFORM}
)
target_include_directories(EditorTests PRIVATE
    "${CMAKE_SOURCE_DIR}/Engine/include"
    "${CMAKE_SOURCE_DIR}/Editor/include"
    "${CMAKE_BINARY_DIR}/generated"
)
target_link_libraries(EditorTests PRIVATE
    Editor
    Engine
    GTest::gtest_main
)

adhoc_apply_target_defaults(EditorTests)

gtest_discover_tests(EditorTests
    DISCOVERY_TIMEOUT 30
)
