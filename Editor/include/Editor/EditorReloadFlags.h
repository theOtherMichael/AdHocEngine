#pragma once

#ifdef ENTERPRISE_INTERNAL

// IMPORTANT: Do NOT edit these while running the editor in developer mode!

constexpr unsigned char EditorReloadFlag_None    = 0x0;
constexpr unsigned char EditorReloadFlag_Debug   = 0x1;
constexpr unsigned char EditorReloadFlag_Dev     = 0x2;
constexpr unsigned char EditorReloadFlag_Release = 0x3;

#endif // ENTERPRISE_INTERNAL
