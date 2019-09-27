#pragma once
#include "HiddenWindow.h"

class AfvBridge : public EuroScopePlugIn::CPlugIn
{
    public:
        AfvBridge(void);
        ~AfvBridge(void);
        void AddMessageToQueue(std::string message);
        void OnTimer(int counter) override;

#ifdef _DEBUG
        bool OnCompileCommand(const char* command);
#endif // _DEBUG

    private:

        void CheckAtisFrequencies(void);
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

        // The currently selected ATIS channel
        EuroScopePlugIn::CGrountToAirChannel currentAtisChannel;

        // The window handle so we can kill it
        HWND hiddenWindow = NULL;

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
};
