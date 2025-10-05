#pragma once

#include <Engine/Core/Assertions.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#include <GLFW/glfw3.h>
#pragma clang diagnostic pop

namespace Engine::Window::Internal
{

class GlfwLifetime
{
public:
    GlfwLifetime() { AssertEval_Eq(glfwInit(), GLFW_TRUE); }

    GlfwLifetime(const GlfwLifetime&)            = delete;
    GlfwLifetime& operator=(const GlfwLifetime&) = delete;

    ~GlfwLifetime() { glfwTerminate(); }
};

} // namespace Engine::Window::Internal
