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

static void CopyFolderContents(const fs::path& sourceFolder, const fs::path& destinationFolder)
{
    std::error_code ec;

    auto sourceFolderIterator = fs::recursive_directory_iterator(sourceFolder, ec);
    if (ec)
        std::cerr << "Failed to create iterator for " << sourceFolder << "! " << ec.message() << "\n";

    for (const auto& sourceFolderEntry : sourceFolderIterator)
    {
        const fs::path& sourcePath = sourceFolderEntry.path();
        fs::path relativePath      = fs::relative(sourcePath, sourceFolder);
        fs::path targetPath        = destinationFolder / relativePath;

        if (fs::is_directory(sourcePath))
        {
            fs::create_directories(targetPath, ec);
            if (ec)
                std::cerr << "Failed to create directory " << targetPath << "! " << ec.message() << "\n";
        }
        else if (fs::is_regular_file(sourcePath))
        {
            fs::copy_file(sourcePath, targetPath, ec);
            if (ec)
                std::cerr << "Failed to copy file " << sourcePath << "! " << ec.message() << "\n";
        }
    }
}

void WaitOnBuildProductChanged(std::filesystem::path& buildProductPath,
                               std::filesystem::file_time_type& buildProductLastModified,
                               std::chrono::steady_clock::time_point& scheduledTimeOut)
{
    std::error_code ec;

    std::cout << "Waiting for \"" << buildProductPath << "\"...\n";
    auto engineLastBuildTime = fs::last_write_time(buildProductPath, ec);
    if (ec)
    {
        std::cerr << "Failed to get last build time for product \"" << buildProductPath << "\"! " << ec.message() << "\n";
        return;
    }

    while (fs::last_write_time(buildProductPath, ec) <= buildProductLastModified)
    {
        std::this_thread::sleep_for(500ms);
        if (steady_clock::now() > scheduledTimeOut)
            break;
    }
}

