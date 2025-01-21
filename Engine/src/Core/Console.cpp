#include <Engine/Core/Console.h>

#include <Engine/Core/Assertions.h>
#include <Engine/Core/Misc.h>

#include <fmt/format.h>

#include <algorithm>
#include <forward_list>
#include <string>
#include <string_view>

namespace Engine::Console
{

struct LogListenerInfo
{
    LogEventCallback callback;
    LogLevel verbosity;
    unsigned int id;

    LogListenerInfo(LogEventCallback callback, LogLevel verbosity, unsigned int id)
        : callback(callback), verbosity(verbosity), id(id)
    {}
};

static std::forward_list<LogListenerInfo> logListenerRegistry;

LogStream::LogStream(LogLevel verbosity, LogEventCallback callback) : callback(callback)
{
    id = 0;
    while (std::any_of(logListenerRegistry.begin(),
                       logListenerRegistry.end(),
                       [this](const LogListenerInfo& info) { return info.id == id; }))
    {
        ++id;
    }

    logListenerRegistry.emplace_front(callback, verbosity, id);
}

LogStream::~LogStream()
{
    logListenerRegistry.remove_if([this](const LogListenerInfo& info) { return info.id == id; });
}

namespace Internal
{

void LogImplementation(LogLevel logLevel, const std::string& formattedMessage)
{
    static_assert(LogLevel::Fatal < LogLevel::Error);
    static_assert(LogLevel::Error < LogLevel::Warning);
    static_assert(LogLevel::Warning < LogLevel::Log);
    static_assert(LogLevel::Log < LogLevel::Trace);

    for (const auto& callbackInfo : logListenerRegistry)
    {
        if (logLevel <= callbackInfo.verbosity)
            callbackInfo.callback(logLevel, formattedMessage);
    }
}

} // namespace Internal

std::ostream& operator<<(std::ostream& os, const LogLevel& logLevel)
{
    switch (logLevel)
    {
    case LogLevel::Fatal: os << "Fatal"; break;
    case LogLevel::Error: os << "Error"; break;
    case LogLevel::Warning: os << "Warning"; break;
    case LogLevel::Log: os << "Log"; break;
    case LogLevel::Trace: os << "Trace"; break;
    default: Assert_NoEntry();
    }

    return os;
}

} // namespace Engine::Console

auto fmt::formatter<::Engine::Console::LogLevel>::format(::Engine::Console::LogLevel logLevel,
                                                         format_context& ctx) const -> format_context::iterator
{
    string_view name = "unknown";

    switch (logLevel)
    {
    case ::Engine::Console::LogLevel::Fatal: name = "Fatal"; break;
    case ::Engine::Console::LogLevel::Error: name = "Error"; break;
    case ::Engine::Console::LogLevel::Warning: name = "Warning"; break;
    case ::Engine::Console::LogLevel::Log: name = "Log"; break;
    case ::Engine::Console::LogLevel::Trace: name = "Trace"; break;
    default: Assert_NoEntry(); break;
    }

    return formatter<string_view>::format(name, ctx);
}
