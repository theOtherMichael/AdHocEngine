#include <Engine/Console.h>

#include <Engine/Core/Misc.h>

#include <algorithm>
#include <forward_list>
#include <string>
#include <string_view>
#include <type_traits>

namespace Engine::Console
{

std::ostream& operator<<(std::ostream& os, const LogLevel& logLevel)
{
    switch (logLevel)
    {
    case LogLevel::Error: os << "Error"; break;
    case LogLevel::Warning: os << "Warning"; break;
    case LogLevel::Message: os << "Message"; break;
    case LogLevel::Trace: os << "Trace"; break;
    default: assert(false);
    }

    return os;
}

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

template <typename E>
constexpr typename std::underlying_type<E>::type EnumValue(E e) noexcept
{
    return static_cast<typename std::underlying_type<E>::type>(e);
}

void LogImplementation(LogLevel logLevel, const std::string& formattedMessage)
{
    // static_assert(to_underlying(LogLevel::Fatal) < to_underlying(LogLevel::Error));
    static_assert(EnumValue(LogLevel::Error) < EnumValue(LogLevel::Warning));
    static_assert(EnumValue(LogLevel::Warning) < EnumValue(LogLevel::Message));
    static_assert(EnumValue(LogLevel::Message) < EnumValue(LogLevel::Trace));

    for (const auto& callbackInfo : logListenerRegistry)
    {
        if (EnumValue(logLevel) <= EnumValue(callbackInfo.verbosity))
            callbackInfo.callback(logLevel, formattedMessage);
    }
}

} // namespace Engine::Console
