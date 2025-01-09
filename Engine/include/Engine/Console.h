#pragma once

#include <Engine/Core/SymbolExportMacros.h>

#include <fmt/format.h>

#include <cassert>
#include <functional>
#include <ostream>
#include <string>
#include <string_view>

namespace Engine::Console
{

enum class LogLevel
{
    // Fatal,
    Error,
    Warning,
    Message,
    Trace,
};

ENGINE_API std::ostream& operator<<(std::ostream& os, const LogLevel& logLevel);

typedef std::function<void(const LogLevel logLevel, const std::string& message)> LogEventCallback;

class ENGINE_API LogStream
{
public:
    LogStream(LogEventCallback callback) : LogStream(LogLevel::Message, callback) {}
    LogStream(LogLevel verbosity, LogEventCallback callback);

    LogStream(const LogStream&)            = delete;
    LogStream& operator=(const LogStream&) = delete;

    ~LogStream();

private:
    LogEventCallback callback;
    unsigned int id;
};

ENGINE_API void LogImplementation(LogLevel logLevel, const std::string& formattedMessage);

template <typename... T>
void LogError(fmt::format_string<T...> message, T&&... fmtArgs)
{
    const auto& formattedMessage = fmt::format(message, std::forward<T&&>(fmtArgs)...);
    LogImplementation(LogLevel::Error, formattedMessage);
}

template <typename... T>
void LogWarning(fmt::format_string<T...> message, T&&... fmtArgs)
{
    const auto& formattedMessage = fmt::format(message, std::forward<T&&>(fmtArgs)...);
    LogImplementation(LogLevel::Warning, formattedMessage);
}

template <typename... T>
void Log(fmt::format_string<T...> message, T&&... fmtArgs)
{
    const auto& formattedMessage = fmt::format(message, std::forward<T&&>(fmtArgs)...);
    LogImplementation(LogLevel::Message, formattedMessage);
}

template <typename... T>
void LogTrace(fmt::format_string<T...> message, T&&... fmtArgs)
{
    const auto& formattedMessage = fmt::format(message, std::forward<T&&>(fmtArgs)...);
    LogImplementation(LogLevel::Trace, formattedMessage);
}

} // namespace Engine::Console
