#pragma once

class AfvRadarScreen : public EuroScopePlugIn::CRadarScreen
{
    public:
        AfvRadarScreen();
        ~AfvRadarScreen();

        // Inherited via CRadarScreen
        void OnAsrContentLoaded(bool Loaded) override;
        void OnAsrContentToBeClosed(void) override;
        void OnAsrContentToBeSaved(void) override;
        void OnMoveScreenObject(
            int ObjectType,
            const char* sObjectId,
            POINT Pt,
            RECT Area,
            bool Released
        ) override;
        void OnRefresh(HDC hdc, int phase) override;

    private:

        bool IsInteger(std::string number) const;
        void Move(int xPos, int yPos);

        // Render data
        bool shouldRender = true;

        // Moveable things
        RECT windowRect;
        RECT txRect;
        RECT rxRect;
        RECT headerRect;
        RECT lastReceivedRect;
        RECT lastReceivedCallsignOneRect;
        RECT lastReceivedCallsignTwoRect;
        RECT lastReceivedCallsignThreeRect;

        // Fixed coordinates
        const int startCoordinate = 100;
        const int windowWidth = 105;
        const int windowHeight = 150;

        const int txRxWidth = 40;
        const int txRxHeight = 15;
        const int txRxGap = 15;
        const int margin = 5;
        const int txRxOffsetY = 10;
        const int headerHeight = 15;

        const int lastReceivedHeaderOffset = 15;
        const int lastReceivedHeight = 15;
        const int lastReceivedDataOffset = 5;

        // Brushes
        HBRUSH backgroundBrush;
        HBRUSH txRxActiveBrush;
        HBRUSH txRxInactiveBrush;
        HBRUSH headerBrush;

        // ASR settings
        std::string settingKeyXPos = "afvXPos";
        std::string settingDescriptionXPos = "AFV Display X Position";

        std::string settingKeyYPos = "afvYPos";
        std::string settingDescriptionYPos = "AFV Display Y Position";

        std::string settingKeyVisible = "afvVisible";
        std::string settingDescriptionVisible = "Render AFV Display";
};

