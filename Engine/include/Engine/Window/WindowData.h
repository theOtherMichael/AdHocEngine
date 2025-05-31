#pragma once

#include <Engine/Core/SymbolExportMacros.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#include <GLFW/glfw3.h>
#pragma clang diagnostic pop

namespace Engine::Window
{

struct ENGINE_API WindowData
{
public:
    GLFWwindow* mainWindowHandle;

    static const WindowData& GetInstance();

    WindowData() = default;
    ~WindowData();

    WindowData(const WindowData&)            = delete;
    WindowData& operator=(const WindowData&) = delete;
};

#if ADHOC_INTERNAL
ENGINE_API WindowData& GetMutableWindowData();
#endif

} // namespace Engine::Window
