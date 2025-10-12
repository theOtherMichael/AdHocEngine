#pragma once

#include <Engine/Core/Console.h>
#include <Engine/Core/Misc.h>
#include <Engine/Core/SymbolExportMacros.h>

#include <fmt/format.h>

#include <source_location>
#include <string_view>

#if ADHOC_WINDOWS
    #define ASSERT_PLATFORM_WINDOWS static_assert(true, "This file can only be compiled on Windows, and it is!")
#else
    #define ASSERT_PLATFORM_WINDOWS static_assert(false, "This file can only be compiled on Windows! " __FILE__)
#endif

#if ADHOC_MAC
    #define ASSERT_PLATFORM_MACOS static_assert(true, "This file can only be compiled on macOS, and it is!")
#else
    #define ASSERT_PLATFORM_MACOS static_assert(false, "This file can only be compiled on macOS! " __FILE__)
#endif

#if ADHOC_DEBUG
    #define ADHOC_ASSERTIONS_ON      1
    #define ADHOC_SLOW_ASSERTIONS_ON 1
#endif

#if ADHOC_DEV
    #define ADHOC_ASSERTIONS_ON      1
    #define ADHOC_SLOW_ASSERTIONS_ON 0
#endif

#if ADHOC_RELEASE
    #define ADHOC_ASSERTIONS_ON      0
    #define ADHOC_SLOW_ASSERTIONS_ON 0
#endif

namespace Engine::Internal
{

ENGINE_API void LogNullAssertionFailure(const bool isFatal,
                                        const std::string_view assertionExpression,
                                        const bool expectedNull,
                                        const std::string_view fmtMessage,
                                        const std::source_location location);

ENGINE_API void LogBooleanAssertionFailure(const bool isFatal,
                                           const std::string_view assertionExpression,
                                           const bool expectedResult,
                                           const bool actualResult,
                                           const std::string_view fmtMessage,
                                           const std::source_location location);

ENGINE_API void LogBinaryAssertionFailure(const bool isFatal,
                                          const std::string_view leftExpression,
                                          const std::string_view leftResult,
                                          const std::string_view rightExpression,
                                          const std::string_view rightResult,
                                          const std::string_view operation,
                                          const std::string_view fmtMessage,
                                          const std::source_location location);

ENGINE_API void LogAssertionTrap(const bool isFatal,
                                 const std::string_view message,
                                 const std::string_view fmtMessage,
                                 const std::source_location location);

ENGINE_API void TriggerFatalErrorResponse();

} // namespace Engine::Internal

#define ADHOC_ASSERT_IMPLEMENTATION_NULL(isFatal, expression, expectedNull, fmtMessage)                                \
    {                                                                                                                  \
        auto expressionResult = expression;                                                                            \
        if (expectedNull && expressionResult != nullptr || !expectedNull && expressionResult == nullptr)               \
        {                                                                                                              \
            ::Engine::Internal::LogNullAssertionFailure(                                                               \
                isFatal, STRINGIFY(expression), expectedNull, fmtMessage, std::source_location::current());            \
                                                                                                                       \
            DEBUG_BREAK();                                                                                             \
                                                                                                                       \
            if (isFatal)                                                                                               \
                ::Engine::Internal::TriggerFatalErrorResponse();                                                       \
        }                                                                                                              \
    }

#define ADHOC_ASSERT_IMPLEMENTATION_BOOLEAN(isFatal, expression, expected, fmtMessage)                                 \
    {                                                                                                                  \
        auto expressionResult = expression;                                                                            \
        if (expressionResult != expected)                                                                              \
        {                                                                                                              \
            ::Engine::Internal::LogBooleanAssertionFailure(isFatal,                                                    \
                                                           STRINGIFY(expression),                                      \
                                                           expected,                                                   \
                                                           expressionResult,                                           \
                                                           fmtMessage,                                                 \
                                                           std::source_location::current());                           \
                                                                                                                       \
            DEBUG_BREAK();                                                                                             \
                                                                                                                       \
            if (isFatal)                                                                                               \
                ::Engine::Internal::TriggerFatalErrorResponse();                                                       \
        }                                                                                                              \
    }

#define ADHOC_ASSERT_IMPLEMENTATION_BINARY(isFatal, leftExpression, rightExpression, operation, fmtMessage)            \
    {                                                                                                                  \
        auto leftExpressionResult  = leftExpression;                                                                   \
        auto rightExpressionResult = rightExpression;                                                                  \
                                                                                                                       \
        auto leftExpressionResultString  = fmt::format("{}", leftExpressionResult);                                    \
        auto rightExpressionResultString = fmt::format("{}", rightExpressionResult);                                   \
                                                                                                                       \
        if (!(leftExpressionResult operation rightExpressionResult))                                                   \
        {                                                                                                              \
            ::Engine::Internal::LogBinaryAssertionFailure(isFatal,                                                     \
                                                          STRINGIFY(leftExpression),                                   \
                                                          leftExpressionResultString,                                  \
                                                          STRINGIFY(rightExpression),                                  \
                                                          rightExpressionResultString,                                 \
                                                          STRINGIFY(operation),                                        \
                                                          fmtMessage,                                                  \
                                                          std::source_location::current());                            \
                                                                                                                       \
            DEBUG_BREAK();                                                                                             \
                                                                                                                       \
            if (isFatal)                                                                                               \
                ::Engine::Internal::TriggerFatalErrorResponse();                                                       \
        }                                                                                                              \
    }

#define ADHOC_ASSERT_IMPLEMENTATION_NO_ENTRY(isFatal, message, fmtMessage)                                             \
    {                                                                                                                  \
        ::Engine::Internal::LogAssertionTrap(isFatal, message, fmtMessage, std::source_location::current());           \
                                                                                                                       \
        DEBUG_BREAK();                                                                                                 \
                                                                                                                       \
        if (isFatal)                                                                                                   \
            ::Engine::Internal::TriggerFatalErrorResponse();                                                           \
    }

#define ADHOC_ASSERT_IMPLEMENTATION_NO_REENTRY(isFatal, message, fmtMessage)                                           \
    {                                                                                                                  \
        static bool CONCATENATE(AdHocAssertNoReentry_HasReachedLine, __LINE__) = false;                                \
                                                                                                                       \
        if (CONCATENATE(AdHocAssertNoReentry_HasReachedLine, __LINE__))                                                \
            ADHOC_ASSERT_IMPLEMENTATION_NO_ENTRY(isFatal, message, fmtMessage);                                        \
                                                                                                                       \
        CONCATENATE(AdHocAssertNoReentry_HasReachedLine, __LINE__) = true;                                             \
    }

#if ADHOC_ASSERTIONS_ON

