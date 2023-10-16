#pragma once

#ifndef _WIN32
static_assert(false);
#endif

#include <atomic>

void WaitForEditorOrEngineRecompile(std::atomic_uchar* reloadFlagsOut);
