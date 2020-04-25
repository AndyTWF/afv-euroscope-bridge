#include "pch.h"
#include "AfvRadarScreen.h"


AfvRadarScreen::AfvRadarScreen()
{
    this->windowRect = {
        this->startCoordinate,
        this->startCoordinate,
        this->startCoordinate + this->windowWidth,
        this->startCoordinate + this->windowHeight,
    };

    this->backgroundBrush = CreateSolidBrush(RGB(0, 0, 0));
}

AfvRadarScreen::~AfvRadarScreen()
{
    DeleteObject(this->backgroundBrush);
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

}

void AfvRadarScreen::OnAsrContentToBeClosed(void)
{
    delete this;
}

void AfvRadarScreen::OnMoveScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, bool Released)
{
    this->windowRect = Area;
}
