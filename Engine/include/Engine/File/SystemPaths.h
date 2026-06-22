#pragma once

#include <Engine/Core/SymbolExportMacros.h>

#include <filesystem>

namespace Engine
{

/// Obtain the path to the current executable.
/// This is the path to the editor in editor builds, and the path to the game executable in game builds.
ENGINE_API std::filesystem::path GetExecutablePath();

} // namespace Engine
