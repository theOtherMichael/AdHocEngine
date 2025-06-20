set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)

if(PORT MATCHES "mimalloc")
    set(VCPKG_LIBRARY_LINKAGE dynamic)
endif()

if(PORT MATCHES "gtest")
    set(VCPKG_LIBRARY_LINKAGE dynamic)
endif()
