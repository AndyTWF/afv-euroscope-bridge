#include "pch.h"
#include "AfvBridge.h"
#include "AfvRadarScreen.h"
#include "HiddenWindow.h"
#include "Api.h"


AfvBridge::AfvBridge(void)
    : EuroScopePlugIn::CPlugIn(
        EuroScopePlugIn::COMPATIBILITY_CODE,
        PLUGIN_TITLE,
        PLUGIN_VERSION,
        PLUGIN_AUTHOR,
        PLUGIN_COPYRIGHT
    )
{
    if (!RegisterClass(&this->windowClass)) {
        this->DisplayUserMessage(
            "AFV_BRIDGE",
            "AFV_BRIDGE",
            "Unable to register window class for AFV Bridge",
            true,
            true,
            true,
            true,
            true
        );
        return;
    }

    this->hiddenWindow = CreateWindow(
        L"AfvBridgeHiddenWindowClass",
        L"AfvBridgeHiddenWindow",
        NULL,
        0,
        0,
        0,
        0,
        NULL,
        NULL,
        GetModuleHandle(NULL),
        reinterpret_cast<LPVOID>(this)
    );

    if (GetLastError() != S_OK) {
        this->DisplayUserMessage(
            "AFV_BRIDGE",
            "AFV_BRIDGE",
            "Unable to open communications for AFV Bridge",
            true,
            true,
            true,
            true,
            true
        );
    }
}

AfvBridge::~AfvBridge(void)
{
    if (this->hiddenWindow != NULL) {
        DestroyWindow(this->hiddenWindow);
    }

    UnregisterClass(this->windowClass.lpszClassName, GetModuleHandle(NULL));
}

/*
    Process the message queue
*/
void AfvBridge::OnTimer(int counter)
{
    this->LoginCheck();
    this->ControllerCheck();

    std::lock_guard<std::mutex> lock(this->messageLock);

    // Process any incoming messages from the standalone client
    while (this->messages.size() != 0) {
        this->ProcessMessage(this->messages.front());
        this->messages.pop();
    }
}

bool AfvBridge::IsTransmitting(void) const
{
    return this->isTransmitting;
}

bool AfvBridge::IsReceiving(void) const
{
    return this->isReceiving;
}

bool AfvBridge::IsVccsOpen(void) const
{
    return this->vccsOpen;
}

bool AfvBridge::IsSettingsOpen(void) const
{
    return this->settingsOpen;
}

const std::set<std::string>& AfvBridge::GetLastTransmitted(void) const
{
    return this->lastTransmitted;
}

#ifdef _DEBUG
bool AfvBridge::OnCompileCommand(const char* command)
{
    std::string commandString(command);

    // Message test
    if (commandString.substr(0, 5)  != ".afv ") {
        return false;
    }

    // Create copy data
    std::string message = commandString.substr(5);
    SendApiMessage(message, this->windowClass.lpszClassName);
    return true;
}
#endif // _DEBUG

/*
    Takes a message, adds it to the queue
*/
void AfvBridge::AddMessageToQueue(std::string message)
{
    std::lock_guard<std::mutex> lock(this->messageLock);
    this->messages.push(message);
}

EuroScopePlugIn::CRadarScreen* AfvBridge::OnRadarScreenCreated(const char* sDisplayName, bool NeedRadarContent, bool GeoReferenced, bool CanBeSaved, bool CanBeCreated)
{
    return new AfvRadarScreen;
}

/*
    If logged in freshly, send a FSD login message.
    If logged out, send a FSD logout message and reset the users frequency and callsign.
*/
void AfvBridge::LoginCheck(void)
{
    if (!this->isLoggedIn && this->GetConnectionType() != EuroScopePlugIn::CONNECTION_TYPE_NO) {
        SendApiMessage("FSD=TRUE", AFV_HIDDEN_WINDOW_CLASS);
        this->isLoggedIn = true;
    }
    else if (this->isLoggedIn && this->GetConnectionType() == EuroScopePlugIn::CONNECTION_TYPE_NO) {
        SendApiMessage("FSD=FALSE", AFV_HIDDEN_WINDOW_CLASS);
        this->isLoggedIn = false;
        this->userFrequency = 199.998;
        this->userCallsign = "";
    }
}

