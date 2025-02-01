#include <Editor/Core/Internal/EditorEntryPoint.h>

#include <Engine/Core/Assertions.h>
#include <Engine/Core/BacktraceSymbolHandler.h>
#include <Engine/Core/Console.h>
#include <Engine/Core/PlatformData.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#if ADHOC_WINDOWS
    #define GLFW_EXPOSE_NATIVE_WIN32
#elif ADHOC_MACOS
    #define GLFW_EXPOSE_NATIVE_COCOA
#endif
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#pragma clang diagnostic pop

#include <bgfx/bgfx.h>
#include <bgfx/platform.h>

namespace Console = Engine::Console;

namespace Editor
{

constexpr bgfx::ViewId kClearView = 0;

static void OnGlfwError(int error, const char* description)
{
    Console::LogError("GLFW error {}: {}", error, description);
}

static void OnGlfwResize(GLFWwindow* window, int width, int height)
{
    bgfx::reset((uint32_t)width, (uint32_t)height, BGFX_RESET_VSYNC);
    bgfx::setViewRect(kClearView, 0, 0, bgfx::BackbufferRatio::Equal);
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

    glfwSetWindowSizeCallback(mainWindowPtr, OnGlfwResize);

    // This prevents bgfx from starting its own render thread
    bgfx::renderFrame();

    bgfx::Init init;
#if ADHOC_MACOS
    init.platformData.nwh = glfwGetCocoaWindow(mainWindowPtr);
#elif ADHOC_WINDOWS
    init.platformData.nwh = glfwGetWin32Window(mainWindowPtr);
#endif

    int width, height;
    glfwGetWindowSize(mainWindowPtr, &width, &height);
    init.resolution.width  = static_cast<uint32_t>(width);
    init.resolution.height = static_cast<uint32_t>(height);
    init.resolution.reset  = BGFX_RESET_VSYNC;

    Assert_True(bgfx::init(init));

    bgfx::setViewClear(kClearView, BGFX_CLEAR_COLOR);
    bgfx::setViewRect(kClearView, 0, 0, bgfx::BackbufferRatio::Equal);

    while (!glfwWindowShouldClose(mainWindowPtr))
    {
        glfwWaitEvents();

        // TODO: Check recompile watch thread

        bgfx::frame();
    }

    bgfx::shutdown();
    glfwTerminate();

    return ReloadOption{.isReloadRequested = false};
}

} // namespace Editor
