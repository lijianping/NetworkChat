#include "chat.h"
#include "my_list_box.h"
#include "rich_edit.h"
#include "resource.h"
#include <fstream>
#include <algorithm>
using namespace std;

extern MySocket *client;
extern map<std::string, HWND> chat_windows;
extern list<string> temp_file;

BOOL CALLBACK ChatDlgProc(HWND hChatDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static USER_INFO remote_user;
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			SetWindowText(hChatDlg, (LPCTSTR)lParam);
			// 保存已打开的聊天对话框及其对应的聊天用户
			chat_windows.insert(pair<string, HWND>((char *)lParam, hChatDlg));
			// 设置消息显示rich edit控件需要回车
			::SendMessage(GetDlgItem(hChatDlg, IDC_MESSAGE_RECORD), ES_WANTRETURN, 0, 0);
			return TRUE;
		}
	case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
			case IDC_SEND:
				{
					char send_text[4096];
					memset(send_text, 0, sizeof(send_text));
					int len = GetDlgItemText(hChatDlg, IDC_SEND_TEXT, send_text, 4096);        // TODO: 超过4096字节的需要分次发送
					if (0 == len) {
						MessageBox(hChatDlg, "发送数据不能为空！", "网络聊天程序", MB_ICONINFORMATION | MB_OK);
						return FALSE;
					}
					//构造发送包，发送聊天数据 群聊通过服务器转发
					string user_send_time(client->user_name());   // 用户及用户发送消息的时间
					user_send_time += " ";
					user_send_time += GetTime();
					user_send_time += "\n";  
					string message(send_text);
					message = user_send_time + message;
					message += "\n";           
				
					char *send_data = new char[sizeof(MSG_INFO) + message.length()];
					memset(send_data, 0, sizeof(MSG_INFO) + message.length());
					pMsgInfo send_msg = (pMsgInfo)send_data;
					string send_user = client->user_name();    // 获取发送者名称并填充
					strncpy_s(send_msg->user_name, send_user.c_str(), send_user.length());
					// 设置发送的数据长度并填充发送的数据
				    send_msg->data_length = message.length();  
					// 赋值发送的数据
					strncpy(send_msg->data(), message.c_str(), message.length());    
					
					//判断发送的消息类型，群消息用tcp发给服务器，私聊消息用UDP发送
					char title[64];
					memset(title, 0, sizeof(title));
					GetWindowText(hChatDlg, title, sizeof(title));
					const string group_msg = "群消息";
					if (string(title) == group_msg)
					{
						send_msg->type = MT_MULTICASTING_TEXT;           // 填充消息类型
						client->Send(send_data, sizeof(MSG_INFO) + message.length());
						SetDlgItemText(hChatDlg, IDC_SEND_TEXT, "");     // 清空发送消息对话框
					}
					else
					{
						send_msg->type = MT_SINGLE_TALK;
						char temp[256];
						memset(temp, 0, sizeof(256));
						sprintf_s(temp, "user name: %s ip address: %s port: %d", remote_user.user_name, \
							      inet_ntoa(remote_user.addr.sin_addr), ntohs(remote_user.addr.sin_port));
						MessageBox(hChatDlg, temp, "Debug", MB_ICONINFORMATION);
						try {
							client->SendTo(send_data, sizeof(MSG_INFO) + message.length(), &remote_user.addr);
						} catch (Err &err) {
							MessageBox(hChatDlg, err.what(), "Error", MB_ICONERROR);
							break;
						}
						RichEdit rich_edit(GetDlgItem(hChatDlg, IDC_MESSAGE_RECORD), IDC_MESSAGE_RECORD);
						pMsgInfo msg = (pMsgInfo)send_data;
						string message(msg->data(), msg->data_length);
						rich_edit.AppendText(message);
						SetDlgItemText(hChatDlg, IDC_SEND_TEXT, "");     // 清空发送消息对话框
						// 保存消息记录

					}
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
			pUserInfo recv_user = (pUserInfo)lParam;
			// 保存远程用户信息
			strncpy_s(remote_user.user_name, recv_user->user_name, strlen(recv_user->user_name));
			remote_user.addr.sin_family = AF_INET;
			remote_user.addr.sin_port = recv_user->addr.sin_port;
			remote_user.addr.sin_addr = recv_user->addr.sin_addr;
			char temp[256];
			memset(temp, 0, sizeof(256));
			sprintf_s(temp, "user name: %s ip address: %s port: %d", recv_user->user_name, \
				      inet_ntoa(recv_user->addr.sin_addr), ntohs(recv_user->addr.sin_port));
			MessageBox(hChatDlg, temp, "Debug", MB_ICONINFORMATION);
			return TRUE;
		}
	case WM_GROUP_TALK:
		{
			//显示发消息的用户名和时间
			pMsgInfo group_msg = (pMsgInfo)lParam;
			RichEdit rich_edit(GetDlgItem(hChatDlg, IDC_MESSAGE_RECORD), IDC_MESSAGE_RECORD);
			rich_edit.AppendText(group_msg->data());
			string group_msg_file("groupmsg.history");
			group_msg_file += client->user_name();
			list<string>::const_iterator find_it = find(temp_file.begin(), temp_file.end(), group_msg_file);
			if (find_it == temp_file.end()) {
				temp_file.insert(find_it, group_msg_file);
				--find_it;   // 使find指向group_msg_file
			}
			// 保存消息记录
			fstream out;
			try {
				open_file(out, *find_it);
			} catch (Err &err) {
				MessageBox(hChatDlg, err.what(), "Error", MB_ICONERROR);
				return FALSE;
			}
			out <<group_msg->data();
			out.close();
			return TRUE;
		}
	case WM_SINGLE_TALK:
		{
	//		MessageBox(hChatDlg, "收到私聊消息", "私聊", 0);
			//显示发消息的用户名和时间
			pMsgInfo single_talk_msg = (pMsgInfo)lParam;
			RichEdit rich_edit(GetDlgItem(hChatDlg, IDC_MESSAGE_RECORD), IDC_MESSAGE_RECORD);
			rich_edit.AppendText(single_talk_msg->data());
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


const string GetTime()
{
	string str_time;
	SYSTEMTIME sys_time;
	GetLocalTime(&sys_time);
	char temp[16];
	memset(temp, 0, sizeof(temp));
	sprintf_s(temp, "%d", sys_time.wYear);
	str_time = string(temp) + '-';
	memset(temp, 0, sizeof(temp));
	sprintf_s(temp, "%d", sys_time.wMonth);
	str_time += (string(temp) + '-');
	memset(temp, 0, sizeof(temp));
	sprintf_s(temp, "%d", sys_time.wDay);
	str_time += (string(temp) + ' ');
	memset(temp, 0, sizeof(temp));
	sprintf_s(temp, "%d", sys_time.wHour);
	str_time += (string(temp) + ':');
	memset(temp, 0, sizeof(temp));
	sprintf_s(temp, "%d", sys_time.wMinute);
	str_time += (string(temp) + ':');
	memset(temp, 0, sizeof(temp));
	sprintf_s(temp, "%d", sys_time.wSecond);
	str_time += temp;
	return str_time;
}

fstream& open_file(fstream &in, const string &file, bool is_in /* = false */) 
{
	in.close();
	in.open(file.c_str(), ios::binary | ios::app | (is_in ? ios::out : ios::in));
	if (!in.is_open())
		LTHROW(ERR_FILE_OPEN_FAILD)
		return in;
}