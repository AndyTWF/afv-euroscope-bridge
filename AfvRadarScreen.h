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
        void OnClickScreenObject(
            int objectType,
            const char* sObjectId,
            POINT pt,
            RECT area,
            int button
        );
        bool OnCompileCommand(const char* sCommandLine) override;
        void OnMoveScreenObject(
            int ObjectType,
            const char* sObjectId,
            POINT Pt,
            RECT Area,
            bool Released
        ) override;
        void OnRefresh(HDC hdc, int phase) override;

    private:

        HBRUSH& GetStatusColourForHeader(int status);
        bool IsInteger(std::string number) const;
        void Move(int xPos, int yPos);

        // Render data
        bool shouldRender = true;

        // Moveable things
        RECT windowRect;
        RECT txRect;
        RECT rxRect;
        RECT headerRect;
        RECT vccsRect;
        RECT settingsRect;
        RECT lastReceivedRect;

        // Fixed coordinates
        const int startCoordinate = 100;
        const int windowWidth = 350;
        const int windowHeight = 40;

        const int buttonsWidth = 30;
        const int txRxWidth = 20;
        const int secondRowHeight = 15;
        const int buttonsGap = 5;
        const int margin = 5;
        const int secondRowOffsetY = 5;
        const int headerHeight = 15;

        const int callsignsWidth = 220;
        const int callsignsHeight = 15;

        // Brushes
        HBRUSH backgroundBrush;
        HBRUSH txRxActiveBrush;
        HBRUSH txRxInactiveBrush;
        HBRUSH headerBrushDisconnected;
        HBRUSH headerBrushConnecting;
        HBRUSH headerBrushConnected;
        HBRUSH buttonOutlineBrush;

        // ASR settings
        std::string settingKeyXPos = "afvXPos";
        std::string settingDescriptionXPos = "AFV Display X Position";

        std::string settingKeyYPos = "afvYPos";
        std::string settingDescriptionYPos = "AFV Display Y Position";

        std::string settingKeyVisible = "afvVisible";
        std::string settingDescriptionVisible = "Render AFV Display";
};

