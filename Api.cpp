#include "pch.h"
#include "Api.h"

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
