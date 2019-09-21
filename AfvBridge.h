#pragma once

class AfvBridge : public EuroScopePlugIn::CPlugIn
{
    public:
        AfvBridge(void);
        void OnTimer(int counter) override;
};
