#include "resource.h"
#include "my_socket.h"
#include "my_list_box.h"
#include "chat.h"
#include <stdio.h>
#include <string>
#include <map>
using namespace std;

MySocket *client;
map<std::string, HWND> chat_windows;

bool HandleMsg(HWND hwnd, MSG_INFO * msg);

INT_PTR CALLBACK MainProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	client = new MySocket;
	DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, (DLGPROC)MainProc, (LPARAM)hInstance);
	delete client;
	return 0;
}

INT_PTR CALLBACK MainProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) 
{
	static HINSTANCE hInstance;
	switch (uMsg) 
	{
	case WM_INITDIALOG:
		{
			client->set_main_hwnd(hwndDlg);
            hInstance = (HINSTANCE)lParam;
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
					client->set_user_name(user_name);
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
						client->InitSocketLib();
						client->ConnectSever(server_ip);
						//TODO:增加UDP，tcp接收线程
						MessageBox(hwndDlg, "Connect server succeed!", "Hit", MB_ICONINFORMATION);
						client->UserLogin();
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
					if (HIWORD(wParam) == LBN_DBLCLK)
					{
						MyListBox list_box(GetDlgItem(hwndDlg, IDC_FRIEND_LIST), IDC_FRIEND_LIST);
						int selected = list_box.GetSelect();
						if (LB_ERR == selected) 
						{
							break;
						}
						char buffer[128];
						memset(buffer, 0, sizeof(buffer));
						list_box.GetText(selected, buffer);
						client->RequestUserIp(buffer, strlen(buffer));
						DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_CHAT), NULL, (DLGPROC)ChatDlgProc, (LPARAM)buffer);
					}
					break;
				}
			}
			return TRUE;
		}
	case WM_CHATMSG:   //自定义消息
		{
			HandleMsg(hwndDlg, (MSG_INFO *)lParam );
			return TRUE;
		}
	case WM_CLOSE:
		client->CloseSocket();
		EndDialog(hwndDlg, 0);
		return TRUE;
	}
	return FALSE;
}

bool HandleMsg(HWND hwnd, MSG_INFO * msg)
{
	if (NULL == msg)
	{
		return false;
	}
	bool ret = true;
	int recv_msg_mark = msg->type;
	switch(recv_msg_mark)
	{
	case MT_MULTICASTING_USERINFO: //多播的在线用户信息
		{
			string users(msg->data());
			MyListBox list_box(GetDlgItem(hwnd, IDC_FRIEND_LIST), IDC_FRIEND_LIST);
			list_box.DeleteAllString();
			int start_pos = 0, end_pos = users.length();
			while (start_pos < end_pos) 
			{
				int mid = users.find('/', start_pos);
				string user_name = users.substr(start_pos, mid - start_pos);
				start_pos = mid + 1;
				list_box.AddString(user_name.c_str());
			}
			break;
		}
	case MT_MULTICASTING_TEXT: //多播的聊天信息
		{
			break; 
		}
	case MT_RESPOND_IP:     //响应ip请求
		{
			pUserInfo user_addr = (pUserInfo)msg->data();
			string user_name = user_addr->user_name;
			map<string, HWND>::const_iterator it = chat_windows.find(user_name);
			if (it == chat_windows.end()) 
			{
				ret = false;
				break;
			}
			::SendMessage(it->second, WM_USER_IP, 0, (LPARAM)msg->data());
			break;
		}
	}
	return ret;
}