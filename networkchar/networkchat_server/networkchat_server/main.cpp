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
	hwnd = CreateWindow ("network", "networkchar聊天程序服务器", WS_OVERLAPPEDWINDOW,
						 CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
					     NULL, NULL, NULL, NULL);
	if (NULL==hwnd)
	{
		::MessageBox(NULL, TEXT("创建窗口失败"), TEXT("错误"), MB_ICONERROR);
		return -1;
	}

	WSADATA wsaData;
	WORD sockVersion = MAKEWORD(2, 2);
	if(::WSAStartup(sockVersion, &wsaData)!=0)
	{
		printf("加载WINSOCK库失败！\n");
		exit(0);
	}
	const unsigned short nPort = 4567;
	//创建监听套接字
	SOCKET sListen = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
#ifdef _DEBUG
	cout <<"listen socket: " <<(int)sListen <<endl;
#endif
	sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(nPort);
//	server_addr.sin_addr.S_un.S_addr = inet_addr("192.168.73.1");
	server_addr.sin_addr.S_un.S_addr = INADDR_ANY;
	//绑定套接字到本地机器
	if(::bind(sListen, (sockaddr*)&server_addr, sizeof(server_addr))==SOCKET_ERROR)
	{
		printf("绑定监听套接字失败\n");
		return -1;
	}
	//将套接字设为窗口通知消息类型
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
			SOCKET s = wParam;//取得有事件发生的套接字句柄
#ifdef _DEBUG
			cout <<"scoket: " <<(int)s <<endl;
			cout <<"Error NO: " <<WSAGETSELECTERROR(lParam) <<endl;
#endif
			//查看是否出错
			if (WSAGETSELECTERROR(lParam))
			{
				int err_no = WSAGetLastError();
				cout <<"ERROR: " <<err_no <<endl;
				::closesocket(s);
				return 0;
			}
			//处理发生的事件
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
					char szText[1024]={0};
					if (::recv(s, szText, sizeof(szText),0)<0)
					{
						closesocket(s);
					}
					else
					{
						cout <<"接收数据：" <<szText <<endl;
					}
					break;
				}
			case FD_CLOSE:
				{
#ifdef _DEBUG
					cout <<"FD_CLOSE message" <<endl;
#endif
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
	case MT_CONNECT_USERINFO://连接用户信息
		{
		    cout <<"连接请求" <<endl;
			break;
		}
	case MT_REQUEST_IP: //ip查询请求
		{
			break;
		}
	case MT_REQUEST_ALLUSERINFO: //请求获取在线用户信息
		{
			break; 
		}
	}
	return false;
}