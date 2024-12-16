#include <cassert>
#include <chrono>
#include <filesystem>
#include <iostream>
#include <string>
#include <string_view>
#include <thread>

#include <Editor/EditorReloadFlags.h>

#include "DynamicLibrary/DynamicLibrary.h"
#include "Misc/Misc.h"

namespace fs = std::filesystem;
using namespace std::chrono_literals;
using std::chrono::steady_clock;

static void PerformFileOperation(const std::string_view operationMessage,
                                 const std::function<void(std::error_code&)>& operation)
{
    std::error_code ec;
    std::cout << operationMessage << "\n";
    operation(ec);
    if (ec)
        std::cerr << "File operation failure! " << ec.message() << "\n";
}

static void CopyFolderContents(const fs::path& sourceFolder, const fs::path& destinationFolder)
{
    std::error_code ec;
    auto sourceFolderIterator = fs::recursive_directory_iterator(sourceFolder, ec);
    if (ec)
    {
        std::cerr << "Failed to create iterator for " << sourceFolder << "! " << ec.message() << "\n";
        return;
    }

    for (const auto& sourceFolderEntry : sourceFolderIterator)
    {
        const fs::path& sourcePath = sourceFolderEntry.path();
        fs::path relativePath      = fs::relative(sourcePath, sourceFolder);
        fs::path targetPath        = destinationFolder / relativePath;

        if (fs::is_directory(sourcePath))
        {
            PerformFileOperation("Creating directory " + sourcePath.string() + "...",
                                 [&](std::error_code& ec) { fs::create_directories(targetPath, ec); });
        }
        else if (fs::is_regular_file(sourcePath))
        {
            PerformFileOperation("Copying file " + sourcePath.string() + "...",
                                 [&](std::error_code& ec) { fs::copy_file(sourcePath, targetPath, ec); });
        }
    }
}

static void WaitOnFileChanged(fs::path& buildProductPath,
                              fs::file_time_type& buildProductLastModified,
                              steady_clock::time_point& scheduledTimeOut)
{
    std::error_code ec;

    std::cout << "Waiting for " << buildProductPath << "...\n";
    auto engineLastBuildTime = fs::last_write_time(buildProductPath, ec);
    if (ec)
    {
        std::cerr << "Failed to get last build time for product " << buildProductPath << "! " << ec.message() << "\n";
        return;
    }

    while (fs::last_write_time(buildProductPath, ec) <= buildProductLastModified)
    {
        std::this_thread::sleep_for(500ms);
        if (steady_clock::now() > scheduledTimeOut)
            break;
    }
}

