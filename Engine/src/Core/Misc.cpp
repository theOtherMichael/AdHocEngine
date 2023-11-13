#include <Enterprise/Core/Misc.h>

#ifdef _WIN32
    #include "Misc_Win.h"
#endif // _WIN32

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
