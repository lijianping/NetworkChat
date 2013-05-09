#include "chat.h"

BOOL CALLBACK ChatDlgProc(HWND hChatDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
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
			}
		  return TRUE;
		}
	case  WM_CLOSE:
		{
			EndDialog(hChatDlg, 0);
			return TRUE;
		}
	}
	return FALSE;
}