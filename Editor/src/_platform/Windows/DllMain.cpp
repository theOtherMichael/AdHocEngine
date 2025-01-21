#include <windows.h>

// HACK: Defining DllMain prevents a bug where DllMain in dependencies are invoked multiple times
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    return TRUE;
}
