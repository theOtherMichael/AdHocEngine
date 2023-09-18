#ifndef _WIN32
static_assert(false);
#endif // _WIN32

#include <cassert>
#include <chrono>
#include <filesystem>
#include <iostream>
#include <string>
#include <thread>

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif // WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <objbase.h>

#include "PlatformHelpers_Win.h"
#include "Debugger_Win.h"

namespace fs = std::filesystem;
using namespace std::chrono_literals;
using std::chrono::steady_clock;

const char* developmentModeOptionString = "--development";
const char* debugModeOptionString       = "--debug";

// IMPORTANT: ReloadSentinel_Win.h in Editor must use these exact constants!
// 0000PPCC
// C = Config
constexpr unsigned char ReloadFlag_None       = 0x0;
constexpr unsigned char ReloadFlag_Debug      = 0x1;
constexpr unsigned char ReloadFlag_Dev        = 0x2;
constexpr unsigned char ReloadFlag_Release    = 0x3;
constexpr unsigned char ReloadFlag_ConfigMask = 0x3;
// P = Project
constexpr unsigned char ReloadFlag_Engine = 0x4;
constexpr unsigned char ReloadFlag_Editor = 0x8;

static HMODULE editorModuleHandle                                 = NULL;
static int (*editorMainFunctionPtr)(int, char*[], unsigned char*) = nullptr;

static void CopyAndLoadDllsForDevelopmentMode(unsigned char reloadFlags)
{
    std::string configName;
    std::string configSuffix;

    switch (reloadFlags & ReloadFlag_ConfigMask)
    {
    case ReloadFlag_Debug:
        configName   = "Debug";
        configSuffix = "D";
        break;
    case ReloadFlag_Dev:
        configName   = "Dev";
        configSuffix = "";
        break;
    case ReloadFlag_Release:
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
        reloadFlags |= ReloadFlag_Engine;

    std::error_code ec;

    if ((reloadFlags & ReloadFlag_Engine) != 0)
    {
        fs::copy_file(builtEngineDllPath,
                      copiedEngineDllPath,
                      fs::copy_options::overwrite_existing,
                      ec);

        if (ec)
            std::cout << "Could not copy Engine DLL! " << ec.message() << std::endl;

        fs::copy_file(builtEnginePdbPath,
                      copiedEnginePdbPath,
                      fs::copy_options::overwrite_existing,
                      ec);

        if (ec)
            std::cout << "Could not copy Engine PDB! " << ec.message() << std::endl;
    }

    if (!fs::exists(builtEditorDllPath))
    {
        std::cout << "Editor DLL not found. Waiting for build to finish..." << std::endl;

        for (auto endTime = steady_clock::now() + 15s; steady_clock::now() < endTime;)
        {
            std::this_thread::sleep_for(300ms);

            if (fs::exists(builtEditorDllPath))
                break;
        }

        reloadFlags |= ReloadFlag_Editor;
    }

    if ((reloadFlags & ReloadFlag_Editor) != 0)
    {
        fs::copy_file(builtEditorDllPath,
                      copiedEditorDllPath,
                      fs::copy_options::overwrite_existing,
                      ec);

        if (ec)
            std::cout << "Could not copy Editor DLL! " << ec.message() << std::endl;

        fs::copy_file(builtEditorPdbPath,
                      copiedEditorPdbPath,
                      fs::copy_options::overwrite_existing,
                      ec);

        if (ec)
            std::cout << "Could not copy Editor PDB! " << ec.message() << std::endl;
    }

    if (!SetDllDirectory(fs::canonical("build\\" + configName + "\\").wstring().c_str()))
    {
        std::cout << "SetDllDirectory() error: " << GetLastErrorAsString() << std::endl;
    }

    editorModuleHandle =
        LoadLibraryEx(UTF8toWCHAR("Editor" + configSuffix + ".dll").c_str(),
                      NULL,
                      LOAD_LIBRARY_SEARCH_USER_DIRS | LOAD_LIBRARY_SEARCH_SYSTEM32);
}

int main(int argc, char* argv[])
{
    bool isDevelopmentMode = false;
    bool isDebugMode       = false;

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], developmentModeOptionString) == 0)
            isDevelopmentMode = true;
        else if (strcmp(argv[i], debugModeOptionString) == 0)
            isDebugMode = true;
    }

    std::cout << "Development Mode: " << isDevelopmentMode << "\n";
    std::cout << "Debug Mode: " << isDebugMode << std::endl;

    unsigned char reloadFlags =
        isDebugMode ? ReloadFlag_Engine | ReloadFlag_Editor | ReloadFlag_Debug
                    : ReloadFlag_Engine | ReloadFlag_Editor | ReloadFlag_Dev;

    bool isComInitialized = false;

    if (isDevelopmentMode)
    {
#ifdef ENTERPRISE_DEBUG
        reloadFlags = ReloadFlag_Engine | ReloadFlag_Editor | ReloadFlag_Debug;
#elif defined(ENTERPRISE_DEV)
        reloadFlags = ReloadFlag_Engine | ReloadFlag_Editor | ReloadFlag_Dev;
#elif defined(ENTERPRISE_RELEASE)
        reloadFlags = ReloadFlag_Engine | ReloadFlag_Editor | ReloadFlag_Release;
#endif // ENTERPRISE_DEBUG

        isComInitialized = CoInitialize(NULL) == S_OK;

        if (!isComInitialized)
        {
            std::cout
                << "CoInitialize() failed! Debugger will not automatically reattach."
                << std::endl;
        }
    }

    bool isDebuggerAttached = IsDebuggerPresent();
    if (isDebuggerAttached && isComInitialized)
    {
        std::cout << "Debugger detected!" << std::endl;
        DetachDebugger(true);
    }

    do
    {
        if (isDevelopmentMode)
        {
            CopyAndLoadDllsForDevelopmentMode(reloadFlags);
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
            std::cout << "Editor DLL load failure! " << GetLastErrorAsString()
                      << std::endl;
            return EXIT_FAILURE;
        }

        editorMainFunctionPtr = (int (*)(int, char*[], unsigned char*))GetProcAddress(
            editorModuleHandle, "EditorMain");

        if (editorMainFunctionPtr == nullptr)
        {
            std::cout << "Editor GetProcAddress failure! " << GetLastErrorAsString()
                      << std::endl;
            return EXIT_FAILURE;
        }

        reloadFlags = ReloadFlag_None;

        if (isDebuggerAttached && isComInitialized)
            AttachDebugger();

        editorMainFunctionPtr(argc, argv, &reloadFlags);

        isDebuggerAttached = IsDebuggerPresent();
        if (isDebuggerAttached && isComInitialized)
            DetachDebugger(true);

        editorMainFunctionPtr = nullptr;
        FreeLibrary(editorModuleHandle);
        editorModuleHandle = NULL;

    } while (reloadFlags != ReloadFlag_None);

    if (isComInitialized)
        CoUninitialize();

    return EXIT_SUCCESS;
}
