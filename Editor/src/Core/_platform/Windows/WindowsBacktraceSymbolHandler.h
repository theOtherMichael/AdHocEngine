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
    explicit WindowsBacktraceSymbolHandler(bool isDeveloperMode);

    WindowsBacktraceSymbolHandler(const WindowsBacktraceSymbolHandler&)            = delete;
    WindowsBacktraceSymbolHandler& operator=(const WindowsBacktraceSymbolHandler&) = delete;

    ~WindowsBacktraceSymbolHandler() override;

private:
    bool isValid = false;
};

typedef WindowsBacktraceSymbolHandler BacktraceSymbolHandler;

} // namespace Editor
