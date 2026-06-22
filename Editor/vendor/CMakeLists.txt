find_package(glfw3 CONFIG REQUIRED)

set(_imgui_dir "${CMAKE_SOURCE_DIR}/Editor/vendor/imgui")

set(_imgui_sources_common
    "${_imgui_dir}/imgui.cpp"
    "${_imgui_dir}/imgui_demo.cpp"
    "${_imgui_dir}/imgui_draw.cpp"
    "${_imgui_dir}/imgui_tables.cpp"
    "${_imgui_dir}/imgui_widgets.cpp"
    "${_imgui_dir}/backends/imgui_impl_glfw.cpp"
)

set(_imgui_sources_mac
    "${_imgui_dir}/backends/imgui_impl_metal.mm"
)

set(_imgui_sources_win
    "${_imgui_dir}/backends/imgui_impl_dx11.cpp"
)

if(APPLE)
    set(_imgui_sources_platform ${_imgui_sources_mac})
elseif(WIN32)
    set(_imgui_sources_platform ${_imgui_sources_win})
endif()

add_library(imgui_obj OBJECT ${_imgui_sources_common} ${_imgui_sources_platform})
target_include_directories(imgui_obj PUBLIC
    "${_imgui_dir}"
    "${_imgui_dir}/backends"
    "${CMAKE_SOURCE_DIR}/Engine/include"
    "${CMAKE_SOURCE_DIR}/Engine/vendor/metal-cpp/include"
    "${CMAKE_SOURCE_DIR}/Editor/include"
)
target_link_libraries(imgui_obj PRIVATE glfw)
target_compile_definitions(imgui_obj PRIVATE
    IMGUI_USER_CONFIG="imuserconfig.h"
)

# Mac ObjC backends: no ARC (the IMGUI_IMPL_METAL_CPP paths use manual
# retain/release, which is illegal under ARC). In non-ARC mode the backend's
# unconditional __bridge_retained/__bridge_transfer casts are inherent no-ops,
# so suppress that diagnostic too. Also suppress Apple docs warnings.
if(APPLE)
    set_source_files_properties(
        "${_imgui_dir}/backends/imgui_impl_metal.mm"
        PROPERTIES COMPILE_OPTIONS "-fno-objc-arc;-Wno-documentation;-Wno-arc-bridge-casts-disallowed-in-nonarc"
    )
endif()
