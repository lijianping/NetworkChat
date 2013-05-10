#ifndef NETWORKCHAT_CLIENT_CHAT_H_
#define NETWORKCHAT_CLIENT_CHAT_H_

#include "my_socket.h"
#include <windows.h>
#include "resource.h"

#define WM_USER_IP (WM_USER + 101)

BOOL CALLBACK ChatDlgProc(HWND hChatDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

#endif