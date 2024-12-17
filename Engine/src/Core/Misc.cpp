#include <Enterprise/Core/Misc.h>

#ifdef ENTERPRISE_WINDOWS
    #include "Misc_Win.h"
#endif // ENTERPRISE_WINDOWS

#if ENTERPRISE_MACOS
    #include "Misc_Mac.h"
#endif // ENTERPRISE_MACOS

namespace Enterprise
{

void PrintBacktrace()
{
    PrintBacktrace_Implementation();
}

} // namespace Enterprise
