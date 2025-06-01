#pragma once

#include <Engine/Core/SymbolExportMacros.h>

namespace Engine
{

struct ENGINE_API MacPlatformData
{
public:
    static const MacPlatformData& GetInstance();

    MacPlatformData() = default;
    ~MacPlatformData();

    MacPlatformData(const MacPlatformData&)            = delete;
    MacPlatformData& operator=(const MacPlatformData&) = delete;
};

#if ADHOC_INTERNAL
ENGINE_API void InitializeProcessInfo();
ENGINE_API MacPlatformData& GetMutablePlatformData();
#endif

typedef MacPlatformData PlatformData;

} // namespace Engine
