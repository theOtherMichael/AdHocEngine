#include <Engine/Core/Assertions.h>

#include <fmt/format.h>

#include <cstdlib>
#include <source_location>

using Engine::Console::LogLevel;

namespace Engine::Internal
{

void LogBooleanAssertionFailure(const bool isFatal,
                                const std::string_view assertionExpression,
                                const bool expectedResult,
                                const bool actualResult,
                                const std::source_location location)
{
    const auto formattedMessage = fmt::format(
        "Assertion failure!\n{:<10} : {}\n{:<10} : {}\n{:<10} : {}\n{:<10} : {}\n{:<10} : {}\n{:<10} : {}\n",
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
        actualResult);

    Console::Internal::LogImplementation(isFatal ? LogLevel::Fatal : LogLevel::Error, formattedMessage);
}

void LogBinaryAssertionFailure(const bool isFatal,
                               const std::string_view leftExpression,
                               const std::string_view leftResult,
                               const std::string_view rightExpression,
                               const std::string_view rightResult,
                               const std::string_view operation,
                               const std::source_location location)
{
    const auto formattedMessage =
        fmt::format("Assertion failure!\n{:<9} : {}\n{:<9} : {}\n{:<9} : {}\n{:<9} : ({}) {} ({})\n{:<9} : {} vs {}\n",
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
                    rightResult);

    Console::Internal::LogImplementation(isFatal ? LogLevel::Fatal : LogLevel::Error, formattedMessage);
}

void LogAssertionTrap(const bool isFatal, const std::string_view message, const std::source_location location)
{
    const auto formattedMessage = fmt::format("Assertion failure!\n{:<9} : {}\n{:<9} : {}\n{:<9} : {}\n{:<9} : {}\n",
                                              "File",
                                              location.file_name(),
                                              "Line",
                                              location.line(),
                                              "Function",
                                              location.function_name(),
                                              "Error",
                                              message);

    Console::Internal::LogImplementation(isFatal ? LogLevel::Fatal : LogLevel::Error, formattedMessage);
}

void TriggerFatalErrorResponse()
{
    // TODO: Throw exception up to Launcher in editor builds
    std::abort();
}

} // namespace Engine::Internal
