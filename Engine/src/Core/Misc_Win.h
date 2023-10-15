#pragma once

#ifndef _WIN32
static_assert(false);
#endif // _WIN32

#include <Enterprise/Core/SharedLibraryExports.h>

namespace Enterprise
{

void PrintBacktrace_Implementation();

} // namespace Enterprise
