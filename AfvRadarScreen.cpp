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
    this->OnAsrContentToBeSaved();
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

    // Last Received
    std::wstring lastReceived = L"Last: BAW47C";
    DrawText(hdc, lastReceived.c_str(), lastReceived.size(), &this->lastReceivedRect, DT_VCENTER | DT_CENTER | DT_WORDBREAK);
}

bool AfvRadarScreen::IsInteger(std::string number) const
{
    return !number.empty() && std::find_if(number.begin(),
        number.end(), [](unsigned char c) { return !std::isdigit(c); }) == number.end();
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
        xPos + this->margin,
        yPos + this->headerHeight + this->txRxOffsetY,
        xPos + this->margin + this->txRxWidth,
        yPos + this->headerHeight + this->txRxOffsetY + this->txRxHeight,
    };

    this->rxRect = {
        xPos + this->margin + this->txRxWidth + this->txRxGap,
        yPos + this->headerHeight + this->txRxOffsetY,
        xPos + this->margin + this->txRxWidth + this->txRxGap + this->txRxWidth,
        yPos + this->headerHeight + this->txRxOffsetY + this->txRxHeight,
    };

    this->lastReceivedRect = {
        xPos + this->margin,
        this->txRect.bottom + this->lastReceivedOffset,
        this->rxRect.right,
        this->txRect.bottom + this->lastReceivedOffset + this->lastReceivedHeight
    };
}

void AfvRadarScreen::OnAsrContentLoaded(bool loaded)
{
    if (!loaded) {
        return;
    }

    // Visibility
    if (this->GetDataFromAsr(this->settingKeyVisible.c_str()) != NULL) {
        this->shouldRender = strcmp(this->GetDataFromAsr(this->settingKeyVisible.c_str()), "0") == 0 ? false : true;
    }

    // Position
    if (
        this->GetDataFromAsr(this->settingKeyXPos.c_str()) != NULL &&
        this->GetDataFromAsr(this->settingKeyYPos.c_str()) != NULL
    ) {
        std::string xPos = this->GetDataFromAsr(this->settingKeyXPos.c_str());
        std::string yPos = this->GetDataFromAsr(this->settingKeyYPos.c_str());

        if (this->IsInteger(xPos) && this->IsInteger(yPos)) {
            this->Move(std::stoi(xPos), std::stoi(yPos));
        }
    }
}

void AfvRadarScreen::OnAsrContentToBeClosed(void)
{
    delete this;
}

void AfvRadarScreen::OnAsrContentToBeSaved(void)
{
    // Visible
    this->SaveDataToAsr(
        this->settingKeyVisible.c_str(),
        this->settingDescriptionVisible.c_str(),
        this->shouldRender ? "1" : "0"
    );

    // X Pos
    this->SaveDataToAsr(
        this->settingKeyXPos.c_str(),
        this->settingDescriptionXPos.c_str(),
        std::to_string(this->windowRect.left).c_str()
    );

    // Y Pos
    this->SaveDataToAsr(
        this->settingKeyYPos.c_str(),
        this->settingDescriptionYPos.c_str(),
        std::to_string(this->windowRect.top).c_str()
    );
}

void AfvRadarScreen::OnMoveScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, bool Released)
{
    this->Move(Area.left, Area.top);
}
