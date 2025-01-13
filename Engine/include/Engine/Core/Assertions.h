#pragma once

#include "Misc.h"

#include <Engine/Core/Console.h>
#include <Engine/Core/Misc.h>
#include <Engine/Core/SymbolExportMacros.h>

#include <source_location>
#include <string_view>

#if ADHOC_DEBUG
    #define ADHOC_ASSERTIONS_ON 1
    #define ADHOC_SLOW_ASSERTIONS_ON 1
#endif

#if ADHOC_DEV
    #define ADHOC_ASSERTIONS_ON 1
    #define ADHOC_SLOW_ASSERTIONS_ON 0
#endif

#if ADHOC_RELEASE
    #define ADHOC_ASSERTIONS_ON 0
    #define ADHOC_SLOW_ASSERTIONS_ON 0
#endif

namespace Engine::Internal
{
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

#define ADHOC_ASSERT_IMPLEMENTATION_BOOLEAN(isFatal, expression, expected, fmtMessage)                                 \
    {                                                                                                                  \
        decltype(expression) expressionResult = expression;                                                            \
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
        decltype(leftExpression) leftExpressionResult   = leftExpression;                                              \
        decltype(rightExpression) rightExpressionResult = rightExpression;                                             \
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

    #define Assert_True(expression) ADHOC_ASSERT_IMPLEMENTATION_BOOLEAN(true, expression, true, "")
    #define Assert_False(expression) ADHOC_ASSERT_IMPLEMENTATION_BOOLEAN(true, expression, false, "")

    #define Expect_True(expression) ADHOC_ASSERT_IMPLEMENTATION_BOOLEAN(false, expression, true, "")
    #define Expect_False(expression) ADHOC_ASSERT_IMPLEMENTATION_BOOLEAN(false, expression, false, "")

    #define AssertEval_True(expression) ADHOC_ASSERT_IMPLEMENTATION_BOOLEAN(true, expression, true, "")
    #define AssertEval_False(expression) ADHOC_ASSERT_IMPLEMENTATION_BOOLEAN(true, expression, false, "")

    #define ExpectEval_True(expression) ADHOC_ASSERT_IMPLEMENTATION_BOOLEAN(false, expression, true, "")
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

    #define Assert_NoEntry() ADHOC_ASSERT_IMPLEMENTATION_NO_ENTRY(true, "Call to Assert_NoEntry()", "")
    #define Assert_NoReentry() ADHOC_ASSERT_IMPLEMENTATION_NO_REENTRY(true, "Call to Assert_NoReentry()", "")

    #define Expect_NoEntry() ADHOC_ASSERT_IMPLEMENTATION_NO_ENTRY(false, "Call to Assert_NoEntry()", "")
    #define Expect_NoReentry() ADHOC_ASSERT_IMPLEMENTATION_NO_REENTRY(false, "Call to Assert_NoReentry()", "")

// Fmt versions

    #define AssertF_True(expression, fmtString, ...)                                                                   \
        ADHOC_ASSERT_IMPLEMENTATION_BOOLEAN(true, expression, true, fmt::format(fmtString, __VA_ARGS__))
    #define AssertF_False(expression, fmtString, ...)                                                                  \
        ADHOC_ASSERT_IMPLEMENTATION_BOOLEAN(true, expression, false, fmt::format(fmtString, __VA_ARGS__))

    #define ExpectF_True(expression, fmtString, ...)                                                                   \
        ADHOC_ASSERT_IMPLEMENTATION_BOOLEAN(false, expression, true, fmt::format(fmtString, __VA_ARGS__))
    #define ExpectF_False(expression, fmtString, ...)                                                                  \
        ADHOC_ASSERT_IMPLEMENTATION_BOOLEAN(false, expression, false, fmt::format(fmtString, __VA_ARGS__))

    #define AssertEvalF_True(expression, fmtString, ...)                                                               \
        ADHOC_ASSERT_IMPLEMENTATION_BOOLEAN(true, expression, true, fmt::format(fmtString, __VA_ARGS__))
    #define AssertEvalF_False(expression, fmtString, ...)                                                              \
        ADHOC_ASSERT_IMPLEMENTATION_BOOLEAN(true, expression, false, fmt::format(fmtString, __VA_ARGS__))

    #define ExpectEvalF_True(expression, fmtString, ...)                                                               \
        ADHOC_ASSERT_IMPLEMENTATION_BOOLEAN(false, expression, true, fmt::format(fmtString, __VA_ARGS__))
    #define ExpectEvalF_False(expression, fmtString, ...)                                                              \
        ADHOC_ASSERT_IMPLEMENTATION_BOOLEAN(false, expression, false, fmt::format(fmtString, __VA_ARGS__))

    #define AssertF_Eq(lhs, rhs, fmtString, ...)                                                                       \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(true, lhs, rhs, ==, fmt::format(fmtString, __VA_ARGS__))
    #define AssertF_Ne(lhs, rhs, fmtString, ...)                                                                       \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(true, lhs, rhs, !=, fmt::format(fmtString, __VA_ARGS__))
    #define AssertF_Lt(lhs, rhs, fmtString, ...)                                                                       \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(true, lhs, rhs, <, fmt::format(fmtString, __VA_ARGS__))
    #define AssertF_Le(lhs, rhs, fmtString, ...)                                                                       \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(true, lhs, rhs, <=, fmt::format(fmtString, __VA_ARGS__))
    #define AssertF_Gt(lhs, rhs, fmtString, ...)                                                                       \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(true, lhs, rhs, >, fmt::format(fmtString, __VA_ARGS__))
    #define AssertF_Ge(lhs, rhs, fmtString, ...)                                                                       \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(true, lhs, rhs, >=, fmt::format(fmtString, __VA_ARGS__))

    #define ExpectF_Eq(lhs, rhs, fmtString, ...)                                                                       \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(false, lhs, rhs, ==, fmt::format(fmtString, __VA_ARGS__))
    #define ExpectF_Ne(lhs, rhs, fmtString, ...)                                                                       \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(false, lhs, rhs, !=, fmt::format(fmtString, __VA_ARGS__))
    #define ExpectF_Lt(lhs, rhs, fmtString, ...)                                                                       \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(false, lhs, rhs, <, fmt::format(fmtString, __VA_ARGS__))
    #define ExpectF_Le(lhs, rhs, fmtString, ...)                                                                       \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(                                                                            \
            false, lhs, rhs, <=, fmt::format(fmtString, __VA_ARGS__, fmt::format(fmtString, __VA_ARGS__)))
    #define ExpectF_Gt(lhs, rhs, fmtString, ...)                                                                       \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(false, lhs, rhs, >, fmt::format(fmtString, __VA_ARGS__))
    #define ExpectF_Ge(lhs, rhs, fmtString, ...)                                                                       \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(false, lhs, rhs, >=, fmt::format(fmtString, __VA_ARGS__))

