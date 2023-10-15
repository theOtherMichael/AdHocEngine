#include <Enterprise/Core/Misc.h>

#ifdef _WIN32
    #include "Misc_Win.h"
#endif // _WIN32

namespace Enterprise
{

void PrintBacktrace()
{
    PrintBacktrace_Implementation();
}

} // namespace Enterprise
