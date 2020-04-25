#include "pch.h"
#include "AfvRadarScreen.h"


AfvRadarScreen::AfvRadarScreen()
{
    this->Move(this->startCoordinate, this->startCoordinate);
    this->backgroundBrush = CreateSolidBrush(RGB(0, 0, 0));
    this->txRxActiveBrush = CreateSolidBrush(RGB(234, 173, 92));
    this->headerBrush = CreateSolidBrush(RGB(128, 128, 128));
}

AfvRadarScreen::~AfvRadarScreen()
{
    DeleteObject(this->backgroundBrush);
    DeleteObject(this->txRxActiveBrush);
    DeleteObject(this->headerBrush);
}

void AfvRadarScreen::OnRefresh(HDC hdc, int phase)
{
    // Dont paint in the wrong phase
    if (phase != EuroScopePlugIn::REFRESH_PHASE_AFTER_TAGS || !this->shouldRender) {
        return;
    }

    //Background
    FillRect(hdc, &this->windowRect, this->backgroundBrush);
    this->AddScreenObject(1, "afvWindow", this->windowRect, true, "");

    // Header Bar
    FillRect(hdc, &this->headerRect, this->headerBrush);
    DrawText(hdc, L"AFV", 3, &this->headerRect, DT_VCENTER | DT_CENTER);

    // TX
    FillRect(hdc, &this->txRect, this->txRxActiveBrush);
    DrawText(hdc, L"TX", 2, &this->txRect, DT_VCENTER | DT_CENTER);

    // RX
    FillRect(hdc, &this->rxRect, this->txRxActiveBrush);
    DrawText(hdc, L"RX", 2, &this->rxRect, DT_VCENTER | DT_CENTER);

}

void AfvRadarScreen::Move(int xPos, int yPos)
{
    this->windowRect = {
        xPos,
        yPos,
        xPos + this->windowWidth,
        yPos + this->windowHeight,
    };

    this->headerRect = {
        xPos,
        yPos,
        xPos + this->windowWidth,
        yPos + this->headerHeight,
    };

    this->txRect = {
        xPos + this->txOffsetX,
        yPos + this->headerHeight + this->txRxOffsetY,
        xPos + this->txOffsetX + this->txRxWidth,
        yPos + this->headerHeight + this->txRxOffsetY + this->txRxHeight,
    };

    this->rxRect = {
        xPos + this->txOffsetX + this->txRxWidth + this->txRxGap,
        yPos + this->headerHeight + this->txRxOffsetY,
        xPos + this->txOffsetX + this->txRxWidth + this->txRxGap + this->txRxWidth,
        yPos + this->headerHeight + this->txRxOffsetY + this->txRxHeight,
    };
}

void AfvRadarScreen::OnAsrContentToBeClosed(void)
{
    delete this;
}

void AfvRadarScreen::OnMoveScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, bool Released)
{
    this->Move(Area.left, Area.top);
}
