#pragma once

#include <Editor/Core/EditorConfigurationMode.h>
#include <Editor/Core/SymbolExportMacros.h>

namespace Editor
{

struct EDITOR_API EditorState
{
public:
    ConfigurationMode currentConfigMode = ConfigurationMode::Release;
    bool isDeveloperMode                = false;

    static const EditorState& GetInstance();

    EditorState()  = default;
    ~EditorState() = default;

    EditorState(const EditorState&)            = delete;
    EditorState& operator=(const EditorState&) = delete;

private:
    friend EDITOR_API EditorState& GetMutableEditorState();
};

#if ADHOC_INTERNAL
EDITOR_API EditorState& GetMutableEditorState();
#endif

} // namespace Editor
