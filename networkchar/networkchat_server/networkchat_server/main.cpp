#include "server.h"
#include <stdio.h>
#include <iostream>
#include <map>
#include <string>
using namespace std;

map<SOCKET, USER_INFO> users;


/*
 * @ brief: ��ȡϵͳʱ��
 * @ return: �ַ���ʽʱ�� ��ʽΪ YYYY-MM-DD hh:mm:ss
 **/
const string GetTime();

LRESULT CALLBACK NetworkProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


map<string, USER_INFO>user_map;
//map<string, USER_INFO>:iterator it_user;

int main() 
{
	WNDCLASS wndclass;
	wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wndclass.lpfnWndProc = NetworkProc ;
	wndclass.cbClsExtra = 0 ;
	wndclass.cbWndExtra = 0 ;
	wndclass.hInstance = NULL;
	wndclass.hIcon = LoadIcon (NULL, IDI_APPLICATION) ;
	wndclass.hCursor = LoadCursor (NULL, IDC_ARROW) ;
	wndclass.hbrBackground= (HBRUSH)GetStockObject(WHITE_BRUSH) ;
	wndclass.lpszMenuName = NULL ;
	wndclass.lpszClassName= "network";
	if (!RegisterClass (&wndclass))
	{
		cout <<"This program requires Windows NT!" <<endl;
		return 0 ;
	}

	HWND hwnd;
	hwnd = CreateWindow ("network", "networkchar������������", WS_OVERLAPPEDWINDOW,
						 CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
					     NULL, NULL, NULL, NULL);
	if (NULL==hwnd)
	{
		cout <<"Create window failed." <<endl;
		return -1;
	}

	WSADATA wsaData;
	WORD sockVersion = MAKEWORD(2, 2);
	if(::WSAStartup(sockVersion, &wsaData)!=0)
	{
		cout <<"Load WinSock lib failed." <<endl;
		return -1;
	}
	const unsigned short nPort = 4567;
	//���������׽���
	SOCKET sListen = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	cout <<"Start running server..." <<endl;
	sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(nPort);
//	server_addr.sin_addr.S_un.S_addr = inet_addr("192.168.73.1");
	server_addr.sin_addr.S_un.S_addr = INADDR_ANY;
	//���׽��ֵ����ػ���
	if(::bind(sListen, (sockaddr*)&server_addr, sizeof(server_addr))==SOCKET_ERROR)
	{
		cout <<"Bind listen socket failed." <<endl;
		return -1;
	}
	//���׽�����Ϊ����֪ͨ��Ϣ����
	::WSAAsyncSelect(sListen, hwnd, WM_SOCKET, FD_ACCEPT|FD_CLOSE);
	::listen(sListen, CLIENT_MAX);

	MSG msg;
	while (GetMessage(&msg,NULL,0,0))
	{
		::TranslateMessage (&msg);
		::DispatchMessage (&msg);
	}
	WSACleanup();

	return 0;
}