/*
    Check for callsign and primary frequency changes and send appropriate messages - only
    if logged in.
*/
void AfvBridge::ControllerCheck(void)
{
    EuroScopePlugIn::CController me = this->ControllerMyself();

    if (!me.IsValid() || !this->isLoggedIn) {
        return;
    }

    // Check callsign
    if (me.GetCallsign() != this->userCallsign) {
        this->userCallsign = me.GetCallsign();
        SendApiMessage("CONTROLLER=" + std::string(me.GetCallsign()), AFV_HIDDEN_WINDOW_CLASS);
    }

    // Check primary frequency
    if (!this->IsFrequencyMatch(this->userFrequency, me.GetPrimaryFrequency())) {
        this->userFrequency = me.GetPrimaryFrequency();
        std::stringstream stream;
        stream << std::fixed << std::setprecision(3) << me.GetPrimaryFrequency();

        SendApiMessage("PRIM=" + stream.str(), AFV_HIDDEN_WINDOW_CLASS);
    }
}

/*
    Process Transmit messages to determine if we're transmitting
*/
void AfvBridge::ProcessTxMessage(std::string message)
{
    std::string setting = message.substr(3);
    if (this->ValidBoolean(setting)) {
        this->isTransmitting = this->ConvertBoolean(setting);
    }
}

/*
    Process Transmit messages to determine if we're receiving
*/
void AfvBridge::ProcessRxMessage(std::string message)
{
    std::string setting = message.substr(3);
    if (this->ValidBoolean(setting)) {
        this->isReceiving = this->ConvertBoolean(setting);
    }
}

/*
    Process Callsign messages to get who last transmitted
*/
void AfvBridge::ProcessCallsignsMessage(std::string message)
{
    std::string callsigns = message.substr(10);
    this->lastTransmitted.clear();

    // Break up the callsigns
    std::vector<std::string> parts;
    std::stringstream ss(callsigns);
    std::string temp;

    while (std::getline(ss, temp, ',')) {
        this->lastTransmitted.insert(temp);
    }
}

/*
    Process messages to do with frequency changes in the AFV client itself
*/
void AfvBridge::ProcessFrequencyChangeMessage(std::string message)
{
    std::vector<std::string> parts;
    std::stringstream ss(message);
    std::string temp;

    while (std::getline(ss, temp, ':')) {
        parts.push_back(temp);
    }

    // Check for enough parts in the message
    if (parts.size() != 3) {
        return;
    }

    // Check for valid frequency
    std::regex frequencyRegex("^1[0-9]{2}\\.[0-9]{3}");
    if (!std::regex_match(parts[0], frequencyRegex)) {
        return;
    }

    // Check for valid settings
    if (!this->ValidBoolean(parts[1]) || !this->ValidBoolean(parts[2])) {
        return;
    }


    this->ToggleFrequency(std::stod(parts[0]), this->ConvertBoolean(parts[1]), this->ConvertBoolean(parts[2]));
}

/*
    Process messages to do with the settings dialog being closed.
*/
void AfvBridge::ProcessSettingsMessage(std::string message)
{
    std::string setting = message.substr(9);
    if (this->ValidBoolean(setting)) {
        this->settingsOpen = this->ConvertBoolean(setting);
    }
}

/*
    Process messages to do with the VCCS dialog being closed.
*/
void AfvBridge::ProcessVCCSMessage(std::string message)
{
    std::string setting = message.substr(5);
    if (this->ValidBoolean(setting)) {
        this->vccsOpen = this->ConvertBoolean(setting);
    }
}

