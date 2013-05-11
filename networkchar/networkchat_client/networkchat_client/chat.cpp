#include "chat.h"

extern map<std::string, HWND> chat_windows;
BOOL CALLBACK ChatDlgProc(HWND hChatDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			SetWindowText(hChatDlg, (LPCTSTR)lParam);
			chat_windows.insert(pair<string, HWND>((char *)lParam, hChatDlg));
			return TRUE;
		}
	case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
			case IDC_SEND:
				{
					MessageBox(NULL, "FASONG", "FDF",0);
					break;
				}
			case IDC_CLOSE_CHAT:
				{
					SendMessage(hChatDlg, WM_CLOSE, 0, 0);
					break;
				}
			}
		  return TRUE;
		}
	case WM_USER_IP:
		{
			pUserInfo user = (pUserInfo)lParam;
#ifdef _DEBUG
			MessageBox(hChatDlg, user->user_name, "Debug", MB_ICONINFORMATION);
			char szPort[16]={0};
			sprintf(szPort,"port:%d", ntohs(user->addr.sin_port));
			MessageBox(hChatDlg, szPort, "Debug", MB_ICONINFORMATION);
			MessageBox(hChatDlg, ::inet_ntoa(user->addr.sin_addr), "Debug", MB_ICONINFORMATION);
#endif
			return TRUE;
		}
	case  WM_CLOSE:
		{
			char name[64];
			memset(name, 0, sizeof(name));
			GetWindowText(hChatDlg, name, sizeof(name));
			chat_windows.erase(name);
			EndDialog(hChatDlg, 0);
			return TRUE;
		}
	}
	return FALSE;
}