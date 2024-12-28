#pragma once

#ifdef ADHOC_WINDOWS
    #ifdef ADHOC_INTERNAL
        #define EDITOR_API __declspec(dllexport)
    #else
        #define EDITOR_API __declspec(dllimport)
    #endif
#else
    #define EDITOR_API
#endif
