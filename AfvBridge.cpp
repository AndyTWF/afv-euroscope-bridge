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

void AfvBridge::OnTimer(int counter)
{

}
