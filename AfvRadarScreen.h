#pragma once

class AfvRadarScreen : public EuroScopePlugIn::CRadarScreen
{
    public:
        AfvRadarScreen();
        ~AfvRadarScreen();

        // Inherited via CRadarScreen
        void OnAsrContentToBeClosed(void) override;
        void OnMoveScreenObject(
            int ObjectType,
            const char* sObjectId,
            POINT Pt,
            RECT Area,
            bool Released
        ) override;
        void OnRefresh(HDC hdc, int phase) override;

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

