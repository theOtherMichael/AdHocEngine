#pragma once

// Macro for exporting symbols from the engine library

#ifdef ENTERPRISE_WINDOWS
    #ifdef ENTERPRISE_EDITOR
        #ifdef ENTERPRISE_ENGINE
            #define ENGINE_API __declspec(dllexport)
        #else
            #define ENGINE_API __declspec(dllimport)
        #endif // ENTERPRISE_ENGINE

    #else // !ENTERPRISE_EDITOR
        #define ENGINE_API

    #endif // ENTERPRISE_EDITOR

#else // !ENTERPRISE_WINDOWS
    #define ENGINE_API
#endif // ENTERPRISE_WINDOWS