// Non-fmt versions

    #define Assert_Null(pointer)    ADHOC_ASSERT_IMPLEMENTATION_NULL(true, pointer, true, "")
    #define Assert_NotNull(pointer) ADHOC_ASSERT_IMPLEMENTATION_NULL(true, pointer, false, "")

    #define Expect_Null(pointer)    ADHOC_ASSERT_IMPLEMENTATION_NULL(false, pointer, true, "")
    #define Expect_NotNull(pointer) ADHOC_ASSERT_IMPLEMENTATION_NULL(false, pointer, false, "")

    #define AssertEval_Null(pointer)    ADHOC_ASSERT_IMPLEMENTATION_NULL(true, pointer, true, "")
    #define AssertEval_NotNull(pointer) ADHOC_ASSERT_IMPLEMENTATION_NULL(true, pointer, false, "")

    #define ExpectEval_Null(pointer)    ADHOC_ASSERT_IMPLEMENTATION_NULL(false, pointer, true, "")
    #define ExpectEval_NotNull(pointer) ADHOC_ASSERT_IMPLEMENTATION_NULL(false, pointer, false, "")

    #define Assert_True(expression)  ADHOC_ASSERT_IMPLEMENTATION_BOOLEAN(true, expression, true, "")
    #define Assert_False(expression) ADHOC_ASSERT_IMPLEMENTATION_BOOLEAN(true, expression, false, "")

    #define Expect_True(expression)  ADHOC_ASSERT_IMPLEMENTATION_BOOLEAN(false, expression, true, "")
    #define Expect_False(expression) ADHOC_ASSERT_IMPLEMENTATION_BOOLEAN(false, expression, false, "")

    #define AssertEval_True(expression)  ADHOC_ASSERT_IMPLEMENTATION_BOOLEAN(true, expression, true, "")
    #define AssertEval_False(expression) ADHOC_ASSERT_IMPLEMENTATION_BOOLEAN(true, expression, false, "")

    #define ExpectEval_True(expression)  ADHOC_ASSERT_IMPLEMENTATION_BOOLEAN(false, expression, true, "")
    #define ExpectEval_False(expression) ADHOC_ASSERT_IMPLEMENTATION_BOOLEAN(false, expression, false, "")

    #define Assert_Eq(lhs, rhs) ADHOC_ASSERT_IMPLEMENTATION_BINARY(true, lhs, rhs, ==, "")
    #define Assert_Ne(lhs, rhs) ADHOC_ASSERT_IMPLEMENTATION_BINARY(true, lhs, rhs, !=, "")
    #define Assert_Lt(lhs, rhs) ADHOC_ASSERT_IMPLEMENTATION_BINARY(true, lhs, rhs, <, "")
    #define Assert_Le(lhs, rhs) ADHOC_ASSERT_IMPLEMENTATION_BINARY(true, lhs, rhs, <=, "")
    #define Assert_Gt(lhs, rhs) ADHOC_ASSERT_IMPLEMENTATION_BINARY(true, lhs, rhs, >, "")
    #define Assert_Ge(lhs, rhs) ADHOC_ASSERT_IMPLEMENTATION_BINARY(true, lhs, rhs, >=, "")

    #define Expect_Eq(lhs, rhs) ADHOC_ASSERT_IMPLEMENTATION_BINARY(false, lhs, rhs, ==, "")
    #define Expect_Ne(lhs, rhs) ADHOC_ASSERT_IMPLEMENTATION_BINARY(false, lhs, rhs, !=, "")
    #define Expect_Lt(lhs, rhs) ADHOC_ASSERT_IMPLEMENTATION_BINARY(false, lhs, rhs, <, "")
    #define Expect_Le(lhs, rhs) ADHOC_ASSERT_IMPLEMENTATION_BINARY(false, lhs, rhs, <=, "")
    #define Expect_Gt(lhs, rhs) ADHOC_ASSERT_IMPLEMENTATION_BINARY(false, lhs, rhs, >, "")
    #define Expect_Ge(lhs, rhs) ADHOC_ASSERT_IMPLEMENTATION_BINARY(false, lhs, rhs, >=, "")

    #define AssertEval_Eq(lhs_evaluated, rhs_discarded)                                                                \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(true, lhs_evaluated, rhs_discarded, ==, "")
    #define AssertEval_Ne(lhs_evaluated, rhs_discarded)                                                                \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(true, lhs_evaluated, rhs_discarded, !=, "")
    #define AssertEval_Lt(lhs_evaluated, rhs_discarded)                                                                \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(true, lhs_evaluated, rhs_discarded, <, "")
    #define AssertEval_Le(lhs_evaluated, rhs_discarded)                                                                \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(true, lhs_evaluated, rhs_discarded, <=, "")
    #define AssertEval_Gt(lhs_evaluated, rhs_discarded)                                                                \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(true, lhs_evaluated, rhs_discarded, >, "")
    #define AssertEval_Ge(lhs_evaluated, rhs_discarded)                                                                \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(true, lhs_evaluated, rhs_discarded, >=, "")

    #define ExpectEval_Eq(lhs_evaluated, rhs_discarded)                                                                \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(false, lhs_evaluated, rhs_discarded, ==, "")
    #define ExpectEval_Ne(lhs_evaluated, rhs_discarded)                                                                \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(false, lhs_evaluated, rhs_discarded, !=, "")
    #define ExpectEval_Lt(lhs_evaluated, rhs_discarded)                                                                \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(false, lhs_evaluated, rhs_discarded, <, "")
    #define ExpectEval_Le(lhs_evaluated, rhs_discarded)                                                                \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(false, lhs_evaluated, rhs_discarded, <=, "")
    #define ExpectEval_Gt(lhs_evaluated, rhs_discarded)                                                                \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(false, lhs_evaluated, rhs_discarded, >, "")
    #define ExpectEval_Ge(lhs_evaluated, rhs_discarded)                                                                \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(false, lhs_evaluated, rhs_discarded, >=, "")

    #define Assert_NoEntry()   ADHOC_ASSERT_IMPLEMENTATION_NO_ENTRY(true, "Call to Assert_NoEntry()", "")
    #define Assert_NoReentry() ADHOC_ASSERT_IMPLEMENTATION_NO_REENTRY(true, "Call to Assert_NoReentry()", "")

    #define Expect_NoEntry()   ADHOC_ASSERT_IMPLEMENTATION_NO_ENTRY(false, "Call to Assert_NoEntry()", "")
    #define Expect_NoReentry() ADHOC_ASSERT_IMPLEMENTATION_NO_REENTRY(false, "Call to Assert_NoReentry()", "")

