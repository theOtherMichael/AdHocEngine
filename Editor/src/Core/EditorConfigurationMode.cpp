#include <Editor/Core/EditorConfigurationMode.h>

#include <Engine/Core/Assertions.h>

#include <fmt/format.h>

#include <string_view>

namespace Editor
{

EDITOR_API std::ostream& operator<<(std::ostream& os, const ConfigurationMode& configMode)
{
    switch (configMode)
    {
    case ::Editor::ConfigurationMode::Debug: os << "Debug"; break;
    case ::Editor::ConfigurationMode::Dev: os << "Dev"; break;
    case ::Editor::ConfigurationMode::Release: os << "Release"; break;
    default: Assert_NoEntry();
    }

    return os;
}

} // namespace Editor

auto fmt::formatter<::Editor::ConfigurationMode>::format(::Editor::ConfigurationMode configMode,
                                                         format_context& ctx) const -> format_context::iterator
{
    std::string_view name = "undefined";

    switch (configMode)
    {
    case ::Editor::ConfigurationMode::Debug: name = "Debug"; break;
    case ::Editor::ConfigurationMode::Dev: name = "Dev"; break;
    case ::Editor::ConfigurationMode::Release: name = "Release"; break;
    default: Assert_NoEntry();
    }

    return formatter<string_view>::format(name, ctx);
}
