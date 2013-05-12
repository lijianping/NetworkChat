#include "resource.h"
#include "my_socket.h"
#include "my_list_box.h"
#include "chat.h"

#include <stdio.h>
#include <fstream>
#include <string>
#include <map>
using namespace std;

MySocket *client;
map<string, HWND> chat_windows;
list<string> temp_file;        // 临时文件列表



bool HandleMsg(HWND hwnd, MSG_INFO * msg);

INT_PTR CALLBACK MainProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	LoadLibrary("riched20.dll");
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
			::EnableWindow(GetDlgItem(hwndDlg, IDC_DISCONNECT_SERVER), FALSE);  // 禁用断开连接
			return TRUE;
		}
	case WM_COMMAND:
		{
			switch (LOWORD(wParam)) 
			{
			case IDC_CONNECT_SERVER:
				{
					// get user name
					char current_user_name[64];
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
					const string invalid_ip("0.0.0.0");
					if (invalid_ip == server_ip) 
					{
						MessageBox(hwndDlg, "Server ip address is invalid!", "HIT", MB_ICONINFORMATION | MB_OK);
						return FALSE;
					}
					// connect to the server
					try {
						client->ConnectSever(server_ip);
						MessageBox(hwndDlg, "Connect server succeed!", "Hit", MB_ICONINFORMATION);
						client->UserLogin();
					} catch (Err &err) {
						MessageBox(hwndDlg, err.what(), "Error At Connect", MB_ICONINFORMATION);
						return FALSE;
					}
					// set the user name read only
					SendMessage(GetDlgItem(hwndDlg, IDC_USER_NAME), EM_SETREADONLY, 1, 0);
					::EnableWindow(GetDlgItem(hwndDlg, IDC_DISCONNECT_SERVER), TRUE);  // 启用断开连接
					::EnableWindow(GetDlgItem(hwndDlg, IDC_CONNECT_SERVER), FALSE);    // 禁用连接
					break;
				}
			case IDC_DISCONNECT_SERVER:          // 断开服务器
				{
					client->DisconnectServer();
					::EnableWindow(GetDlgItem(hwndDlg, IDC_DISCONNECT_SERVER), FALSE);  // 禁用断开连接
					::EnableWindow(GetDlgItem(hwndDlg, IDC_CONNECT_SERVER), TRUE);      // 启用连接
					MyListBox friend_list(GetDlgItem(hwndDlg, IDC_FRIEND_LIST), IDC_FRIEND_LIST);
					friend_list.DeleteAllString();
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
						list_box.GetText(selected, buffer);    // 获取好友列表名称
						// 判断聊天对话框是否打开，已打开则不用再打开
						map<string, HWND>::const_iterator it = chat_windows.find(buffer);
						if (it == chat_windows.end())    // 打开好友聊天窗口
						{
							// HIT: 在未加入判断时，不能正确将服务端响应的数据传送到聊天对话框中
							//如果点击的用户是“群聊”,则不用发送查询Ip请求
							if (string(buffer) != string("群消息"))
							{
								try {
									client->RequestUserIp(buffer, strlen(buffer));
								} catch (Err &err) {
									MessageBox(hwndDlg, err.what() , "Error", MB_ICONERROR);
									break;
								}
							}
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
					client->DisconnectServer();
					list<string>::const_iterator file_index = temp_file.begin();
					while (file_index != temp_file.end()) {
						remove((*file_index).c_str());
						temp_file.erase(file_index);
						file_index = temp_file.begin();
					}
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
					client->DisconnectServer();
					list<string>::const_iterator file_index = temp_file.begin();
					while (file_index != temp_file.end()) {
						remove((*file_index).c_str());
						temp_file.erase(file_index);
						file_index = temp_file.begin();
					}
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
			list_box.AddString("群消息");
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
			map<string,HWND>::const_iterator it_group = chat_windows.find("群消息");
			if (it_group == chat_windows.end())
			{
				std::string file_name;
				file_name += msg->user_name;
				file_name += ".";
				file_name += "notshow";
				fstream out;
				try {
					open_file(out, file_name.c_str());
				} catch (Err &err) {
					MessageBox(hwnd, err.what(), "Error", MB_ICONERROR | MB_OK);
					break;
				}
			    out <<msg->user_name <<endl;
				out <<msg->data() <<endl;
				//TODO:保存消息
				MessageBox(hwnd, msg->data(), TEXT("收到群聊消息_xieruwenben"), MB_OK);
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
				MessageBox(hwnd, "can not find window hwnd", "Error", MB_ICONERROR);
				break;
			}
			::SendMessage(it->second, WM_USER_IP, 0, (LPARAM)msg->data());
			break;
		}
	case MT_SINGLE_TALK:   // 私聊信息
		{
			std::string file_name;
			file_name +=  msg->user_name;
			file_name += "--";
			file_name += client->user_name();
			//文本保存发消息的用户名和时间
			std::string msg_user_text;
			msg_user_text += msg->user_name;
			msg_user_text += ' ';
			msg_user_text += GetTime();
			msg_user_text += '\n';
			msg_user_text += msg->data();
	//		MessageBox(NULL,msg->data(), TEXT("收到私聊消息_写入文本"), MB_OK);
			//如果与发来此消息的用户的聊天窗口已打开，则发送到此窗口
			map<string,HWND>::const_iterator it_user_window = chat_windows.find(msg->user_name);
			if (it_user_window != chat_windows.end())
			{
				BringWindowToTop(it_user_window->second);
				SendMessage(it_user_window->second, WM_SINGLE_TALK, 0, (LPARAM)msg);
			}
			break;
		}
	}
	return true;
}


