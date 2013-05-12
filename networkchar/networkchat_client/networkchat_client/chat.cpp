#include "chat.h"
#include "my_list_box.h"
#include "rich_edit.h"
#include "resource.h"

extern MySocket *client;
extern map<std::string, HWND> chat_windows;

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
					MyListBox chat_record(GetDlgItem(hChatDlg, IDC_MESSAGE_RECORD), IDC_MESSAGE_RECORD);
					char send_text[1024] = {0};
					int len = GetDlgItemText(hChatDlg, IDC_SEND_TEXT, send_text, 1024);        // HIT: 超过1024字节的需要分次发送
					if (0 == len) {
						MessageBox(hChatDlg, "发送数据不能为空！", "网络聊天程序", MB_ICONINFORMATION | MB_OK);
						return FALSE;
					}
					//构造发送包，发送聊天数据 群聊通过服务器转发
					char send_buff[sizeof(MSG_INFO)+1024];
					pMsgInfo send_msg = (pMsgInfo)send_buff;
					string send_user = client->user_name();    // 获取发送者名称并填充
					strncpy_s(send_msg->user_name, send_user.c_str(), send_user.length());
				    send_msg->data_length = len;               // 设置发送的数据长度并填充发送的数据
					strncpy(send_msg->data(), send_text, len);

					//判断发送的消息类型，群消息用tcp发给服务器，私聊消息用UDP发送
					char title[64];
					memset(title, 0, sizeof(title));
					GetWindowText(hChatDlg, title, sizeof(title));
					const string group_msg = "群消息";
					if (string(title) == group_msg)
					{
						send_msg->type = MT_MULTICASTING_TEXT;           // 填充消息类型
						client->Send(send_buff, sizeof(MSG_INFO) + len);
						SetDlgItemText(hChatDlg, IDC_SEND_TEXT, "");     // 清空发送消息对话框
					}
					else
					{
						//TODO: 发送
						MessageBox(hChatDlg, TEXT("私聊消息"), TEXT("发送"), 0);
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
	case WM_GROUP_TALK:
		{
			//显示发消息的用户名和时间
			pMsgInfo group_msg = (pMsgInfo)lParam;
			std::string user_name_time;
			user_name_time += group_msg->user_name;
			user_name_time += ' ';
			user_name_time += GetTime();
			user_name_time += '\n';
			user_name_time += group_msg->data();
			user_name_time += '\n';
			RichEdit rich_edit(GetDlgItem(hChatDlg, IDC_MESSAGE_RECORD), IDC_MESSAGE_RECORD);
			rich_edit.AppendText(user_name_time);
			return TRUE;
		}
	case WM_SINGLE_TALK:
		{
			MessageBox(hChatDlg, "收到私聊消息", "私聊", 0);
			//TODO:
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