function(adhoc_bootstrap_llvm)
    if(APPLE)
        set(_clang_bin "${ADHOC_LLVM_ROOT}/bin/clang")
    elseif(WIN32)
        set(_clang_bin "${ADHOC_LLVM_ROOT}/bin/clang-cl.exe")
    else()
        message(FATAL_ERROR "Unsupported host platform — only Mac and Windows are supported.")
    endif()

    if(NOT EXISTS "${_clang_bin}")
        find_package(Python3 QUIET COMPONENTS Interpreter)
        if(NOT Python3_FOUND)
            message(FATAL_ERROR
                "Python3 is required to bootstrap LLVM. "
                "Install it and re-run CMake, or manually extract LLVM ${ADHOC_LLVM_VERSION} "
                "to ${ADHOC_LLVM_ROOT}.")
        endif()
        message(STATUS "Bootstrapping LLVM ${ADHOC_LLVM_VERSION} into ${ADHOC_LLVM_ROOT}...")
        execute_process(
            COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_SOURCE_DIR}/scripts/bootstrap_llvm.py"
                "--version" "${ADHOC_LLVM_VERSION}"
                "--url"    "${ADHOC_LLVM_URL}"
                "--sha256" "${ADHOC_LLVM_SHA256}"
                "--output-dir" "${ADHOC_LLVM_ROOT}"
            RESULT_VARIABLE _bootstrap_result
        )
        if(NOT _bootstrap_result EQUAL 0)
            message(FATAL_ERROR "LLVM bootstrap failed (exit code ${_bootstrap_result}).")
        endif()
    endif()

    if(APPLE)
        set(CMAKE_C_COMPILER   "${ADHOC_LLVM_ROOT}/bin/clang"   CACHE FILEPATH "C compiler"   FORCE)
        set(CMAKE_CXX_COMPILER "${ADHOC_LLVM_ROOT}/bin/clang++" CACHE FILEPATH "C++ compiler" FORCE)
    elseif(WIN32)
        set(CMAKE_C_COMPILER   "${ADHOC_LLVM_ROOT}/bin/clang-cl.exe" CACHE FILEPATH "C compiler"   FORCE)
        set(CMAKE_CXX_COMPILER "${ADHOC_LLVM_ROOT}/bin/clang-cl.exe" CACHE FILEPATH "C++ compiler" FORCE)
        set(CMAKE_LINKER       "${ADHOC_LLVM_ROOT}/bin/lld-link.exe" CACHE FILEPATH "Linker"       FORCE)
    endif()

    # Windows: hard-link extension-free aliases for IDE-facing tools so VS Code
    # settings can use the same path on all platforms (e.g. clangd, clang-format).
    # Hard links need no elevation on NTFS. Runs every configure so existing
    # installs get the alias even if bootstrap ran before this was added.
    if(WIN32)
        foreach(_tool IN ITEMS clangd clang-format)
            set(_src "${ADHOC_LLVM_ROOT}/bin/${_tool}.exe")
            set(_dst "${ADHOC_LLVM_ROOT}/bin/${_tool}")
            if(EXISTS "${_src}" AND NOT EXISTS "${_dst}")
                file(CREATE_LINK "${_src}" "${_dst}" RESULT _link_result)
                if(NOT _link_result EQUAL 0)
                    message(WARNING "Could not create hard link for ${_tool} (${_link_result})")
                endif()
            endif()
        endforeach()
        unset(_tool)
        unset(_src)
        unset(_dst)
        unset(_link_result)
    endif()

    message(STATUS "LLVM toolchain: ${ADHOC_LLVM_ROOT}")
endfunction()
