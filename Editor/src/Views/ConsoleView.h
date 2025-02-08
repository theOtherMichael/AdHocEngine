#pragma once

#include <Engine/Core/Console.h>

#include <string>

namespace Editor::Views
{

void HandleConsoleViewLogs(const Engine::Console::LogLevel logLevel, const std::string& message);

void DrawConsoleView();

} // namespace Editor::Views
