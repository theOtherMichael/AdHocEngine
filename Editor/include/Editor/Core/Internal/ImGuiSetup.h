#pragma once

namespace Editor::Internal
{

void InitializeImGui();

void ShutdownImGui();

void StartImGuiFrame();

void PumpImGuiPlatformWindows();

void EndImGuiFrame();

} // namespace Editor::Internal
