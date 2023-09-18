#pragma once

#ifdef _WIN32
    #ifdef ENTERPRISE_EDITOR
        #ifdef ENTERPRISE_ENGINE
            #define ENTERPRISE_API __declspec(dllexport)
        #else
            #define ENTERPRISE_API __declspec(dllimport)
        #endif // ENTERPRISE_ENGINE

    #else // !ENTERPRISE_EDITOR
        #define ENTERPRISE_API
    #endif // ENTERPRISE_EDITOR

#else // !_WIN32
    #define ENTERPRISE_API
#endif // _WIN32
