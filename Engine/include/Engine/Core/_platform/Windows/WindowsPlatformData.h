#pragma once

#include <Engine/Core/SymbolExportMacros.h>

#include <windows.h>

#if !ADHOC_WINDOWS
static_assert(false);
#endif

namespace Engine
{

struct ENGINE_API WindowsPlatformData
{
public:
    HANDLE processHandle = NULL;

    static const WindowsPlatformData& GetInstance();

    WindowsPlatformData() = default;
    ~WindowsPlatformData();

    WindowsPlatformData(const WindowsPlatformData&)            = delete;
    WindowsPlatformData& operator=(const WindowsPlatformData&) = delete;
};

#if ADHOC_INTERNAL
ENGINE_API WindowsPlatformData& GetMutablePlatformData();
ENGINE_API void InitializePlatformData();
#endif

typedef WindowsPlatformData PlatformData;

} // namespace Engine
