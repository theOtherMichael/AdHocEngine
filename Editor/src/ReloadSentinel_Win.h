#pragma once

#ifndef ENTERPRISE_WINDOWS
static_assert(false);
#endif

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif // WIN32_LEAN_AND_MEAN
#include <windows.h>

DWORD WINAPI WaitForEditorOrEngineRecompile(LPVOID reloadFlagsOutPtr);
