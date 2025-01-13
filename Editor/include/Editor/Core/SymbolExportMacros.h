#pragma once

#if ADHOC_WINDOWS
    #if ADHOC_EDITOR_PROJECT
        #define EDITOR_API __declspec(dllexport)
    #else
        #define EDITOR_API __declspec(dllimport)
    #endif
#else
    #define EDITOR_API
#endif
