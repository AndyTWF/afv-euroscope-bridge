#include "pch.h"
#include "HiddenWindow.h"
#include "AfvBridge.h"

AfvBridge * bridge = nullptr;

LRESULT CALLBACK HiddenWindow(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
        case WM_CREATE : {
            bridge = reinterpret_cast<AfvBridge*>(reinterpret_cast<CREATESTRUCT*>(lParam)->lpCreateParams);
            return TRUE;
        }
        case WM_COPYDATA: {
            COPYDATASTRUCT * data = reinterpret_cast<COPYDATASTRUCT *>(lParam);

            if (data != nullptr && data->dwData == 666 && data->lpData != nullptr && bridge != nullptr) {
                bridge->AddMessageToQueue(reinterpret_cast<const char*>(data->lpData));
            }
            return TRUE;
        }
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}
