#if !(defined(__APPLE__) && defined(__MACH__))
static_assert(false);
#endif // !(defined(__APPLE__) && defined(__MACH__))

#include <mach-o/dyld.h>
#include <dlfcn.h>
#include <cstdlib>
#include <cstring>
#include <chrono>
#include <filesystem>
#include <iostream>
#include <thread>

#include <Editor/EditorReloadFlags.h>

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

static void* CopyAndLoadEditorModule(unsigned char reloadFlags)
{
    constexpr auto maxPathBufferSize = PATH_MAX + 1;
    uint32_t pathBufferLength        = maxPathBufferSize;
    char cPathToLauncher[pathBufferLength];
    if (_NSGetExecutablePath(cPathToLauncher, &pathBufferLength) != 0)
    {
        std::cerr << "Failed to get path to launcher!\n";
        return nullptr;
    }
    char realCPathToLauncher[maxPathBufferSize];
    if (realpath(cPathToLauncher, realCPathToLauncher) == NULL)
    {
        std::cerr << "Failed to resolve real path to launcher!\n";
        return nullptr;
    }

    fs::path pathToLauncher                   = realCPathToLauncher;
    fs::path pathToBuildFolder                = pathToLauncher.parent_path().parent_path();
    fs::path pathToReloadCache                = pathToBuildFolder / "Launcher/reload_cache";
    fs::path pathToEngineVcpkgInstalledFolder = pathToBuildFolder.parent_path() / "Engine/vcpkg_installed/uni-dynamic";
    fs::path pathToEditorVcpkgInstalledFolder = pathToBuildFolder.parent_path() / "Editor/vcpkg_installed/uni-dynamic";

    fs::path pathToEngineVcpkgDependencies;
    fs::path pathToEditorVcpkgDependencies;
    std::string configSuffix;

    switch (reloadFlags & EditorReloadFlag_ConfigMask)
    {
    case EditorReloadFlag_Debug:
        configSuffix                  = "D";
        pathToEngineVcpkgDependencies = pathToEngineVcpkgInstalledFolder / "debug/lib";
        pathToEditorVcpkgDependencies = pathToEditorVcpkgInstalledFolder / "debug/lib";
        std::cout << "Debug configuration selected\n";
        break;
    case EditorReloadFlag_Dev:
        configSuffix                  = "Dev";
        pathToEngineVcpkgDependencies = pathToEngineVcpkgInstalledFolder / "lib";
        pathToEditorVcpkgDependencies = pathToEditorVcpkgInstalledFolder / "lib";
        std::cout << "Dev configuration selected\n";
        break;
    case EditorReloadFlag_Release:
        configSuffix                  = "";
        pathToEngineVcpkgDependencies = pathToEngineVcpkgInstalledFolder / "lib";
        pathToEditorVcpkgDependencies = pathToEditorVcpkgInstalledFolder / "lib";
        std::cout << "Release configuration selected\n";
        break;
    default: assert(false); break;
    }

    std::string engineDylibName = "libEngine" + configSuffix + ".dylib";
    std::string engineDsymName  = "libEngine" + configSuffix + ".dylib.dsym";
    std::string editorDylibName = "libEditor" + configSuffix + ".dylib";
    std::string editorDsymName  = "libEditor" + configSuffix + ".dylib.dsym";

    fs::path builtEngineDylibPath = pathToBuildFolder / "Engine" / engineDylibName;
    fs::path builtEngineDsymPath  = pathToBuildFolder / "Engine" / engineDsymName;
    fs::path builtEditorDylibPath = pathToBuildFolder / "Editor" / editorDylibName;
    fs::path builtEditorDsymPath  = pathToBuildFolder / "Editor" / editorDsymName;

    std::cout << "Waiting for build products...\n";
    static fs::file_time_type engineDylibLastModified;
    static fs::file_time_type engineDsymLastModified;
    static fs::file_time_type editorDylibLastModified;
    static fs::file_time_type editorDsymLastModified;

    auto scheduledTimeOut = steady_clock::now() + 15s;

    WaitOnBuildProductChanged(builtEngineDylibPath, engineDylibLastModified, scheduledTimeOut);
    WaitOnBuildProductChanged(builtEngineDsymPath, engineDsymLastModified, scheduledTimeOut);
    WaitOnBuildProductChanged(builtEditorDylibPath, editorDylibLastModified, scheduledTimeOut);
    WaitOnBuildProductChanged(builtEditorDsymPath, editorDsymLastModified, scheduledTimeOut);

    std::error_code ec;

    std::cout << "Deleting old build products...\n";
    fs::remove_all(pathToReloadCache);
    fs::create_directory(pathToReloadCache);

    std::cout << "Copying new build products...\n";
    fs::copy_file(builtEngineDylibPath, pathToReloadCache / engineDylibName, ec);
    if (ec)
        std::cerr << "Failed to copy Engine dynamic library! " << ec.message() << "\n";
    fs::copy(builtEngineDsymPath, pathToReloadCache / engineDsymName, ec);
    if (ec)
        std::cerr << "Failed to copy Engine symbols! " << ec.message() << "\n";
    fs::copy_file(builtEditorDylibPath, pathToReloadCache / editorDylibName, ec);
    if (ec)
        std::cerr << "Failed to copy Editor dynamic library! " << ec.message() << "\n";
    fs::copy(builtEditorDsymPath, pathToReloadCache / editorDsymName, ec);
    if (ec)
        std::cerr << "Failed to copy Editor symbols! " << ec.message() << "\n";

    std::cout << "Copying vcpkg dependencies for Engine...\n";
    CopyFolderContents(pathToEngineVcpkgDependencies, pathToReloadCache);
    std::cout << "Copying vcpkg dependencies for Editor...\n";
    CopyFolderContents(pathToEditorVcpkgDependencies, pathToReloadCache);

    auto currentTime        = fs::file_time_type::clock::now();
    engineDylibLastModified = currentTime;
    engineDsymLastModified  = currentTime;
    editorDylibLastModified = currentTime;
    editorDsymLastModified  = currentTime;

    std::cout << "Loading editor dynamic library...\n";
    std::string relativePathToLibEditor = "@loader_path/reload_cache/" + editorDylibName;
    return dlopen(relativePathToLibEditor.c_str(), RTLD_NOW | RTLD_LOCAL | RTLD_FIRST);
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
    }

    void* editorModuleHandle                             = nullptr;
    unsigned char (*editorMainFunctionPtr)(int, char*[]) = nullptr;

    do
    {
        if (isDeveloperMode)
        {
            std::cout << "Loading editor in developer mode...\n";
            editorModuleHandle = CopyAndLoadEditorModule(reloadFlags);
        }
        else
        {
            if (isDebugMode)
            {
                std::cout << "Loadng editor in debug mode...\n";
                editorModuleHandle =
                    dlopen("@loader_path/../Frameworks/debug/libEditorD.dylib", RTLD_LAZY | RTLD_LOCAL | RTLD_FIRST);
            }
            else
            {
                std::cout << "Loadng editor in release mode...\n";
                editorModuleHandle =
                    dlopen("@loader_path/../Frameworks/libEditor.dylib", RTLD_LAZY | RTLD_LOCAL | RTLD_FIRST);
            }
        }

        if (editorModuleHandle == nullptr)
        {
            std::cerr << "Error loading editor dylib! " << dlerror() << "\n";
            return EXIT_FAILURE;
        }

        editorMainFunctionPtr = (unsigned char (*)(int, char*[]))dlsym(editorModuleHandle, "EditorMain");
        if (editorMainFunctionPtr == nullptr)
        {
            std::cerr << "Error retrieving address of EditorMain()! " << dlerror() << "\n";
            return EXIT_FAILURE;
        }

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

        editorMainFunctionPtr = nullptr;
        dlclose(editorModuleHandle);
        editorModuleHandle = nullptr;
    } while (reloadFlags != EditorReloadFlag_None);

    return EXIT_SUCCESS;
}
