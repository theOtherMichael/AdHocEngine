#pragma once

#include <Engine/Core/Formatters/EnumFormatter.h>
#include <Engine/Core/SymbolExportMacros.h>

#include <fmt/format.h>

#include <functional>
#include <ostream>
#include <string>
#include <string_view>

namespace Engine::Console
{

INJECT_ENUM_FORMATTER

enum class LogLevel
{
    Fatal,
    Error,
    Warning,
    Log,
    Trace,
};

using LogEventCallback = std::function<void(const LogLevel logLevel, const std::string& message)>;

class Logger
{
public:
    ENGINE_API Logger(LogEventCallback callback) : Logger(LogLevel::Log, callback) {}

    ENGINE_API Logger(LogLevel verbosity, LogEventCallback callback);

    Logger(const Logger&)            = delete;
    Logger& operator=(const Logger&) = delete;

    ENGINE_API ~Logger();

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

} // namespace Engine::Console
