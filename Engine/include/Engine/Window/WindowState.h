#pragma once

#include <Engine/Common/Singleton.h>
#include <Engine/Core/SymbolExportMacros.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#include <GLFW/glfw3.h>
#pragma clang diagnostic pop

namespace Engine::Window
{

class ENGINE_API WindowState : public ImmutableSingleton<WindowState>
{
public:
    GLFWwindow* mainWindowHandle;
};

} // namespace Engine::Window
