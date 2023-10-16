#pragma once

// IMPORTANT: Do NOT edit these while running the editor in development mode!

// 0000PPCC
constexpr unsigned char EditorReloadFlag_None = 0x0;
// C = Config, two-bit number
constexpr unsigned char EditorReloadFlag_Debug      = 0x1;
constexpr unsigned char EditorReloadFlag_Dev        = 0x2;
constexpr unsigned char EditorReloadFlag_Release    = 0x3;
constexpr unsigned char EditorReloadFlag_ConfigMask = 0x3;
// P = Project, two-bit bit field
constexpr unsigned char EditorReloadFlag_Engine = 0x4;
constexpr unsigned char EditorReloadFlag_Editor = 0x8;
