#pragma once

#if !(defined(__APPLE__) && defined(__MACH__))
static_assert(false);
#endif // !(defined(__APPLE__) && defined(__MACH__))

#include <Enterprise/Core/SymbolExportMacros.h>

namespace Enterprise
{

void PrintBacktrace_Implementation();

} // namespace Enterprise
