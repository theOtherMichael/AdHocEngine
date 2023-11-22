#if !(defined(__APPLE__) && defined(__MACH__))
static_assert(false);
#endif // !(defined(__APPLE__) && defined(__MACH__))

#include <dlfcn.h>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include <Editor/EditorReloadFlags.h>

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
    }
    
    void* editorModuleHandle = nullptr;
    unsigned char (*editorMainFunctionPtr)(int, char*[]) = nullptr;
    
    do
    {
        if (isDevelopmentMode)
        {
//            editorModuleHandle = CopyAndLoadEditorModule(reloadFlags);
        }
        else
        {
            if (isDebugMode)
                editorModuleHandle = dlopen("libEditorD.dylib", RTLD_LAZY | RTLD_LOCAL);
            else
                editorModuleHandle = dlopen("libEditor.dylib", RTLD_LAZY | RTLD_LOCAL);
        }

        if (editorModuleHandle == nullptr)
        {
            std::cerr << "Error loading editor DLL! " << dlerror() << "\n";
            return EXIT_FAILURE;
        }

        editorMainFunctionPtr = (unsigned char (*)(int, char*[]))dlsym(
            editorModuleHandle, "EditorMain");

        if (editorMainFunctionPtr == nullptr)
        {
            std::cerr << "Error retrieving address of EditorMain()! "
                      << dlerror() << "\n";
            return EXIT_FAILURE;
        }

        reloadFlags = editorMainFunctionPtr(argc, argv);

        switch (reloadFlags & EditorReloadFlag_ConfigMask)
        {
        case EditorReloadFlag_None: break;
        case EditorReloadFlag_Debug: isDebugMode = true; break;
        case EditorReloadFlag_Release: isDebugMode = false; break;
        case EditorReloadFlag_Dev:
            std::cerr << "Dev configuration was requested in Editor reload! "
                         "Dev configuration is only available in Development Mode. "
                         "Release will be used instead.\n";
            isDebugMode = false;
            break;
        default: assert(false); break;
        }

        editorMainFunctionPtr = nullptr;
        dlclose(editorModuleHandle);
        editorModuleHandle = nullptr;

    } while (reloadFlags != EditorReloadFlag_None);
    
    return EXIT_SUCCESS;
}