/*
    Check the message type and delegate it to another method
*/
void AfvBridge::ProcessMessage(std::string message)
{
    if (message.substr(0, 3) == "TX=") {
        this->ProcessTxMessage(message);
    }
    else if (message.substr(0, 3) == "RX=") {
        this->ProcessRxMessage(message);
    }
    else if (message.substr(0, 10) == "CALLSIGNS=") {
        this->ProcessCallsignsMessage(message);
    }
    else if (message.substr(0, 5) == "VCCS=") {
        this->ProcessVCCSMessage(message);
    }
    else if (message.substr(0, 9) == "SETTINGS=") {
        this->ProcessSettingsMessage(message);
    }
    else {
        this->ProcessFrequencyChangeMessage(message);
    }
}

bool AfvBridge::ValidBoolean(std::string boolean) const
{
    std::transform(boolean.begin(), boolean.end(), boolean.begin(), ::toupper);
    return boolean == "TRUE" || boolean == "FALSE";
}

bool AfvBridge::ConvertBoolean(std::string boolean) const
{
    std::transform(boolean.begin(), boolean.end(), boolean.begin(), ::toupper);
    return boolean == "TRUE";
}

/*
    Find the matching frequency and setup text chat on it.
*/
void AfvBridge::ToggleFrequency(double frequency, bool receive, bool transmit)
{
    // If the frequency is our primary frequency, ignore it - ES automatically sets text RCV/XMT when
    // primary is selected.
    EuroScopePlugIn::CGrountToAirChannel primaryChannel = this->GetPrimaryFrequency();
    if (primaryChannel.IsValid() && this->IsFrequencyMatch(frequency, primaryChannel)) {
        return;
    }

    // Otherwise, loop the frequencies and toggle the first one.
    EuroScopePlugIn::CGrountToAirChannel channel = this->GroundToArChannelSelectFirst();
    while (true) {
        if (!channel.IsValid()) {
            return;
        }

        if (this->IsFrequencyMatch(frequency, channel) && !this->IsAtisChannel(channel.GetVoiceChannel())) {

            if (channel.GetIsTextReceiveOn() != receive) {
                channel.ToggleTextReceive();
            }

            if (channel.GetIsTextTransmitOn() != transmit) {
                channel.ToggleTextTransmit();
            }

            return;
        }

        channel = this->GroundToArChannelSelectNext(channel);
    }
}

/*
    Returns true if the given channel has a frequency within a reasonable deviation
    of the target frequency.
*/
bool AfvBridge::IsFrequencyMatch(double targetFrequency, EuroScopePlugIn::CGrountToAirChannel channel)
{
    return std::abs(channel.GetFrequency() - targetFrequency) < this->frequencyDeviation;
}

/*
    Returns true if the given channel has a frequency within a reasonable deviation
    of the target frequency.
*/
bool AfvBridge::IsFrequencyMatch(double targetFrequency, double matchFrequency)
{
    return std::abs(matchFrequency - targetFrequency) < this->frequencyDeviation;
}

/*
    Checks if the channel is an ATIS channel.
*/
bool AfvBridge::IsAtisChannel(std::string channel) const
{
    std::transform(channel.begin(), channel.end(), channel.begin(),
        [](unsigned char c) { return std::tolower(c); });

    return channel.find("_atis") != std::string::npos;
}

/*
    Find the channel which is the primary frequency.
*/
EuroScopePlugIn::CGrountToAirChannel AfvBridge::GetPrimaryFrequency(void)
{
    EuroScopePlugIn::CGrountToAirChannel channel = this->GroundToArChannelSelectFirst();
    while (true) {
        if (!channel.IsValid()) {
            break;
        }

        if (channel.GetIsPrimary()) {
            return channel;
        }

        channel = this->GroundToArChannelSelectNext(channel);
    }

    return EuroScopePlugIn::CGrountToAirChannel();
}
