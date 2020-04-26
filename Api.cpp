#include "pch.h"
#include "Api.h"

const std::wstring exeName = L"GeoVR.VATSIM.ControllerClient.exe";

/*
    Send a message to the specified target window
*/
void SendApiMessage(std::string message, std::wstring target)
{
    COPYDATASTRUCT cds;
    cds.dwData = 666;
    cds.cbData = message.size() + 1;
    cds.lpData = (PVOID)message.c_str();

    // Find the hidden window
    HWND window = FindWindowEx(NULL, NULL, target.c_str(), NULL);
    if (window == NULL) {
        return;
    }

    // Send the data
    SendMessage(window, WM_COPYDATA, reinterpret_cast<WPARAM>(window), reinterpret_cast<LPARAM>(&cds));
}

/*
    Start up the AFV client
*/
void StartAfvClient(void)
{
    PROCESSENTRY32W pe32 = { 0 };
    HANDLE    hSnap;
    int       iDone;
    int       iTime = 60;
    bool      bProcessFound;

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
            afvRunning = true;
            iDone = 0;
        }
    }

    // If AFV is not running, load it
    if (!afvRunning) {
        std::wstring path = GetDllPath();
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
            std::wstring message = L"Failed To Load AFV Client Process, Code = " + code;
            MessageBox(NULL, message.c_str(), L"AFV Bridge Bootstrap Error", MB_OK | MB_ICONERROR);
            throw std::exception();
        }
    }
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
        std::wstring message = L"Failed To Load Module Handle, Code = " + code;
        MessageBox(NULL, message.c_str(), L"AFV Bridge Bootstrap Error", MB_OK | MB_ICONERROR);
        throw std::exception();
    }

    if (GetModuleFileName(hm, path, sizeof(path)) == 0)
    {
        std::wstring code = std::to_wstring(GetLastError());
        std::wstring message = L"Failed To Get Module Path, Code = " + code;
        MessageBox(NULL, message.c_str(), L"AFV Bridge Bootstrap Error", MB_OK | MB_ICONERROR);
        throw std::exception();
    }

    return path;
}