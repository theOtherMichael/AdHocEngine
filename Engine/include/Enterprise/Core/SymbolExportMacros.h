#pragma once

#ifdef ENTERPRISE_WINDOWS
    #ifdef ENTERPRISE_EDITOR
        #ifdef ENTERPRISE_ENGINE
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
