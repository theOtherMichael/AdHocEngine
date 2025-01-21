#include <Editor/Core/EditorConfigurationMode.h>
#include <Editor/Core/EditorState.h>
#include <Editor/Core/Internal/EditorEntryPoint.h>
#include <Engine/Core/Assertions.h>
#include <Engine/Core/Console.h>

#include <fmt/format.h>

#include <string>

namespace Console = Engine::Console;
using Console::LogLevel;

static void OnEngineLogEvent(const LogLevel logLevel, const std::string& message)
{
    switch (logLevel)
    {
    case LogLevel::Fatal: [[fallthrough]];
    case LogLevel::Error: [[fallthrough]];
    case LogLevel::Warning: fmt::println(stderr, "[{}] {}", logLevel, message); break;
    case LogLevel::Log: [[fallthrough]];
    case LogLevel::Trace: fmt::println("[{}] {}", logLevel, message); break;
    default: Assert_NoEntry(); break;
    }
}

int main(int argc, char* argv[])
{
    // TODO: Reload if mi-malloc isn't injected (Mac)

    auto mainLogStream = Console::LogStream(LogLevel::Trace, OnEngineLogEvent);

    Console::Log("Starting Ad Hoc Launcher...");

    bool isDeveloperMode = false;

    // clang-format off
#if ADHOC_DEBUG
    const auto compiledConfigMode = Editor::ConfigurationMode::Debug;
#elif ADHOC_DEV
    const auto compiledConfigMode = Editor::ConfigurationMode::Dev;
#elif ADHOC_RELEASE
    const auto compiledConfigMode = Editor::ConfigurationMode::Release;
#else
    Assert_NoEntry();
#endif
    // clang-format on

    auto selectedConfigMode = compiledConfigMode;

    Console::Log("Command line arguments:");
    for (int i = 1; i < argc; i++)
    {
        Console::Log("  {}", argv[i]);

        if (strcmp(argv[i], "--developer") == 0)
            isDeveloperMode = true;
        else if (strcmp(argv[i], "--debug") == 0)
            selectedConfigMode = Editor::ConfigurationMode::Debug;
        else if (strcmp(argv[i], "--dev") == 0)
            selectedConfigMode = Editor::ConfigurationMode::Dev;
        else if (strcmp(argv[i], "--release") == 0)
            selectedConfigMode = Editor::ConfigurationMode::Release;
    }

    if (compiledConfigMode != selectedConfigMode)
    {
        if (isDeveloperMode)
        {
            Console::LogWarning("{} mode override was specified alongside --developer! The override will be ignored.",
                                selectedConfigMode);
        }
        else
        {
            // TODO: Relaunch in the override mode
        }
    }

    auto& editorState = Editor::GetMutableEditorState();

    Console::Log("Developer Mode: {}", isDeveloperMode);
    editorState.isDeveloperMode = isDeveloperMode;
    Console::Log("Selected config: {}", compiledConfigMode);
    editorState.currentConfigMode = compiledConfigMode;

    auto reloadFlags = Editor::EditorMain(argc, argv);

    // TODO: Handle reload scenarios:
    // - User mode switching
    // - Developer mode reloading
    // - Fatal Error handling

    return EXIT_SUCCESS;
}
