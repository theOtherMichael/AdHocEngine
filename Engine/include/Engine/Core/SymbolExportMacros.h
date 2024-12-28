#pragma once

#ifdef ADHOC_WINDOWS
    #ifdef ADHOC_EDITOR
        #ifdef ADHOC_ENGINE
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
