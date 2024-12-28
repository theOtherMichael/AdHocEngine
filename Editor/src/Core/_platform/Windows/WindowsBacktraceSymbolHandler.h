#pragma once

#include "../Base/BaseBacktraceSymbolHandler.h"

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

namespace Editor
{

class WindowsBacktraceSymbolHandler : public BaseBacktraceSymbolHandler
{
public:
    WindowsBacktraceSymbolHandler(bool isDeveloperMode);
    ~WindowsBacktraceSymbolHandler() override;

private:
    bool isValid = false;
};

typedef WindowsBacktraceSymbolHandler BacktraceSymbolHandler;

} // namespace Editor
