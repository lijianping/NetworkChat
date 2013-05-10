#include "chat.h"
#include <string>
#include <map>
using namespace std;

extern map<string, HWND> chat_windows;


BOOL CALLBACK ChatDlgProc(HWND hChatDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static string chat_user_name;
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			char *name = (char *)lParam;
			chat_user_name = name;
			chat_windows[chat_user_name] = hChatDlg;
			return TRUE;
		}
	case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
			case IDC_SEND:
				{
					MessageBox(hChatDlg, chat_user_name.c_str(), "FDF",0);
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
			MessageBox(hChatDlg, ::inet_ntoa(user->addr.sin_addr), "Debug", MB_ICONINFORMATION);
#endif
			return TRUE;
		}
	case  WM_CLOSE:
		{
			chat_windows.erase(chat_windows.find(chat_user_name));
			EndDialog(hChatDlg, 0);
			return TRUE;
		}
	}
	return FALSE;
}