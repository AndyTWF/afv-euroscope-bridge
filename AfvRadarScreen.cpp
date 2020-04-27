#include "pch.h"
#include "AfvRadarScreen.h"
#include "AfvBridge.h"
#include "Conversions.h"
#include "Api.h"


AfvRadarScreen::AfvRadarScreen()
{
    this->Move(this->startCoordinate, this->startCoordinate);
    this->backgroundBrush = CreateSolidBrush(RGB(32, 32, 32));
    this->txRxActiveBrush = CreateSolidBrush(RGB(234, 173, 92));
    this->txRxInactiveBrush = CreateSolidBrush(RGB(0, 0, 0));
    this->headerBrushDisconnected = CreateSolidBrush(RGB(255, 97, 93));
    this->headerBrushConnecting = CreateSolidBrush(RGB(253, 173, 92));
    this->headerBrushConnected = CreateSolidBrush(RGB(13, 134, 93));
    this->buttonOutlineBrush = CreateSolidBrush(RGB(102, 102, 102));
}

AfvRadarScreen::~AfvRadarScreen()
{
    this->OnAsrContentToBeSaved();
    DeleteObject(this->backgroundBrush);
    DeleteObject(this->txRxActiveBrush);
    DeleteObject(this->txRxInactiveBrush);
    DeleteObject(this->headerBrushDisconnected);
    DeleteObject(this->headerBrushConnecting);
    DeleteObject(this->headerBrushConnected);
    DeleteObject(this->buttonOutlineBrush);
}

void AfvRadarScreen::OnRefresh(HDC hdc, int phase)
{
    // Dont paint in the wrong phase
    if (phase != EuroScopePlugIn::REFRESH_PHASE_AFTER_TAGS || !this->shouldRender) {
        return;
    }

    // Make all text white
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(255, 255, 255));

    AfvBridge * plugin = (AfvBridge *) this->GetPlugIn();

    //Background
    FillRect(hdc, &this->windowRect, this->backgroundBrush);
    this->AddScreenObject(1, "afvWindow", this->windowRect, true, "");

    // Header Bar
    FillRect(hdc, &this->headerRect, this->GetStatusColourForHeader(plugin->GetAfvConnectionStatus()));
    DrawText(hdc, L"AFV", 3, &this->headerRect, DT_VCENTER | DT_CENTER);

    // Settings Button
    FrameRect(hdc, &this->settingsRect, plugin->IsSettingsOpen() ? this->txRxActiveBrush : this->buttonOutlineBrush);
    DrawText(hdc, L"SET", 3, &this->settingsRect, DT_VCENTER | DT_CENTER);
    this->AddScreenObject(1, "setButton", this->settingsRect, true, "");

    // VCCS Button
    FrameRect(hdc, &this->vccsRect, plugin->IsVccsOpen() ? this->txRxActiveBrush : this->buttonOutlineBrush);
    DrawText(hdc, L"COMM", 4, &this->vccsRect, DT_VCENTER | DT_CENTER);
    this->AddScreenObject(1, "vccsButton", this->vccsRect, true, "");

    // TX
    FillRect(hdc, &this->txRect, plugin->IsTransmitting() ? this->txRxActiveBrush : this->txRxInactiveBrush);
    DrawText(hdc, L"TX", 2, &this->txRect, DT_VCENTER | DT_CENTER);

    // RX
    FillRect(hdc, &this->rxRect, plugin->IsReceiving() ? this->txRxActiveBrush : this->txRxInactiveBrush);
    DrawText(hdc, L"RX", 2, &this->rxRect, DT_VCENTER | DT_CENTER);

    // Last Received Callsign
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::wstring last(L"Last: " + converter.from_bytes(plugin->GetLastTransmitted()));

    DrawText(hdc, last.c_str(), last.size() , &this->lastReceivedRect, DT_VCENTER | DT_LEFT);
}

HBRUSH& AfvRadarScreen::GetStatusColourForHeader(int status)
{
    switch (status) {
        case AfvBridge::AFV_STATUS_CONNECTED:
            return this->headerBrushConnected;
        case AfvBridge::AFV_STATUS_CONNECTING:
            return this->headerBrushConnecting;
        default:
            return this->headerBrushDisconnected;
    }
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

    this->settingsRect = {
        xPos + this->margin,
        this->headerRect.bottom + this->secondRowOffsetY,
        xPos + this->margin + this->buttonsWidth,
        this->headerRect.bottom + this->secondRowOffsetY + this->secondRowHeight,
    };

    this->vccsRect = {
        this->settingsRect.right + this->buttonsGap,
        this->headerRect.bottom + this->secondRowOffsetY,
        this->settingsRect.right + this->buttonsGap + this->buttonsWidth,
        this->headerRect.bottom + this->secondRowOffsetY + this->secondRowHeight,
    };

    this->txRect = {
        this->vccsRect.right + this->buttonsGap,
        this->headerRect.bottom + this->secondRowOffsetY,
        this->vccsRect.right + this->buttonsGap + this->txRxWidth,
        this->headerRect.bottom + this->secondRowOffsetY + this->secondRowHeight,
    };

    this->rxRect = {
        this->txRect.right + this->buttonsGap,
        this->headerRect.bottom + this->secondRowOffsetY,
        this->txRect.right + this->buttonsGap + this->txRxWidth,
        this->headerRect.bottom + this->secondRowOffsetY + this->secondRowHeight,
    };

    this->lastReceivedRect = {
        this->rxRect.right + this->buttonsGap,
        this->headerRect.bottom + this->secondRowOffsetY,
        this->rxRect.right + this->buttonsGap + this->callsignsWidth,
        this->headerRect.bottom + this->secondRowOffsetY + this->secondRowHeight
    };
}

void AfvRadarScreen::OnAsrContentLoaded(bool loaded)
{
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

void AfvRadarScreen::OnClickScreenObject(int objectType, const char* sObjectId, POINT pt, RECT area, int button)
{
    std::string screenObject(sObjectId);

    if (screenObject == "setButton") {
        std::string value = ConvertBool(!((AfvBridge*)this->GetPlugIn())->IsSettingsOpen());
        SendSelfMessage("SETTINGS=" + value);
        SendAfvClientMessage("SETTINGS=" + value);
    } else if (screenObject == "vccsButton") {
        std::string value = ConvertBool(!((AfvBridge*)this->GetPlugIn())->IsVccsOpen());
        SendSelfMessage("VCCS=" + value);
        SendAfvClientMessage("VCCS=" + value);
    }
}

bool AfvRadarScreen::OnCompileCommand(const char* sCommandLine)
{
    std::string command(sCommandLine);

    if (command == ".afv show") {
        this->shouldRender = true;
        return true;
    }

    if (command == ".afv hide") {
        this->shouldRender = false;
        return true;
    }

    return false;
}

void AfvRadarScreen::OnMoveScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, bool Released)
{
    this->Move(Area.left, Area.top);
}