    #define AssertEvalF_Eq(lhs_evaluated, rhs_discarded, fmtString, ...)                                               \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(true, lhs_evaluated, rhs_discarded, ==)
    #define AssertEvalF_Ne(lhs_evaluated, rhs_discarded, fmtString, ...)                                               \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(true, lhs_evaluated, rhs_discarded, !=)
    #define AssertEvalF_Lt(lhs_evaluated, rhs_discarded, fmtString, ...)                                               \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(true, lhs_evaluated, rhs_discarded, <)
    #define AssertEvalF_Le(lhs_evaluated, rhs_discarded, fmtString, ...)                                               \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(true, lhs_evaluated, rhs_discarded, <=)
    #define AssertEvalF_Gt(lhs_evaluated, rhs_discarded, fmtString, ...)                                               \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(true, lhs_evaluated, rhs_discarded, >)
    #define AssertEvalF_Ge(lhs_evaluated, rhs_discarded, fmtString, ...)                                               \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(true, lhs_evaluated, rhs_discarded, >=)

    #define ExpectEvalF_Eq(lhs_evaluated, rhs_discarded, fmtString, ...)                                               \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(false, lhs_evaluated, rhs_discarded, ==)
    #define ExpectEvalF_Ne(lhs_evaluated, rhs_discarded, fmtString, ...)                                               \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(false, lhs_evaluated, rhs_discarded, !=)
    #define ExpectEvalF_Lt(lhs_evaluated, rhs_discarded, fmtString, ...)                                               \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(false, lhs_evaluated, rhs_discarded, <)
    #define ExpectEvalF_Le(lhs_evaluated, rhs_discarded, fmtString, ...)                                               \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(false, lhs_evaluated, rhs_discarded, <=)
    #define ExpectEvalF_Gt(lhs_evaluated, rhs_discarded, fmtString, ...)                                               \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(false, lhs_evaluated, rhs_discarded, >)
    #define ExpectEvalF_Ge(lhs_evaluated, rhs_discarded, fmtString, ...)                                               \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(false, lhs_evaluated, rhs_discarded, >=)

