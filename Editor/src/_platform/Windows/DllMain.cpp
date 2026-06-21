#include <windows.h>

// HACK: Defining DllMain prevents a bug where DllMain in dependencies are invoked multiple times
BOOL WINAPI DllMain([[maybe_unused]] HINSTANCE hinstDLL,
                    [[maybe_unused]] DWORD fdwReason,
                    [[maybe_unused]] LPVOID lpvReserved)
{
    return TRUE;
}