LRESULT CALLBACK NetworkProc(HWND hwnd,	UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_SOCKET:
		{
			SOCKET s = wParam;//ȡ�����¼��������׽��־��
#ifdef _DEBUG
			cout <<"scoket: " <<(int)s <<endl;
			cout <<"Error NO: " <<WSAGETSELECTERROR(lParam) <<endl;
#endif
			//�鿴�Ƿ����
			if (WSAGETSELECTERROR(lParam))
			{
				int err_no = WSAGetLastError();
				cout <<"ERROR: " <<err_no <<endl;
				::closesocket(s);
				return 0;
			}
			//���������¼�
			switch(WSAGETSELECTEVENT(lParam))
			{
			case FD_ACCEPT:
				{
#ifdef _DEBUG
					cout <<"FD_ACCEPT message" <<endl;
					cout <<"list:" <<s<<endl;
#endif              
					sockaddr_in remote_addr;
					int remote_length = sizeof(remote_addr);
					SOCKET client = ::accept(s, (SOCKADDR*)&remote_addr, &remote_length);
					::WSAAsyncSelect(client, hwnd, WM_SOCKET, FD_WRITE | FD_READ | FD_CLOSE);
					cout <<"new connection" <<endl;
					cout <<"IP: " <<::inet_ntoa(remote_addr.sin_addr) <<endl;
					cout <<"Port: " <<::ntohs(remote_addr.sin_port) <<endl;
					// ���ӵ�¼�û��ĵ�ַ��Ϣ
					USER_INFO user;
					user.addr = remote_addr;  // ��ȡ�û���ip��ַ  
					users.insert(pair<SOCKET, USER_INFO>(client, user));
					//
					break;
				}
			case FD_WRITE:
				{
#ifdef _DEBUG
					cout <<endl;
					cout <<"FD_WRITE message" <<endl;
#endif
					break;
				}
			case FD_READ:
				{
#ifdef _DEBUG
					cout <<endl;
					cout <<"FD_READ message" <<endl;
					cout << "s:" << s <<endl;
#endif
					char szText[1024];
					memset(szText, 0, sizeof(szText));
					int len = ::recv(s, szText, sizeof(szText),0);
					if (0 == len || SOCKET_ERROR == len)
					{
						closesocket(s);
					}
					else
					{
							//�����յ�����Ϣ
						HandleMessage(szText, s);
					}
					break;
				}
			case FD_CLOSE:
				{
#ifdef _DEBUG
					cout <<endl;
					cout <<"FD_CLOSE message" <<endl;
#endif
					cout <<endl;
					cout <<users[s].user_name <<" sign out at " <<GetTime() <<endl;
					users.erase(s);   // �Ƴ��û�
					::closesocket(s);
					map<SOCKET, USER_INFO>::const_iterator it;
					std::string user_name;
					int data_length = 0;
					// ��ʽ���ߺ�������
					for (it = users.begin(); it != users.end(); ++it) {
						user_name += it->second.user_name;
						data_length += strlen(it->second.user_name);
						user_name += '/';
						data_length++;
					}
					// ������Ϣ�ṹ
					char *buff = new char[sizeof(MSG_INFO) + data_length];
					memset(buff, 0, sizeof(buff));
					pMsgInfo msg_info = (pMsgInfo)buff;
					msg_info->type = MT_MULTICASTING_USERINFO;        // ��ʼ����Ϣ����
					char host_name[128];
					memset(host_name, 0, sizeof(host_name));
					::gethostname(host_name, sizeof(host_name));      // ��ȡ������
					hostent *host = ::gethostbyname(host_name);
					msg_info->addr = inet_addr(host->h_addr_list[0]); // ��ʼ��������ip��ַ
					// ��ʼ������������
					strncpy_s(msg_info->user_name,"server", sizeof(msg_info->user_name));
					msg_info->data_length = data_length;     // ��ʼ�������ֶγ���
					strncpy(msg_info->data(), user_name.c_str(), (size_t)user_name.length());
					// �������û��������º����б�
					for (it = users.begin(); it != users.end(); ++it)
					{
						::send(it->first, buff, sizeof(MSG_INFO) + data_length, 0);
					}
					delete [] buff;
					buff = NULL;
					
					break;
				}
			}
			return 0;
		}
	case WM_DESTROY:
		{
			::PostQuitMessage(0);
			return 0;
		}
	}
	return ::DefWindowProc(hwnd, uMsg, wParam, lParam);
}

