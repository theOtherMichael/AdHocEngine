#pragma once

#ifdef ENTERPRISE_WINDOWS
    #ifdef ENTERPRISE_INTERNAL
        #define EDITOR_API __declspec(dllexport)
    #else
        #define EDITOR_API __declspec(dllimport)
    #endif
#else
    #define EDITOR_API
#endif
