set(ADHOC_LLVM_VERSION "20.1.4")

# LLVM 20.x uses the LLVM-<ver>-macOS-ARM64.tar.xz naming convention on Mac.
# On Windows, the .tar.xz for x86_64-pc-windows-msvc is extracted directly.
# Fill in SHA256 after downloading to enable verification; leave empty to skip (prints a warning).
if(APPLE)
    set(ADHOC_LLVM_URL
        "https://github.com/llvm/llvm-project/releases/download/llvmorg-${ADHOC_LLVM_VERSION}/LLVM-${ADHOC_LLVM_VERSION}-macOS-ARM64.tar.xz")
    set(ADHOC_LLVM_SHA256 "debb43b7b364c5cf864260d84ba1b201d49b6460fe84b76eaa65688dfadf19d2")
elseif(WIN32)
    set(ADHOC_LLVM_URL
        "https://github.com/llvm/llvm-project/releases/download/llvmorg-${ADHOC_LLVM_VERSION}/clang+llvm-${ADHOC_LLVM_VERSION}-x86_64-pc-windows-msvc.tar.xz")
    set(ADHOC_LLVM_SHA256 "")
endif()

set(ADHOC_LLVM_ROOT "${CMAKE_SOURCE_DIR}/tools/llvm/${ADHOC_LLVM_VERSION}")
