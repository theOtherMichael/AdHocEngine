#pragma once

#include "Misc.h"

#include <fmt/format.h>

#ifdef ENTERPRISE_DEBUG
    #define ASSERTIONS_ENABLED
    #define SLOWASSERTIONS_ENABLED
#endif // ENTERPRISE_DEBUG

#ifdef ENTERPRISE_DEV
    #define ASSERTIONS_ENABLED
#endif // ENTERPRISE_DEV

#define ASSERT_IMPLEMENTATION(expression, message)                                                                     \
    if (expression) {}                                                                                                 \
    else                                                                                                               \
    {                                                                                                                  \
        bool isMessageEmpty = std::char_traits<char>::length(message) == 0;                                            \
        fmt::print(                                                                                                    \
            "Assertion failed! {0} at \"{1}\":{2}\n", isMessageEmpty ? #expression : message, __FILE__, __LINE__);     \
                                                                                                                       \
        Engine::PrintBacktrace();                                                                                      \
        DEBUGBREAK();                                                                                                  \
        abort();                                                                                                       \
    }

#ifdef ASSERTIONS_ENABLED

    #define ASSERT(condition) ASSERT_IMPLEMENTATION(condition, "")
    #define ASSERTF(condition, message) ASSERT_IMPLEMENTATION(condition, message)

    #define VERIFY(condition) ASSERT_IMPLEMENTATION(condition, "")
    #define VERIFYF(condition, message) ASSERT_IMPLEMENTATION(condition, message)

    #define ASSERT_NOENTRY() ASSERTF(false, "Call to ASSERT_NOENTRY()")
    #define ASSERT_NOREENTRY()                                                                                         \
        {                                                                                                              \
            static bool isLineReached##__LINE__ = false;                                                               \
            ASSERTF(!isLineReached##__LINE__, "Second call to ASSERT_NOREENTRY()");                                    \
            isLineReached##__LINE__ = true;                                                                            \
        }

    #define ASSERT_CODE(code)                                                                                          \
        do                                                                                                             \
        {                                                                                                              \
            code;                                                                                                      \
        } while (false);

#else

    #define ASSERT(condition)
    #define ASSERTF(condition, message)

    #define VERIFY(condition)                                                                                          \
        if (condition) {}                                                                                              \
        else {}
    #define VERIFYF(condition, message)                                                                                \
        if (condition) {}                                                                                              \
        else {}

    #define ASSERT_NOENTRY()
    #define ASSERT_NOREENTRY()

    #define ASSERT_CODE(code)

#endif // ASSERTIONS_ENABLED

#ifdef SLOWASSERTIONS_ENABLED

    #define ASSERT_SLOW(condition) ASSERT_IMPLEMENTATION(condition, "")
    #define ASSERTF_SLOW(condition, message) ASSERT_IMPLEMENTATION(condition, message)

    #define VERIFY_SLOW(condition) ASSERT_IMPLEMENTATION(condition, "")
    #define VERIFYF_SLOW(condition, message) ASSERT_IMPLEMENTATION(condition, message)

    #define ASSERT_CODE_SLOW(code)                                                                                     \
        do                                                                                                             \
        {                                                                                                              \
            code;                                                                                                      \
        } while (false);

#else
    #define ASSERT_SLOW(condition)
    #define ASSERTF_SLOW(condition, message)

    #define VERIFY_SLOW(condition)                                                                                     \
        if (condition) {}                                                                                              \
        else {}
    #define VERIFYF_SLOW(condition, message)                                                                           \
        if (condition) {}                                                                                              \
        else {}

    #define ASSERT_CODE_SLOW(code)

#endif // SLOWASSERTIONS_ENABLED
