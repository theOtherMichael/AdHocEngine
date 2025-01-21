#include <Editor/Core/EditorState.h>

namespace Editor
{

const EditorState& EditorState::GetInstance()
{
    const auto& constInstance = GetMutableEditorState();
    return constInstance;
}

EditorState& GetMutableEditorState()
{
    static EditorState instance;
    return instance;
}

} // namespace Editor
