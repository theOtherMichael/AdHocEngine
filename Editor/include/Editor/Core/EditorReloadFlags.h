#pragma once

// WARNING: Don't edit this file in developer mode, it is consumed by Launcher!

#ifdef ENTERPRISE_INTERNAL

constexpr unsigned char EditorReloadFlag_None = 0x0;

// 0000PPCC
//       ^^ config bits, two-bit number

constexpr unsigned char EditorReloadFlag_Debug      = 0x1;
constexpr unsigned char EditorReloadFlag_Dev        = 0x2;
constexpr unsigned char EditorReloadFlag_Release    = 0x3;
constexpr unsigned char EditorReloadFlag_ConfigMask = 0x3;

// 0000PPCC
//     ^^ project bits, two-bit bit field

constexpr unsigned char EditorReloadFlag_Engine      = 0x4;
constexpr unsigned char EditorReloadFlag_Editor      = 0x8;
constexpr unsigned char EditorReloadFlag_ProjectMask = 0x8;

#endif // ENTERPRISE_INTERNAL
