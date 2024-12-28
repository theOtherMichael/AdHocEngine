#pragma once

#include <Enterprise/Core/MiscMacros.h>

// clang-format off

#if defined(ENTERPRISE_WINDOWS)
    #define PLATFORM_HEADER(relativePath) STRINGIFY(CONCATENATE(_platform/Windows/Windows, relativePath))
#elif defined(ENTERPRISE_MACOS)
    #define PLATFORM_HEADER(relativePath) STRINGIFY(CONCATENATE(_platform/Mac/Mac, relativePath))
#else
static_assert(false);
#endif

// clang-format on
