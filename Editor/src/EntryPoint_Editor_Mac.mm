#if !(defined(__APPLE__) && defined(__MACH__))
static_assert(false);
#endif // !(defined(__APPLE__) && defined(__MACH))

#include <fmt/format.h>
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#include <GLFW/glfw3.h>
#pragma clang diagnostic pop

#include <Editor/EditorReloadFlags.h>

#import <Foundation/Foundation.h>

static void OnGlfwError(int error, const char* description)
{
    fmt::print(stderr, "GLFW error {}: {}\n", error, description);
}

static void AddHandlerForFile(const char* editorLibPath, dispatch_queue_t reloadFileWatchQueue)
{
    NSURL* editorLibUrl = [NSURL fileURLWithPath:[NSString stringWithCString:editorLibPath
                                                                    encoding:NSUTF8StringEncoding]];

    int editorLibFileDescriptor = open([editorLibUrl fileSystemRepresentation], O_EVTONLY);

    dispatch_source_t editorLibDispatchSource = dispatch_source_create(DISPATCH_SOURCE_TYPE_VNODE,
                                                                       editorLibFileDescriptor,
                                                                       DISPATCH_VNODE_DELETE | DISPATCH_VNODE_RENAME,
                                                                       reloadFileWatchQueue);
    dispatch_source_set_event_handler(editorLibDispatchSource, ^{
        auto eventSourceFlag = dispatch_source_get_data(editorLibDispatchSource);
        if (eventSourceFlag & DISPATCH_VNODE_DELETE)
        {
            fmt::print("A file was deleted\n");
            dispatch_source_cancel(editorLibDispatchSource);
        }
        else if (eventSourceFlag & DISPATCH_VNODE_RENAME)
        {
            fmt::print("A file was renamed\n");
        }
        NSLog(@"Change at %@ of type %lu", editorLibUrl, eventSourceFlag);
    });

    dispatch_source_set_cancel_handler(editorLibDispatchSource, ^{ close(editorLibFileDescriptor); });

    dispatch_resume(editorLibDispatchSource);
}

static dispatch_queue_t StartReloadWatch(std::atomic_uchar* reloadFlagsOutPtr)
{
    dispatch_queue_t reloadFileWatchQueue =
        dispatch_queue_create("com.MichaelMartz.Enterprise.ReloadFileWatchQueue", 0);

    AddHandlerForFile("./libEditor.dylib", reloadFileWatchQueue);
    AddHandlerForFile("./libEditorD.dylib", reloadFileWatchQueue);
    AddHandlerForFile("./libEngine.dylib", reloadFileWatchQueue);
    AddHandlerForFile("./libEngineD.dylib", reloadFileWatchQueue);

    return reloadFileWatchQueue;
}

static void StopReloadWatch(dispatch_queue_t dispatchQueue)
{
    // TODO: Stop the queue watching for all files
}

extern "C" unsigned char EditorMain(int argc, char* argv[])
{
    @autoreleasepool
    {
        bool isDeveloperMode = false;
        for (int i = 1; i < argc; i++)
        {
            if (strcmp(argv[i], "--developer") == 0)
            {
                isDeveloperMode = true;
                break;
            }
        }

        dispatch_queue_t reloadWatchQueue;
        std::atomic_uchar reloadFlags = EditorReloadFlag_None;
        if (isDeveloperMode)
            reloadWatchQueue = StartReloadWatch(&reloadFlags);

        glfwSetErrorCallback(OnGlfwError);

        if (!glfwInit())
        {
            StopReloadWatch(reloadWatchQueue);
            return EditorReloadFlag_None;
        }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        GLFWwindow* mainWindowPtr = glfwCreateWindow(1024, 768, "Window Title", nullptr, nullptr);

        if (!mainWindowPtr)
        {
            glfwTerminate();
            StopReloadWatch(reloadWatchQueue);
            return EditorReloadFlag_None;
        }

        while (!glfwWindowShouldClose(mainWindowPtr))
        {
            glfwWaitEvents();

            if (reloadFlags != EditorReloadFlag_None)
            {
                // Dump editor state here
                break;
            }
        }

        glfwTerminate();
        StopReloadWatch(reloadWatchQueue);
        return EditorReloadFlag_None;
    }
}