// Fmt versions

    #define Assert_Null_Fmt(pointer, fmtString, ...)                                                                   \
        ADHOC_ASSERT_IMPLEMENTATION_NULL(true, pointer, true, fmt::format(fmtString, __VA_ARGS__))
    #define Assert_NotNull_Fmt(pointer, fmtString, ...)                                                                \
        ADHOC_ASSERT_IMPLEMENTATION_NULL(true, pointer, false, fmt::format(fmtString, __VA_ARGS__))

    #define Expect_Null_Fmt(pointer, fmtString, ...)                                                                   \
        ADHOC_ASSERT_IMPLEMENTATION_NULL(false, pointer, true, fmt::format(fmtString, __VA_ARGS__))
    #define Expect_NotNull_Fmt(pointer, fmtString, ...)                                                                \
        ADHOC_ASSERT_IMPLEMENTATION_NULL(false, pointer, false, fmt::format(fmtString, __VA_ARGS__))

    #define AssertEval_Null_Fmt(pointer, fmtString, ...)                                                               \
        ADHOC_ASSERT_IMPLEMENTATION_NULL(true, pointer, true, fmt::format(fmtString, __VA_ARGS__))
    #define AssertEval_NotNull_Fmt(pointer, fmtString, ...)                                                            \
        ADHOC_ASSERT_IMPLEMENTATION_NULL(true, pointer, false, fmt::format(fmtString, __VA_ARGS__))

    #define ExpectEval_Null_Fmt(pointer, fmtString, ...)                                                               \
        ADHOC_ASSERT_IMPLEMENTATION_NULL(false, pointer, true, fmt::format(fmtString, __VA_ARGS__))
    #define ExpectEval_NotNull_Fmt(pointer, fmtString, ...)                                                            \
        ADHOC_ASSERT_IMPLEMENTATION_NULL(false, pointer, false, fmt::format(fmtString, __VA_ARGS__))

    #define Assert_True_Fmt(expression, fmtString, ...)                                                                \
        ADHOC_ASSERT_IMPLEMENTATION_BOOLEAN(true, expression, true, fmt::format(fmtString, __VA_ARGS__))
    #define Assert_False_Fmt(expression, fmtString, ...)                                                               \
        ADHOC_ASSERT_IMPLEMENTATION_BOOLEAN(true, expression, false, fmt::format(fmtString, __VA_ARGS__))

    #define Expect_True_Fmt(expression, fmtString, ...)                                                                \
        ADHOC_ASSERT_IMPLEMENTATION_BOOLEAN(false, expression, true, fmt::format(fmtString, __VA_ARGS__))
    #define Expect_False_Fmt(expression, fmtString, ...)                                                               \
        ADHOC_ASSERT_IMPLEMENTATION_BOOLEAN(false, expression, false, fmt::format(fmtString, __VA_ARGS__))

    #define AssertEval_True_Fmt(expression, fmtString, ...)                                                            \
        ADHOC_ASSERT_IMPLEMENTATION_BOOLEAN(true, expression, true, fmt::format(fmtString, __VA_ARGS__))
    #define AssertEval_False_Fmt(expression, fmtString, ...)                                                           \
        ADHOC_ASSERT_IMPLEMENTATION_BOOLEAN(true, expression, false, fmt::format(fmtString, __VA_ARGS__))

    #define ExpectEval_True_Fmt(expression, fmtString, ...)                                                            \
        ADHOC_ASSERT_IMPLEMENTATION_BOOLEAN(false, expression, true, fmt::format(fmtString, __VA_ARGS__))
    #define ExpectEval_False_Fmt(expression, fmtString, ...)                                                           \
        ADHOC_ASSERT_IMPLEMENTATION_BOOLEAN(false, expression, false, fmt::format(fmtString, __VA_ARGS__))

    #define Assert_Eq_Fmt(lhs, rhs, fmtString, ...)                                                                    \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(true, lhs, rhs, ==, fmt::format(fmtString, __VA_ARGS__))
    #define Assert_Ne_Fmt(lhs, rhs, fmtString, ...)                                                                    \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(true, lhs, rhs, !=, fmt::format(fmtString, __VA_ARGS__))
    #define Assert_Lt_Fmt(lhs, rhs, fmtString, ...)                                                                    \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(true, lhs, rhs, <, fmt::format(fmtString, __VA_ARGS__))
    #define Assert_Le_Fmt(lhs, rhs, fmtString, ...)                                                                    \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(true, lhs, rhs, <=, fmt::format(fmtString, __VA_ARGS__))
    #define Assert_Gt_Fmt(lhs, rhs, fmtString, ...)                                                                    \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(true, lhs, rhs, >, fmt::format(fmtString, __VA_ARGS__))
    #define Assert_Ge_Fmt(lhs, rhs, fmtString, ...)                                                                    \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(true, lhs, rhs, >=, fmt::format(fmtString, __VA_ARGS__))

    #define Expect_Eq_Fmt(lhs, rhs, fmtString, ...)                                                                    \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(false, lhs, rhs, ==, fmt::format(fmtString, __VA_ARGS__))
    #define Expect_Ne_Fmt(lhs, rhs, fmtString, ...)                                                                    \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(false, lhs, rhs, !=, fmt::format(fmtString, __VA_ARGS__))
    #define Expect_Lt_Fmt(lhs, rhs, fmtString, ...)                                                                    \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(false, lhs, rhs, <, fmt::format(fmtString, __VA_ARGS__))
    #define Expect_Le_Fmt(lhs, rhs, fmtString, ...)                                                                    \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(                                                                            \
            false, lhs, rhs, <=, fmt::format(fmtString, __VA_ARGS__, fmt::format(fmtString, __VA_ARGS__)))
    #define Expect_Gt_Fmt(lhs, rhs, fmtString, ...)                                                                    \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(false, lhs, rhs, >, fmt::format(fmtString, __VA_ARGS__))
    #define Expect_Ge_Fmt(lhs, rhs, fmtString, ...)                                                                    \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(false, lhs, rhs, >=, fmt::format(fmtString, __VA_ARGS__))

    #define AssertEval_Eq_Fmt(lhs_evaluated, rhs_discarded, fmtString, ...)                                            \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(true, lhs_evaluated, rhs_discarded, ==, fmt::format(fmtString, __VA_ARGS__))
    #define AssertEval_Ne_Fmt(lhs_evaluated, rhs_discarded, fmtString, ...)                                            \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(true, lhs_evaluated, rhs_discarded, !=, fmt::format(fmtString, __VA_ARGS__))
    #define AssertEval_Lt_Fmt(lhs_evaluated, rhs_discarded, fmtString, ...)                                            \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(true, lhs_evaluated, rhs_discarded, <, fmt::format(fmtString, __VA_ARGS__))
    #define AssertEval_Le_Fmt(lhs_evaluated, rhs_discarded, fmtString, ...)                                            \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(true, lhs_evaluated, rhs_discarded, <=, fmt::format(fmtString, __VA_ARGS__))
    #define AssertEval_Gt_Fmt(lhs_evaluated, rhs_discarded, fmtString, ...)                                            \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(true, lhs_evaluated, rhs_discarded, >, fmt::format(fmtString, __VA_ARGS__))
    #define AssertEval_Ge_Fmt(lhs_evaluated, rhs_discarded, fmtString, ...)                                            \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(true, lhs_evaluated, rhs_discarded, >=, fmt::format(fmtString, __VA_ARGS__))

    #define ExpectEval_Eq_Fmt(lhs_evaluated, rhs_discarded, fmtString, ...)                                            \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(false, lhs_evaluated, rhs_discarded, ==, fmt::format(fmtString, __VA_ARGS__))
    #define ExpectEval_Ne_Fmt(lhs_evaluated, rhs_discarded, fmtString, ...)                                            \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(false, lhs_evaluated, rhs_discarded, !=, fmt::format(fmtString, __VA_ARGS__))
    #define ExpectEval_Lt_Fmt(lhs_evaluated, rhs_discarded, fmtString, ...)                                            \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(false, lhs_evaluated, rhs_discarded, <, fmt::format(fmtString, __VA_ARGS__))
    #define ExpectEval_Le_Fmt(lhs_evaluated, rhs_discarded, fmtString, ...)                                            \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(false, lhs_evaluated, rhs_discarded, <=, fmt::format(fmtString, __VA_ARGS__))
    #define ExpectEval_Gt_Fmt(lhs_evaluated, rhs_discarded, fmtString, ...)                                            \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(false, lhs_evaluated, rhs_discarded, >, fmt::format(fmtString, __VA_ARGS__))
    #define ExpectEval_Ge_Fmt(lhs_evaluated, rhs_discarded, fmtString, ...)                                            \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(false, lhs_evaluated, rhs_discarded, >=, fmt::format(fmtString, __VA_ARGS__))

    #define Assert_NoEntry_Fmt(fmtString, ...)                                                                         \
        ADHOC_ASSERT_IMPLEMENTATION_NO_ENTRY(true, "Call to Assert_NoEntry()", fmt::format(fmtString, __VA_ARGS__))
    #define Assert_NoReentry_Fmt(fmtString, ...)                                                                       \
        ADHOC_ASSERT_IMPLEMENTATION_NO_REENTRY(true, "Call to Assert_NoReentry()", fmt::format(fmtString, __VA_ARGS__))

    #define Expect_NoEntry_Fmt(fmtString, ...)                                                                         \
        ADHOC_ASSERT_IMPLEMENTATION_NO_ENTRY(false, "Call to Assert_NoEntry()", fmt::format(fmtString, __VA_ARGS__))
    #define Expect_NoReentry_Fmt(fmtString, ...)                                                                       \
        ADHOC_ASSERT_IMPLEMENTATION_NO_REENTRY(false, "Call to Assert_NoReentry()", fmt::format(fmtString, __VA_ARGS__))

    #define Assert_Code(code)                                                                                          \
        do                                                                                                             \
        {                                                                                                              \
            code                                                                                                       \
        } while (false)

