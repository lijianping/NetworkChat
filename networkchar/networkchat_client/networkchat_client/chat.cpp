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
			char *buff = (char *)lParam;
			int length_buff = strlen(buff);
			char *name_mark = strchr(buff, '/');
			//保存聊天对象的用户名
			strncpy_s(remote_user.user_name, buff, name_mark -  buff);
			
			chat_windows.insert(pair<string, HWND>(remote_user.user_name, hChatDlg));
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
					GetDlgItemText(hChatDlg, IDC_SEND_TEXT, send_text, 1024);
					//chat_record.AddString(send_text);
					//构造发送包，发送聊天数据
					char send_buff[sizeof(MSG_INFO)+1024];
					pMsgInfo send_msg = (pMsgInfo)send_buff;
					strncpy_s(send_msg->user_name, client->UserName().c_str(), strlen(client->UserName().c_str()));
					send_msg->data_length = strlen(send_text);
					strncpy(send_msg->data(), send_text, strlen(send_text));
					//判断发送的消息类型，群消息用tcp发给服务器，私聊消息用UDP发送
					if (strcmp(remote_user.user_name,"群聊")==0)
					{
					//	MessageBox(hChatDlg, TEXT("群聊消息"), TEXT("发送"),0);
						send_msg->type = MT_MULTICASTING_TEXT;
						client->Send(send_buff, sizeof(MSG_INFO)+strlen((send_text)));
					}
					else
					{
						send_msg->type = MT_SINGLE_TALK;
						char szRemote[64];
						sprintf(szRemote,"ip:%s,port:%d", inet_ntoa(remote_user.addr.sin_addr),ntohs(remote_user.addr.sin_port));
						MessageBox(hChatDlg, szRemote, "远端地址",0);
						int send_len = client->SendTo(send_buff, sizeof(MSG_INFO)+strlen(send_text), &remote_user.addr);
						char temp[32];
						sprintf(temp, "send length: %d", send_len);
						MessageBox(hChatDlg, temp, TEXT("发送"),0);
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
#ifdef _DEBUG
			MessageBox(hChatDlg, recv_user->user_name, "Debug", MB_ICONINFORMATION);
			char szPort[16]={0};
			sprintf(szPort,"port:%d", ntohs(recv_user->addr.sin_port));
			MessageBox(hChatDlg, szPort, "Debug", MB_ICONINFORMATION);
			MessageBox(hChatDlg, ::inet_ntoa(recv_user->addr.sin_addr), "Debug", MB_ICONINFORMATION);
#endif
			//保存远程用户信息
			strncpy_s(remote_user.user_name, recv_user->user_name, strlen(recv_user->user_name));
			remote_user.addr.sin_family = AF_INET;
			remote_user.addr.sin_port = recv_user->addr.sin_port;
			remote_user.addr.sin_addr = recv_user->addr.sin_addr;

			return TRUE;
		}
	case WM_GROUP_TALK:
		{
		//		MessageBox(hChatDlg, TEXT("显示群消息"), TEXT("debug"), 0);
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
#ifdef _DEBUG
				char temp[64];
				sprintf_s(temp, "line count: %d", rich_edit.GetLineCount());
				MessageBox(hChatDlg, temp, "Debug", MB_ICONINFORMATION);
#endif
				int tail = rich_edit.GetTail();
				rich_edit.SetSel(tail+1, tail+1);
				SendMessage(GetDlgItem(hChatDlg, IDC_MESSAGE_RECORD), WM_VSCROLL, SB_BOTTOM, 0);
				::SendMessage(GetDlgItem(hChatDlg, IDC_MESSAGE_RECORD), EM_REPLACESEL, FALSE, (long)user_name_time.c_str());
#ifdef _DEBUG
				sprintf_s(temp, "line count: %d", rich_edit.GetLineCount());
				MessageBox(hChatDlg, temp, "Debug", MB_ICONINFORMATION);
#endif
			return TRUE;
		}
	case WM_SINGLE_TALK:
		{
			MessageBox(hChatDlg, "收到私聊消息", "私聊", 0);
			//显示发消息的用户名和时间
			pMsgInfo single_talk_msg = (pMsgInfo)lParam;
			std::string user_name_time;
			user_name_time += single_talk_msg->user_name;
			user_name_time += ' ';
			user_name_time += GetTime();
			user_name_time += '\n';
			user_name_time += single_talk_msg->data();
			user_name_time += '\n';
			RichEdit rich_edit(GetDlgItem(hChatDlg, IDC_MESSAGE_RECORD), IDC_MESSAGE_RECORD);
#ifdef _DEBUG
			char temp[64];
			sprintf_s(temp, "line count: %d", rich_edit.GetLineCount());
			MessageBox(hChatDlg, temp, "Debug", MB_ICONINFORMATION);
#endif
			int tail = rich_edit.GetTail();
			rich_edit.SetSel(tail+1, tail+1);
			SendMessage(GetDlgItem(hChatDlg, IDC_MESSAGE_RECORD), WM_VSCROLL, SB_BOTTOM, 0);
			::SendMessage(GetDlgItem(hChatDlg, IDC_MESSAGE_RECORD), EM_REPLACESEL, FALSE, (long)user_name_time.c_str());
#ifdef _DEBUG
			sprintf_s(temp, "line count: %d", rich_edit.GetLineCount());
			MessageBox(hChatDlg, temp, "Debug", MB_ICONINFORMATION);
#endif
			return TRUE;
		}
	case  WM_CLOSE:
		{
			char name[64];
			memset(name, 0, sizeof(name));
			chat_windows.erase(remote_user.user_name);
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