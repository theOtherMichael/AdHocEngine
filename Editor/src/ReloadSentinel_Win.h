#pragma once

#include <atomic>

#ifndef _WIN32
static_assert(false);
#endif

// IMPORTANT: EntryPoint_Win.cpp in Launcher must use these exact constants!
// 0000PPCC
// C = Config
constexpr unsigned char ReloadFlag_None       = 0x0;
constexpr unsigned char ReloadFlag_Debug      = 0x1;
constexpr unsigned char ReloadFlag_Dev        = 0x2;
constexpr unsigned char ReloadFlag_Release    = 0x3;
constexpr unsigned char ReloadFlag_ConfigMask = 0x3;
// P = Project
constexpr unsigned char ReloadFlag_Engine = 0x4;
constexpr unsigned char ReloadFlag_Editor = 0x8;

namespace Editor
{

void WaitForDirChange(std::atomic_uchar* reloadFlagsOut);

} // namespace Editor
