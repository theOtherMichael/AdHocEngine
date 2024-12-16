#pragma once

#ifndef ENTERPRISE_WINDOWS
static_assert(false);
#endif // ENTERPRISE_WINDOWS

void AttachDebugger();
void DetachDebugger(bool waitForBreakOrEnd);
