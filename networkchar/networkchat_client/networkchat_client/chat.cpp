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
	static int chat_width, history_width;     // ����ʱ�Ĵ��ڿ�ȼ��鿴��ʷ��¼ʱ��������
	static int is_show_history;
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			SetWindowText(hChatDlg, (LPCTSTR)lParam);
			// �����Ѵ򿪵�����Ի������Ӧ�������û�
			chat_windows.insert(pair<string, HWND>((char *)lParam, hChatDlg));
			// ������Ϣ��ʾrich edit�ؼ���Ҫ�س�
			::SendMessage(GetDlgItem(hChatDlg, IDC_MESSAGE_RECORD), ES_WANTRETURN, 0, 0);
			// ��ȡ��Ϣ
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
					int len = GetDlgItemText(hChatDlg, IDC_SEND_TEXT, send_text, 4096);        // TODO: ����4096�ֽڵ���Ҫ�ִη���
					if (0 == len) {
						MessageBox(hChatDlg, "�������ݲ���Ϊ�գ�", "�����������", MB_ICONINFORMATION | MB_OK);
						return FALSE;
					}
					//���췢�Ͱ��������������� Ⱥ��ͨ��������ת��
					string user_send_time(client->user_name());   // �û����û�������Ϣ��ʱ��
					user_send_time += " ";
					user_send_time += GetTime();
					user_send_time += "\n";  
					string message(send_text);
					message = user_send_time + message;          
				    message += "\n";
					char *send_data = new char[sizeof(MSG_INFO) + message.length()];
					memset(send_data, 0, sizeof(MSG_INFO) + message.length());
					pMsgInfo send_msg = (pMsgInfo)send_data;
					string send_user = client->user_name();    // ��ȡ���������Ʋ����
					strncpy_s(send_msg->user_name, send_user.c_str(), send_user.length());
					// ���÷��͵����ݳ��Ȳ���䷢�͵�����
				    send_msg->data_length = message.length();  
					// ��ֵ���͵�����
					strncpy(send_msg->data(), message.c_str(), message.length());    
					
					//�жϷ��͵���Ϣ���ͣ�Ⱥ��Ϣ��tcp������������˽����Ϣ��UDP����
					char title[64];
					memset(title, 0, sizeof(title));
					GetWindowText(hChatDlg, title, sizeof(title));
					const string group_msg = "Ⱥ��Ϣ";
					if (string(title) == group_msg)
					{
						send_msg->type = MT_MULTICASTING_TEXT;           // �����Ϣ����
						client->Send(send_data, sizeof(MSG_INFO) + message.length());
						SetDlgItemText(hChatDlg, IDC_SEND_TEXT, "");     // ��շ�����Ϣ�Ի���
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
						SetDlgItemText(hChatDlg, IDC_SEND_TEXT, "");     // ��շ�����Ϣ�Ի���
						// ������Ϣ��¼
						char chat_name[64];
						memset(chat_name, 0, sizeof(chat_name));
						GetWindowText(hChatDlg, chat_name, sizeof(chat_name));
						string file_name = client->user_name();
						file_name += "-";
						file_name += chat_name;
						file_name += ".history";         // ��ʷ��¼�ļ��� ���û�-�����û�.history ��ʽ
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
					if (is_show_history % 2) {        // �鿴��ʷ��¼
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
			// ����Զ���û���Ϣ
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
			//��ʾ����Ϣ���û�����ʱ��
			pMsgInfo group_msg = (pMsgInfo)lParam;
			RichEdit rich_edit(GetDlgItem(hChatDlg, IDC_MESSAGE_RECORD), IDC_MESSAGE_RECORD);
			rich_edit.AppendText(group_msg->data());
			string file_name = client->user_name();
			file_name += "-";
			file_name += "Ⱥ��Ϣ.history";
			list<string>::const_iterator find_it = find(temp_file.begin(), temp_file.end(), file_name);
			if (find_it == temp_file.end()) {
				temp_file.insert(find_it, file_name);
				--find_it;   // ʹfindָ��file_name
			}
			// ������Ϣ��¼
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
			//��ʾ����Ϣ���û�����ʱ��
			pMsgInfo single_talk_msg = (pMsgInfo)lParam;
			RichEdit rich_edit(GetDlgItem(hChatDlg, IDC_MESSAGE_RECORD), IDC_MESSAGE_RECORD);
			rich_edit.AppendText(single_talk_msg->data());

			char chat_name[64];
			memset(chat_name, 0, sizeof(chat_name));
			GetWindowText(hChatDlg, chat_name, sizeof(chat_name));
			string file_name = client->user_name();
			file_name += "-";
			file_name += chat_name;
			file_name += ".history";         // ��ʷ��¼�ļ��� ���û�-�����û�.history ��ʽ
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