#pragma once

#include <Enterprise/Core/SymbolExportMacros.h>

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif
#include "Windows.h"

#ifndef ENTERPRISE_WINDOWS
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

#ifdef ENTERPRISE_INTERNAL
ENGINE_API WindowsPlatformData& GetMutablePlatformData();
ENGINE_API void InitializePlatformData();
#endif

typedef WindowsPlatformData PlatformData;

} // namespace Engine
