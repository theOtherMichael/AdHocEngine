#include <Editor/Core/Internal/EditorEntryPoint.h>

#include <Engine/Core/BacktraceSymbolHandler.h>
#include <Engine/Core/Console.h>
#include <Engine/Core/PlatformData.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#include <GLFW/glfw3.h>
#pragma clang diagnostic pop

namespace Console = Engine::Console;

namespace Editor
{

static void OnGlfwError(int error, const char* description)
{
    Console::LogError("GLFW error {}: {}", error, description);
}

ReloadOption EditorMain(int argc, char* argv[])
{
    Engine::InitializePlatformData();
    auto symbolHandler = Engine::BacktraceSymbolHandler{};

    // TODO: Reimplement the recompile watch thread

    glfwSetErrorCallback(OnGlfwError);

    if (!glfwInit())
        Console::LogFatal("glfwInit() failure!");

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* mainWindowPtr = glfwCreateWindow(1024, 768, "Window Title", nullptr, nullptr);

    if (!mainWindowPtr)
    {
        glfwTerminate();
        Console::LogFatal("glfwCreateWindow() failure!");
    }

    while (!glfwWindowShouldClose(mainWindowPtr))
    {
        glfwWaitEvents();

        // TODO: Actual editor stuff

        // TODO: Check recompile watch thread
    }

    glfwTerminate();

    return ReloadOption{.isReloadRequested = false};
}

} // namespace Editor
