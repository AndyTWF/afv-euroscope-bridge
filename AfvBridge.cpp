#include "pch.h"
#include "AfvBridge.h"
#include "AfvRadarScreen.h"
#include "HiddenWindow.h"


AfvBridge::AfvBridge(void)
    : EuroScopePlugIn::CPlugIn(
        EuroScopePlugIn::COMPATIBILITY_CODE,
        PLUGIN_TITLE,
        PLUGIN_VERSION,
        PLUGIN_AUTHOR,
        PLUGIN_COPYRIGHT
    )
{
    RegisterClass(&this->windowClass);

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

const std::set<std::string>& AfvBridge::GetLastTransmitted(void) const
{
    return this->lastTransmitted;
}

#ifdef _DEBUG
bool AfvBridge::OnCompileCommand(const char* command)
{
    std::string commandString(command);

    // Receive light check
    if (commandString == ".afv receive") {
        this->isReceiving = !this->isReceiving;
        return true;
    }

    // Transmit light check
    if (commandString == ".afv transmit") {
        this->isTransmitting = !this->isTransmitting;
        return true;
    }

    // Last callsign transmitting check
    if (commandString.substr(0, 10) == ".afv last ") {
        this->lastTransmitted.clear();
        std::string callsigns = commandString.substr(10);

        size_t pos = 0;
        while ((pos = callsigns.find(' ')) != std::string::npos) {
            this->lastTransmitted.insert(callsigns.substr(0, pos));
            callsigns.erase(0, pos + 1);
        }

        // Pick up the last one
        this->lastTransmitted.insert(callsigns);
        return true;
    }

    // Message test
    if (commandString != ".afv message") {
        return false;
    }

    // Create copy data
    std::string message = commandString.substr(5);
    std::replace(message.begin(), message.end(), ' ', ':');

    COPYDATASTRUCT cds;
    cds.dwData = 666;
    cds.cbData = message.size() + 1;
    cds.lpData = (PVOID) message.c_str();

    // Find the hidden window
    HWND window = FindWindowEx(NULL, NULL, this->windowClass.lpszClassName, NULL);
    if (window == NULL) {
        return true;
    }

    // Send the data
    SendMessage(window, WM_COPYDATA, reinterpret_cast<WPARAM>(window), reinterpret_cast<LPARAM>(&cds));
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

void AfvBridge::ProcessMessage(std::string message)
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

bool AfvBridge::ValidBoolean(std::string boolean) const
{
    return boolean == "True" || boolean == "False";
}

bool AfvBridge::ConvertBoolean(std::string boolean) const
{
    return boolean == "True";
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
