#pragma once

#include "../Base/BaseBacktraceSymbolHandler.h"

#include <windows.h>

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
