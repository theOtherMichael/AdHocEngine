#include <Editor/Core/Internal/EditorMain.h>
#include <Engine/Core/Assertions.h>
#include <Engine/Core/Console.h>
#include <Engine/Core/Formatters/EnumFormatter.h>
#include <Engine/Core/PlatformAbstraction.h>
#include <Engine/Core/RuntimeInfo.h>

#include <fmt/format.h>

#include <string_view>
#include <variant>

#if PLATFORM_HEADER_EXISTS(MimallocInjectionImpl.h)
#include PLATFORM_HEADER(MimallocInjectionImpl.h)
#define PLATFORM_SUPPORTS_MIMALLOC_INJECTION 1
#else
#define PLATFORM_SUPPORTS_MIMALLOC_INJECTION 0
#endif

namespace Console = Engine::Console;
using Console::LogLevel;

using Editor::Internal::EditorExitResult;
using Editor::Internal::EditorReloadResult;

INJECT_ENUM_FORMATTER

enum class ConfigurationMode
{
    Debug,
    Dev,
    Release,
};

static void LogToStdOut(const LogLevel logLevel, const std::string_view message)
{
    switch (logLevel)
    {
    case LogLevel::Fatal:
        [[fallthrough]];
    case LogLevel::Error:
        [[fallthrough]];
    case LogLevel::Warning:
        fmt::println(stderr, "[{}] {}", logLevel, message);
        break;
    case LogLevel::Log:
        [[fallthrough]];
    case LogLevel::Trace:
        fmt::println("[{}] {}", logLevel, message);
        break;
    default:
        Assert_NoEntry();
        break;
    }
}

int main(int argc, char* argv[])
{
    auto mainLogger = Console::Logger(LogLevel::Trace, LogToStdOut);

    Console::Log("Starting Ad Hoc Launcher...");

#if PLATFORM_SUPPORTS_MIMALLOC_INJECTION
    Platform::EnsureMimallocInjected(argc, argv);
#endif

#if ADHOC_DEBUG
    const auto compiledConfigMode = ConfigurationMode::Debug;
#elif ADHOC_DEV
    const auto compiledConfigMode = ConfigurationMode::Dev;
#elif ADHOC_RELEASE
    const auto compiledConfigMode = ConfigurationMode::Release;
#else
    Assert_NoEntry();
#endif

    auto selectedConfigMode = compiledConfigMode;
    auto isDeveloperMode    = false;

    Console::Log("Command line arguments:");
    for (int i = 1; i < argc; i++)
    {
        Console::Log("  {}", argv[i]);

        if (strcmp(argv[i], "--debug") == 0)
            selectedConfigMode = ConfigurationMode::Debug;
        else if (strcmp(argv[i], "--dev") == 0)
            selectedConfigMode = ConfigurationMode::Dev;
        else if (strcmp(argv[i], "--release") == 0)
            selectedConfigMode = ConfigurationMode::Release;

        else if (strcmp(argv[i], "--developer") == 0)
            isDeveloperMode = true;
    }

    Console::Log("Configuration: {}", selectedConfigMode);
    Console::Log("Developer Mode: {}", isDeveloperMode);

    if (selectedConfigMode != compiledConfigMode)
    {
        if (isDeveloperMode)
        {
            Console::LogWarning("--{0} was specified alongside --developer. "
                                "Developer mode is not compatible with configuration overrides. "
                                "Launch will continue in {1} configuration",
                                selectedConfigMode,
                                compiledConfigMode);
        }
        else
        {
            // TODO: Relaunch in selectedConfigMode
        }
    }

    auto& mutableRuntimeInfo           = Engine::RuntimeInfo::MutableInstance();
    mutableRuntimeInfo.IsDeveloperMode = isDeveloperMode;

    auto editorMainResult = Editor::Internal::EditorMain(argc, argv);

    if (std::holds_alternative<EditorExitResult>(editorMainResult))
    {
        return std::get<EditorExitResult>(editorMainResult).ExitCode;
    }
    else if (std::holds_alternative<EditorReloadResult>(editorMainResult))
    {
        // TODO: Trigger reload
        Console::LogError("Reloading is not yet implemented");
        return EXIT_SUCCESS;
    }

    Assert_NoEntry();
    return EXIT_FAILURE;
}
