#include <Engine/Core/_platform/Windows/WindowsPlatformData.h>

#include <Engine/Core/Console.h>
#include <Engine/Core/PlatformHelpers.h>

#include <windows.h>

#if !ADHOC_WINDOWS
static_assert(false);
#endif

namespace Engine
{

WindowsPlatformData& GetMutablePlatformData()
{
    static WindowsPlatformData instance;
    return instance;
}

void InitializePlatformData()
{
    auto& data = GetMutablePlatformData();

    Console::Log("Acquiring application process handle...");

    auto process = GetCurrentProcess();
    if (!DuplicateHandle(process, process, process, &data.processHandle, 0, FALSE, DUPLICATE_SAME_ACCESS))
    {
        Console::LogError("Could not obtain application process handle! {}", Windows::GetLastErrorMessage());
    }
}

const WindowsPlatformData& WindowsPlatformData::GetInstance()
{
    return GetMutablePlatformData();
}

WindowsPlatformData::~WindowsPlatformData()
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

} // namespace Engine
