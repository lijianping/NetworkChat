#include "resource.h"
#include "my_socket.h"
#include <Windows.h>

INT_PTR CALLBACK MainProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
SOCKET communicate;   // client communicate socket
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, (DLGPROC)MainProc, (LPARAM)&hInstance);
	return 0;
}

INT_PTR CALLBACK MainProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) 
{
	int a=0;
	switch (uMsg) 
	{
	case WM_INITDIALOG:
		return TRUE;
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
#ifdef _DEBUG
					MessageBox(hwndDlg, user_name, "Debug", MB_ICONINFORMATION);
#endif
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
#ifdef _DEBUG
					MessageBox(hwndDlg, server_ip, "Debug", MB_ICONINFORMATION);
#endif
					// init socket lib
					MySocket init_sock;
					
					// get local host name
					char host_name[256];
					memset(host_name, 0, sizeof(host_name));
					::gethostname(host_name, sizeof(host_name));
					// get local host ip address
					hostent *host_ip = ::gethostbyname(host_name);
					// use the first ip
					char *p = host_ip->h_addr_list[0];
					if (NULL == p) 
					{
						MessageBox(hwndDlg, "Get local host ip address failed!", "HIT", MB_ICONERROR | MB_OK);
						return FALSE;
					}
					in_addr addr;
					memcpy(&addr.S_un.S_addr, p, host_ip->h_length);
#ifdef _DEBUG
					MessageBox(hwndDlg, ::inet_ntoa(addr), "Debug", MB_ICONINFORMATION);
#endif
					// client communicate socket
					communicate = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
					if (INVALID_SOCKET == communicate)
					{
						MessageBox(hwndDlg, "Create socket failed!", "Error", MB_ICONERROR);
						return FALSE;
					}
					const unsigned short port = 4567;
					sockaddr_in server_addr;
					server_addr.sin_family = AF_INET;
					server_addr.sin_port = htons(port);
					server_addr.sin_addr = addr;
					if (-1 == ::connect(communicate, (sockaddr *)&server_addr, sizeof(server_addr)))
					{
						MessageBox(hwndDlg, "Connect server failed!", "Error", MB_ICONERROR);
						return FALSE;
					} 
					MessageBox(hwndDlg, "Connect server succeed!", "Hit", MB_ICONINFORMATION);
					// set the user name read only
					SendMessage(GetDlgItem(hwndDlg, IDC_USER_NAME), EM_SETREADONLY, 1, 0);
					break;
				}
			}
			return TRUE;
		}
	case WM_CLOSE:
		EndDialog(hwndDlg, 0);
		return TRUE;
	}
	return FALSE;
}