#else

// clang-format off

    // Non-fmt versions

    #define Assert_Null(pointer) {}
    #define Assert_NotNull(pointer) {}

    #define Expect_Null(pointer) {}
    #define Expect_NotNull(pointer) {}

    #define AssertEval_Null(pointer) { pointer; }
    #define AssertEval_NotNull(pointer) { pointer; }

    #define ExpectEval_Null(pointer) { pointer; }
    #define ExpectEval_NotNull(pointer) { pointer; }

    #define Assert_True(expression) {}
    #define Assert_False(expression) {}

    #define Expect_True(expression) {}
    #define Expect_False(expression) {}

    #define AssertEval_True(expression) { expression; }
    #define AssertEval_False(expression) { expression; }

    #define ExpectEval_True(expression) { expression; }
    #define ExpectEval_False(expression) { expression; }

    #define Assert_Eq(lhs, rhs) {}
    #define Assert_Ne(lhs, rhs) {}
    #define Assert_Lt(lhs, rhs) {}
    #define Assert_Le(lhs, rhs) {}
    #define Assert_Gt(lhs, rhs) {}
    #define Assert_Ge(lhs, rhs) {}

    #define Expect_Eq(lhs, rhs) {}
    #define Expect_Ne(lhs, rhs) {}
    #define Expect_Lt(lhs, rhs) {}
    #define Expect_Le(lhs, rhs) {}
    #define Expect_Gt(lhs, rhs) {}
    #define Expect_Ge(lhs, rhs) {}

    #define AssertEval_Eq(lhs_evaluated, rhs_discarded) { lhs_evaluated; }
    #define AssertEval_Ne(lhs_evaluated, rhs_discarded) { lhs_evaluated; }
    #define AssertEval_Lt(lhs_evaluated, rhs_discarded) { lhs_evaluated; }
    #define AssertEval_Le(lhs_evaluated, rhs_discarded) { lhs_evaluated; }
    #define AssertEval_Gt(lhs_evaluated, rhs_discarded) { lhs_evaluated; }
    #define AssertEval_Ge(lhs_evaluated, rhs_discarded) { lhs_evaluated; }

    #define ExpectEval_Eq(lhs_evaluated, rhs_discarded) { lhs_evaluated; }
    #define ExpectEval_Ne(lhs_evaluated, rhs_discarded) { lhs_evaluated; }
    #define ExpectEval_Lt(lhs_evaluated, rhs_discarded) { lhs_evaluated; }
    #define ExpectEval_Le(lhs_evaluated, rhs_discarded) { lhs_evaluated; }
    #define ExpectEval_Gt(lhs_evaluated, rhs_discarded) { lhs_evaluated; }
    #define ExpectEval_Ge(lhs_evaluated, rhs_discarded) { lhs_evaluated; }

    #define Assert_NoEntry() {}
    #define Assert_NoReentry() {}

    #define Expect_NoEntry() {}
    #define Expect_NoReentry() {}

    // Fmt versions

    #define Assert_Null_Fmt(pointer, fmtString, ...) {}
    #define Assert_NotNull_Fmt(pointer, fmtString, ...) {}

    #define Expect_Null_Fmt(pointer, fmtString, ...) {}
    #define Expect_NotNull_Fmt(pointer, fmtString, ...) {}

    #define AssertEval_Null_Fmt(pointer, fmtString, ...) { pointer; }
    #define AssertEval_NotNull_Fmt(pointer, fmtString, ...) { pointer; }

    #define ExpectEval_Null_Fmt(pointer, fmtString, ...) { pointer; }
    #define ExpectEval_NotNull_Fmt(pointer, fmtString, ...) { pointer; }

    #define Assert_True_Fmt(expression, fmtString, ...) {}
    #define Assert_False_Fmt(expression, fmtString, ...) {}

    #define Expect_True_Fmt(expression, fmtString, ...) {}
    #define Expect_False_Fmt(expression, fmtString, ...) {}

    #define AssertEval_True_Fmt(expression, fmtString, ...) { expression; }
    #define AssertEval_False_Fmt(expression, fmtString, ...) { expression; }

    #define ExpectEval_True_Fmt(expression, fmtString, ...) { expression; }
    #define ExpectEval_False_Fmt(expression, fmtString, ...) { expression; }

    #define Assert_Eq_Fmt(lhs, rhs, fmtString, ...) {}
    #define Assert_Ne_Fmt(lhs, rhs, fmtString, ...) {}
    #define Assert_Lt_Fmt(lhs, rhs, fmtString, ...) {}
    #define Assert_Le_Fmt(lhs, rhs, fmtString, ...) {}
    #define Assert_Gt_Fmt(lhs, rhs, fmtString, ...) {}
    #define Assert_Ge_Fmt(lhs, rhs, fmtString, ...) {}

    #define Expect_Eq_Fmt(lhs, rhs, fmtString, ...) {}
    #define Expect_Ne_Fmt(lhs, rhs, fmtString, ...) {}
    #define Expect_Lt_Fmt(lhs, rhs, fmtString, ...) {}
    #define Expect_Le_Fmt(lhs, rhs, fmtString, ...) {}
    #define Expect_Gt_Fmt(lhs, rhs, fmtString, ...) {}
    #define Expect_Ge_Fmt(lhs, rhs, fmtString, ...) {}

    #define AssertEval_Eq_Fmt(lhs_evaluated, rhs_discarded, fmtString, ...) { lhs_evaluated; }
    #define AssertEval_Ne_Fmt(lhs_evaluated, rhs_discarded, fmtString, ...) { lhs_evaluated; }
    #define AssertEval_Lt_Fmt(lhs_evaluated, rhs_discarded, fmtString, ...) { lhs_evaluated; }
    #define AssertEval_Le_Fmt(lhs_evaluated, rhs_discarded, fmtString, ...) { lhs_evaluated; }
    #define AssertEval_Gt_Fmt(lhs_evaluated, rhs_discarded, fmtString, ...) { lhs_evaluated; }
    #define AssertEval_Ge_Fmt(lhs_evaluated, rhs_discarded, fmtString, ...) { lhs_evaluated; }

    #define ExpectEval_Eq_Fmt(lhs_evaluated, rhs_discarded, fmtString, ...) { lhs_evaluated; }
    #define ExpectEval_Ne_Fmt(lhs_evaluated, rhs_discarded, fmtString, ...) { lhs_evaluated; }
    #define ExpectEval_Lt_Fmt(lhs_evaluated, rhs_discarded, fmtString, ...) { lhs_evaluated; }
    #define ExpectEval_Le_Fmt(lhs_evaluated, rhs_discarded, fmtString, ...) { lhs_evaluated; }
    #define ExpectEval_Gt_Fmt(lhs_evaluated, rhs_discarded, fmtString, ...) { lhs_evaluated; }
    #define ExpectEval_Ge_Fmt(lhs_evaluated, rhs_discarded, fmtString, ...) { lhs_evaluated; }

    #define Assert_NoEntry_Fmt(fmtString, ...) {}
    #define Assert_NoReentry_Fmt(fmtString, ...) {}

    #define Expect_NoEntry_Fmt(fmtString, ...) {}
    #define Expect_NoReentry_Fmt(fmtString, ...) {}

    #define Assert_Code(code) {}

