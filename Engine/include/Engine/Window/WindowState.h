#pragma once

#include <Engine/Core/SymbolExportMacros.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#include <GLFW/glfw3.h>
#pragma clang diagnostic pop

namespace Engine::Window
{

struct ENGINE_API WindowState
{
public:
    GLFWwindow* mainWindowHandle;

    static const WindowState& GetInstance();

    WindowState() = default;
    ~WindowState();

    WindowState(const WindowState&)            = delete;
    WindowState& operator=(const WindowState&) = delete;
};

#if ADHOC_INTERNAL
ENGINE_API WindowState& GetMutableWindowState();
#endif

} // namespace Engine::Window
