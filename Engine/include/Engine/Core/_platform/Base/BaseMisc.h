#pragma once

#include <Engine/Core/SymbolExportMacros.h>

#include <filesystem>

namespace Engine
{

ENGINE_API std::filesystem::path GetExecutablePath();

}
