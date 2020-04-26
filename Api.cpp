#include "pch.h"
#include "Api.h"

const std::wstring exeName = L"GeoVR.VATSIM.ControllerClient.exe";

struct GetAfvWindowArgs {
    const DWORD afvProcessId;
    HWND afvWindowHandle;
};

/*
    Send a message to the specified target window
*/
void SendSelfMessage(std::string message)
{
    COPYDATASTRUCT cds;
    cds.dwData = 666;
    cds.cbData = message.size() + 1;
    cds.lpData = (PVOID)message.c_str();

    // Find the hidden window
    HWND window = FindWindowEx(NULL, NULL, HIDDEN_WINDOW_CLASS, NULL);
    SendMessageToTarget(window, message);
}

/*
    Send a message to the AFV client
*/
void SendAfvClientMessage(std::string message)
{
    DWORD afvProcessId = GetAfvProcessId();

    if (afvProcessId == NULL) {
        return;
    }

    GetAfvWindowArgs args = {
        afvProcessId,
        NULL
    };

    if (EnumWindows(&GetAfvWindow, (LPARAM)&args) == FALSE) {
        return;
    }

    SendMessageToTarget(args.afvWindowHandle, message);
}

/*
    Send a message to the specified target
*/
void SendMessageToTarget(HWND target, std::string message)
{
    COPYDATASTRUCT cds;
    cds.dwData = 666;
    cds.cbData = message.size() + 1;
    cds.lpData = (PVOID)message.c_str();

    if (target == NULL) {
        return;
    }

    // Send the data
    SendMessage(target, WM_COPYDATA, reinterpret_cast<WPARAM>(target), reinterpret_cast<LPARAM>(&cds));
}

/*
    Get the main AFV window by process ID / title
*/
BOOL CALLBACK GetAfvWindow(HWND hwnd, LPARAM lParam) 
{
    GetAfvWindowArgs* args = (GetAfvWindowArgs*)lParam;
    DWORD windowPid;
    GetWindowThreadProcessId(hwnd, &windowPid);

    if (windowPid == args->afvProcessId) {
        wchar_t buffer[1024];
        GetWindowText(
            hwnd,
            buffer,
            1024
        );

        // Many of the windows register here, so just use the main one
        if (std::wstring(buffer) == L"AFVControllerClient") {
            args->afvWindowHandle = hwnd;
        }
    }

    return TRUE;
}

/*
    Start up the AFV client
*/
void StartAfvClient(void)
{
    // If AFV is not running, load it
    if (GetAfvProcessId() == NULL) {
        std::wstring path = GetDllPath();

        if (path == L"") {
            return;
        }

        std::wstring afvExe = path.substr(0, path.find_last_of(L'\\') + 1) + exeName;

        // Additional information
        STARTUPINFO si;
        PROCESS_INFORMATION pi;

        // set the size of the structures
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        ZeroMemory(&pi, sizeof(pi));

       // start the program up
       BOOL loaded = CreateProcess(
            afvExe.c_str(),   // the path
            NULL,        // Command line
            NULL,           // Process handle not inheritable
            NULL,           // Thread handle not inheritable
            FALSE,          // Set handle inheritance to FALSE
            0,              // No creation flags
            NULL,           // Use parent's environment block
            NULL,           // Use parent's starting directory 
            &si,            // Pointer to STARTUPINFO structure
            &pi             // Pointer to PROCESS_INFORMATION structure (removed extra parentheses)
        );

        // Close process and thread handles. 
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);

        // If somethings broken, error message and crash the plugin
        if (!loaded) {
            std::wstring code = std::to_wstring(GetLastError());
            std::wstring message = L"Failed to load AFV client, please load it manually.\r\n\r\n";
            message += L"Failed to load AFV Client process. Code = " + code;
            MessageBox(GetActiveWindow(), message.c_str(), L"AFV Bridge Bootstrap Error", MB_OK | MB_ICONERROR);
        }
    }
}

DWORD GetAfvProcessId(void)
{
    PROCESSENTRY32W pe32 = { 0 };
    HANDLE    hSnap;
    int       iDone;
    int       iTime = 60;

    hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    pe32.dwSize = sizeof(PROCESSENTRY32W);
    Process32First(hSnap, &pe32);

    bool afvRunning = false;
    iDone = 1;

    // Check to see if AFV is already running
    while (iDone) {
        iDone = Process32Next(hSnap, &pe32);
        if (std::wstring(pe32.szExeFile) == exeName)
        {
            return pe32.th32ProcessID;
        }
    }

    return NULL;
}

/*
    Get the path to the DLL
*/
std::wstring GetDllPath(void)
{
    wchar_t path[MAX_PATH];
    HMODULE hm = NULL;

    if (GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
        GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        (LPCWSTR)&EuroScopePlugInInit, &hm) == 0)
    {
        std::wstring code = std::to_wstring(GetLastError());
        std::wstring message = L"Failed to load AFV client, please load it manually.\r\n\r\n";
        message += L"Failed to load codule handle. Code = " + code;
        MessageBox(GetActiveWindow(), message.c_str(), L"AFV Bridge Bootstrap Error", MB_OK | MB_ICONERROR);
        return L"";
    }

    if (GetModuleFileName(hm, path, sizeof(path)) == 0)
    {
        std::wstring code = std::to_wstring(GetLastError());
        std::wstring message = L"Failed to load AFV client, please load it manually.\r\n\r\n";
        message += L"Failed to get module path. Code = " + code;
        MessageBox(GetActiveWindow(), message.c_str(), L"AFV Bridge Bootstrap Error", MB_OK | MB_ICONERROR);
        return L"";
    }

    return path;
}