bool HandleMessage(char* recv_buffer, SOCKET current_socket)
{
	MSG_INFO *recv_message=(MSG_INFO *)recv_buffer;
	int recv_msg_mark = recv_message->type;
	switch(recv_msg_mark)
	{
	case MT_CONNECT_USERINFO://�����û���Ϣ
		{
#ifdef _DEBUG
			cout <<"Current Connect User: " <<recv_message->user_name \
				 <<" length: " << strlen(recv_message->user_name)<<endl;
#endif
			// �ж��û��Ƿ����
			map<SOCKET, USER_INFO>::const_iterator user_it = users.begin();
			while (user_it != users.end()) {
				if (user_it->second.user_name == string(recv_message->user_name)) {
					break;
				}
				++user_it;
			}
			if (user_it != users.end()) {       // �û����ڣ�֪ͨ�ͻ��˸����û���
				cout <<recv_message->user_name <<" is exist." <<endl;
				cout <<"Notify the client change the user name." <<endl;
				char *send_buffer = new char[sizeof(MSG_INFO) + 2];
				memset(send_buffer, 0, sizeof(send_buffer));
				pMsgInfo send_msg = (pMsgInfo)send_buffer;
				send_msg->type = MT_MULTICASTING_USERINFO;         // �˴�ʵ����ֻ�ᷢ�͸���¼���û�
				strncpy_s(send_msg->user_name, "server", sizeof("server"));
				send_msg->data_length = 2;
				strncpy(send_msg->data(), "/", sizeof("/"));
				::send(current_socket, send_buffer, sizeof(MSG_INFO) + 2, 0);
				delete [] send_buffer;
				send_buffer = NULL;
				users.erase(current_socket);
				break;
			}
			// �����û���
			strncpy_s(users[current_socket].user_name, \
				      recv_message->user_name, \
					  sizeof(recv_message->user_name));
			sockaddr_in *remote = (sockaddr_in *)recv_message->data();
			users[current_socket].addr.sin_port = remote->sin_port;       // ��ȡ�û�UDP�󶨵Ķ˿�
			cout <<endl;
			cout <<users[current_socket].user_name <<" sign in at " <<GetTime() <<endl;
			cout <<"UDP addr: " <<inet_ntoa(users[current_socket].addr.sin_addr) <<" UDP port: " <<::ntohs(remote->sin_port) <<endl;
			map<SOCKET, USER_INFO>::const_iterator it;
			std::string user_name;
			int data_length = 0;
			// ��ʽ���ߺ�������
			for (it = users.begin(); it != users.end(); ++it) {
				user_name += it->second.user_name;
			    data_length += strlen(it->second.user_name);
				user_name += '/';
			    data_length++;
			}
			// ������Ϣ�ṹ
		    char *buff = new char[sizeof(MSG_INFO) + data_length];
		    memset(buff, 0, sizeof(buff));
			pMsgInfo msg_info = (pMsgInfo)buff;
			msg_info->type = MT_MULTICASTING_USERINFO;        // ��ʼ����Ϣ����
			char host_name[128];
			memset(host_name, 0, sizeof(host_name));
			::gethostname(host_name, sizeof(host_name));      // ��ȡ������
			hostent *host = ::gethostbyname(host_name);
			msg_info->addr = inet_addr(host->h_addr_list[0]); // ��ʼ��������ip��ַ
			// ��ʼ������������
			strncpy_s(msg_info->user_name,"server", sizeof("server"));
			msg_info->data_length = data_length;     // ��ʼ�������ֶγ���
			strncpy(msg_info->data(), user_name.c_str(), (size_t)user_name.length());
			// �������û��������º����б�
			for (it = users.begin(); it != users.end(); ++it)
			{
				::send(it->first, buff, sizeof(MSG_INFO) + data_length, 0);
			}
			delete [] buff;
			buff = NULL;
			/*
			// HIT: �ಥ�ھ������в��ܽ������ݣ������滻ΪTCP
			// �ಥ�û���Ϣ
			SOCKET s = ::socket(AF_INET, SOCK_DGRAM, 0);
			sockaddr_in si;
			si.sin_family = AF_INET;
			si.sin_port = ::htons(5000);
			si.sin_addr.S_un.S_addr =::inet_addr("234.5.6.7");  
			sendto(s, buff, sizeof(MSG_INFO) + data_length, 0, (sockaddr *)&si, sizeof(si));
			*/
			break;
		}
	case MT_REQUEST_IP: //ip��ѯ����
		{
			cout<<"Inquiry ip address:"<<endl;
			map<SOCKET, USER_INFO>::iterator it_user;
			string inquiry_name(recv_message->data());
			if (inquiry_name == "Ⱥ��Ϣ") {
				cout <<"This is a group message, not a ip query." <<endl;
				break;
			}
		    cout <<recv_message->user_name <<" request query " <<inquiry_name <<"'s ip address." <<endl;
			
			//�����û�����map���ҵ����û�
			for(it_user = users.begin(); it_user != users.end(); ++it_user)
			{
				if (inquiry_name == it_user->second.user_name)
				{
					break;
				}
			}
			if (it_user == users.end())
			{
				cout<<"Can not find this user."<<endl;
				break;
			}
			USER_INFO queryed_user;
			// ��ѯ����û���
			strncpy_s(queryed_user.user_name, inquiry_name.c_str(), inquiry_name.length());
		    queryed_user.addr = it_user->second.addr;   //��ȡ��ѯip�û���ַ�ṹ
			// ���췢�����ݣ���Ӧ�ͻ��˵�ip��ѯ����
			char buff[sizeof(MSG_INFO) + sizeof(USER_INFO)];
			memset(buff, 0, sizeof(buff));
			pMsgInfo respond_msg = (pMsgInfo)buff;
			respond_msg->type = MT_RESPOND_IP;
			strncpy_s(respond_msg->user_name, "server", sizeof("server"));
			respond_msg->data_length = sizeof(USER_INFO);
			memcpy(respond_msg->data(), &queryed_user, sizeof(USER_INFO));
			int send_length = ::send(current_socket, buff, sizeof(MSG_INFO) + sizeof(USER_INFO), 0);
			break;
		}
	case MT_REQUEST_ALLUSERINFO: // �����ȡ�����û���Ϣ
		{
			//TODO:������MT_CONNECT_USERINFOͬ
			break; 
		}
	case MT_MULTICASTING_TEXT: //Ⱥ����Ϣ
		{ 
			map<SOCKET, USER_INFO>::iterator it_user;
			//��ÿһ�������û�����Ⱥ����Ϣ
			for (it_user = users.begin(); it_user != users.end(); ++it_user)
			{
				::send(it_user->first, (char *)recv_message, sizeof(MSG_INFO) + recv_message->data_length, 0);
			}
			break;
		}
	default:
		{
		  cout <<"Unknown message type." <<endl;
		  break;
		}
		return true;
	}
	return false;
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