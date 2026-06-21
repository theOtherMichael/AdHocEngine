#include <Editor/Core/Internal/EditorMain.h>

#include <Editor/Core/Internal/ImGuiSetup.h>

#include <asl/finally.h>
#include <Engine/Core/Console.h>
#include <Engine/Graphics/ApiMode.h>
#include <Engine/Graphics/GraphicsContext.h>
#include <Engine/Window/Internal/WindowSetup.h>
#include <Engine/Window/WindowState.h>

#include <Views/ConsoleView.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#include <GLFW/glfw3.h>
#pragma clang diagnostic pop

#include <imgui.h>

namespace Console = Engine::Console;

namespace Editor::Internal
{

static void Update()
{
    Internal::StartImGuiFrame();
    const auto endImGuiFrameAction = asl::finally(Internal::EndImGuiFrame);

    // TODO: Service the source-mode reload IPC endpoint (prompt + serialize/
    // relaunch on a `reload` signal from the run task). See docs/SourceMode.md.
    // TODO: Editor state machine?

    ImGui::ShowDemoWindow();
    Views::DrawConsoleView();
}

EditorMainResult EditorMain([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
    try
    {
        // TODO: Acquire the single-instance lock and bind the source-mode
        // reload IPC endpoint here. See docs/SourceMode.md.

        Engine::Window::Internal::CreateMainWindow();
        const auto shutDownWindowAction = asl::finally(Engine::Window::Internal::DestroyMainWindow);

        const auto initialApiMode = Engine::Graphics::GetDefaultApiMode();
        Engine::Graphics::SetApiMode(initialApiMode);

        const auto shutdownGraphicsContextAction = asl::finally(Engine::Graphics::Internal::DestroyContext);

        InitializeImGui();
        const auto shutdownImGuiAction = asl::finally(ShutdownImGui);

        // TODO: Should this logger really live here?
        auto consoleViewLogger = Console::Logger(Console::LogLevel::Log, Views::HandleConsoleViewLogs);

        const auto& windowState  = Engine::Window::WindowState::MutableInstance();
        auto* const windowHandle = windowState.mainWindowHandle;

        // TODO: Should I roll the following into an editor-wide state machine?

        while (!glfwWindowShouldClose(windowHandle))
        {
            glfwPollEvents();
            Update();
        }
    }
    catch (const std::exception& e)
    {
        Console::LogFatal("Unhandled exception in EditorMain(): {}", e.what());
        return EditorExitResult{.ExitCode = EXIT_FAILURE};
    }
    catch (...)
    {
        Console::LogFatal("Unknown exception in EditorMain()");
        return EditorExitResult{.ExitCode = EXIT_FAILURE};
    }

    return EditorExitResult{.ExitCode = EXIT_SUCCESS};
}

} // namespace Editor::Internal
