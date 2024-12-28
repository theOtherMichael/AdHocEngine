#pragma once

namespace Editor
{

class BaseBacktraceSymbolHandler
{
public:
    BaseBacktraceSymbolHandler() = default;
    explicit BaseBacktraceSymbolHandler(const bool isDeveloperMode){};

    BaseBacktraceSymbolHandler(const BaseBacktraceSymbolHandler& other)            = delete;
    BaseBacktraceSymbolHandler& operator=(const BaseBacktraceSymbolHandler& other) = delete;

    virtual ~BaseBacktraceSymbolHandler(){};
};

} // namespace Editor
