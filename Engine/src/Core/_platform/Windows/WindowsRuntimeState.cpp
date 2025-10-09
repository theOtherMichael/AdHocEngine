#include <Engine/Core/_platform/Windows/WindowsRuntimeState.h>

#include <Engine/Core/Assertions.h>
#include <Engine/Core/Console.h>
#include <Engine/Core/PlatformHelpers.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#pragma clang diagnostic pop

#include <windows.h>

ASSERT_PLATFORM_WINDOWS;

namespace Engine
{

const WindowsRuntimeState& WindowsRuntimeState::GetInstance()
{
    return GetMutableRuntimeState();
}

WindowsRuntimeState::~WindowsRuntimeState()
{
    if (processHandle == NULL)
        return;

    Console::Log("Closing application process handle...");

    if (!CloseHandle(processHandle))
    {
        Console::LogError("Could not close application process handle! {}", Windows::GetLastErrorMessage());
    }

    processHandle = NULL;
}

void InitializeProcessInfo()
{
    auto& data = GetMutableRuntimeState();

    Console::Log("Acquiring application process handle...");

    auto process = GetCurrentProcess();
    if (!DuplicateHandle(process, process, process, &data.processHandle, 0, FALSE, DUPLICATE_SAME_ACCESS))
    {
        Console::LogError("Could not obtain application process handle! {}", Windows::GetLastErrorMessage());
    }
}

WindowsRuntimeState& GetMutableRuntimeState()
{
    static WindowsRuntimeState instance;
    return instance;
}

} // namespace Engine
