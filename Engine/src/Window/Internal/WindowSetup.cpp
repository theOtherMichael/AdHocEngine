#include <Engine/Window/Internal/WindowSetup.h>

#include "SetMainWindowIcon.h"
#include <Engine/Core/Console.h>
#include <Engine/Graphics/GraphicsContext.h>
#include <Engine/Window/WindowState.h>

#include <GLFW/glfw3.h>

namespace Engine::Window::Internal
{

static void OnGlfwError(int error, const char* description)
{
    Console::LogError("GLFW error {}: {}", error, description);
}

static void OnGlfwFramebufferResize(GLFWwindow*, int width, int height)
{
    const auto context = Graphics::GetContext();
    context->OnFramebufferResize(width, height);
}

static void OnGlfwWindowResize(GLFWwindow* window, int width, int height)
{
    // Update();
    //  TODO: How do we drive Update() from here? Should we even bother?
}

void CreateMainWindow()
{
    glfwSetErrorCallback(OnGlfwError);

    if (glfwInit() == GLFW_FALSE)
        throw std::runtime_error("GLFW could not be initialized. See GLFW error callback for details.");

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    // TODO: Get initial window settings from disk

    auto* const mainWindowHandle = glfwCreateWindow(1024, 768, "Window Title", nullptr, nullptr);
    if (mainWindowHandle == nullptr)
        throw std::runtime_error("Window creation failed. See GLFW error callback for details.");

    auto& windowState            = Engine::Window::WindowState::MutableInstance();
    windowState.mainWindowHandle = mainWindowHandle;

    glfwSetFramebufferSizeCallback(mainWindowHandle, OnGlfwFramebufferResize);
    glfwSetWindowSizeCallback(mainWindowHandle, OnGlfwWindowResize);

    SetMainWindowIcon();
}

void DestroyMainWindow()
{
    glfwTerminate();
}

} // namespace Engine::Window::Internal
