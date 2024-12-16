#include <Enterprise/Core/Misc.h>

#ifdef ENTERPRISE_WINDOWS
    #include "Misc_Win.h"
#endif // ENTERPRISE_WINDOWS

#if defined(__APPLE__) && defined(__MACH__)
    #include "Misc_Mac.h"
#endif // defined(__APPLE__) && defined(__MACH__)

namespace Enterprise
{

void PrintBacktrace()
{
    PrintBacktrace_Implementation();
}

} // namespace Enterprise
