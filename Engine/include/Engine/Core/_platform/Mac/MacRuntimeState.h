#pragma once

#include <Engine/Core/SymbolExportMacros.h>

namespace Engine
{

struct ENGINE_API MacRuntimeState
{
public:
    static const MacRuntimeState& GetInstance();

    MacRuntimeState() = default;
    ~MacRuntimeState();

    MacRuntimeState(const MacRuntimeState&)            = delete;
    MacRuntimeState& operator=(const MacRuntimeState&) = delete;
};

#if ADHOC_INTERNAL
ENGINE_API void InitializeProcessInfo();
ENGINE_API MacRuntimeState& GetMutableRuntimeState();
#endif

typedef MacRuntimeState RuntimeState;

} // namespace Engine
