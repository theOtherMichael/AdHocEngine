#include <Engine/Core/_platform/Windows/WindowsRuntimeInfo.h>

#include <Engine/Common/PlatformHelpers.h>
#include <Engine/Core/Assertions.h>
#include <Engine/Core/Console.h>

#include <windows.h>

static_assert(ADHOC_WINDOWS);

namespace Engine::Platform
{

PlatformRuntimeInfo::PlatformRuntimeInfo()
{
    const auto pseudoHandle = GetCurrentProcess();

    const auto acquireRealHandleResult =
        DuplicateHandle(pseudoHandle, pseudoHandle, pseudoHandle, &processHandle, 0, FALSE, DUPLICATE_SAME_ACCESS);

    Assert_Ne_Fmt(acquireRealHandleResult, 0, "Could not acquire process handle! {}", GetLastErrorMessage());
}

PlatformRuntimeInfo::~PlatformRuntimeInfo()
{
    Assert_NotNull(processHandle);

    Console::Log("Closing application pseudoHandle handle...");

    const auto closeHandleResult = CloseHandle(processHandle);

    Assert_Ne_Fmt(closeHandleResult, 0, "Could not close process handle! {}", GetLastErrorMessage());

    processHandle = NULL;
}

} // namespace Engine::Platform
