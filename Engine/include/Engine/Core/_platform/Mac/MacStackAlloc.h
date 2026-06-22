#pragma once

#include <alloca.h>

/// Portably allocate on the extended stack within the caller's stack frame.
/// Must be a macro so the allocation lives in the caller's frame, not this header's.
#define STACK_ALLOC(size) alloca(size)
