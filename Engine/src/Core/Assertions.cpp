#include <Engine/Core/Assertions.h>

#include <fmt/format.h>

#include <cstdlib>
#include <source_location>

using Engine::Console::LogLevel;

namespace Engine::Internal
{

void LogNullAssertionFailure(const bool isFatal,
                             const std::string_view assertionExpression,
                             const bool expectedNull,
                             const std::string_view fmtMessage,
                             const std::source_location location)
{
    const auto formattedMessage =
        fmt::format("Assertion failure!\n{:<10} : {}\n{:<10} : {}\n{:<10} : {}\n{:<10} : {}\n{:<10} : {}{}",
                    "File",
                    location.file_name(),
                    "Line",
                    location.line(),
                    "Function",
                    location.function_name(),
                    "Expression",
                    assertionExpression,
                    "Expected",
                    expectedNull ? "null" : "non-null",
                    fmtMessage.empty() ? "" : fmt::format("\n{:<10} : {}", "Message", fmtMessage));

    Console::Internal::LogImplementation(isFatal ? LogLevel::Fatal : LogLevel::Error, formattedMessage);
}

void LogBooleanAssertionFailure(const bool isFatal,
                                const std::string_view assertionExpression,
                                const bool expectedResult,
                                const bool actualResult,
                                const std::string_view fmtMessage,
                                const std::source_location location)
{
    const auto formattedMessage = fmt::format(
        "Assertion failure!\n{:<10} : {}\n{:<10} : {}\n{:<10} : {}\n{:<10} : {}\n{:<10} : {}\n{:<10} : {}{}",
        "File",
        location.file_name(),
        "Line",
        location.line(),
        "Function",
        location.function_name(),
        "Expression",
        assertionExpression,
        "Expected",
        expectedResult,
        "Actual",
        actualResult,
        fmtMessage.empty() ? "" : fmt::format("\n{:<10} : {}", "Message", fmtMessage));

    Console::Internal::LogImplementation(isFatal ? LogLevel::Fatal : LogLevel::Error, formattedMessage);
}

void LogBinaryAssertionFailure(const bool isFatal,
                               const std::string_view leftExpression,
                               const std::string_view leftResult,
                               const std::string_view rightExpression,
                               const std::string_view rightResult,
                               const std::string_view operation,
                               const std::string_view fmtMessage,
                               const std::source_location location)
{
    const auto formattedMessage =
        fmt::format("Assertion failure!\n{:<9} : {}\n{:<9} : {}\n{:<9} : {}\n{:<9} : ({}) {} ({})\n{:<9} : {} vs {}{}",
                    "File",
                    location.file_name(),
                    "Line",
                    location.line(),
                    "Function",
                    location.function_name(),
                    "Condition",
                    leftExpression,
                    operation,
                    rightExpression,
                    "Compared",
                    leftResult,
                    rightResult,
                    fmtMessage.empty() ? "" : fmt::format("\n{:<9} : {}", "Message", fmtMessage));

    Console::Internal::LogImplementation(isFatal ? LogLevel::Fatal : LogLevel::Error, formattedMessage);
}

void LogAssertionTrap(const bool isFatal,
                      const std::string_view message,
                      const std::string_view fmtMessage,
                      const std::source_location location)
{
    const auto formattedMessage =
        fmt::format("Assertion failure!\n{:<9} : {}\n{:<9} : {}\n{:<9} : {}\n{:<9} : {}{}",
                    "File",
                    location.file_name(),
                    "Line",
                    location.line(),
                    "Function",
                    location.function_name(),
                    "Error",
                    message,
                    fmtMessage.empty() ? "" : fmt::format("\n{:<9} : {}", "Message", fmtMessage));

    Console::Internal::LogImplementation(isFatal ? LogLevel::Fatal : LogLevel::Error, formattedMessage);
}

void TriggerFatalErrorResponse()
{
    // TODO: Throw exception up to Launcher in editor builds
    std::abort();
}

} // namespace Engine::Internal
