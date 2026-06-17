# Ensures the in-tree vcpkg submodule is checked out and bootstrapped before
# CMAKE_TOOLCHAIN_FILE is set.  Re-bootstraps automatically when the submodule
# commit changes.

function(adhoc_bootstrap_vcpkg)
    set(_vcpkg_root "${CMAKE_SOURCE_DIR}/tools/vcpkg")

    if(WIN32)
        set(_bootstrap_script "${_vcpkg_root}/bootstrap-vcpkg.bat")
        set(_vcpkg_exe "${_vcpkg_root}/vcpkg.exe")
    else()
        set(_bootstrap_script "${_vcpkg_root}/bootstrap-vcpkg.sh")
        set(_vcpkg_exe "${_vcpkg_root}/vcpkg")
    endif()

    # Step 1: ensure the submodule is checked out.
    if(NOT EXISTS "${_bootstrap_script}")
        find_package(Git QUIET)
        if(NOT Git_FOUND)
            message(FATAL_ERROR
                "Git is required to initialize the vcpkg submodule. "
                "Install Git and re-run CMake, or manually clone vcpkg into ${_vcpkg_root}.")
        endif()
        message(STATUS "Initializing vcpkg submodule...")
        execute_process(
            COMMAND "${GIT_EXECUTABLE}" submodule update --init --recursive -- tools/vcpkg
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
            RESULT_VARIABLE _submodule_result
        )
        if(NOT _submodule_result EQUAL 0)
            message(FATAL_ERROR "Failed to initialize vcpkg submodule (exit code ${_submodule_result}).")
        endif()
    endif()

    # Step 2: figure out the pinned commit of the submodule, if available.
    set(_pinned_commit "")
    find_package(Git QUIET)
    if(Git_FOUND)
        execute_process(
            COMMAND "${GIT_EXECUTABLE}" rev-parse "HEAD:tools/vcpkg"
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
            OUTPUT_VARIABLE _pinned_commit
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
            RESULT_VARIABLE _rev_parse_result
        )
        if(NOT _rev_parse_result EQUAL 0)
            set(_pinned_commit "")
        endif()
    endif()

    set(_stamp_file "${CMAKE_SOURCE_DIR}/vcpkg_bootstrapped_commit.txt")

    # Step 3: decide whether bootstrap is needed.
    set(_needs_bootstrap FALSE)
    if(NOT EXISTS "${_vcpkg_exe}")
        set(_needs_bootstrap TRUE)
    elseif(_pinned_commit)
        if(NOT EXISTS "${_stamp_file}")
            set(_needs_bootstrap TRUE)
        else()
            file(READ "${_stamp_file}" _stamped_commit)
            string(STRIP "${_stamped_commit}" _stamped_commit)
            if(NOT _stamped_commit STREQUAL _pinned_commit)
                message(STATUS "vcpkg submodule commit changed — re-bootstrapping.")
                set(_needs_bootstrap TRUE)
            endif()
        endif()
    endif()

    # Step 4: run the bootstrap script.
    if(_needs_bootstrap)
        message(STATUS "Bootstrapping vcpkg in ${_vcpkg_root}...")
        if(WIN32)
            execute_process(
                COMMAND cmd /c "${_bootstrap_script}" -disableMetrics
                WORKING_DIRECTORY "${_vcpkg_root}"
                RESULT_VARIABLE _bootstrap_result
            )
        else()
            execute_process(
                COMMAND sh "${_bootstrap_script}" -disableMetrics
                WORKING_DIRECTORY "${_vcpkg_root}"
                RESULT_VARIABLE _bootstrap_result
            )
        endif()
        if(NOT _bootstrap_result EQUAL 0)
            message(FATAL_ERROR "vcpkg bootstrap failed (exit code ${_bootstrap_result}).")
        endif()
        if(_pinned_commit)
            file(WRITE "${_stamp_file}" "${_pinned_commit}\n")
        endif()
    endif()

    message(STATUS "vcpkg: ${_vcpkg_root}")
endfunction()
