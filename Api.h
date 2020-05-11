#pragma once

void SendSelfMessage(std::string message);
void SendAfvClientMessage(std::string message);
void StartAfvClient(void);
void SendMessageToTarget(HWND target, std::string message);
BOOL CALLBACK GetAfvWindow(HWND hwnd, LPARAM lParam);
DWORD GetAfvProcessId(void);
std::wstring GetDllPath(void);