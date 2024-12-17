#pragma once

#ifndef ENTERPRISE_MACOS
static_assert(false);
#endif // !ENTERPRISE_MACOS

#include <Enterprise/Core/SymbolExportMacros.h>

namespace Enterprise
{

void PrintBacktrace_Implementation();

} // namespace Enterprise
