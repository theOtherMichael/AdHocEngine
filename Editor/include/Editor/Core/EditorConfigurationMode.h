#pragma once

#include <Editor/Core/SymbolExportMacros.h>

#include <fmt/format.h>

namespace Editor
{

enum class ConfigurationMode
{
    Debug,
    Dev,
    Release
};

EDITOR_API std::ostream& operator<<(std::ostream& os, const ConfigurationMode& configMode);

} // namespace Editor

template <>
struct EDITOR_API fmt::formatter<::Editor::ConfigurationMode> : formatter<string_view>
{
    auto format(::Editor::ConfigurationMode configMode, format_context& ctx) const -> format_context::iterator;
};
