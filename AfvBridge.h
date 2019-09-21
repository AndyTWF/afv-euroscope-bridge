#pragma once
#include "HiddenWindow.h"

class AfvBridge : public EuroScopePlugIn::CPlugIn
{
    public:
        AfvBridge(void);
        void AddMessageToQueue(std::string message);
        void OnTimer(int counter) override;

#ifdef _DEBUG
        bool OnCompileCommand(const char* command);
#endif // _DEBUG

    private:

        void ProcessMessage(std::string message);
        bool ValidBoolean(std::string boolean) const;
        bool ConvertBoolean(std::string boolean) const;
        void ToggleFrequency(double frequency, bool receive, bool transmit);

        // Lock for the message queue
        std::mutex messageLock;

        // Internal message quque
        std::queue<std::string> messages;

        // Allowed deviation in frequencies due to FP rounding
        const double frequencyDeviation = 0.0001;

        // Class for our window
        WNDCLASS windowClass = {
           NULL,
           HiddenWindow,
           NULL,
           NULL,
           GetModuleHandle(NULL),
           NULL,
           NULL,
           NULL,
           NULL,
           L"AfvBridgeHiddenWindowClass"
        };

        // Converts string messages to the wide type needed by windows.
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
};
