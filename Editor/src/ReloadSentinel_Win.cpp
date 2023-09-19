#include "ReloadSentinel_Win.h"

#include <array>
#include <atomic>
#include <cassert>
#include <string_view>

#include <fmt/format.h>

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif // WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <Enterprise/Core/PlatformHelpers_Win.h>

static const std::array<std::string_view, 6> pathsToBuildProducts = {
    "Debug\\Editor\\EditorD.dll",
    "Debug\\Engine\\EngineD.dll",
    "Dev\\Editor\\Editor.dll",
    "Dev\\Engine\\Engine.dll",
    "Release\\Editor\\Editor.dll",
    "Release\\Engine\\Engine.dll"};

namespace Editor
{

void WaitForDirChange(std::atomic_uchar* reloadFlagsOut)
{
    HANDLE buildDirectoryHandle =
        CreateFile(L"build",
                   FILE_LIST_DIRECTORY,
                   FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                   NULL,
                   OPEN_ALWAYS,
                   FILE_FLAG_BACKUP_SEMANTICS,
                   NULL);

    char buffer[1024];
    DWORD bytesReturned;

    while (ReadDirectoryChangesW(
        buildDirectoryHandle,
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

            if (notifyInfo->Action != FILE_ACTION_MODIFIED &&
                notifyInfo->Action != FILE_ACTION_RENAMED_NEW_NAME)
                continue;

            std::string modifiedFile = WCHARtoUTF8(
                notifyInfo->FileName, notifyInfo->FileNameLength / sizeof(WCHAR));

            for (const std::string_view watchedFile : pathsToBuildProducts)
            {
                if (modifiedFile != watchedFile)
                    continue;

                unsigned char reloadFlags = ReloadFlag_None;

                size_t configNameLength = 0;

                if (modifiedFile.compare(0, 3, "Dev") == 0)
                {
                    configNameLength = 3;
                    reloadFlags      = ReloadFlag_Dev;
                }
                else if (modifiedFile.compare(0, 5, "Debug") == 0)
                {
                    configNameLength = 5;
                    reloadFlags      = ReloadFlag_Debug;
                }
                else if (modifiedFile.compare(0, 7, "Release") == 0)
                {
                    configNameLength = 7;
                    reloadFlags      = ReloadFlag_Release;
                }
                else
                {
                    assert(false); // TODO: Replace with Enterprise assert
                }

                if (modifiedFile.compare(configNameLength + 1, 6, "Engine") == 0)
                {
                    reloadFlags |= ReloadFlag_Engine;
                }
                else if (modifiedFile.compare(configNameLength + 1, 6, "Editor") == 0)
                {
                    reloadFlags |= ReloadFlag_Editor;
                }
                else
                {
                    assert(false); // TODO: Replace with Enterprise assert
                }

                reloadFlagsOut->store(reloadFlags);
                break;
            }
        } while (notifyInfo->NextEntryOffset != 0);

        if (*reloadFlagsOut != ReloadFlag_None)
        {
            fmt::print("Reloading Editor...\n");
            break;
        }
    }
}

} // namespace Editor