static HMODULE CopyAndLoadEditorModule(unsigned char reloadFlags)
{
    TCHAR cPathToLauncher[MAX_PATH];
    if (!GetModuleFileName(NULL, cPathToLauncher, MAX_PATH))
    {
        std::cerr << "Failed to get path to build folder!\n";
        return nullptr;
    }
    fs::path pathToLauncher    = cPathToLauncher;
    fs::path pathToBuildFolder = pathToLauncher.parent_path().parent_path();
    fs::path pathToReloadCache = pathToBuildFolder / "Launcher\\reload_cache";

    fs::path pathToRepository                 = pathToBuildFolder.parent_path();
    fs::path pathToEngineVcpkgInstalledFolder = pathToRepository / "Engine\\vcpkg_installed\\dynamic\\x64-windows";
    fs::path pathToEditorVcpkgInstalledFolder = pathToRepository / "Editor\\vcpkg_installed\\dynamic\\x64-windows";

    fs::path pathToEngineVcpkgDependencies;
    fs::path pathToEditorVcpkgDependencies;
    std::string configSuffix;

    switch (reloadFlags & EditorReloadFlag_ConfigMask)
    {
    case EditorReloadFlag_Debug:
        configSuffix                  = "D";
        pathToEngineVcpkgDependencies = pathToEngineVcpkgInstalledFolder / "debug\\bin";
        pathToEditorVcpkgDependencies = pathToEditorVcpkgInstalledFolder / "debug\\bin";
        std::cout << "Debug configuration selected\n";
        break;
    case EditorReloadFlag_Dev:
        configSuffix                  = "Dev";
        pathToEngineVcpkgDependencies = pathToEngineVcpkgInstalledFolder / "bin";
        pathToEditorVcpkgDependencies = pathToEditorVcpkgInstalledFolder / "bin";
        std::cout << "Dev configuration selected\n";
        break;
    case EditorReloadFlag_Release:
        configSuffix                  = "";
        pathToEngineVcpkgDependencies = pathToEngineVcpkgInstalledFolder / "bin";
        pathToEditorVcpkgDependencies = pathToEditorVcpkgInstalledFolder / "bin";
        std::cout << "Release configuration selected\n";
        break;
    default: assert(false); break;
    }

    std::string engineModuleName     = "Engine" + configSuffix + ".dll";
    std::string engineSymbolFileName = "Engine" + configSuffix + ".pdb";
    std::string editorModuleName     = "Editor" + configSuffix + ".dll";
    std::string editorSymbolFileName = "Editor" + configSuffix + ".pdb";

    fs::path builtEngineModulePath  = pathToBuildFolder / "Engine" / engineModuleName;
    fs::path builtEngineSymbolsPath = pathToBuildFolder / "Engine" / engineSymbolFileName;
    fs::path builtEditorModulePath  = pathToBuildFolder / "Editor" / editorModuleName;
    fs::path builtEditorSymbolsPath = pathToBuildFolder / "Editor" / editorSymbolFileName;

    std::cout << "Waiting for build products...\n";
    static fs::file_time_type engineModuleLastModified;
    static fs::file_time_type engineSymbolsLastModified;
    static fs::file_time_type editorModuleLastModified;
    static fs::file_time_type editorSymbolsLastModified;

    auto scheduledTimeOut = steady_clock::now() + 15s;

    WaitOnBuildProductChanged(builtEngineModulePath, engineModuleLastModified, scheduledTimeOut);
    WaitOnBuildProductChanged(builtEngineSymbolsPath, engineSymbolsLastModified, scheduledTimeOut);
    WaitOnBuildProductChanged(builtEditorModulePath, editorModuleLastModified, scheduledTimeOut);
    WaitOnBuildProductChanged(builtEditorSymbolsPath, editorSymbolsLastModified, scheduledTimeOut);

    std::error_code ec;
    
    std::cout << "Deleting old build products...\n";
    fs::remove_all(pathToReloadCache, ec);
    if (ec)
        std::cerr << "Failed to clear reload cache! " << ec.message() << "\n";
    fs::create_directory(pathToReloadCache, ec);
    if (ec)
        std::cerr << "Failed to recreate cache folder! " << ec.message() << "\n";

    std::cout << "Copying new build products...\n";
    fs::copy_file(builtEngineModulePath, pathToReloadCache / engineModuleName, ec);
    if (ec)
        std::cerr << "Failed to copy Engine dynamic library! " << ec.message() << "\n";
    fs::copy(builtEngineSymbolsPath, pathToReloadCache / engineSymbolFileName, ec);
    if (ec)
        std::cerr << "Failed to copy Engine symbols! " << ec.message() << "\n";
    fs::copy_file(builtEditorModulePath, pathToReloadCache / editorModuleName, ec);
    if (ec)
        std::cerr << "Failed to copy Editor dynamic library! " << ec.message() << "\n";
    fs::copy(builtEditorSymbolsPath, pathToReloadCache / editorSymbolFileName, ec);
    if (ec)
        std::cerr << "Failed to copy Editor symbols! " << ec.message() << "\n";

    std::cout << "Copying vcpkg dependencies for Engine...\n";
    CopyFolderContents(pathToEngineVcpkgDependencies, pathToReloadCache);
    std::cout << "Copying vcpkg dependencies for Editor...\n";
    CopyFolderContents(pathToEditorVcpkgDependencies, pathToReloadCache);

    auto currentTime          = fs::file_time_type::clock::now();
    engineModuleLastModified  = currentTime;
    engineSymbolsLastModified = currentTime;
    editorModuleLastModified  = currentTime;
    editorSymbolsLastModified = currentTime;

    std::cout << "Loading editor dynamic library...\n";
    return LoadLibraryEx((pathToReloadCache / editorModuleName).wstring().c_str(), NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
}

int main(int argc, char* argv[])
{
    std::cout << "Starting Enterprise...\n";

    bool isDeveloperMode = false;
    bool isDebugMode     = false;

    std::cout << "Command line arguments:\n";
    for (int i = 1; i < argc; i++)
    {
        std::cout << "  " << argv[i] << "\n";

        if (strcmp(argv[i], "--developer") == 0)
            isDeveloperMode = true;
        else if (strcmp(argv[i], "--debug") == 0)
            isDebugMode = true;
    }

    if (isDeveloperMode)
        std::cout << "--developer specified, launching in developer mode\n";
    else if (isDebugMode)
        std::cout << "--debug specified, launching in debug mode\n";

    // bool isComInitialized     = false;
    unsigned char reloadFlags = EditorReloadFlag_None;

    if (isDeveloperMode)
    {
        reloadFlags = EditorReloadFlag_Engine | EditorReloadFlag_Editor;
#ifdef ENTERPRISE_DEBUG
        reloadFlags |= EditorReloadFlag_Debug;
#elif defined(ENTERPRISE_DEV)
        reloadFlags |= EditorReloadFlag_Dev;
#elif defined(ENTERPRISE_RELEASE)
        reloadFlags |= EditorReloadFlag_Release;
#endif // ENTERPRISE_DEBUG

        // isComInitialized = CoInitialize(NULL) == S_OK;
        // if (!isComInitialized)
        //{
        //     std::cerr << "COM initialization failed! "
        //                  "Debuggers will not automatically reattach.\n";
        // }
    }

    // bool isDebuggerAttached = IsDebuggerPresent();
    // if (isDebuggerAttached && isComInitialized)
    //{
    //     std::cout << "Debugger detected!\n";
    //     DetachDebugger(true);
    // }

    HMODULE editorModuleHandle                           = NULL;
    unsigned char (*editorMainFunctionPtr)(int, char*[]) = nullptr;

    do
    {
        if (isDeveloperMode)
        {
            editorModuleHandle = CopyAndLoadEditorModule(reloadFlags);
        }
        else
        {
            if (isDebugMode)
            {
                std::cout << "Loadng editor in debug mode...\n";

                TCHAR cPathToLauncher[MAX_PATH];
                if (!GetModuleFileName(NULL, cPathToLauncher, MAX_PATH))
                {
                    std::cerr << "Failed to get path to build folder!\n";
                }
                else
                {
                    fs::path pathToLauncher       = cPathToLauncher;
                    fs::path pathToDebugEditorDll = pathToLauncher.parent_path() / "debug" / "EditorD.dll";

                    editorModuleHandle =
                        LoadLibraryEx(pathToDebugEditorDll.wstring().c_str(), NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
                }
            }
            else
            {
                std::cout << "Loadng editor in release mode...\n";
                editorModuleHandle = LoadLibrary(L"Editor.dll");
            }
        }

        if (editorModuleHandle == NULL)
        {
            std::cerr << "Error loading editor DLL! " << GetLastErrorAsString() << "\n";
            return EXIT_FAILURE;
        }

        editorMainFunctionPtr = (unsigned char (*)(int, char*[]))GetProcAddress(editorModuleHandle, "EditorMain");

        if (editorMainFunctionPtr == nullptr)
        {
            std::cerr << "Error retrieving address of EditorMain()! " << GetLastErrorAsString() << "\n";
            return EXIT_FAILURE;
        }

        // if (isDebuggerAttached /* && isComInitialized*/)
        //     AttachDebugger();

        reloadFlags = editorMainFunctionPtr(argc, argv);

        if (!isDeveloperMode)
        {
            switch (reloadFlags & EditorReloadFlag_ConfigMask)
            {
            case EditorReloadFlag_None: break;
            case EditorReloadFlag_Debug: isDebugMode = true; break;
            case EditorReloadFlag_Release: isDebugMode = false; break;
            case EditorReloadFlag_Dev:
                std::cerr << "Dev configuration was requested in Editor reload! "
                             "Dev configuration is only available in developer mode. "
                             "Release will be used instead.\n";
                isDebugMode = false;
                break;
            default: assert(false); break;
            }
        }

        // isDebuggerAttached = IsDebuggerPresent();
        // if (isDebuggerAttached && isComInitialized)
        //     DetachDebugger(true);

        editorMainFunctionPtr = nullptr;
        FreeLibrary(editorModuleHandle);
        editorModuleHandle = NULL;
    } while (reloadFlags != EditorReloadFlag_None);

    // if (isComInitialized)
    //     CoUninitialize();

    return EXIT_SUCCESS;
}
