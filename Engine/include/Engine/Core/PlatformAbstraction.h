#pragma once

#include <Engine/Core/MiscMacros.h>

// clang-format off

#if defined(ADHOC_WINDOWS)
    #define PLATFORM_HEADER(relativePath) STRINGIFY(CONCATENATE(_platform/Windows/Windows, relativePath))
#elif defined(ADHOC_MACOS)
    #define PLATFORM_HEADER(relativePath) STRINGIFY(CONCATENATE(_platform/Mac/Mac, relativePath))
#else
static_assert(false);
#endif

// clang-format on
