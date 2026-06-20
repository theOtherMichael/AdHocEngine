find_package(GTest CONFIG REQUIRED)
include(GoogleTest)

set(ENGINETESTS_SOURCES
    src/Common/ThreadSafeViewsTests.cpp
    src/Core/AssertionTests.cpp
    src/Core/ConsoleTests.cpp
    src/asl/CastTests.cpp
    src/asl/FinallyTests.cpp
)

# Two executables: one linking against the shared Engine, one against EngineStatic.
foreach(_kind IN ITEMS "" "Static")
    set(_target "EngineTests${_kind}")
    add_executable(${_target} ${ENGINETESTS_SOURCES})

    target_include_directories(${_target} PRIVATE
        "${CMAKE_SOURCE_DIR}/Engine/include"
    )
    target_link_libraries(${_target} PRIVATE
        Engine${_kind}
        GTest::gtest_main
    )

    adhoc_apply_target_defaults(${_target})

    # EngineTests (shared) needs EngineD.dll and its transitive DLL dependencies
    # beside the exe so that gtest_discover_tests can run it immediately after linking.
    if(_kind STREQUAL "")
        add_custom_command(TARGET ${_target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_RUNTIME_DLLS:${_target}>
                $<TARGET_FILE_DIR:${_target}>
            COMMAND_EXPAND_LISTS
        )
    endif()

    gtest_discover_tests(${_target}
        TEST_SUFFIX "_${_kind}"
        DISCOVERY_TIMEOUT 30
    )
endforeach()
