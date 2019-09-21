#include "pch.h"
#include "AfvBridge.h"


AfvBridge::AfvBridge(void)
    : EuroScopePlugIn::CPlugIn(
        EuroScopePlugIn::COMPATIBILITY_CODE,
        PLUGIN_TITLE,
        PLUGIN_VERSION,
        PLUGIN_AUTHOR,
        PLUGIN_COPYRIGHT
    )
{

}

/*
    Process the message queue
*/
void AfvBridge::OnTimer(int counter)
{
    this->messages.push("118.500:true:true");

    std::lock_guard<std::mutex> lock(this->messageLock);

    if (this->messages.size() == 0) {
        return;
    }

    while (this->messages.size() != 0) {
        this->ProcessMessage(this->messages.front());
        this->messages.pop();
    }
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
    return boolean == "true" || boolean == "false";
}

bool AfvBridge::ConvertBoolean(std::string boolean) const
{
    return boolean == "true";
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

            return;
        }

        channel = this->GroundToArChannelSelectNext(channel);
    }
}
