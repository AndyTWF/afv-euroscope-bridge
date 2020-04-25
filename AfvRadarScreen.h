#pragma once

class AfvRadarScreen : public EuroScopePlugIn::CRadarScreen
{
    public:
        AfvRadarScreen();
        ~AfvRadarScreen();
        void OnRefresh(HDC hdc, int phase) override;
        // Inherited via CRadarScreen
        virtual void OnAsrContentToBeClosed(void) override;

    private:

        bool shouldRender = true;
        RECT windowRect;

        // Fixed coordinates
        const int startCoordinate = 100;
        const int windowWidth = 100;
        const int windowHeight = 100;

        // Brushes
        HBRUSH backgroundBrush;
};

