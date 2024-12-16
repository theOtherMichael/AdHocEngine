#pragma once

#ifndef ENTERPRISE_WINDOWS
static_assert(false);
#endif // ENTERPRISE_WINDOWS

#include <Enterprise/Core/SymbolExportMacros.h>

namespace Enterprise
{

void PrintBacktrace_Implementation();

} // namespace Enterprise
