#pragma once

#include <Engine/Core/PlatformAbstraction.h>
#include PLATFORM_HEADER(Debugging.h)

#ifndef DEBUG_BREAK
static_assert(false, "DEBUG_BREAK() is not implemented for this platform");
#endif
