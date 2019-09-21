#pragma once

class AfvBridge : public EuroScopePlugIn::CPlugIn
{
    public:
        AfvBridge(void);
        void OnTimer(int counter) override;

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
};
