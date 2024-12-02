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

static void* CopyAndLoadEditorModule(unsigned char reloadFlags)
{
    std::string configSuffix;

    switch (reloadFlags)
    {
    case EditorReloadFlag_Debug:
        configSuffix = "D";
        std::cout << "Debug configuration selected\n";
        break;
    case EditorReloadFlag_Dev:
        configSuffix = "Dev";
        std::cout << "Dev configuration selected\n";
        break;
    case EditorReloadFlag_Release:
        configSuffix = "";
        std::cout << "Release configuration selected\n";
        break;
    default: assert(false); break;
    }

    char cPathToLauncher[PATH_MAX + 1];
    uint32_t pathBufferLength = sizeof(cPathToLauncher);
    if (_NSGetExecutablePath(cPathToLauncher, &pathBufferLength) != 0)
    {
        std::cerr << "Failed to get path to build folder!\n";
        return nullptr;
    }
    char realCPathToLauncher[PATH_MAX + 1];
    if (realpath(cPathToLauncher, realCPathToLauncher) == NULL)
    {
        std::cerr << "Failed to resolve path to build folder!\n";
        return nullptr;
    }

    fs::path pathToLauncher    = realCPathToLauncher;
    fs::path pathToBuildFolder = pathToLauncher.parent_path().parent_path();

    std::string engineDylibName = "libEngine" + configSuffix + ".dylib";
    std::string engineDsymName  = "libEngine" + configSuffix + ".dylib.dsym";
    std::string editorDylibName = "libEditor" + configSuffix + ".dylib";
    std::string editorDsymName  = "libEditor" + configSuffix + ".dylib.dsym";

    fs::path builtEngineDylibPath = pathToBuildFolder / "Engine" / engineDylibName;
    fs::path builtEngineDsymPath  = pathToBuildFolder / "Engine" / engineDsymName;
    fs::path builtEditorDylibPath = pathToBuildFolder / "Editor" / editorDylibName;
    fs::path builtEditorDsymPath  = pathToBuildFolder / "Editor" / editorDsymName;

    fs::path copiedEngineDylibPath = pathToBuildFolder / "Launcher/reload_cache" / engineDylibName;
    fs::path copiedEngineDsymPath  = pathToBuildFolder / "Launcher/reload_cache" / engineDsymName;
    fs::path copiedEditorDylibPath = pathToBuildFolder / "Launcher/reload_cache" / editorDylibName;
    fs::path copiedEditorDsymPath  = pathToBuildFolder / "Launcher/reload_cache" / editorDsymName;

    static fs::file_time_type engineDylibLastModified;
    static fs::file_time_type engineDsymLastModified;
    static fs::file_time_type editorDylibLastModified;
    static fs::file_time_type editorDsymLastModified;

    std::cout << "Waiting for build products...\n";

    auto scheduledTimeOut = steady_clock::now() + 15s;

    std::cout << "Waiting for " << builtEngineDylibPath << "...";
    while (fs::last_write_time(builtEngineDylibPath) <= engineDylibLastModified)
    {
        std::this_thread::sleep_for(500ms);
        if (steady_clock::now() > scheduledTimeOut)
            break;
    }
    std::cout << " done!\nWaiting for " << builtEngineDsymPath << "...";
    while (fs::last_write_time(builtEngineDsymPath) <= engineDsymLastModified)
    {
        std::this_thread::sleep_for(500ms);
        if (steady_clock::now() > scheduledTimeOut)
            break;
    }
    std::cout << " done!\nWaiting for " << builtEditorDylibPath << "...";
    while (fs::last_write_time(builtEditorDylibPath) <= editorDylibLastModified)
    {
        std::this_thread::sleep_for(500ms);
        if (steady_clock::now() > scheduledTimeOut)
            break;
    }
    std::cout << " done!\nWaiting for " << builtEditorDsymPath << "...";
    while (fs::last_write_time(builtEditorDsymPath) <= editorDsymLastModified)
    {
        std::this_thread::sleep_for(500ms);
        if (steady_clock::now() > scheduledTimeOut)
            break;
    }
    std::cout << " done!\n";

    std::error_code ec;

    std::cout << "Deleting old build products...\n";
    fs::remove_all(pathToBuildFolder / "Launcher/reload_cache");
    fs::create_directory(pathToBuildFolder / "Launcher/reload_cache");

    std::cout << "Copying new build products...\n";
    fs::copy_file(builtEngineDylibPath, copiedEngineDylibPath, ec);
    if (ec)
        std::cerr << "Failed to copy Engine dynamic library! " << ec.message() << "\n";
    fs::copy(builtEngineDsymPath, copiedEngineDsymPath, fs::copy_options::overwrite_existing, ec);
    if (ec)
        std::cerr << "Failed to copy Engine symbols! " << ec.message() << "\n";
    fs::copy_file(builtEditorDylibPath, copiedEditorDylibPath, fs::copy_options::overwrite_existing, ec);
    if (ec)
        std::cerr << "Failed to copy Editor dynamic library! " << ec.message() << "\n";
    fs::copy(builtEditorDsymPath, copiedEditorDsymPath, fs::copy_options::overwrite_existing, ec);
    if (ec)
        std::cerr << "Failed to copy Editor symbols! " << ec.message() << "\n";

    auto currentTime        = fs::file_time_type::clock::now();
    engineDylibLastModified = currentTime;
    engineDsymLastModified  = currentTime;
    editorDylibLastModified = currentTime;
    editorDsymLastModified  = currentTime;

    std::cout << "Loading editor dynamic library...\n";
    // TODO: Do we have to worry about finding the editor library finding the engine library?
    return dlopen(editorDylibName.c_str(), RTLD_LAZY | RTLD_LOCAL);
}

int main(int argc, char* argv[])
{
    std::cout << "Starting Enterprise...\n";

    bool isDeveloperMode = false;
    bool isDebugMode     = false;

    std::cout << "Command line arguments:\n";
    for (int i = 1; i < argc; i++)
    {
        std::cout << "    " << argv[i] << "\n";

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
#ifdef ENTERPRISE_DEBUG
        reloadFlags = EditorReloadFlag_Debug;
#elif defined(ENTERPRISE_DEV)
        reloadFlags = EditorReloadFlag_Dev;
#elif defined(ENTERPRISE_RELEASE)
        reloadFlags = EditorReloadFlag_Release;
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
                editorModuleHandle = dlopen("libEditorD.dylib", RTLD_LAZY | RTLD_LOCAL);
            }
            else
            {
                std::cout << "Loadng editor in release mode...\n";
                editorModuleHandle = dlopen("libEditor.dylib", RTLD_LAZY | RTLD_LOCAL);
            }
        }

        if (editorModuleHandle == nullptr)
        {
            std::cerr << "Error loading editor DLL! " << dlerror() << "\n";
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
            switch (reloadFlags)
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
