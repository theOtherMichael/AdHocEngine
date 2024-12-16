#pragma once

// Macro for exporting symbols from the editor library

#ifdef ENTERPRISE_WINDOWS
    #ifdef ENTERPRISE_INTERNAL
        #define EDITOR_API __declspec(dllexport)
    #else
        #define EDITOR_API __declspec(dllimport)
    #endif // ENTERPRISE_INTERNAL

#else // !ENTERPRISE_WINDOWS
    #define EDITOR_API
#endif // ENTERPRISE_WINDOWS
