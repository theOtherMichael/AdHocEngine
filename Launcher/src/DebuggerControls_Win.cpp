#include "DebuggerControls_Win.h"

#include <iostream>

#include "PlatformHelpers_Launcher_Win.h"

// https://msdn.microsoft.com/en-us/library/yf86a8ts.aspx
#pragma warning(disable : 4278)
#pragma warning(disable : 4146)
#import "libid:80cc9f66-e7d8-4ddd-85b6-d9e6cd0e93e2" version("8.0") lcid("0") raw_interfaces_only named_guids
#pragma warning(default : 4146)
#pragma warning(default : 4278)

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif // WIN32_LEAN_AND_MEAN
#include <windows.h>

// https://handmade.network/forums/wip/t/1479-sample_code_to_programmatically_attach_visual_studio_to_a_process
static EnvDTE::Process* FindVSProcess(DWORD targetPID)
{
    HRESULT result;

    static const wchar_t* visualStudioProgID = L"VisualStudio.DTE";

    CLSID visualStudioClsid;
    if (CLSIDFromProgID(visualStudioProgID, &visualStudioClsid) != S_OK)
    {
        std::cout << "Error: did not find CLSID for Visual Studio!" << std::endl;
        return nullptr;
    }

    IUnknown* visualStudioActiveObject;
    result = GetActiveObject(visualStudioClsid, NULL, &visualStudioActiveObject);
    if (FAILED(result))
        return nullptr;

    EnvDTE::_DTE* visualStudioInterface;

    result = visualStudioActiveObject->QueryInterface(&visualStudioInterface);
    if (FAILED(result))
        return nullptr;

    EnvDTE::Debugger* visualStudioDebugger;
    result = visualStudioInterface->get_Debugger(&visualStudioDebugger);
    if (FAILED(result))
        return nullptr;

    EnvDTE::Processes* localProcesses;
    result = visualStudioDebugger->get_LocalProcesses(&localProcesses);
    if (FAILED(result))
        return nullptr;

    long localProcessCount = 0;
    result                 = localProcesses->get_Count(&localProcessCount);
    if (FAILED(result))
        return nullptr;

    EnvDTE::Process* visualStudioProcess = nullptr;

    // 0th currentProcess is invalid
    for (int processIndex = 1; processIndex < localProcessCount; ++processIndex)
    {
        EnvDTE::Process* currentProcess;

        const int retryIntervalInMilliseconds = 10;
        const int maxRetries                  = 500;

        for (int reattemptNumber = 0; reattemptNumber < maxRetries; reattemptNumber++)
        {
            result = localProcesses->Item(variant_t(processIndex), &currentProcess);

            if (SUCCEEDED(result))
                break;

            if (result != RPC_E_CALL_REJECTED)
                break;

            Sleep(retryIntervalInMilliseconds);
        }

        if (FAILED(result))
            continue;

        long currentPID;
        result = currentProcess->get_ProcessID(&currentPID);

        if (SUCCEEDED(result) && currentPID == targetPID)
        {
            visualStudioProcess = currentProcess;
            break;
        }
    }

    return visualStudioProcess;
}

void AttachDebugger()
{
    DWORD targetPID          = GetCurrentProcessId();
    EnvDTE::Process* process = FindVSProcess(targetPID);

    if (!process)
    {
        std::cout << "Could not find visual studio for attach!" << std::endl;
        return;
    }

    process->Attach();
}

void DetachDebugger(bool waitForBreakOrEnd)
{
    DWORD targetPID          = GetCurrentProcessId();
    EnvDTE::Process* process = FindVSProcess(targetPID);

    if (!process)
    {
        std::cout << "Could not find visual studio for detach!" << std::endl;
        return;
    }

    process->Detach(variant_t(waitForBreakOrEnd));
}
