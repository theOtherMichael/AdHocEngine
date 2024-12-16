#include "ReloadSentinel_Win.h"

#include <array>
#include <atomic>
#include <cassert>
#include <string_view>

#include <fmt/format.h>

#include <Enterprise/Core/Assertions.h>
#include <Enterprise/Core/PlatformHelpers_Win.h>

#include <Editor/Core/EditorReloadFlags.h>

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif // WIN32_LEAN_AND_MEAN
#include <windows.h>

static const std::array<std::string_view, 6> pathsToBuildProducts = {"Debug\\Editor\\EditorD.dll",
                                                                     "Debug\\Engine\\EngineD.dll",
                                                                     "Dev\\Editor\\Editor.dll",
                                                                     "Dev\\Engine\\Engine.dll",
                                                                     "Release\\Editor\\Editor.dll",
                                                                     "Release\\Engine\\Engine.dll"};

DWORD WINAPI WaitForEditorOrEngineRecompile(LPVOID reloadFlagsOutPtr)
{
    std::atomic_uchar& reloadFlagsOutRef = *reinterpret_cast<std::atomic_uchar*>(reloadFlagsOutPtr);

    HANDLE buildDirectoryHandle = CreateFile(L"build",
                                             FILE_LIST_DIRECTORY,
                                             FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                                             NULL,
                                             OPEN_ALWAYS,
                                             FILE_FLAG_BACKUP_SEMANTICS,
                                             NULL);

    if (buildDirectoryHandle == INVALID_HANDLE_VALUE)
    {
        fmt::print(
            stderr, "Watcher thread could not open handle to build directory! Error: {}\n", GetLastErrorAsString());
        return EXIT_FAILURE;
    }

    char buffer[1024];
    DWORD bytesReturned;

    while (ReadDirectoryChangesW(buildDirectoryHandle,
                                 buffer,
                                 sizeof(buffer),
                                 TRUE,
                                 FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_FILE_NAME,
                                 &bytesReturned,
                                 NULL,
                                 NULL))
    {
        FILE_NOTIFY_INFORMATION* notifyInfo = nullptr;
        size_t nextBufferPos                = 0;

        do
        {
            notifyInfo = (FILE_NOTIFY_INFORMATION*)&buffer[nextBufferPos];
            nextBufferPos += notifyInfo->NextEntryOffset;

            if (notifyInfo->Action != FILE_ACTION_MODIFIED && notifyInfo->Action != FILE_ACTION_RENAMED_NEW_NAME)
                continue;

            std::string modifiedFile = WCHARtoUTF8(notifyInfo->FileName, notifyInfo->FileNameLength / sizeof(WCHAR));

            for (const std::string_view watchedFile : pathsToBuildProducts)
            {
                if (modifiedFile != watchedFile)
                    continue;

                unsigned char reloadFlags = EditorReloadFlag_None;

                size_t configNameLength = 0;
                if (modifiedFile.compare(0, 3, "Dev") == 0)
                {
                    configNameLength = std::char_traits<char>::length("Dev");
                    reloadFlags      = EditorReloadFlag_Dev;
                }
                else if (modifiedFile.compare(0, 5, "Debug") == 0)
                {
                    configNameLength = std::char_traits<char>::length("Debug");
                    reloadFlags      = EditorReloadFlag_Debug;
                }
                else if (modifiedFile.compare(0, 7, "Release") == 0)
                {
                    configNameLength = std::char_traits<char>::length("Release");
                    reloadFlags      = EditorReloadFlag_Release;
                }
                else
                {
                    ASSERT_NOENTRY();
                }

                if (modifiedFile.compare(configNameLength + 1, 6, "Engine") == 0)
                {
                    reloadFlags |= EditorReloadFlag_Engine;
                }
                else if (modifiedFile.compare(configNameLength + 1, 6, "Editor") == 0)
                {
                    reloadFlags |= EditorReloadFlag_Editor;
                }
                else
                {
                    ASSERT_NOENTRY();
                }

                reloadFlagsOutRef = reloadFlags;
                break;
            }
        } while (notifyInfo->NextEntryOffset != 0);

        if (reloadFlagsOutRef != EditorReloadFlag_None)
        {
            fmt::print("Reloading Editor...\n");
            SetLastError(ERROR_OPERATION_ABORTED);
            break;
        }
    }

    if (GetLastError() != ERROR_OPERATION_ABORTED)
    {
        fmt::print(stderr, "Watcher thread ReadDirectoryChangesW() failure! Error: {}\n", GetLastErrorAsString());
    }

    return EXIT_SUCCESS;
}
