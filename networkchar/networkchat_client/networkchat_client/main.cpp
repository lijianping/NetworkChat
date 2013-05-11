#include "resource.h"
#include "my_socket.h"
#include "my_list_box.h"
#include "chat.h"
#include "save_msg.h"
#include <stdio.h>
#include <string>
#include <map>
using namespace std;

MySocket *client;
map<std::string, HWND> chat_windows;
char current_user_name[64] = {0};
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
			HICON main_icon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
			if (main_icon) 
			{
				SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, (LPARAM)main_icon);
				SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)main_icon);
			}
			//SendDlgItemMessage(hwndDlg, IDC_FRIEND_LIST, LB_ADDSTRING, 0, (LPARAM)"sele");
			return TRUE;
		}
	case WM_COMMAND:
		{
			switch (LOWORD(wParam)) 
			{
			case IDC_CONNECT_SERVER:
				{
					// get user name
					memset(current_user_name, 0, sizeof(current_user_name));
					if (0 == GetWindowText(GetDlgItem(hwndDlg, IDC_USER_NAME), current_user_name, sizeof(current_user_name))) 
					{
						MessageBox(hwndDlg, "User Name can not be empty!", "HIT", MB_ICONINFORMATION | MB_OK);
						return FALSE;
					}
					client->set_user_name(current_user_name);
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
						MessageBox(hwndDlg, "Connect server succeed!", "Hit", MB_ICONINFORMATION);
						client->UserLogin();
						//TODO:增加UDP接收线程

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
						// 判断聊天对话框是否打开，已打开则不用再打开
						map<string, HWND>::const_iterator it = chat_windows.find(buffer);
						if (it == chat_windows.end())    // 打开好友聊天窗口
						{
							// HIT: 在未加入判断时，不能正确将服务端响应的数据传送到聊天对话框中
							//如果点击的用户是“群聊”,则不用发送查询Ip请求
							if (strcmp(buffer, "群聊")!=0)
							{
							   client->RequestUserIp(buffer, strlen(buffer));
							}
							//把当前用户名追加在后面，再在聊天对话框里面解析出来
							strcat_s(buffer, "/");
							strcat_s(buffer, current_user_name);
							DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_CHAT), NULL, (DLGPROC)ChatDlgProc, (LPARAM)buffer);
						}
						else 
						{
							BringWindowToTop(it->second);    // 将当前选中好友窗口移到顶层
						}
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
		{
			map<string, HWND>::const_iterator it = chat_windows.begin();
			if (it == chat_windows.end()) 
			{
				if (IDYES == MessageBox(hwndDlg, "是否退出聊天程序？", "网络聊天程序", MB_YESNO))
				{
					client->CloseSocket();
					EndDialog(hwndDlg, 0);
				}
			} 
			else 
			{
				if (IDYES == MessageBox(hwndDlg, "是否关闭所有聊天窗口并退出？", "网络聊天程序", MB_YESNO))
				{
					while (it != chat_windows.end())
					{
						SendMessage((it++)->second, WM_CLOSE, 0, 0);
					}
					client->CloseSocket();
					EndDialog(hwndDlg, 0);
				}
			}
			return TRUE;
		}
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
			//默认添加一个“群聊”成员
			list_box.AddString("群聊");
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
			//判断多播聊天窗口是否打开，未打开则把消息写入文本
			map<string,HWND>::const_iterator it_group = chat_windows.find("群聊");
			if (it_group == chat_windows.end())
			{
				std::string file_name;
				file_name += msg->user_name;
				file_name += "--";
				file_name += current_user_name;
				//保存发消息的用户名和时间
				std::string user_name_time;
				user_name_time += msg->user_name;
				user_name_time += ' ';
				user_name_time += GetTime();
				SaveMsg save_msg(file_name.c_str());
				save_msg.SaveMsgText(user_name_time.c_str());
				
				//TODO:保存消息
				MessageBox(NULL,msg->data(), TEXT("收到群聊消息_xieruwenben"), MB_OK);
			}
			else
			{
				BringWindowToTop(it_group->second);
				SendMessage(it_group->second, WM_GROUP_TALK, 0, (LPARAM)msg);
			}
			break; 
		}
	case MT_RESPOND_IP:     //响应ip请求
		{
			pUserInfo user_addr = (pUserInfo)msg->data();
			string user_name = user_addr->user_name;
			map<string, HWND>::const_iterator it = chat_windows.find(user_name);
			if (it == chat_windows.end()) 
			{
				return false;
				break;
			}
			::SendMessage(it->second, WM_USER_IP, 0, (LPARAM)msg->data());
			break;
		}
	}
	return true;
}