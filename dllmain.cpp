// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include "AfvBridge.h"
#include "Api.h"

AfvBridge bridge;

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

/*
    Called by Euroscope when the plugin is loaded, either on startup (if previously loaded and saved
    in settings) or when manually loaded by the user.
*/
void __declspec(dllexport) EuroScopePlugInInit(EuroScopePlugIn::CPlugIn** ppPlugInInstance)
{
    // Start up AFV standalone
    StartAfvClient();

    // Give ES the plugin instance and run the post initialisation method.
    *ppPlugInInstance = &bridge;
}


/*
    Called by Euroscope when the plugin is unloaded, either by the user or on exit.
*/
void __declspec(dllexport) EuroScopePlugInExit(void)
{
    // Tell AFV we're shutting down
    SendAfvClientMessage("CLOSING");
}
