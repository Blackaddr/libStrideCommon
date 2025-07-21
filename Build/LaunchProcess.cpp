/*
 * LaunchProcess.cpp
 *
 *  Created on: Jan. 25, 2022
 *      Author: blackaddr
 */
#include <cstdio>
#include <string>
#include "Util/CommonDefs.h"
#include "Util/ErrorMessage.h"
#include "Build/LaunchProcess.h"

#if defined(LINUX) || defined(MACOS)
#include <unistd.h>
#elif defined(WINDOWS)
#include <windows.h>
#endif

namespace stride {

#if defined(LINUX)
int LaunchProcessCommandLinux(const std::string& command, std::string& result, const std::string&launchDir, unsigned timeoutSeconds)
{

    noteMessage(std::string("LaunchProcessLinux(): executing '" + command + "'" + " in folder \"" + launchDir + "\""));

    std::string commandFull;
    if (!launchDir.empty()) {
        commandFull = "cd \"" + launchDir + "\" && " + command;
    } else {
        commandFull = command;
    }

    commandFull += " 2>&1";
    //commandFull += " 3>&1- 1>&2- 2>&3-"; // swap stderr and stdout

    FILE* pipe = popen(commandFull.c_str(), "r");
    if (!pipe) {
        errorMessage("LaunchProcessLinux(): popen returned error");
        return FAILURE;
    }
    char buffer[128];
    while(!feof(pipe)) {
        if(fgets(buffer, 128, pipe) != NULL) {
            result += buffer;
        }
    }
    pclose(pipe);
    return SUCCESS;
}

int LaunchProcessCommandListLinux(const std::vector<std::string>& command, std::string& result, const std::string&launchDir)
{
    UNUSED(command);
    UNUSED(result);
    UNUSED(launchDir);

    return FAILURE;
}
#endif

#if defined(MACOS)
int LaunchProcessCommandMacos(const std::string& command, std::string& result, const std::string&launchDir, unsigned timeoutSeconds)
{

    noteMessage(std::string("LaunchProcessCommandMacos(): executing '" + command + "'" + " in folder \"" + launchDir + "\""));

    std::string commandFull;
    if (!launchDir.empty()) {
        commandFull = "cd \"" + launchDir + "\" && " + command;
    } else {
        commandFull = command;
    }

    commandFull += " 2>&1";
    //commandFull += " 3>&1- 1>&2- 2>&3-"; // swap stderr and stdout

    FILE* pipe = popen(commandFull.c_str(), "r");
    if (!pipe) {
        errorMessage("LaunchProcessCommandMacos(): popen returned error");
        return FAILURE;
    }
    char buffer[128];
    while(!feof(pipe)) {
        if(fgets(buffer, 128, pipe) != NULL) {
            result += buffer;
        }
    }
    pclose(pipe);
    return SUCCESS;
}

int LaunchProcessCommandListMacos(const std::vector<std::string>& command, std::string& result, const std::string&launchDir)
{
    UNUSED(command);
    UNUSED(result);
    UNUSED(launchDir);

    return FAILURE;
}
#endif

#ifdef WINDOWS

// Create a struct to pass data to/from the stdout pipe read thread
struct WinReadThreadData {
    HANDLE hStdOutRead;  // File handle to read from process stdout
    HANDLE hTerminate;   // Event to signal to the thread it should terminate
    std::string result;  // the stdout contents
};

// We need a separate thread that will drain the execution process stdout, otherwise
// it will not complete and get blocked.
DWORD __stdcall readDataFromExtProgram(void * argh)
{
    if (!argh) { return FALSE; }
    WinReadThreadData* dataPtr = (WinReadThreadData*)argh;
    constexpr size_t BUFSIZE = 4096;
    DWORD dwRead;
    CHAR chBuf[BUFSIZE];
    BOOL bSuccess = FALSE;

    dataPtr->result.clear();

    for (;;)
    {
        // ReadFile() is difficult to perform non-blocking. So, first peek to see
        // if any bytes are available. If so, read them. If not, check to see if
        // the thread should exit. Exit is signalled when the process it's reading
        // from is finished.
        DWORD bytesAvailable = 0;
        PeekNamedPipe(dataPtr->hStdOutRead, NULL, 0, NULL, &bytesAvailable, NULL);

        if (bytesAvailable < 1) {
            // check if event signalled
            if (WaitForSingleObject(dataPtr->hTerminate, 0) == WAIT_OBJECT_0) {
                ResetEvent(dataPtr->hTerminate);
                break;
            }
        } else {
            bSuccess = ReadFile(dataPtr->hStdOutRead, chBuf, BUFSIZE, &dwRead, NULL);
            if (dwRead >  0) {
                dataPtr->result.append(chBuf);
            }
            if (!bSuccess) break;
        }
    }

    return 0;
}

// Reference for this is based on https://docs.microsoft.com/en-us/windows/win32/procthread/creating-a-child-process-with-redirected-input-and-output
// However, it didn't work for reading stdout, so more changes were necessary to set up a separate read thread and termination signalling
// Inspired by https://stackoverflow.com/questions/42402673/createprocess-and-capture-stdout
int LaunchProcessCommandWindows(const std::string& command, std::string& result, const std::string&launchDir, unsigned timeoutSeconds)
{
    int returnStatus = SUCCESS;  // set to FAILURE if any errors occur

	std::string commandFull;
    if (!launchDir.empty()) {
        commandFull = "cmd.exe /C set PATH= && cd /D \"" + launchDir + "\" && " + command;
    } else {
        commandFull = "cmd.exe /C set PATH= && " + command ;
    }

    HANDLE g_hChildStd_OUT_Rd = NULL;
    HANDLE g_hChildStd_OUT_Wr = NULL;
    HANDLE g_hReadThread      = NULL;

    SECURITY_ATTRIBUTES saAttr;
    // Set the bInheritHandle flag so pipe handles are inherited.
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;

    // Create a pipe for the child process's STDOUT.
     if ( !CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &saAttr, 0) ) { return FAILURE; }

