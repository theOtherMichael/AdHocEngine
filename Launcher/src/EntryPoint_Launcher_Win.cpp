#ifndef _WIN32
static_assert(false);
#endif // _WIN32

#include <cassert>
#include <chrono>
#include <filesystem>
#include <iostream>
#include <string>
#include <thread>

#include <Editor/EditorReloadFlags.h>

#include "PlatformHelpers_Launcher_Win.h"
#include "DebuggerControls_Win.h"

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif // WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <objbase.h>

namespace fs = std::filesystem;
using namespace std::chrono_literals;
using std::chrono::steady_clock;

static HMODULE CopyAndLoadEditorModule(unsigned char reloadFlags)
{
    std::string configName;
    std::string configSuffix;

    switch (reloadFlags & EditorReloadFlag_ConfigMask)
    {
    case EditorReloadFlag_Debug:
        configName   = "Debug";
        configSuffix = "D";
        break;
    case EditorReloadFlag_Dev:
        configName   = "Dev";
        configSuffix = "";
        break;
    case EditorReloadFlag_Release:
        configName   = "Release";
        configSuffix = "";
        break;
    default: assert(false); break;
    }

    fs::path builtEngineDllPath =
        "build/" + configName + "/Engine/Engine" + configSuffix + ".dll";
    fs::path copiedEngineDllPath =
        "build/" + configName + "/Engine" + configSuffix + ".dll";
    fs::path builtEnginePdbPath =
        "build/" + configName + "/Engine/Engine" + configSuffix + ".pdb";
    fs::path copiedEnginePdbPath =
        "build/" + configName + "/Engine" + configSuffix + ".pdb";
    fs::path builtEditorDllPath =
        "build/" + configName + "/Editor/Editor" + configSuffix + ".dll";
    fs::path copiedEditorDllPath =
        "build/" + configName + "/Editor" + configSuffix + ".dll";
    fs::path builtEditorPdbPath =
        "build/" + configName + "/Editor/Editor" + configSuffix + ".pdb";
    fs::path copiedEditorPdbPath =
        "build/" + configName + "/Editor" + configSuffix + ".pdb";

    if (!fs::exists(copiedEngineDllPath))
        reloadFlags |= EditorReloadFlag_Engine;

    if ((reloadFlags & EditorReloadFlag_Engine) != 0)
    {
        std::error_code ec;
        fs::copy_file(builtEngineDllPath,
                      copiedEngineDllPath,
                      fs::copy_options::overwrite_existing,
                      ec);

        if (ec)
            std::cout << "Could not copy Engine DLL! " << ec.message() << "\n";

        fs::copy_file(builtEnginePdbPath,
                      copiedEnginePdbPath,
                      fs::copy_options::overwrite_existing,
                      ec);

        if (ec)
            std::cout << "Could not copy Engine PDB! " << ec.message() << "\n";
    }

    if (!fs::exists(builtEditorDllPath))
    {
        std::cout << "Editor DLL not found. Waiting for build to finish...\n";

        for (auto endTime = steady_clock::now() + 15s; steady_clock::now() < endTime;)
        {
            std::this_thread::sleep_for(300ms);
            if (fs::exists(builtEditorDllPath))
                break;
        }

        reloadFlags |= EditorReloadFlag_Editor;
    }

    if ((reloadFlags & EditorReloadFlag_Editor) != 0)
    {
        std::error_code ec;
        fs::copy_file(builtEditorDllPath,
                      copiedEditorDllPath,
                      fs::copy_options::overwrite_existing,
                      ec);

        if (ec)
            std::cout << "Could not copy Editor DLL! " << ec.message() << "\n";

        fs::copy_file(builtEditorPdbPath,
                      copiedEditorPdbPath,
                      fs::copy_options::overwrite_existing,
                      ec);

        if (ec)
            std::cout << "Could not copy Editor PDB! " << ec.message() << "\n";
    }

    if (!SetDllDirectory(fs::canonical("build\\" + configName + "\\").wstring().c_str()))
    {
        std::cout << "SetDllDirectory() error: " << GetLastErrorAsString() << "\n";
    }

    return LoadLibraryEx(UTF8toWCHAR("Editor" + configSuffix + ".dll").c_str(),
                         NULL,
                         LOAD_LIBRARY_SEARCH_USER_DIRS | LOAD_LIBRARY_SEARCH_SYSTEM32);
}

int main(int argc, char* argv[])
{
    bool isDevelopmentMode = false;
    bool isDebugMode       = false;
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--development") == 0)
            isDevelopmentMode = true;
        else if (strcmp(argv[i], "--debug") == 0)
            isDebugMode = true;
    }
    std::cout << "Development Mode: " << isDevelopmentMode << "\n";
    std::cout << "Debug Mode: " << isDebugMode << "\n";

    bool isComInitialized     = false;
    unsigned char reloadFlags = EditorReloadFlag_None;

    if (isDevelopmentMode)
    {
        reloadFlags = EditorReloadFlag_Engine | EditorReloadFlag_Editor;
#ifdef ENTERPRISE_DEBUG
        reloadFlags |= EditorReloadFlag_Debug;
#elif defined(ENTERPRISE_DEV)
        reloadFlags |= EditorReloadFlag_Dev;
#elif defined(ENTERPRISE_RELEASE)
        reloadFlags |= EditorReloadFlag_Release;
#endif // ENTERPRISE_DEBUG

        isComInitialized = CoInitialize(NULL) == S_OK;
        if (!isComInitialized)
        {
            std::cout
                << "CoInitialize() failed! Debugger will not automatically reattach.\n";
        }
    }

    bool isDebuggerAttached = IsDebuggerPresent();
    if (isDebuggerAttached && isComInitialized)
    {
        std::cout << "Debugger detected!\n";
        DetachDebugger(true);
    }

    HMODULE editorModuleHandle                           = NULL;
    unsigned char (*editorMainFunctionPtr)(int, char*[]) = nullptr;

    do
    {
        if (isDevelopmentMode)
        {
            editorModuleHandle = CopyAndLoadEditorModule(reloadFlags);
        }
        else
        {
            if (isDebugMode)
                editorModuleHandle = LoadLibrary(L"EditorD.dll");
            else
                editorModuleHandle = LoadLibrary(L"Editor.dll");
        }

        if (editorModuleHandle == NULL)
        {
            std::cout << "Editor DLL load failure! " << GetLastErrorAsString() << "\n";
            return EXIT_FAILURE;
        }

        editorMainFunctionPtr = (unsigned char (*)(int, char*[]))GetProcAddress(
            editorModuleHandle, "EditorMain");

        if (editorMainFunctionPtr == nullptr)
        {
            std::cout << "Editor GetProcAddress failure! " << GetLastErrorAsString()
                      << "\n";
            return EXIT_FAILURE;
        }

        if (isDebuggerAttached && isComInitialized)
            AttachDebugger();

        reloadFlags = editorMainFunctionPtr(argc, argv);

        isDebuggerAttached = IsDebuggerPresent();
        if (isDebuggerAttached && isComInitialized)
            DetachDebugger(true);

        editorMainFunctionPtr = nullptr;
        FreeLibrary(editorModuleHandle);
        editorModuleHandle = NULL;

    } while (reloadFlags != EditorReloadFlag_None);

    if (isComInitialized)
        CoUninitialize();

    return EXIT_SUCCESS;
}
