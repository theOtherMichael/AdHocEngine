#pragma once

#include <Editor/Core/EditorConfigurationMode.h>
#include <Editor/Core/SymbolExportMacros.h>

namespace Editor
{

struct ReloadOption
{
    ConfigurationMode configToLoad = ConfigurationMode::Release;
    bool isReloadRequested         = false;
};

EDITOR_API ReloadOption EditorMain(int argc, char* argv[]);

} // namespace Editor
