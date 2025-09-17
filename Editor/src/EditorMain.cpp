#include <Editor/Core/Internal/EditorMain.h>

#include <Engine/Core/Assertions.h>
#include <Engine/Core/BacktraceSymbolHandler.h>
#include <Engine/Core/Console.h>
#include <Engine/Core/Misc.h>
#include <Engine/Core/PlatformData.h>
#include <Engine/Core/PlatformHelpers.h>
#include <Engine/Graphics/D3D11GraphicsContext.h>
#include <Engine/Graphics/GraphicsContext.h>
#include <Engine/Window/Internal/GlfwLifetime.h>
#include <Engine/Window/WindowData.h>
#include <Engine/Window/WindowIcon.h>

#include <Views/ConsoleView.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#include <GLFW/glfw3.h>
#pragma clang diagnostic pop

#include <imgui.h>
#if ADHOC_WINDOWS
    #include <backends/imgui_impl_dx11.h>
#endif
#if ADHOC_MACOS
    #include <backends/imgui_impl_metal.h>
#endif
#include <backends/imgui_impl_glfw.h>

namespace Console = Engine::Console;
using Engine::Graphics::ApiMode;

namespace Editor
{

static void Update()
{
#if ADHOC_WINDOWS
    ImGui_ImplDX11_NewFrame();
#endif
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // TODO: Check recompile watch thread

    ImGui::ShowDemoWindow();
    Views::DrawConsoleView();

    ImGui::Render();

    // TODO: Handle other backends

#if ADHOC_WINDOWS
    auto dx11Context = Engine::Graphics::GetContextAs<Engine::Graphics::D3D11GraphicsContext>();

    const float clearColor[4] = {0.1f, 0.1f, 0.1f, 1.0f};
    dx11Context->pd3dDeviceContext->OMSetRenderTargets(1, &dx11Context->mainRenderTargetView, nullptr);
    dx11Context->pd3dDeviceContext->ClearRenderTargetView(dx11Context->mainRenderTargetView, clearColor);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
#endif

    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }

#if ADHOC_WINDOWS
    dx11Context->Present();
#endif
}

static void OnGlfwError(int error, const char* description)
{
    Console::LogError("GLFW error {}: {}", error, description);
}

static void OnGlfwFramebufferResize(GLFWwindow*, int width, int height)
{
    auto context = Engine::Graphics::GetMutableContext();
    context->OnFramebufferResize(width, height);
}

static void OnGlfwWindowSize(GLFWwindow* window, int width, int height)
{
    Update();
}

ReloadOption EditorMain(int argc, char* argv[])
{
    Engine::InitializeProcessInfo();
    auto symbolHandler = Engine::BacktraceSymbolHandler{};

    // TODO: Reimplement the recompile watch thread

    glfwSetErrorCallback(OnGlfwError);

    auto glfwLifetime = Engine::Window::Internal::GlfwLifetime{};

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* mainWindowHandle = glfwCreateWindow(1024, 768, "Window Title", nullptr, nullptr);
    Assert_NotNull(mainWindowHandle);
    Engine::Window::GetMutableWindowData().mainWindowHandle = mainWindowHandle;

    auto graphicsApiLifetime = Engine::Graphics::ApiLifetime{};

#if ADHOC_WINDOWS
    Engine::Window::Windows::SetWindowIconWithEmbeddedIconResource(mainWindowHandle, Windows::Resources::IDI_ICON1);
#endif

    // TODO: Get current mode from persistent data, then from heuristic
#if ADHOC_WINDOWS
    Engine::Graphics::SetApiMode(ApiMode::D3D11);
#elif ADHOC_MACOS
    Engine::Graphics::SetApiMode(ApiMode::Metal);
#else
    Assert_NoEntry();
#endif

    glfwSetFramebufferSizeCallback(mainWindowHandle, OnGlfwFramebufferResize);
    glfwSetWindowSizeCallback(mainWindowHandle, OnGlfwWindowSize);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    auto imguiIniFilePath      = Engine::GetExecutablePath().parent_path() / "imgui.ini";
    auto imguiIniFilePathAsStr = imguiIniFilePath.string();

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    io.IniFilename = imguiIniFilePathAsStr.c_str();

    {
// TODO: Handle other backends
#if ADHOC_MACOS
        // Metal goes here (for now)
#elif ADHOC_WINDOWS
        auto dx11Context = Engine::Graphics::GetContextAs<Engine::Graphics::D3D11GraphicsContext>();
        ImGui_ImplGlfw_InitForOther(mainWindowHandle, true);
        ImGui_ImplDX11_Init(dx11Context->pd3dDevice, dx11Context->pd3dDeviceContext);
#endif
    }

    auto logStream = Console::LogStream(Console::LogLevel::Log, Views::HandleConsoleViewLogs);

    while (!glfwWindowShouldClose(mainWindowHandle))
    {
        glfwPollEvents();
        Update();
    }

#if ADHOC_WINDOWS
    ImGui_ImplDX11_Shutdown();
#endif
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    return ReloadOption{.isReloadRequested = false};
}

} // namespace Editor
