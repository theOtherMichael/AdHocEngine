#include <Enterprise/Core/_platform/Windows/WindowsPlatformData.h>

#include <Enterprise/Core/PlatformHelpers.h>

#include <fmt/format.h>

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif
#include "Windows.h"

#ifndef ENTERPRISE_WINDOWS
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

    fmt::print("Acquiring application process handle...\n");

    auto process = GetCurrentProcess();
    if (!DuplicateHandle(process, process, process, &data.processHandle, 0, FALSE, DUPLICATE_SAME_ACCESS))
    {
        fmt::print(stderr, "Could not obtain application process handle! {}\n", Windows::GetLastErrorMessage());
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

    fmt::print("Closing application process handle...\n");

    if (!CloseHandle(processHandle))
    {
        fmt::print(stderr, "Could not close application process handle! {}\n", Windows::GetLastErrorMessage());
    }

    processHandle = NULL;
}

} // namespace Engine
