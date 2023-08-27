#pragma once

#ifdef _WIN32

#ifdef EP_EDITOR

#ifdef EP_BUILD_ENGINE
#define EP_API __declspec(dllexport)
#else
#define EP_API __declspec(dllimport)
#endif // EP_BUILD_ENGINE

#else // !EP_EDITOR
#define EP_API
#endif // EP_EDITOR

#else // !_WIN32
#define EP_API
#endif // _WIN32
