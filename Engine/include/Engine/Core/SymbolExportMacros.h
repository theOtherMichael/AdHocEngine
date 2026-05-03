#pragma once

#if ADHOC_EDITOR

#if ADHOC_WINDOWS

#if ADHOC_ENGINE_PROJECT
#define ENGINE_API __declspec(dllexport)
#else
#define ENGINE_API __declspec(dllimport)
#endif

#else // ADHOC_MAC

#if ADHOC_ENGINE_PROJECT
#define ENGINE_API __attribute__((visibility("default")))
#else
#define ENGINE_API
#endif // ADHOC_ENGINE_PROJECT

#endif

#else // !ADHOC_EDITOR

#define ENGINE_API

#endif // ADHOC_EDITOR