    #define AssertF_NoEntry(fmtString, ...)                                                                            \
        ADHOC_ASSERT_IMPLEMENTATION_NO_ENTRY(true, "Call to Assert_NoEntry()", fmt::format(fmtString, __VA_ARGS__))
    #define AssertF_NoReentry(fmtString, ...)                                                                          \
        ADHOC_ASSERT_IMPLEMENTATION_NO_REENTRY(true, "Call to Assert_NoReentry()", fmt::format(fmtString, __VA_ARGS__))

    #define ExpectF_NoEntry(fmtString, ...)                                                                            \
        ADHOC_ASSERT_IMPLEMENTATION_NO_ENTRY(false, "Call to Assert_NoEntry()", fmt::format(fmtString, __VA_ARGS__))
    #define ExpectF_NoReentry(fmtString, ...)                                                                          \
        ADHOC_ASSERT_IMPLEMENTATION_NO_REENTRY(false, "Call to Assert_NoReentry()", fmt::format(fmtString, __VA_ARGS__))

    #define Assert_Code(code)                                                                                          \
        do                                                                                                             \
        {                                                                                                              \
            code                                                                                                       \
        } while (false)

#else

// clang-format off

    // Non-fmt versions

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

    #define AssertF_True(expression, fmtString, ...) {}
    #define AssertF_False(expression, fmtString, ...) {}

    #define ExpectF_True(expression, fmtString, ...) {}
    #define ExpectF_False(expression, fmtString, ...) {}

    #define AssertEvalF_True(expression, fmtString, ...) { expression; }
    #define AssertEvalF_False(expression, fmtString, ...) { expression; }

    #define ExpectEvalF_True(expression, fmtString, ...) { expression; }
    #define ExpectEvalF_False(expression, fmtString, ...) { expression; }

    #define AssertF_Eq(lhs, rhs, fmtString, ...) {}
    #define AssertF_Ne(lhs, rhs, fmtString, ...) {}
    #define AssertF_Lt(lhs, rhs, fmtString, ...) {}
    #define AssertF_Le(lhs, rhs, fmtString, ...) {}
    #define AssertF_Gt(lhs, rhs, fmtString, ...) {}
    #define AssertF_Ge(lhs, rhs, fmtString, ...) {}

    #define ExpectF_Eq(lhs, rhs, fmtString, ...) {}
    #define ExpectF_Ne(lhs, rhs, fmtString, ...) {}
    #define ExpectF_Lt(lhs, rhs, fmtString, ...) {}
    #define ExpectF_Le(lhs, rhs, fmtString, ...) {}
    #define ExpectF_Gt(lhs, rhs, fmtString, ...) {}
    #define ExpectF_Ge(lhs, rhs, fmtString, ...) {}

    #define AssertEvalF_Eq(lhs_evaluated, rhs_discarded, fmtString, ...) { lhs_evaluated; }
    #define AssertEvalF_Ne(lhs_evaluated, rhs_discarded, fmtString, ...) { lhs_evaluated; }
    #define AssertEvalF_Lt(lhs_evaluated, rhs_discarded, fmtString, ...) { lhs_evaluated; }
    #define AssertEvalF_Le(lhs_evaluated, rhs_discarded, fmtString, ...) { lhs_evaluated; }
    #define AssertEvalF_Gt(lhs_evaluated, rhs_discarded, fmtString, ...) { lhs_evaluated; }
    #define AssertEvalF_Ge(lhs_evaluated, rhs_discarded, fmtString, ...) { lhs_evaluated; }

