#include "pch.h"
#include "HiddenWindow.h"
#include "AfvBridge.h"

AfvBridge * bridge = nullptr;

std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> stringConvertor;

LRESULT CALLBACK HiddenWindow(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
        case WM_CREATE : {
            bridge = reinterpret_cast<AfvBridge*>(reinterpret_cast<CREATESTRUCT*>(lParam)->lpCreateParams);
            bridge->OnTimer(1);
            return TRUE;
        }
        case WM_COPYDATA: {
            COPYDATASTRUCT * data = reinterpret_cast<COPYDATASTRUCT *>(lParam);

            if (data->dwData == 666 && bridge != nullptr) {
                bridge->AddMessageToQueue(stringConvertor.to_bytes(reinterpret_cast<LPCTSTR>(data->lpData)));
            }
        }
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}