    // Ensure the read handle to the pipe for STDOUT is not inherited.
    if ( !SetHandleInformation(g_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0) ) {
        CloseHandle(g_hChildStd_OUT_Rd);
        CloseHandle(g_hChildStd_OUT_Wr);
        return FAILURE;
    }

    STARTUPINFO startInfo;
    ZeroMemory( &startInfo, sizeof(STARTUPINFO) );
    startInfo.cb = sizeof(STARTUPINFO);
    startInfo.hStdError  = g_hChildStd_OUT_Wr;
    startInfo.hStdOutput = g_hChildStd_OUT_Wr;
    startInfo.dwFlags |= STARTF_USESTDHANDLES;

    noteMessage(std::string("LaunchProcessLinux(): executing '" + commandFull + "'"));
    PROCESS_INFORMATION processInfo;
    ZeroMemory(&processInfo, sizeof(processInfo));
    if (CreateProcessA(NULL, // Null module name
            const_cast<char *>(commandFull.c_str()),
            NULL,  // Process handle not inheritable
            NULL,  // thread handle not inheritable
            TRUE,  // handles are inherited
            CREATE_NO_WINDOW, // no creation flags
            NULL, // use parents environment block
            NULL, // use parents starting directory
            &startInfo, &processInfo))
    {

        WinReadThreadData threadData;
        threadData.hStdOutRead = g_hChildStd_OUT_Rd;

        // Creat an event to signal to the read thread to terminate
        threadData.hTerminate = CreateEventA(NULL, TRUE, FALSE, NULL);

        // create a separate thread to drain the stdout from the primary execution process
        g_hReadThread = CreateThread(0, 0, readDataFromExtProgram, &threadData, 0, NULL);

        DWORD timeout = (timeoutSeconds == 0) ? INFINITE : timeoutSeconds * 1000;

        // Wait for the execution process to finish
        DWORD execResult = WaitForSingleObject(processInfo.hProcess, timeout);
        SetEvent(threadData.hTerminate);  // signal the read thread to stop
        switch(execResult) {  // check for process return error
        case WAIT_TIMEOUT :
            TerminateProcess(processInfo.hProcess, 1 /* exit code*/);
            errorMessage("LaunchProcessCommandWindows(): command has timed out");
            returnStatus = FAILURE;
            break;
        default: break;
        }

        execResult = WaitForSingleObject(g_hReadThread, timeout);
        switch(execResult) {
        case WAIT_TIMEOUT :
            TerminateThread(g_hReadThread, 1 /* exit code*/);
            errorMessage("LaunchProcessCommandWindows(): read thread has timed out");
            returnStatus = FAILURE;
            break;
        default: break;
        }

        result = threadData.result;

        CloseHandle(processInfo.hProcess);
        CloseHandle(processInfo.hThread);
        CloseHandle(g_hChildStd_OUT_Wr);
        CloseHandle(g_hChildStd_OUT_Rd);
        CloseHandle(g_hReadThread);

        noteMessage(std::string("LaunchProcessLinux(): finished executing '" + command + "'"));

        return returnStatus;

    } else {
		errorMessage("LaunchProcessCommandWindows() failed");
		return FAILURE;
    }
}

int LaunchProcessCommandListWindows(const std::vector<std::string>& command, std::string& result, const std::string&launchDir)
{
    UNUSED(command);
    UNUSED(result);
    UNUSED(launchDir);

    return FAILURE;
}
#endif

int LaunchProcessCommand(const std::string& command, std::string& result, const std::string& launchDir, unsigned timeoutSeconds)
{
#if defined(LINUX)
    return LaunchProcessCommandLinux(command, result, launchDir, timeoutSeconds);
#elif defined(MACOS)
    return LaunchProcessCommandMacos(command, result, launchDir, timeoutSeconds);
#elif defined (WINDOWS)
    return LaunchProcessCommandWindows(command, result, launchDir, timeoutSeconds);
#else
#error Unsupported OS in LaunchProcess.cpp
    return FAILURE;
#endif
}

int LaunchProcessCommandList(const std::vector<std::string>& commandVec, std::string& result, const std::string&launchDir)
{
#if defined(LINUX)
    return LaunchProcessCommandListLinux(commandVec, result, launchDir);
#elif defined(MACOS)
    return LaunchProcessCommandListMacos(commandVec, result, launchDir);
#elif defined(WINDOWS)
    return LaunchProcessCommandListWindows(commandVec, result, launchDir);
#else
#error Unsupported OS in LaunchProcess.cpp
    return FAILURE;
#endif
}


}
