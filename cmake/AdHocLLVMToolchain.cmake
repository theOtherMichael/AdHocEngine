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

    message(STATUS "LLVM toolchain: ${ADHOC_LLVM_ROOT}")
endfunction()
