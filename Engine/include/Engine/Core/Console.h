#pragma once

#include <Engine/Core/SymbolExportMacros.h>

#include <fmt/format.h>

#include <cstdlib>
#include <functional>
#include <ostream>
#include <string>
#include <string_view>

namespace Engine::Console
{

enum class LogLevel
{
    Fatal,
    Error,
    Warning,
    Log,
    Trace,
};

typedef std::function<void(const LogLevel logLevel, const std::string& message)> LogEventCallback;

class LogStream
{
public:
    LogStream(LogEventCallback callback) : LogStream(LogLevel::Log, callback) {}
    ENGINE_API LogStream(LogLevel verbosity, LogEventCallback callback);

    LogStream(const LogStream&)            = delete;
    LogStream& operator=(const LogStream&) = delete;

    ENGINE_API ~LogStream();

private:
    LogEventCallback callback;
    unsigned int id;
};

namespace Internal
{

ENGINE_API void LogImplementation(LogLevel logLevel, const std::string& formattedMessage);

}

template <typename... T>
void LogFatal(fmt::format_string<T...> message, T&&... fmtArgs)
{
    const auto& formattedMessage = fmt::format(message, std::forward<T&&>(fmtArgs)...);
    Internal::LogImplementation(LogLevel::Fatal, formattedMessage);
    std::abort();
}

template <typename... T>
void LogError(fmt::format_string<T...> message, T&&... fmtArgs)
{
    const auto& formattedMessage = fmt::format(message, std::forward<T&&>(fmtArgs)...);
    Internal::LogImplementation(LogLevel::Error, formattedMessage);
}

template <typename... T>
void LogWarning(fmt::format_string<T...> message, T&&... fmtArgs)
{
    const auto& formattedMessage = fmt::format(message, std::forward<T&&>(fmtArgs)...);
    Internal::LogImplementation(LogLevel::Warning, formattedMessage);
}

template <typename... T>
void Log(fmt::format_string<T...> message, T&&... fmtArgs)
{
    const auto& formattedMessage = fmt::format(message, std::forward<T&&>(fmtArgs)...);
    Internal::LogImplementation(LogLevel::Log, formattedMessage);
}

template <typename... T>
void LogTrace(fmt::format_string<T...> message, T&&... fmtArgs)
{
    const auto& formattedMessage = fmt::format(message, std::forward<T&&>(fmtArgs)...);
    Internal::LogImplementation(LogLevel::Trace, formattedMessage);
}

ENGINE_API std::ostream& operator<<(std::ostream& os, const LogLevel& logLevel);

} // namespace Engine::Console

template <>
struct fmt::formatter<::Engine::Console::LogLevel> : formatter<string_view>
{
    ENGINE_API auto format(::Engine::Console::LogLevel logLevel, format_context& ctx) const -> format_context::iterator;
};
