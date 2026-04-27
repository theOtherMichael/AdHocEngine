#pragma once

#include <Editor/Core/SymbolExportMacros.h>

#include <string_view>
#include <variant>

namespace Editor::Internal
{

struct EDITOR_API EditorExitResult
{
    int ExitCode;
};

enum class EditorReloadMode
{
    Debug,
    Release,
};

struct EDITOR_API EditorReloadResult
{
    EditorReloadMode Mode;
};

using EditorMainResult = std::variant<EditorExitResult, EditorReloadResult>;

EDITOR_API EditorMainResult EditorMain(int argc, char* argv[]);

} // namespace Editor::Internal
