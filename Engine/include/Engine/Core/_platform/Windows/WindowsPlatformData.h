#pragma once

#include <Engine/Core/SymbolExportMacros.h>

#include <windows.h>

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
ENGINE_API void InitializeProcessInfo();
ENGINE_API WindowsPlatformData& GetMutablePlatformData();
#endif

typedef WindowsPlatformData PlatformData;

} // namespace Engine
