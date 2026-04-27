#pragma once

#include <Engine/Core/PlatformAbstraction.h>

#if PLATFORM_HEADER_EXISTS(RuntimeInfo.h)
#include PLATFORM_HEADER(RuntimeInfo.h)
#define PLATFORM_HAS_RUNTIME_INFO 1
#else
#define PLATFORM_HAS_RUNTIME_INFO 0
#endif

#include <Engine/Common/Singleton.h>
#include <Engine/Core/SymbolExportMacros.h>

#if PLATFORM_HAS_RUNTIME_INFO
#include <memory>
#endif

namespace Engine
{

class ENGINE_API RuntimeInfo : public ImmutableSingleton<RuntimeInfo>
{
public:
    RuntimeInfo()
        : IsDeveloperMode(false)
#if PLATFORM_HAS_RUNTIME_INFO
          ,
          platformInfo(std::make_unique<Platform::PlatformRuntimeInfo>())
#endif
    {}

    bool IsDeveloperMode;

#if PLATFORM_HAS_RUNTIME_INFO
    const std::unique_ptr<Platform::PlatformRuntimeInfo> platformInfo;
#endif
};

} // namespace Engine
