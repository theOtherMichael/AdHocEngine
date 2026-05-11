#pragma once

#include <Engine/Common/Singleton.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#include <GLFW/glfw3.h>
#pragma clang diagnostic pop

namespace Engine::Window
{

class WindowState : public ImmutableSingleton<WindowState>
{
public:
    GLFWwindow* mainWindowHandle;
};

} // namespace Engine::Window
