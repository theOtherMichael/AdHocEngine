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

Logger::Logger(LogLevel verbosity, LogEventCallback callback) : callback(callback)
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

Logger::~Logger()
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

} // namespace Engine::Console
