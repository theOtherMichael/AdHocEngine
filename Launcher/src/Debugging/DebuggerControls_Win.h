#pragma once

#ifndef _WIN32
static_assert(false);
#endif // _WIN32

void AttachDebugger();
void DetachDebugger(bool waitForBreakOrEnd);