// clang-format on

#endif

#if ADHOC_SLOW_ASSERTIONS_ON

// Non-fmt versions

    #define Assert_Null_Slow(pointer)    ADHOC_ASSERT_IMPLEMENTATION_NULL(true, pointer, true, "")
    #define Assert_NotNull_Slow(pointer) ADHOC_ASSERT_IMPLEMENTATION_NULL(true, pointer, false, "")

    #define Expect_Null_Slow(pointer)    ADHOC_ASSERT_IMPLEMENTATION_NULL(false, pointer, true, "")
    #define Expect_NotNull_Slow(pointer) ADHOC_ASSERT_IMPLEMENTATION_NULL(false, pointer, false, "")

    #define AssertEval_Null_Slow(pointer)    ADHOC_ASSERT_IMPLEMENTATION_NULL(true, pointer, true, "")
    #define AssertEval_NotNull_Slow(pointer) ADHOC_ASSERT_IMPLEMENTATION_NULL(true, pointer, false, "")

    #define ExpectEval_Null_Slow(pointer)    ADHOC_ASSERT_IMPLEMENTATION_NULL(false, pointer, true, "")
    #define ExpectEval_NotNull_Slow(pointer) ADHOC_ASSERT_IMPLEMENTATION_NULL(false, pointer, false, "")

    #define Assert_True_Slow(expression)  ADHOC_ASSERT_IMPLEMENTATION_BOOLEAN(true, expression, true, "")
    #define Assert_False_Slow(expression) ADHOC_ASSERT_IMPLEMENTATION_BOOLEAN(true, expression, false, "")

    #define Expect_True_Slow(expression)  ADHOC_ASSERT_IMPLEMENTATION_BOOLEAN(false, expression, true, "")
    #define Expect_False_Slow(expression) ADHOC_ASSERT_IMPLEMENTATION_BOOLEAN(false, expression, false, "")

    #define AssertEval_True_Slow(expression)  ADHOC_ASSERT_IMPLEMENTATION_BOOLEAN(true, expression, true, "")
    #define AssertEval_False_Slow(expression) ADHOC_ASSERT_IMPLEMENTATION_BOOLEAN(true, expression, false, "")

    #define ExpectEval_True_Slow(expression)  ADHOC_ASSERT_IMPLEMENTATION_BOOLEAN(false, expression, true, "")
    #define ExpectEval_False_Slow(expression) ADHOC_ASSERT_IMPLEMENTATION_BOOLEAN(false, expression, false, "")

    #define Assert_Eq_Slow(lhs, rhs) ADHOC_ASSERT_IMPLEMENTATION_BINARY(true, lhs, rhs, ==, "")
    #define Assert_Ne_Slow(lhs, rhs) ADHOC_ASSERT_IMPLEMENTATION_BINARY(true, lhs, rhs, !=, "")
    #define Assert_Lt_Slow(lhs, rhs) ADHOC_ASSERT_IMPLEMENTATION_BINARY(true, lhs, rhs, <, "")
    #define Assert_Le_Slow(lhs, rhs) ADHOC_ASSERT_IMPLEMENTATION_BINARY(true, lhs, rhs, <=, "")
    #define Assert_Gt_Slow(lhs, rhs) ADHOC_ASSERT_IMPLEMENTATION_BINARY(true, lhs, rhs, >, "")
    #define Assert_Ge_Slow(lhs, rhs) ADHOC_ASSERT_IMPLEMENTATION_BINARY(true, lhs, rhs, >=, "")

    #define Expect_Eq_Slow(lhs, rhs) ADHOC_ASSERT_IMPLEMENTATION_BINARY(false, lhs, rhs, ==, "")
    #define Expect_Ne_Slow(lhs, rhs) ADHOC_ASSERT_IMPLEMENTATION_BINARY(false, lhs, rhs, !=, "")
    #define Expect_Lt_Slow(lhs, rhs) ADHOC_ASSERT_IMPLEMENTATION_BINARY(false, lhs, rhs, <, "")
    #define Expect_Le_Slow(lhs, rhs) ADHOC_ASSERT_IMPLEMENTATION_BINARY(false, lhs, rhs, <=, "")
    #define Expect_Gt_Slow(lhs, rhs) ADHOC_ASSERT_IMPLEMENTATION_BINARY(false, lhs, rhs, >, "")
    #define Expect_Ge_Slow(lhs, rhs) ADHOC_ASSERT_IMPLEMENTATION_BINARY(false, lhs, rhs, >=, "")

    #define AssertEval_Eq_Slow(lhs_evaluated, rhs_discarded)                                                           \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(true, lhs_evaluated, rhs_discarded, ==, "")
    #define AssertEval_Ne_Slow(lhs_evaluated, rhs_discarded)                                                           \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(true, lhs_evaluated, rhs_discarded, !=, "")
    #define AssertEval_Lt_Slow(lhs_evaluated, rhs_discarded)                                                           \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(true, lhs_evaluated, rhs_discarded, <, "")
    #define AssertEval_Le_Slow(lhs_evaluated, rhs_discarded)                                                           \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(true, lhs_evaluated, rhs_discarded, <=, "")
    #define AssertEval_Gt_Slow(lhs_evaluated, rhs_discarded)                                                           \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(true, lhs_evaluated, rhs_discarded, >, "")
    #define AssertEval_Ge_Slow(lhs_evaluated, rhs_discarded)                                                           \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(true, lhs_evaluated, rhs_discarded, >=, "")

    #define ExpectEval_Eq_Slow(lhs_evaluated, rhs_discarded)                                                           \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(false, lhs_evaluated, rhs_discarded, ==, "")
    #define ExpectEval_Ne_Slow(lhs_evaluated, rhs_discarded)                                                           \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(false, lhs_evaluated, rhs_discarded, !=, "")
    #define ExpectEval_Lt_Slow(lhs_evaluated, rhs_discarded)                                                           \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(false, lhs_evaluated, rhs_discarded, <, "")
    #define ExpectEval_Le_Slow(lhs_evaluated, rhs_discarded)                                                           \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(false, lhs_evaluated, rhs_discarded, <=, "")
    #define ExpectEval_Gt_Slow(lhs_evaluated, rhs_discarded)                                                           \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(false, lhs_evaluated, rhs_discarded, >, "")
    #define ExpectEval_Ge_Slow(lhs_evaluated, rhs_discarded)                                                           \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(false, lhs_evaluated, rhs_discarded, >=, "")