    #define ExpectEvalF_Eq(lhs_evaluated, rhs_discarded, fmtString, ...) { lhs_evaluated; }
    #define ExpectEvalF_Ne(lhs_evaluated, rhs_discarded, fmtString, ...) { lhs_evaluated; }
    #define ExpectEvalF_Lt(lhs_evaluated, rhs_discarded, fmtString, ...) { lhs_evaluated; }
    #define ExpectEvalF_Le(lhs_evaluated, rhs_discarded, fmtString, ...) { lhs_evaluated; }
    #define ExpectEvalF_Gt(lhs_evaluated, rhs_discarded, fmtString, ...) { lhs_evaluated; }
    #define ExpectEvalF_Ge(lhs_evaluated, rhs_discarded, fmtString, ...) { lhs_evaluated; }

    #define AssertF_NoEntry(fmtString, ...) {}
    #define AssertF_NoReentry(fmtString, ...) {}

    #define ExpectF_NoEntry(fmtString, ...) {}
    #define ExpectF_NoReentry(fmtString, ...) {}

    #define Assert_Code(code) {}

// clang-format on

#endif

#if ADHOC_SLOW_ASSERTIONS_ON

// Non-fmt versions

    #define Assert_True_Slow(expression) ADHOC_ASSERT_IMPLEMENTATION_BOOLEAN(true, expression, true, "")
    #define Assert_False_Slow(expression) ADHOC_ASSERT_IMPLEMENTATION_BOOLEAN(true, expression, false, "")

    #define Expect_True_Slow(expression) ADHOC_ASSERT_IMPLEMENTATION_BOOLEAN(false, expression, true, "")
    #define Expect_False_Slow(expression) ADHOC_ASSERT_IMPLEMENTATION_BOOLEAN(false, expression, false, "")

    #define AssertEval_True_Slow(expression) ADHOC_ASSERT_IMPLEMENTATION_BOOLEAN(true, expression, true, "")
    #define AssertEval_False_Slow(expression) ADHOC_ASSERT_IMPLEMENTATION_BOOLEAN(true, expression, false, "")

    #define ExpectEval_True_Slow(expression) ADHOC_ASSERT_IMPLEMENTATION_BOOLEAN(false, expression, true, "")
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

    #define AssertF_True_Slow(expression, fmtString, ...)                                                              \
        ADHOC_ASSERT_IMPLEMENTATION_BOOLEAN(true, expression, true, fmt::format(fmtString, __VA_ARGS__))
    #define AssertF_False_Slow(expression, fmtString, ...)                                                             \
        ADHOC_ASSERT_IMPLEMENTATION_BOOLEAN(true, expression, false, fmt::format(fmtString, __VA_ARGS__))

    #define ExpectF_True_Slow(expression, fmtString, ...)                                                              \
        ADHOC_ASSERT_IMPLEMENTATION_BOOLEAN(false, expression, true, fmt::format(fmtString, __VA_ARGS__))
    #define ExpectF_False_Slow(expression, fmtString, ...)                                                             \
        ADHOC_ASSERT_IMPLEMENTATION_BOOLEAN(false, expression, false, fmt::format(fmtString, __VA_ARGS__))

    #define AssertEvalF_True_Slow(expression, fmtString, ...)                                                          \
        ADHOC_ASSERT_IMPLEMENTATION_BOOLEAN(true, expression, true, fmt::format(fmtString, __VA_ARGS__))
    #define AssertEvalF_False_Slow(expression, fmtString, ...)                                                         \
        ADHOC_ASSERT_IMPLEMENTATION_BOOLEAN(true, expression, false, fmt::format(fmtString, __VA_ARGS__))

    #define ExpectEvalF_True_Slow(expression, fmtString, ...)                                                          \
        ADHOC_ASSERT_IMPLEMENTATION_BOOLEAN(false, expression, true, fmt::format(fmtString, __VA_ARGS__))
    #define ExpectEvalF_False_Slow(expression, fmtString, ...)                                                         \
        ADHOC_ASSERT_IMPLEMENTATION_BOOLEAN(false, expression, false, fmt::format(fmtString, __VA_ARGS__))

