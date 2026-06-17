#include <Engine/Core/Debugging.h>

#include <sys/sysctl.h>
#include <sys/types.h>
#include <unistd.h>

#include <array>

static_assert(ADHOC_MAC);

namespace Engine::Platform
{

// Apple QA1361: a process is traced when P_TRACED is set in its kinfo_proc.
bool IsBeingDebuggedImpl()
{
    auto info           = kinfo_proc{};
    info.kp_proc.p_flag = 0;

    auto mib  = std::array{CTL_KERN, KERN_PROC, KERN_PROC_PID, getpid()};
    auto size = sizeof(info);

    if (sysctl(mib.data(), static_cast<u_int>(mib.size()), &info, &size, nullptr, 0) != 0)
        return false;

    return (info.kp_proc.p_flag & P_TRACED) != 0;
}

} // namespace Engine::Platform
