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
			// �����Ѵ򿪵�����Ի������Ӧ�������û�
			chat_windows.insert(pair<string, HWND>((char *)lParam, hChatDlg));
			// ������Ϣ��ʾrich edit�ؼ���Ҫ�س�
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
					int len = GetDlgItemText(hChatDlg, IDC_SEND_TEXT, send_text, 1024);        // HIT: ����1024�ֽڵ���Ҫ�ִη���
					if (0 == len) {
						MessageBox(hChatDlg, "�������ݲ���Ϊ�գ�", "�����������", MB_ICONINFORMATION | MB_OK);
						return FALSE;
					}
					//���췢�Ͱ��������������� Ⱥ��ͨ��������ת��
					char send_buff[sizeof(MSG_INFO)+1024];
					pMsgInfo send_msg = (pMsgInfo)send_buff;
					string send_user = client->user_name();    // ��ȡ���������Ʋ����
					strncpy_s(send_msg->user_name, send_user.c_str(), send_user.length());
				    send_msg->data_length = len;               // ���÷��͵����ݳ��Ȳ���䷢�͵�����
					strncpy(send_msg->data(), send_text, len);

					//�жϷ��͵���Ϣ���ͣ�Ⱥ��Ϣ��tcp������������˽����Ϣ��UDP����
					char title[64];
					memset(title, 0, sizeof(title));
					GetWindowText(hChatDlg, title, sizeof(title));
					const string group_msg = "Ⱥ��Ϣ";
					if (string(title) == group_msg)
					{
						send_msg->type = MT_MULTICASTING_TEXT;           // �����Ϣ����
						client->Send(send_buff, sizeof(MSG_INFO) + len);
						SetDlgItemText(hChatDlg, IDC_SEND_TEXT, "");     // ��շ�����Ϣ�Ի���
					}
					else
					{
						//TODO: ����
						MessageBox(hChatDlg, TEXT("˽����Ϣ"), TEXT("����"), 0);
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
			//��ʾ����Ϣ���û�����ʱ��
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