    #define AssertF_Eq_Slow(lhs, rhs, fmtString, ...)                                                                  \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(true, lhs, rhs, ==, fmt::format(fmtString, __VA_ARGS__))
    #define AssertF_Ne_Slow(lhs, rhs, fmtString, ...)                                                                  \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(true, lhs, rhs, !=, fmt::format(fmtString, __VA_ARGS__))
    #define AssertF_Lt_Slow(lhs, rhs, fmtString, ...)                                                                  \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(true, lhs, rhs, <, fmt::format(fmtString, __VA_ARGS__))
    #define AssertF_Le_Slow(lhs, rhs, fmtString, ...)                                                                  \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(true, lhs, rhs, <=, fmt::format(fmtString, __VA_ARGS__))
    #define AssertF_Gt_Slow(lhs, rhs, fmtString, ...)                                                                  \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(true, lhs, rhs, >, fmt::format(fmtString, __VA_ARGS__))
    #define AssertF_Ge_Slow(lhs, rhs, fmtString, ...)                                                                  \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(true, lhs, rhs, >=, fmt::format(fmtString, __VA_ARGS__))

    #define ExpectF_Eq_Slow(lhs, rhs, fmtString, ...)                                                                  \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(false, lhs, rhs, ==, fmt::format(fmtString, __VA_ARGS__))
    #define ExpectF_Ne_Slow(lhs, rhs, fmtString, ...)                                                                  \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(false, lhs, rhs, !=, fmt::format(fmtString, __VA_ARGS__))
    #define ExpectF_Lt_Slow(lhs, rhs, fmtString, ...)                                                                  \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(false, lhs, rhs, <, fmt::format(fmtString, __VA_ARGS__))
    #define ExpectF_Le_Slow(lhs, rhs, fmtString, ...)                                                                  \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(                                                                            \
            false, lhs, rhs, <=, fmt::format(fmtString, __VA_ARGS__, fmt::format(fmtString, __VA_ARGS__)))
    #define ExpectF_Gt_Slow(lhs, rhs, fmtString, ...)                                                                  \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(false, lhs, rhs, >, fmt::format(fmtString, __VA_ARGS__))
    #define ExpectF_Ge_Slow(lhs, rhs, fmtString, ...)                                                                  \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(false, lhs, rhs, >=, fmt::format(fmtString, __VA_ARGS__))

    #define AssertEvalF_Eq_Slow(lhs_evaluated, rhs_discarded, fmtString, ...)                                          \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(true, lhs_evaluated, rhs_discarded, ==)
    #define AssertEvalF_Ne_Slow(lhs_evaluated, rhs_discarded, fmtString, ...)                                          \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(true, lhs_evaluated, rhs_discarded, !=)
    #define AssertEvalF_Lt_Slow(lhs_evaluated, rhs_discarded, fmtString, ...)                                          \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(true, lhs_evaluated, rhs_discarded, <)
    #define AssertEvalF_Le_Slow(lhs_evaluated, rhs_discarded, fmtString, ...)                                          \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(true, lhs_evaluated, rhs_discarded, <=)
    #define AssertEvalF_Gt_Slow(lhs_evaluated, rhs_discarded, fmtString, ...)                                          \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(true, lhs_evaluated, rhs_discarded, >)
    #define AssertEvalF_Ge_Slow(lhs_evaluated, rhs_discarded, fmtString, ...)                                          \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(true, lhs_evaluated, rhs_discarded, >=)

    #define ExpectEvalF_Eq_Slow(lhs_evaluated, rhs_discarded, fmtString, ...)                                          \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(false, lhs_evaluated, rhs_discarded, ==)
    #define ExpectEvalF_Ne_Slow(lhs_evaluated, rhs_discarded, fmtString, ...)                                          \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(false, lhs_evaluated, rhs_discarded, !=)
    #define ExpectEvalF_Lt_Slow(lhs_evaluated, rhs_discarded, fmtString, ...)                                          \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(false, lhs_evaluated, rhs_discarded, <)
    #define ExpectEvalF_Le_Slow(lhs_evaluated, rhs_discarded, fmtString, ...)                                          \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(false, lhs_evaluated, rhs_discarded, <=)
    #define ExpectEvalF_Gt_Slow(lhs_evaluated, rhs_discarded, fmtString, ...)                                          \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(false, lhs_evaluated, rhs_discarded, >)
    #define ExpectEvalF_Ge_Slow(lhs_evaluated, rhs_discarded, fmtString, ...)                                          \
        ADHOC_ASSERT_IMPLEMENTATION_BINARY(false, lhs_evaluated, rhs_discarded, >=)

    #define Assert_Code_Slow(code)                                                                                     \
        do                                                                                                             \
        {                                                                                                              \
            code                                                                                                       \
        } while (false)

