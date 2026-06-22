#pragma once

#include <Engine/Core/PlatformAbstraction.h>
#include PLATFORM_HEADER(StackAlloc.h)

#ifndef STACK_ALLOC
static_assert(false, "STACK_ALLOC() is not implemented for this platform");
#endif
