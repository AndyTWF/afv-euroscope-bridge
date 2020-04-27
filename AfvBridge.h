#pragma once
#include "HiddenWindow.h"

class AfvBridge : public EuroScopePlugIn::CPlugIn
{
    public:
        AfvBridge(void);
        ~AfvBridge(void);
        void AddMessageToQueue(std::string message);
        EuroScopePlugIn::CRadarScreen* OnRadarScreenCreated(
            const char* sDisplayName,
            bool NeedRadarContent,
            bool GeoReferenced,
            bool CanBeSaved,
            bool CanBeCreated
        ) override;
        void OnTimer(int counter) override;
        bool IsTransmitting(void) const;
        bool IsReceiving(void) const;
        bool IsVccsOpen(void) const;
        bool IsSettingsOpen(void) const;
        int GetAfvConnectionStatus(void) const;
        const std::string& GetLastTransmitted(void) const;

        const static int AFV_STATUS_DISCONNECTED = 1;
        const static int AFV_STATUS_CONNECTING = 2;
        const static int AFV_STATUS_CONNECTED = 3;

#ifdef _DEBUG
        bool OnCompileCommand(const char* command);
#endif // _DEBUG

    private:

        void LoginCheck(void);
        void ControllerCheck(void);
        void ProcessTxMessage(std::string message);
        void ProcessRxMessage(std::string message);
        void ProcessCallsignsMessage(std::string message);
        void ProcessFrequencyChangeMessage(std::string message);
        void ProcessSettingsMessage(std::string message);
        void ProcessVCCSMessage(std::string message);
        void ProcessResetMessage(void);
        void ProcessMessage(std::string message);
        void ProcessAfvStatusMessage(std::string message);
        bool ValidBoolean(std::string boolean) const;
        bool ConvertBoolean(std::string boolean) const;
        void ToggleFrequency(double frequency, bool receive, bool transmit);
        bool IsFrequencyMatch(double targetFrequency, EuroScopePlugIn::CGrountToAirChannel channel);
        bool IsFrequencyMatch(double targetFrequency, double matchFrequency);
        bool IsAtisChannel(std::string channel) const;
        EuroScopePlugIn::CGrountToAirChannel GetPrimaryFrequency(void);

        // Lock for the message queue
        std::mutex messageLock;

        // Internal message quque
        std::queue<std::string> messages;

        // Allowed deviation in frequencies due to FP rounding
        const double frequencyDeviation = 0.0001;

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
           HIDDEN_WINDOW_CLASS
        };

        // Transmitting and Receiving
        bool isTransmitting = false;
        bool isReceiving = false;
        std::string lastTransmitted;


        // AFV Status
        int afvConnectionStatus = AfvBridge::AFV_STATUS_DISCONNECTED;
        bool vccsOpen = false;
        bool settingsOpen = false;
        
        // ES status
        bool isLoggedIn = false;
        std::string userCallsign = "";
        double userFrequency = 199.998;

};
