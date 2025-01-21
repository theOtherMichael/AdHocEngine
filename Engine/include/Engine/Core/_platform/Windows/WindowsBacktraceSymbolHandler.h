#pragma once

#include "../Base/BaseBacktraceSymbolHandler.h"
#include <Engine/Core/SymbolExportMacros.h>

#include <windows.h>

namespace Engine
{

class ENGINE_API WindowsBacktraceSymbolHandler : public BaseBacktraceSymbolHandler
{
public:
    WindowsBacktraceSymbolHandler();

    WindowsBacktraceSymbolHandler(const WindowsBacktraceSymbolHandler&)            = delete;
    WindowsBacktraceSymbolHandler& operator=(const WindowsBacktraceSymbolHandler&) = delete;

    ~WindowsBacktraceSymbolHandler() override;

private:
    bool isValid = false;
};

typedef WindowsBacktraceSymbolHandler BacktraceSymbolHandler;

} // namespace Engine