// Fmt versions

    #define Assert_Null_Slow_Fmt(pointer, fmtString, ...)                                                              \
        ADHOC_ASSERT_IMPLEMENTATION_NULL(true, pointer, true, fmt::format(fmtString, __VA_ARGS__))
    #define Assert_NotNull_Slow_Fmt(pointer, fmtString, ...)                                                           \
        ADHOC_ASSERT_IMPLEMENTATION_NULL(true, pointer, false, fmt::format(fmtString, __VA_ARGS__))

    #define Expect_Null_Slow_Fmt(pointer, fmtString, ...)                                                              \
        ADHOC_ASSERT_IMPLEMENTATION_NULL(false, pointer, true, fmt::format(fmtString, __VA_ARGS__))
    #define Expect_NotNull_Slow_Fmt(pointer, fmtString, ...)                                                           \
        ADHOC_ASSERT_IMPLEMENTATION_NULL(false, pointer, false, fmt::format(fmtString, __VA_ARGS__))

    #define AssertEval_Null_Slow_Fmt(pointer, fmtString, ...)                                                          \
        ADHOC_ASSERT_IMPLEMENTATION_NULL(true, pointer, true, fmt::format(fmtString, __VA_ARGS__))
    #define AssertEval_NotNull_Slow_Fmt(pointer, fmtString, ...)                                                       \
        ADHOC_ASSERT_IMPLEMENTATION_NULL(true, pointer, false, fmt::format(fmtString, __VA_ARGS__))

    #define ExpectEval_Null_Slow_Fmt(pointer, fmtString, ...)                                                          \
        ADHOC_ASSERT_IMPLEMENTATION_NULL(false, pointer, true, fmt::format(fmtString, __VA_ARGS__))
    #define ExpectEval_NotNull_Slow_Fmt(pointer, fmtString, ...)                                                       \
        ADHOC_ASSERT_IMPLEMENTATION_NULL(false, pointer, false, fmt::format(fmtString, __VA_ARGS__))

    #define Assert_True_Slow_Fmt(expression, fmtString, ...)                                                           \
        ADHOC_ASSERT_IMPLEMENTATION_BOOLEAN(true, expression, true, fmt::format(fmtString, __VA_ARGS__))
    #define Assert_False_Slow_Fmt(expression, fmtString, ...)                                                          \
        ADHOC_ASSERT_IMPLEMENTATION_BOOLEAN(true, expression, false, fmt::format(fmtString, __VA_ARGS__))

    #define Expect_True_Slow_Fmt(expression, fmtString, ...)                                                           \
        ADHOC_ASSERT_IMPLEMENTATION_BOOLEAN(false, expression, true, fmt::format(fmtString, __VA_ARGS__))
    #define Expect_False_Slow_Fmt(expression, fmtString, ...)                                                          \
        ADHOC_ASSERT_IMPLEMENTATION_BOOLEAN(false, expression, false, fmt::format(fmtString, __VA_ARGS__))

    #define AssertEval_True_Slow_Fmt(expression, fmtString, ...)                                                       \
        ADHOC_ASSERT_IMPLEMENTATION_BOOLEAN(true, expression, true, fmt::format(fmtString, __VA_ARGS__))
    #define AssertEval_False_Slow_Fmt(expression, fmtString, ...)                                                      \
        ADHOC_ASSERT_IMPLEMENTATION_BOOLEAN(true, expression, false, fmt::format(fmtString, __VA_ARGS__))

    #define ExpectEval_True_Slow_Fmt(expression, fmtString, ...)                                                       \
        ADHOC_ASSERT_IMPLEMENTATION_BOOLEAN(false, expression, true, fmt::format(fmtString, __VA_ARGS__))
    #define ExpectEval_False_Slow_Fmt(expression, fmtString, ...)                                                      \
        ADHOC_ASSERT_IMPLEMENTATION_BOOLEAN(false, expression, false, fmt::format(fmtString, __VA_ARGS__))

    #define Assert_Eq_Slow_Fmt(lhs, rhs, fmtString, ...)                                                               \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(true, lhs, rhs, ==, fmt::format(fmtString, __VA_ARGS__))
    #define Assert_Ne_Slow_Fmt(lhs, rhs, fmtString, ...)                                                               \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(true, lhs, rhs, !=, fmt::format(fmtString, __VA_ARGS__))
    #define Assert_Lt_Slow_Fmt(lhs, rhs, fmtString, ...)                                                               \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(true, lhs, rhs, <, fmt::format(fmtString, __VA_ARGS__))
    #define Assert_Le_Slow_Fmt(lhs, rhs, fmtString, ...)                                                               \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(true, lhs, rhs, <=, fmt::format(fmtString, __VA_ARGS__))
    #define Assert_Gt_Slow_Fmt(lhs, rhs, fmtString, ...)                                                               \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(true, lhs, rhs, >, fmt::format(fmtString, __VA_ARGS__))
    #define Assert_Ge_Slow_Fmt(lhs, rhs, fmtString, ...)                                                               \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(true, lhs, rhs, >=, fmt::format(fmtString, __VA_ARGS__))

    #define Expect_Eq_Slow_Fmt(lhs, rhs, fmtString, ...)                                                               \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(false, lhs, rhs, ==, fmt::format(fmtString, __VA_ARGS__))
    #define Expect_Ne_Slow_Fmt(lhs, rhs, fmtString, ...)                                                               \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(false, lhs, rhs, !=, fmt::format(fmtString, __VA_ARGS__))
    #define Expect_Lt_Slow_Fmt(lhs, rhs, fmtString, ...)                                                               \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(false, lhs, rhs, <, fmt::format(fmtString, __VA_ARGS__))
    #define Expect_Le_Slow_Fmt(lhs, rhs, fmtString, ...)                                                               \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(                                                                            \
            false, lhs, rhs, <=, fmt::format(fmtString, __VA_ARGS__, fmt::format(fmtString, __VA_ARGS__)))
    #define Expect_Gt_Slow_Fmt(lhs, rhs, fmtString, ...)                                                               \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(false, lhs, rhs, >, fmt::format(fmtString, __VA_ARGS__))
    #define Expect_Ge_Slow_Fmt(lhs, rhs, fmtString, ...)                                                               \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(false, lhs, rhs, >=, fmt::format(fmtString, __VA_ARGS__))

    #define AssertEval_Eq_Slow_Fmt(lhs_evaluated, rhs_discarded, fmtString, ...)                                       \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(true, lhs_evaluated, rhs_discarded, ==, fmt::format(fmtString, __VA_ARGS__))
    #define AssertEval_Ne_Slow_Fmt(lhs_evaluated, rhs_discarded, fmtString, ...)                                       \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(true, lhs_evaluated, rhs_discarded, !=, fmt::format(fmtString, __VA_ARGS__))
    #define AssertEval_Lt_Slow_Fmt(lhs_evaluated, rhs_discarded, fmtString, ...)                                       \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(true, lhs_evaluated, rhs_discarded, <, fmt::format(fmtString, __VA_ARGS__))
    #define AssertEval_Le_Slow_Fmt(lhs_evaluated, rhs_discarded, fmtString, ...)                                       \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(true, lhs_evaluated, rhs_discarded, <=, fmt::format(fmtString, __VA_ARGS__))
    #define AssertEval_Gt_Slow_Fmt(lhs_evaluated, rhs_discarded, fmtString, ...)                                       \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(true, lhs_evaluated, rhs_discarded, >, fmt::format(fmtString, __VA_ARGS__))
    #define AssertEval_Ge_Slow_Fmt(lhs_evaluated, rhs_discarded, fmtString, ...)                                       \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(true, lhs_evaluated, rhs_discarded, >=, fmt::format(fmtString, __VA_ARGS__))

    #define ExpectEval_Eq_Slow_Fmt(lhs_evaluated, rhs_discarded, fmtString, ...)                                       \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(false, lhs_evaluated, rhs_discarded, ==, fmt::format(fmtString, __VA_ARGS__))
    #define ExpectEval_Ne_Slow_Fmt(lhs_evaluated, rhs_discarded, fmtString, ...)                                       \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(false, lhs_evaluated, rhs_discarded, !=, fmt::format(fmtString, __VA_ARGS__))
    #define ExpectEval_Lt_Slow_Fmt(lhs_evaluated, rhs_discarded, fmtString, ...)                                       \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(false, lhs_evaluated, rhs_discarded, <, fmt::format(fmtString, __VA_ARGS__))
    #define ExpectEval_Le_Slow_Fmt(lhs_evaluated, rhs_discarded, fmtString, ...)                                       \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(false, lhs_evaluated, rhs_discarded, <=, fmt::format(fmtString, __VA_ARGS__))
    #define ExpectEval_Gt_Slow_Fmt(lhs_evaluated, rhs_discarded, fmtString, ...)                                       \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(false, lhs_evaluated, rhs_discarded, >, fmt::format(fmtString, __VA_ARGS__))
    #define ExpectEval_Ge_Slow_Fmt(lhs_evaluated, rhs_discarded, fmtString, ...)                                       \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(false, lhs_evaluated, rhs_discarded, >=, fmt::format(fmtString, __VA_ARGS__))

    #define Assert_Code_Slow(code)                                                                                     \
        do                                                                                                             \
        {                                                                                                              \
            code                                                                                                       \
        } while (false)

