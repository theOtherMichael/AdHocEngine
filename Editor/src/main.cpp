#include "Core/BacktraceSymbolHandler.h"
#include <Editor/Core/EditorReloadFlags.h>
#include <Editor/Core/SymbolExportMacros.h>

#include <Engine/Core/Assertions.h>
#include <Engine/Core/Console.h>
#include <Engine/Core/PlatformData.h>

#include <fmt/format.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#include <GLFW/glfw3.h>
#pragma clang diagnostic pop

#include <filesystem>
#include <iostream>

namespace Console = Engine::Console;
using Console::LogLevel;

namespace fs = std::filesystem;

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

static void OnGlfwError(int error, const char* description)
{
    Console::LogError("GLFW error {}: {}", error, description);
}

extern "C" EDITOR_API unsigned char EditorMain(int argc, char* argv[])
{
    auto mainLogStream = Console::LogStream(LogLevel::Trace, OnEngineLogEvent);

    bool isDeveloperMode = false;
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--developer") == 0)
        {
            isDeveloperMode = true;
            break;
        }
    }

    Engine::InitializePlatformData();

    auto symbolHandler = Editor::BacktraceSymbolHandler(isDeveloperMode);

    // TODO: Reimplement the recompile watch thread

    glfwSetErrorCallback(OnGlfwError);

    if (!glfwInit())
    {
        return EditorReloadFlag_None;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* mainWindowPtr = glfwCreateWindow(1024, 768, "Window Title", nullptr, nullptr);

    if (!mainWindowPtr)
    {
        glfwTerminate();
        return EditorReloadFlag_None;
    }

    while (!glfwWindowShouldClose(mainWindowPtr))
    {
        glfwWaitEvents();

        // TODO: Check recompile watch thread
    }

    glfwTerminate();

    return EditorReloadFlag_None;
}
