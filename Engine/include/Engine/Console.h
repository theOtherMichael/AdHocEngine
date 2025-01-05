#pragma once

#include <Engine/Core/SymbolExportMacros.h>

#include <fmt/format.h>

#include <functional>
#include <string>
#include <string_view>

namespace Engine::Console
{

typedef std::function<void(const std::string& message)> LogEventCallback;

enum class LogLevel
{
    // Fatal,
    Error,
    Warning,
    Message,
    Trace,
};

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
    LogLevel verbosity;
    unsigned int id;
};

ENGINE_API void LogImplementation(LogLevel verbosity, const std::string& formattedMessage);

template <typename... Args>
void LogError(const std::string_view message, Args&&... args)
{
    const auto& formattedMessage = fmt::format(message, std::forward<Args>(args)...);
    LogImplementation(LogLevel::Error, formattedMessage);
}

template <typename... Args>
void LogWarning(const std::string_view message, Args&&... args)
{
    const auto& formattedMessage = fmt::format(message, std::forward<Args>(args)...);
    LogImplementation(LogLevel::Warning, formattedMessage);
}

template <typename... Args>
void Log(const std::string_view message, Args&&... args)
{
    const auto& formattedMessage = fmt::format(message, std::forward<Args>(args)...);
    LogImplementation(LogLevel::Message, formattedMessage);
}

template <typename... Args>
void LogTrace(const std::string_view message, Args&&... args)
{
    const auto& formattedMessage = fmt::format(message, std::forward<Args>(args)...);
    LogImplementation(LogLevel::Trace, formattedMessage);
}

} // namespace Engine::Console
