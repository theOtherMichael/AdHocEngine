#pragma once

#include <Engine/Core/MiscMacros.h>

// clang-format off

#if ADHOC_WINDOWS
    #define PLATFORM_HEADER(relativePath) STRINGIFY(CONCATENATE(_platform/Windows/Windows, relativePath))
#elif ADHOC_MAC
    #define PLATFORM_HEADER(relativePath) STRINGIFY(CONCATENATE(_platform/Mac/Mac, relativePath))
#else
static_assert(false);
#endif

// clang-format on

#if ADHOC_WINDOWS
    #define ADHOC_PLATFORM_CURRENT_NAMESPACE Windows
#elif ADHOC_MAC
    #define ADHOC_PLATFORM_CURRENT_NAMESPACE Mac
#else
static_assert(false, "ADHOC_INJECT_PLATFORM_NAMESPACE() requires a definition for ADHOC_PLATFORM_CURRENT_NAMESPACE");
#endif

#define ADHOC_INJECT_PLATFORM_NAMESPACE                                                                                \
    namespace Platform                                                                                                 \
    {                                                                                                                  \
    namespace Windows                                                                                                  \
    {}                                                                                                                 \
    namespace Mac                                                                                                      \
    {}                                                                                                                 \
    namespace Current = ADHOC_PLATFORM_CURRENT_NAMESPACE;                                                              \
    }
