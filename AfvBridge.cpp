#include "pch.h"
#include "AfvBridge.h"
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

#ifdef _DEBUG
bool AfvBridge::OnCompileCommand(const char* command)
{
    std::string commandString(command);

    // Check message
    if (commandString.substr(0, 5) != ".afv ") {
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
    EuroScopePlugIn::CGrountToAirChannel channel = this->GroundToArChannelSelectFirst();

    while (true) {
        if (!channel.IsValid()) {
            return;
        }

        if (std::abs(channel.GetFrequency() - frequency) < this->frequencyDeviation) {

            if (channel.GetIsTextReceiveOn() != receive) {
                channel.ToggleTextReceive();
            }

            if (channel.GetIsTextTransmitOn() != transmit) {
                channel.ToggleTextTransmit();
            }
        }

        channel = this->GroundToArChannelSelectNext(channel);
    }
}