#else

// clang-format off

    // Non-fmt versions

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

    #define AssertF_True_Slow(expression, fmtString, ...) {}
    #define AssertF_False_Slow(expression, fmtString, ...) {}

    #define ExpectF_True_Slow(expression, fmtString, ...) {}
    #define ExpectF_False_Slow(expression, fmtString, ...) {}

    #define AssertEvalF_True_Slow(expression, fmtString, ...) { expression; }
    #define AssertEvalF_False_Slow(expression, fmtString, ...) { expression; }

    #define ExpectEvalF_True_Slow(expression, fmtString, ...) { expression; }
    #define ExpectEvalF_False_Slow(expression, fmtString, ...) { expression; }

    #define AssertF_Eq_Slow(lhs, rhs, fmtString, ...) {}
    #define AssertF_Ne_Slow(lhs, rhs, fmtString, ...) {}
    #define AssertF_Lt_Slow(lhs, rhs, fmtString, ...) {}
    #define AssertF_Le_Slow(lhs, rhs, fmtString, ...) {}
    #define AssertF_Gt_Slow(lhs, rhs, fmtString, ...) {}
    #define AssertF_Ge_Slow(lhs, rhs, fmtString, ...) {}

    #define ExpectF_Eq_Slow(lhs, rhs, fmtString, ...) {}
    #define ExpectF_Ne_Slow(lhs, rhs, fmtString, ...) {}
    #define ExpectF_Lt_Slow(lhs, rhs, fmtString, ...) {}
    #define ExpectF_Le_Slow(lhs, rhs, fmtString, ...) {}
    #define ExpectF_Gt_Slow(lhs, rhs, fmtString, ...) {}
    #define ExpectF_Ge_Slow(lhs, rhs, fmtString, ...) {}

    #define AssertEvalF_Eq_Slow(lhs_evaluated, rhs_discarded, fmtString, ...) { lhs_evaluated; }
    #define AssertEvalF_Ne_Slow(lhs_evaluated, rhs_discarded, fmtString, ...) { lhs_evaluated; }
    #define AssertEvalF_Lt_Slow(lhs_evaluated, rhs_discarded, fmtString, ...) { lhs_evaluated; }
    #define AssertEvalF_Le_Slow(lhs_evaluated, rhs_discarded, fmtString, ...) { lhs_evaluated; }
    #define AssertEvalF_Gt_Slow(lhs_evaluated, rhs_discarded, fmtString, ...) { lhs_evaluated; }
    #define AssertEvalF_Ge_Slow(lhs_evaluated, rhs_discarded, fmtString, ...) { lhs_evaluated; }

    #define ExpectEvalF_Eq_Slow(lhs_evaluated, rhs_discarded, fmtString, ...) { lhs_evaluated; }
    #define ExpectEvalF_Ne_Slow(lhs_evaluated, rhs_discarded, fmtString, ...) { lhs_evaluated; }
    #define ExpectEvalF_Lt_Slow(lhs_evaluated, rhs_discarded, fmtString, ...) { lhs_evaluated; }
    #define ExpectEvalF_Le_Slow(lhs_evaluated, rhs_discarded, fmtString, ...) { lhs_evaluated; }
    #define ExpectEvalF_Gt_Slow(lhs_evaluated, rhs_discarded, fmtString, ...) { lhs_evaluated; }
    #define ExpectEvalF_Ge_Slow(lhs_evaluated, rhs_discarded, fmtString, ...) { lhs_evaluated; }

    #define Assert_Code_Slow(code) {}

// clang-format on

#endif
