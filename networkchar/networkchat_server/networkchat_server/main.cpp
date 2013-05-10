#include "server.h"
#include <stdio.h>
#include <iostream>
#include <map>
#include <string>
using namespace std;

map<SOCKET, USER_INFO> users;

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
		MessageBox ( NULL, TEXT ("This program requires Windows NT!"),
			TEXT("error"), MB_ICONERROR);
		return 0 ;
	}

	HWND hwnd;
	hwnd = CreateWindow ("network", "networkchar������������", WS_OVERLAPPEDWINDOW,
						 CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
					     NULL, NULL, NULL, NULL);
	if (NULL==hwnd)
	{
		::MessageBox(NULL, TEXT("��������ʧ��"), TEXT("����"), MB_ICONERROR);
		return -1;
	}

	WSADATA wsaData;
	WORD sockVersion = MAKEWORD(2, 2);
	if(::WSAStartup(sockVersion, &wsaData)!=0)
	{
		printf("����WINSOCK��ʧ�ܣ�\n");
		exit(0);
	}
	const unsigned short nPort = 4567;
	//���������׽���
	SOCKET sListen = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
#ifdef _DEBUG
	cout <<"listen socket: " <<(int)sListen <<endl;
#endif
	sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(nPort);
//	server_addr.sin_addr.S_un.S_addr = inet_addr("192.168.73.1");
	server_addr.sin_addr.S_un.S_addr = INADDR_ANY;
	//���׽��ֵ����ػ���
	if(::bind(sListen, (sockaddr*)&server_addr, sizeof(server_addr))==SOCKET_ERROR)
	{
		printf("�󶨼����׽���ʧ��\n");
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
					user.addr = remote_addr;    
					users.insert(pair<SOCKET, USER_INFO>(client, user));
					//
					break;
				}
			case FD_WRITE:
				{
#ifdef _DEBUG
					cout <<"FD_WRITE message" <<endl;
#endif
					break;
				}
			case FD_READ:
				{
#ifdef _DEBUG
					cout <<"FD_READ message" <<endl;
#endif
					char szText[1024];
					memset(szText, 0, sizeof(szText));
					if (::recv(s, szText, sizeof(szText),0)<0)
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
					cout <<"FD_CLOSE message" <<endl;
#endif
					users.erase(s);   // �Ƴ��û�
					::closesocket(s);
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
		    cout <<"��������" <<endl;
			// �����û���
			users[current_socket].user_name = recv_message->user_name;  
#ifdef _DEBUG
			cout <<"Current Connect User: " <<recv_message->user_name  <<"length: " << strlen(recv_message->user_name)<<endl;
			cout <<"String User: " <<users[current_socket].user_name <<"length: " <<users[current_socket].user_name.length() <<endl;
#endif
			//�ಥ�û���Ϣ
			SOCKET s = ::socket(AF_INET, SOCK_DGRAM, 0);
			sockaddr_in si;
			si.sin_family = AF_INET;
			si.sin_port = ::htons(5000);
			si.sin_addr.S_un.S_addr =::inet_addr("234.5.6.7");  

			map<SOCKET, USER_INFO>::iterator it;
			std::string user_name;
			int data_length = 0;
			for (it = users.begin(); it != users.end(); ++it) {
				user_name += it->second.user_name;
			    data_length += it->second.user_name.length();
				user_name += '/';
			    data_length++;	
			}
			
		    char *buff = new char[sizeof(MSG_INFO) + data_length];
		    memset(buff, 0, sizeof(buff));
			pMsgInfo msg_info = (pMsgInfo)buff;
			msg_info->type = MT_MULTICASTING_USERINFO;//��Ϣ����Ϊ�㲥���û���Ϣ
			//just����ip
			msg_info->addr = inet_addr("192.168.1.110");
			strncpy(msg_info->user_name,"server", sizeof(msg_info->user_name));
			//todo:δ��ʼ��ip�ֶ�
			msg_info->data_length = data_length;
			strncpy(msg_info->data(), user_name.c_str(), user_name.length());
			buff[sizeof(MSG_INFO) + data_length] = '\0';
			cout<<"buff:"<<buff<<endl;
			cout<<"�û���Ϣ��"<<msg_info->data()<<endl;
			//��ʾ��ǰ�����û�
			ShowOnlineUser(NULL);
			cout<<"�������ݳ��ȣ�"<<sendto(s, buff, sizeof(MSG_INFO) + data_length, 0, (sockaddr *)&si, sizeof(si));
			delete [] buff;
			break;
		}
	case MT_REQUEST_IP: //ip��ѯ����
		{
			break;
		}
	case MT_REQUEST_ALLUSERINFO: //�����ȡ�����û���Ϣ
		{
			break; 
		}
		return true;
	}
	return false;
}

bool ShowOnlineUser(USER_INFO *pOnlineUser)
{
	cout<<"��ǰ�����û���Ϣ��"<<endl;
	cout<<"�û���--"<<"IP��ַ"<<endl;
	map<SOCKET, USER_INFO>::iterator it_user;
	std::string user_name;
	int data_length = 0;
	for (it_user = users.begin(); it_user != users.end(); ++it_user) 
	{
		cout<<it_user->second.user_name<<"--";
		cout<<inet_ntoa(it_user->second.addr.sin_addr)<<endl;
	}
	return true;
}