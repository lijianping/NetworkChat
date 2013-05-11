#include "chat.h"
#include "my_list_box.h"
#include "resource.h"

extern MySocket *client;
extern map<std::string, HWND> chat_windows;
BOOL CALLBACK ChatDlgProc(HWND hChatDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static USER_INFO remote_user;
	static char user_name[64] = {0};
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			SetWindowText(hChatDlg, (LPCTSTR)lParam);
			char *buff = (char *)lParam;
			int length_buff = strlen(buff);
			char *name_mark = strchr(buff, '/');
			//�������������û���
			strncpy_s(remote_user.user_name, buff, name_mark -  buff);
			//�����Լ����û���
			strncpy_s(user_name, name_mark+1, length_buff - (name_mark - buff) - 1);
			chat_windows.insert(pair<string, HWND>(remote_user.user_name, hChatDlg));
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
					//���췢�Ͱ���������������
					char send_buff[sizeof(MSG_INFO)+1024];
					pMsgInfo send_msg = (pMsgInfo)send_buff;
					strncpy_s(send_msg->user_name, user_name, strlen(user_name));
					send_msg->data_length = strlen(send_text);
					strncpy(send_msg->data(), send_text, strlen(send_text));
					//�жϷ��͵���Ϣ���ͣ�Ⱥ��Ϣ��tcp������������˽����Ϣ��UDP����
					if (strcmp(remote_user.user_name,"Ⱥ��")==0)
					{
					//	MessageBox(hChatDlg, TEXT("Ⱥ����Ϣ"), TEXT("����"),0);
						send_msg->type = MT_MULTICASTING_TEXT;
						client->Send(send_buff, sizeof(MSG_INFO)+strlen((send_text)));
					}
					else
					{
						//TODO:����
						MessageBox(hChatDlg, TEXT("˽����Ϣ"), TEXT("����"),0);
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
		//		MessageBox(hChatDlg, TEXT("��ʾȺ��Ϣ"), TEXT("debug"), 0);
				//��ʾ����Ϣ���û�����ʱ��
				pMsgInfo group_msg = (pMsgInfo)lParam;
				std::string user_name_time;
				user_name_time += group_msg->user_name;
				user_name_time += ' ';
				user_name_time += GetTime();
	    		MyListBox chat_record(GetDlgItem(hChatDlg, IDC_MESSAGE_RECORD), IDC_MESSAGE_RECORD);
				chat_record.AppendString(user_name_time.c_str());
				chat_record.AppendString((char *)group_msg->data());
			return TRUE;
		}
	case WM_SINGLE_TALK:
		{
			MessageBox(hChatDlg, "�յ�˽����Ϣ", "˽��", 0);
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