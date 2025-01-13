#pragma once

#if ADHOC_WINDOWS
    #if ADHOC_EDITOR
        #if ADHOC_ENGINE_PROJECT
            #define ENGINE_API __declspec(dllexport)
        #else
            #define ENGINE_API __declspec(dllimport)
        #endif
    #else
        #define ENGINE_API
    #endif
#else
    #define ENGINE_API
#endif