#else

// clang-format off

    // Non-fmt versions

    #define Assert_Null_Slow(pointer) {}
    #define Assert_NotNull_Slow(pointer) {}

    #define Expect_Null_Slow(pointer) {}
    #define Expect_NotNull_Slow(pointer) {}

    #define AssertEval_Null_Slow(pointer) { pointer; }
    #define AssertEval_NotNull_Slow(pointer) { pointer; }

    #define ExpectEval_Null_Slow(pointer) { pointer; }
    #define ExpectEval_NotNull_Slow(pointer) { pointer; }

    #define Assert_True_Slow(expression) {}
    #define Assert_False_Slow(expression) {}

    #define Expect_True_Slow(expression) {}
    #define Expect_False_Slow(expression) {}

    #define AssertEval_True_Slow(expression) { expression; }
    #define AssertEval_False_Slow(expression) { expression; }

    #define ExpectEval_True_Slow(expression) { expression; }
    #define ExpectEval_False_Slow(expression) { expression; }

    #define Assert_Eq_Slow(lhs, rhs) {}
    #define Assert_Ne_Slow(lhs, rhs) {}
    #define Assert_Lt_Slow(lhs, rhs) {}
    #define Assert_Le_Slow(lhs, rhs) {}
    #define Assert_Gt_Slow(lhs, rhs) {}
    #define Assert_Ge_Slow(lhs, rhs) {}

    #define Expect_Eq_Slow(lhs, rhs) {}
    #define Expect_Ne_Slow(lhs, rhs) {}
    #define Expect_Lt_Slow(lhs, rhs) {}
    #define Expect_Le_Slow(lhs, rhs) {}
    #define Expect_Gt_Slow(lhs, rhs) {}
    #define Expect_Ge_Slow(lhs, rhs) {}

    #define AssertEval_Eq_Slow(lhs_evaluated, rhs_discarded) { lhs_evaluated; }
    #define AssertEval_Ne_Slow(lhs_evaluated, rhs_discarded) { lhs_evaluated; }
    #define AssertEval_Lt_Slow(lhs_evaluated, rhs_discarded) { lhs_evaluated; }
    #define AssertEval_Le_Slow(lhs_evaluated, rhs_discarded) { lhs_evaluated; }
    #define AssertEval_Gt_Slow(lhs_evaluated, rhs_discarded) { lhs_evaluated; }
    #define AssertEval_Ge_Slow(lhs_evaluated, rhs_discarded) { lhs_evaluated; }

    #define ExpectEval_Eq_Slow(lhs_evaluated, rhs_discarded) { lhs_evaluated; }
    #define ExpectEval_Ne_Slow(lhs_evaluated, rhs_discarded) { lhs_evaluated; }
    #define ExpectEval_Lt_Slow(lhs_evaluated, rhs_discarded) { lhs_evaluated; }
    #define ExpectEval_Le_Slow(lhs_evaluated, rhs_discarded) { lhs_evaluated; }
    #define ExpectEval_Gt_Slow(lhs_evaluated, rhs_discarded) { lhs_evaluated; }
    #define ExpectEval_Ge_Slow(lhs_evaluated, rhs_discarded) { lhs_evaluated; }

    // Fmt versions

    #define Assert_Null_Slow_Fmt(pointer, fmtString, ...) {}
    #define Assert_NotNull_Slow_Fmt(pointer, fmtString, ...) {}

    #define Expect_Null_Slow_Fmt(pointer, fmtString, ...) {}
    #define Expect_NotNull_Slow_Fmt(pointer, fmtString, ...) {}

    #define AssertEval_Null_Slow_Fmt(pointer, fmtString, ...) { pointer; }
    #define AssertEval_NotNull_Slow_Fmt(pointer, fmtString, ...) { pointer; }

    #define ExpectEval_Null_Slow_Fmt(pointer, fmtString, ...) { pointer; }
    #define ExpectEval_NotNull_Slow_Fmt(pointer, fmtString, ...) { pointer; }

    #define Assert_True_Slow_Fmt(expression, fmtString, ...) {}
    #define Assert_False_Slow_Fmt(expression, fmtString, ...) {}

    #define Expect_True_Slow_Fmt(expression, fmtString, ...) {}
    #define Expect_False_Slow_Fmt(expression, fmtString, ...) {}

    #define AssertEval_True_Slow_Fmt(expression, fmtString, ...) { expression; }
    #define AssertEval_False_Slow_Fmt(expression, fmtString, ...) { expression; }

    #define ExpectEval_True_Slow_Fmt(expression, fmtString, ...) { expression; }
    #define ExpectEval_False_Slow_Fmt(expression, fmtString, ...) { expression; }

    #define Assert_Eq_Slow_Fmt(lhs, rhs, fmtString, ...) {}
    #define Assert_Ne_Slow_Fmt(lhs, rhs, fmtString, ...) {}
    #define Assert_Lt_Slow_Fmt(lhs, rhs, fmtString, ...) {}
    #define Assert_Le_Slow_Fmt(lhs, rhs, fmtString, ...) {}
    #define Assert_Gt_Slow_Fmt(lhs, rhs, fmtString, ...) {}
    #define Assert_Ge_Slow_Fmt(lhs, rhs, fmtString, ...) {}

    #define Expect_Eq_Slow_Fmt(lhs, rhs, fmtString, ...) {}
    #define Expect_Ne_Slow_Fmt(lhs, rhs, fmtString, ...) {}
    #define Expect_Lt_Slow_Fmt(lhs, rhs, fmtString, ...) {}
    #define Expect_Le_Slow_Fmt(lhs, rhs, fmtString, ...) {}
    #define Expect_Gt_Slow_Fmt(lhs, rhs, fmtString, ...) {}
    #define Expect_Ge_Slow_Fmt(lhs, rhs, fmtString, ...) {}

    #define AssertEval_Eq_Slow_Fmt(lhs_evaluated, rhs_discarded, fmtString, ...) { lhs_evaluated; }
    #define AssertEval_Ne_Slow_Fmt(lhs_evaluated, rhs_discarded, fmtString, ...) { lhs_evaluated; }
    #define AssertEval_Lt_Slow_Fmt(lhs_evaluated, rhs_discarded, fmtString, ...) { lhs_evaluated; }
    #define AssertEval_Le_Slow_Fmt(lhs_evaluated, rhs_discarded, fmtString, ...) { lhs_evaluated; }
    #define AssertEval_Gt_Slow_Fmt(lhs_evaluated, rhs_discarded, fmtString, ...) { lhs_evaluated; }
    #define AssertEval_Ge_Slow_Fmt(lhs_evaluated, rhs_discarded, fmtString, ...) { lhs_evaluated; }

    #define ExpectEval_Eq_Slow_Fmt(lhs_evaluated, rhs_discarded, fmtString, ...) { lhs_evaluated; }
    #define ExpectEval_Ne_Slow_Fmt(lhs_evaluated, rhs_discarded, fmtString, ...) { lhs_evaluated; }
    #define ExpectEval_Lt_Slow_Fmt(lhs_evaluated, rhs_discarded, fmtString, ...) { lhs_evaluated; }
    #define ExpectEval_Le_Slow_Fmt(lhs_evaluated, rhs_discarded, fmtString, ...) { lhs_evaluated; }
    #define ExpectEval_Gt_Slow_Fmt(lhs_evaluated, rhs_discarded, fmtString, ...) { lhs_evaluated; }
    #define ExpectEval_Ge_Slow_Fmt(lhs_evaluated, rhs_discarded, fmtString, ...) { lhs_evaluated; }

    #define Assert_Code_Slow(code) {}

// clang-format on

#endif
