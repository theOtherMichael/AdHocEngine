#pragma once

#include <Engine/Core/Misc.h>

// clang-format off

#if ADHOC_WINDOWS
#define PLATFORM_HEADER(relativePath) STRINGIFY(CONCATENATE(_platform/Windows/Windows, relativePath))
#elif ADHOC_MAC
#define PLATFORM_HEADER(relativePath) STRINGIFY(CONCATENATE(_platform/Mac/Mac, relativePath))
#else
static_assert(false, "PLATFORM_HEADER is not defined for the current platform!");
#endif

// clang-format on

#define PLATFORM_HEADER_EXISTS_IMPL(expandedPath) __has_include(expandedPath)
#define PLATFORM_HEADER_EXISTS(relativePath) PLATFORM_HEADER_EXISTS_IMPL(PLATFORM_HEADER(relativePath))
