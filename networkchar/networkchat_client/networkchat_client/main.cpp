#include "resource.h"
#include "my_socket.h"
#include "chat.h"
#include <stdio.h>
#include <string>
#include <Windows.h>

MySocket client;

INT_PTR CALLBACK MainProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, (DLGPROC)MainProc, (LPARAM)hInstance);
	return 0;
}

INT_PTR CALLBACK MainProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) 
{
	static HINSTANCE hInstance;
	switch (uMsg) 
	{
	case WM_INITDIALOG:
		{
			client.set_main_hwnd(hwndDlg);
            hInstance = (HINSTANCE)lParam;
			
			::SendDlgItemMessage(hwndDlg, IDC_FRIEND_LIST, LB_INSERTSTRING, 0, (long)"Japin");
			::SendDlgItemMessage(hwndDlg, IDC_FRIEND_LIST, LB_ADDSTRING, 0, (long)"多播群");
			return TRUE;
		}
	case WM_COMMAND:
		{
			switch (LOWORD(wParam)) 
			{
			case IDC_CONNECT_SERVER:
				{
					// get user name
					char user_name[256];
					memset(user_name, 0, sizeof(user_name));
					if (0 == GetWindowText(GetDlgItem(hwndDlg, IDC_USER_NAME), user_name, sizeof(user_name))) 
					{
						MessageBox(hwndDlg, "User Name can not be empty!", "HIT", MB_ICONINFORMATION | MB_OK);
						return FALSE;
					}
					client.set_user_name(user_name);
					// get server ip address
					char server_ip[16];
					memset(server_ip, 0, sizeof(server_ip));
					GetWindowText(GetDlgItem(hwndDlg, IDC_SERVER_IP), server_ip, sizeof(server_ip));
					const char invalid_ip[] = "0.0.0.0";
					if (0 == strcmp(server_ip, invalid_ip)) 
					{
						MessageBox(hwndDlg, "Server ip address is invalid!", "HIT", MB_ICONINFORMATION | MB_OK);
						return FALSE;
					}
					// connect to the server
					try {
						client.InitSocketLib();
						client.ConnectSever(server_ip);
						MessageBox(hwndDlg, "Connect server succeed!", "Hit", MB_ICONINFORMATION);
						client.UserLogin();
					} catch (Err &err) {
						MessageBox(hwndDlg, err.what(), "Error!", MB_ICONINFORMATION);
						return FALSE;
					}
					// set the user name read only
					SendMessage(GetDlgItem(hwndDlg, IDC_USER_NAME), EM_SETREADONLY, 1, 0);
					break;
				}
			case IDC_FRIEND_LIST:
				{
					//收到双击用户列表中用户名的消息
					if (HIWORD(wParam)==LBN_DBLCLK)
					{
						DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_CHAT), NULL, (DLGPROC)ChatDlgProc, NULL);
					}
					break;
				}
			}
			return TRUE;
		}
		//自定义消息
	case WM_CHATMSG:
		{
			MSG_INFO *message=(MSG_INFO *)lParam;

			MessageBox(NULL, message->data(), TEXT("用户信息"), 0);
			return TRUE;
		}
	case WM_CLOSE:
		client.CloseSocket();
		EndDialog(hwndDlg, 0);
		return TRUE;
	}
	return FALSE;
}