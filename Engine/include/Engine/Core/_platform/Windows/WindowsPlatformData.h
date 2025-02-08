#pragma once

#include <Engine/Core/SymbolExportMacros.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#include <GLFW/glfw3.h>
#pragma clang diagnostic pop

#include <windows.h>

#if !ADHOC_WINDOWS
static_assert(false);
#endif

namespace Engine
{

struct ENGINE_API WindowsPlatformData
{
public:
    HANDLE processHandle = NULL;
    HWND windowHandle    = NULL;

    static const WindowsPlatformData& GetInstance();

    WindowsPlatformData() = default;
    ~WindowsPlatformData();

    WindowsPlatformData(const WindowsPlatformData&)            = delete;
    WindowsPlatformData& operator=(const WindowsPlatformData&) = delete;
};

#if ADHOC_INTERNAL
ENGINE_API WindowsPlatformData& GetMutablePlatformData();
ENGINE_API void InitializeProcessInfo();
ENGINE_API void SetNativeWindowHandle(GLFWwindow* window);
#endif

typedef WindowsPlatformData PlatformData;

} // namespace Engine
