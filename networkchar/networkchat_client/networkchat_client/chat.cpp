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
	static int chat_width, history_width;     // 聊天时的窗口宽度及查看历史记录时的聊天宽度
	static int is_show_history;
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			SetWindowText(hChatDlg, (LPCTSTR)lParam);
			// 保存已打开的聊天对话框及其对应的聊天用户
			chat_windows.insert(pair<string, HWND>((char *)lParam, hChatDlg));
			// 设置消息显示rich edit控件需要回车
			::SendMessage(GetDlgItem(hChatDlg, IDC_MESSAGE_RECORD), ES_WANTRETURN, 0, 0);
			// 读取消息
			string file_name = client->user_name();
			file_name += "-";
			file_name += (char *)lParam;
			string file_name_not_show = file_name + ".notshow";
			file_name += ".history";
			list<string>::const_iterator find_it;
			find_it = find(temp_file.begin(), temp_file.end(), file_name);
			if (find_it == temp_file.end()) {
				temp_file.insert(find_it, file_name);
				--find_it;
			}
			fstream in, out;
			try {
				open_file(in, file_name_not_show);
				open_file(out, *find_it);
			} catch (Err &err) {
				MessageBox(hChatDlg, err.what(), "Error At HandleMessage", MB_ICONERROR);
				return FALSE;
			}
			RichEdit rich_edit(GetDlgItem(hChatDlg, IDC_MESSAGE_RECORD), IDC_MESSAGE_RECORD);
			string chat_msg;
			while (getline(in, chat_msg)) {
				chat_msg += "\n";
				out <<chat_msg;
				rich_edit.AppendText(chat_msg);
			}
			out.close();
			in.close();
			remove(file_name_not_show.c_str());

			RECT chat_rect, rect;
			GetWindowRect(hChatDlg, &chat_rect);
			GetWindowRect(GetDlgItem(hChatDlg, IDC_MESSAGE_RECORD), &rect);
#ifdef MyDebug
			char temp[64];
			sprintf_s(temp, "left: %d, top: %d, right: %d, bottom: %d", chat_rect.left, chat_rect.top, chat_rect.right, chat_rect.bottom);
			MessageBox(hChatDlg, temp, "Debug", MB_ICONINFORMATION);
#endif
			history_width = chat_rect.right - chat_rect.left;
			chat_width = rect.right - rect.left + 30;
			MoveWindow(hChatDlg, chat_rect.left, chat_rect.top, chat_width, chat_rect.bottom - chat_rect.top, TRUE);
			is_show_history = 0;
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
#ifdef MyDebug
						char temp[256];
						memset(temp, 0, sizeof(256));
						sprintf_s(temp, "user name: %s ip address: %s port: %d", remote_user.user_name, \
							      inet_ntoa(remote_user.addr.sin_addr), ntohs(remote_user.addr.sin_port));
						MessageBox(hChatDlg, temp, "Debug", MB_ICONINFORMATION);
#endif
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
						char chat_name[64];
						memset(chat_name, 0, sizeof(chat_name));
						GetWindowText(hChatDlg, chat_name, sizeof(chat_name));
						string file_name = client->user_name();
						file_name += "-";
						file_name += chat_name;
						file_name += ".history";         // 历史记录文件名 主用户-聊天用户.history 格式
						list<string>::const_iterator find_it;
						find_it = find(temp_file.begin(), temp_file.end(), file_name);
						if (find_it == temp_file.end()) {
							temp_file.insert(find_it, file_name);
							--find_it;
						}
						fstream out;
						try {
							open_file(out, *find_it);
						} catch (Err &err) {
							MessageBox(hChatDlg, err.what(), "Error", MB_ICONERROR);
							break;
						}
						out <<message;
						out.close();
					}
					break;
				}
			case IDC_SHOW_HISTORY:
				{
					RECT chat_rect;
					GetWindowRect(hChatDlg, &chat_rect);
					is_show_history++;
					if (is_show_history % 2) {        // 查看历史记录
						MoveWindow(hChatDlg, chat_rect.left, chat_rect.top, history_width, chat_rect.bottom - chat_rect.top, TRUE);
						
						string file_name = client->user_name();
						file_name += "-";
						char title[32] = {0};
						GetWindowText(hChatDlg, title, sizeof(title));
						file_name += title;
						file_name += ".history";
						fstream in;
						try {
							open_file(in, file_name);
						} catch (Err &err) {
							MessageBox(hChatDlg, err.what(), "Error", MB_ICONERROR);
							break;
						}
						RichEdit history(GetDlgItem(hChatDlg, IDC_MESSAGE_HISTORY), IDC_MESSAGE_HISTORY);
						string chat_msg;
						while (getline(in, chat_msg)) {
							chat_msg += "\n";
							history.AppendText(chat_msg);
						}
						in.close();
					} else {
						MoveWindow(hChatDlg, chat_rect.left, chat_rect.top, chat_width, chat_rect.bottom - chat_rect.top, TRUE);
						SetDlgItemText(hChatDlg, IDC_MESSAGE_HISTORY, "");
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
#ifdef MyDebug		
			char temp[256];
			memset(temp, 0, sizeof(256));
			sprintf_s(temp, "user name: %s ip address: %s port: %d", recv_user->user_name, \
				      inet_ntoa(recv_user->addr.sin_addr), ntohs(recv_user->addr.sin_port));
			MessageBox(hChatDlg, temp, "Debug", MB_ICONINFORMATION);
#endif
			return TRUE;
		}
	case WM_GROUP_TALK:
		{
			//显示发消息的用户名和时间
			pMsgInfo group_msg = (pMsgInfo)lParam;
			RichEdit rich_edit(GetDlgItem(hChatDlg, IDC_MESSAGE_RECORD), IDC_MESSAGE_RECORD);
			rich_edit.AppendText(group_msg->data());
			string file_name = client->user_name();
			file_name += "-";
			file_name += "群消息.history";
			list<string>::const_iterator find_it = find(temp_file.begin(), temp_file.end(), file_name);
			if (find_it == temp_file.end()) {
				temp_file.insert(find_it, file_name);
				--find_it;   // 使find指向file_name
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
			//显示发消息的用户名和时间
			pMsgInfo single_talk_msg = (pMsgInfo)lParam;
			RichEdit rich_edit(GetDlgItem(hChatDlg, IDC_MESSAGE_RECORD), IDC_MESSAGE_RECORD);
			rich_edit.AppendText(single_talk_msg->data());

			char chat_name[64];
			memset(chat_name, 0, sizeof(chat_name));
			GetWindowText(hChatDlg, chat_name, sizeof(chat_name));
			string file_name = client->user_name();
			file_name += "-";
			file_name += chat_name;
			file_name += ".history";         // 历史记录文件名 主用户-聊天用户.history 格式
			list<string>::const_iterator find_it;
			find_it = find(temp_file.begin(), temp_file.end(), file_name);
			if (find_it == temp_file.end()) {
				temp_file.insert(find_it, file_name);
				--find_it;
			}
			fstream out;
			try {
				open_file(out, *find_it);
			} catch (Err &err) {
				MessageBox(hChatDlg, err.what(), "Error", MB_ICONERROR);
				break;
			}
			out <<single_talk_msg->data();
			out.close();

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
	in.open(file.c_str(), ios::binary | ios::app  | (is_in ? ios::out : ios::in));
	if (!in.is_open())
		LTHROW(ERR_FILE_OPEN_FAILD)
	return in;
}