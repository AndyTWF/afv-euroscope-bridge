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

        void Move(int xPos, int yPos);

        bool shouldRender = true;

        // Moveable things
        RECT windowRect;
        RECT txRect;
        RECT rxRect;
        RECT headerRect;

        // Fixed coordinates
        const int startCoordinate = 100;
        const int windowWidth = 105;
        const int windowHeight = 100;

        const int txRxWidth = 40;
        const int txRxHeight = 15;
        const int txRxGap = 15;
        const int txOffsetX = 5;
        const int txRxOffsetY = 10;
        const int headerHeight = 15;


        // Brushes
        HBRUSH backgroundBrush;
        HBRUSH txRxActiveBrush;
        HBRUSH headerBrush;
};

