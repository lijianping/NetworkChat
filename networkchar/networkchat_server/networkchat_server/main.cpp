#include "server.h"
#include <iostream>
using namespace std;

LRESULT CALLBACK NetworkProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

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
			TEXT("error"), MB_ICONERROR) ;
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

LRESULT CALLBACK NetworkProc (
	HWND hwnd,
	UINT uMsg, 
	WPARAM wParam,
	LPARAM lParam
	)
{
	switch (uMsg)
	{
	case WM_SOCKET:
		{
			SOCKET s=wParam;//ȡ�����¼��������׽��־��
			//�鿴�Ƿ����
			if (!WSAGETSELECTERROR(lParam))
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
					sockaddr_in remote_addr;
					int remote_length = sizeof(remote_addr);
					SOCKET client = ::accept(s, (SOCKADDR*)&remote_addr, &remote_length);
					::WSAAsyncSelect(client, hwnd, WM_SOCKET, FD_WRITE|FD_ACCEPT|FD_CLOSE);
					cout <<"new connection" <<endl;
					break;
				}
			case FD_WRITE:
				{
					break;
				}
			case FD_READ:
				{
					char szText[1024]={0};
					if (::recv(s, szText, sizeof(szText),0)==-1)
					{
						closesocket(s);
					}
					else
					{
						cout <<"�������ݣ�" <<szText <<endl;
					}
					break;
				}
			case FD_CLOSE:
				{
					cout <<"Close socket at FD_CLOSE" <<endl;
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
	USER_MESSAGE* recv_message=(USER_MESSAGE* )recv_buffer;
	int recv_msg_mark = recv_message->message_mark;
	switch(recv_msg_mark)
	{
	case MT_CONNECT_USERINFO://�����û���Ϣ
		{
		    cout <<"��������" <<endl;
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
	}
	return false;
}