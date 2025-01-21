#pragma once

#include <Engine/Core/SymbolExportMacros.h>

namespace Engine
{

class ENGINE_API BaseBacktraceSymbolHandler
{
public:
    BaseBacktraceSymbolHandler() = default;

    BaseBacktraceSymbolHandler(const BaseBacktraceSymbolHandler& other)            = delete;
    BaseBacktraceSymbolHandler& operator=(const BaseBacktraceSymbolHandler& other) = delete;

    virtual ~BaseBacktraceSymbolHandler(){};
};

} // namespace Engine
