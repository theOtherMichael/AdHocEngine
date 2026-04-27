#pragma once

#include <Engine/Core/SymbolExportMacros.h>

#include <filesystem>
#include <string>

#define CONCATENATE(left, right) left##right

#define STRINGIFY_IMPLEMENTATION(macro) #macro

#define STRINGIFY(macro) STRINGIFY_IMPLEMENTATION(macro)

#define BIT(n) (1ull << (n))

namespace Engine
{

/// Portably allocate on the extended stack.
ENGINE_API void* StackAlloc(size_t size);

/// Obtain the current call stack.
ENGINE_API std::string GetBacktrace();

// TODO: Move this into different header? Runtime.h, maybe?

/// Obtain the path to the current executable.
/// This is the path to the editor in editor builds, and the path to the game executable in game builds.
ENGINE_API std::filesystem::path GetExecutablePath();

} // namespace Engine