static DynamicLibrary CopyAndLoadEditorModule(unsigned char reloadFlags)
{
    fs::path pathToLauncher    = Misc::GetLauncherPath();
    fs::path pathToBuildFolder = pathToLauncher.parent_path().parent_path();
    fs::path pathToReloadCache = pathToBuildFolder / "Launcher" / "reload_cache";
    fs::path pathToRepository  = pathToBuildFolder.parent_path();

#ifdef _WIN32
    fs::path pathToEngineVcpkgDependencies = pathToRepository / "Engine\\vcpkg_installed\\dynamic\\x64-windows";
    fs::path pathToEditorVcpkgDependencies = pathToRepository / "Editor\\vcpkg_installed\\dynamic\\x64-windows";
    fs::path vcPkgOutputFolderName         = "bin";
    std::string dynamicLibraryExtension    = ".dll";
    std::string symbolsFileExtension       = ".pdb";

#elif defined(__APPLE__) && DEFINED(__MACH__)
    fs::path pathToEngineVcpkgInstalledFolder = pathToRepository / "Engine/vcpkg_installed/uni-dynamic";
    fs::path pathToEditorVcpkgInstalledFolder = pathToRepository / "Editor/vcpkg_installed/uni-dynamic";
    fs::path vcPkgOutputFolderName            = "lib";
    std::string dynamicLibraryExtension       = ".dylib";
    std::string symbolsFileExtension          = ".dylib.dsym";

#else
    static_assert(false);
#endif

    std::string configSuffix;

    switch (reloadFlags & EditorReloadFlag_ConfigMask)
    {
    case EditorReloadFlag_Debug:
        configSuffix = "D";
        pathToEngineVcpkgDependencies /= "debug" / vcPkgOutputFolderName;
        pathToEditorVcpkgDependencies /= "debug" / vcPkgOutputFolderName;
        std::cout << "Debug configuration selected\n";
        break;
    case EditorReloadFlag_Dev:
        configSuffix = "Dev";
        pathToEngineVcpkgDependencies /= vcPkgOutputFolderName;
        pathToEditorVcpkgDependencies /= vcPkgOutputFolderName;
        std::cout << "Dev configuration selected\n";
        break;
    case EditorReloadFlag_Release:
        configSuffix = "";
        pathToEngineVcpkgDependencies /= vcPkgOutputFolderName;
        pathToEditorVcpkgDependencies /= vcPkgOutputFolderName;
        std::cout << "Release configuration selected\n";
        break;
    default: assert(false); break;
    }

    std::string engineModuleName     = "Engine" + configSuffix + dynamicLibraryExtension;
    std::string engineSymbolFileName = "Engine" + configSuffix + symbolsFileExtension;
    std::string editorModuleName     = "Editor" + configSuffix + dynamicLibraryExtension;
    std::string editorSymbolFileName = "Editor" + configSuffix + symbolsFileExtension;

    fs::path builtEngineModulePath  = pathToBuildFolder / "Engine" / engineModuleName;
    fs::path builtEngineSymbolsPath = pathToBuildFolder / "Engine" / engineSymbolFileName;
    fs::path builtEditorModulePath  = pathToBuildFolder / "Editor" / editorModuleName;
    fs::path builtEditorSymbolsPath = pathToBuildFolder / "Editor" / editorSymbolFileName;

    fs::path copiedEngineModulePath  = pathToReloadCache / engineModuleName;
    fs::path copiedEngineSymbolsPath = pathToReloadCache / engineSymbolFileName;
    fs::path copiedEditorModulePath  = pathToReloadCache / editorModuleName;
    fs::path copiedEditorSymbolsPath = pathToReloadCache / editorSymbolFileName;

    static fs::file_time_type engineModuleLastModified;
    static fs::file_time_type engineSymbolsLastModified;
    static fs::file_time_type editorModuleLastModified;
    static fs::file_time_type editorSymbolsLastModified;
    auto scheduledTimeOut = steady_clock::now() + 15s;

    std::cout << "Waiting for build products...\n";
    if ((reloadFlags & EditorReloadFlag_Engine) != 0)
    {
        WaitOnFileChanged(builtEngineModulePath, engineModuleLastModified, scheduledTimeOut);
        WaitOnFileChanged(builtEngineSymbolsPath, engineSymbolsLastModified, scheduledTimeOut);
    }
    if ((reloadFlags & EditorReloadFlag_Editor) != 0)
    {
        WaitOnFileChanged(builtEditorModulePath, editorModuleLastModified, scheduledTimeOut);
        WaitOnFileChanged(builtEditorSymbolsPath, editorSymbolsLastModified, scheduledTimeOut);
    }

    PerformFileOperation(
        "Deleting reload cache...", [&](std::error_code& ec) { fs::remove_all(pathToReloadCache, ec); });
    PerformFileOperation(
        "Recreating reload cache...", [&](std::error_code& ec) { fs::create_directory(pathToReloadCache, ec); });

    PerformFileOperation("Copying engine library...",
                         [&](std::error_code& ec)
                         { fs::copy_file(builtEngineModulePath, copiedEngineModulePath, ec); });
    PerformFileOperation("Copying engine symbols...",
                         [&](std::error_code& ec) { fs::copy(builtEngineSymbolsPath, copiedEngineSymbolsPath, ec); });

    PerformFileOperation("Copying editor library...",
                         [&](std::error_code& ec)
                         { fs::copy_file(builtEditorModulePath, copiedEditorModulePath, ec); });
    PerformFileOperation("Copying editor symbols...",
                         [&](std::error_code& ec) { fs::copy(builtEditorSymbolsPath, copiedEditorSymbolsPath, ec); });

    std::cout << "Copying vcpkg dependencies for engine...\n";
    CopyFolderContents(pathToEngineVcpkgDependencies, pathToReloadCache);
    std::cout << "Copying vcpkg dependencies for editor...\n";
    CopyFolderContents(pathToEditorVcpkgDependencies, pathToReloadCache);

    auto currentTime          = fs::file_time_type::clock::now();
    engineModuleLastModified  = currentTime;
    engineSymbolsLastModified = currentTime;
    editorModuleLastModified  = currentTime;
    editorSymbolsLastModified = currentTime;

    std::cout << "Loading editor dynamic library...\n";
    return DynamicLibrary(copiedEditorModulePath);
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

#ifdef _WIN32
    fs::path pathToLibraries     = Misc::GetLauncherPath().parent_path();
    fs::path nameOfDebugModule   = "EditorD.dll";
    fs::path nameOfReleaseModule = "Editor.dll";
#elif defined(__APPLE__) && DEFINED(__MACH__)
    fs::path pathToLibraries     = "@loader_path/../Frameworks";
    fs::path nameOfDebugModule   = "libEditorD.dylib";
    fs::path nameOfReleaseModule = "libEditor.dylib";
#else
    static_assert(false);
#endif

    do
    {
        DynamicLibrary editorModule;

        if (isDeveloperMode)
        {
            editorModule = CopyAndLoadEditorModule(reloadFlags);
        }
        else if (isDebugMode)
        {
            std::cout << "Loading editor in debug mode...\n";
            editorModule = DynamicLibrary(pathToLibraries / "debug" / nameOfDebugModule);
        }
        else
        {
            std::cout << "Loading editor in release mode...\n";
            editorModule = DynamicLibrary(pathToLibraries / nameOfReleaseModule);
        }

        if (!editorModule.IsValid())
        {
            std::cerr << "Failed to load editor library, exiting...\n";
            return EXIT_FAILURE;
        }

        auto editorMainFunction = editorModule.GetFunctionPtr<unsigned char(int, char*[])>("EditorMain");
        if (!editorMainFunction)
        {
            std::cerr << "Failed to load EditorMain(), exiting...\n";
            return EXIT_FAILURE;
        }

        // if (isDebuggerAttached /* && isComInitialized*/)
        //     AttachDebugger();

        reloadFlags = editorMainFunction(argc, argv);

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
    } while (reloadFlags != EditorReloadFlag_None);

    // if (isComInitialized)
    //     CoUninitialize();

    return EXIT_SUCCESS;
}
