#include "server.h"
#include <stdio.h>
#include <iostream>
#include <map>
#include <string>
using namespace std;

map<SOCKET, USER_INFO> users;


/*
 * @ brief: 获取系统时间
 * @ return: 字符格式时间 格式为 YYYY-MM-DD hh:mm:ss
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
		MessageBox ( NULL, TEXT ("This program requires Windows NT!"),
			TEXT("error"), MB_ICONERROR);
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

LRESULT CALLBACK NetworkProc(HWND hwnd,	UINT uMsg, WPARAM wParam, LPARAM lParam)
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
					// 增加登录用户的地址信息
					USER_INFO user;
					user.addr = remote_addr;  // 获取用户的ip地址  
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
							//处理收到的消息
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
					users.erase(s);   // 移出用户
					map<SOCKET, USER_INFO>::const_iterator it;
					std::string user_name;
					int data_length = 0;
					// 格式在线好友名称
					for (it = users.begin(); it != users.end(); ++it) {
						user_name += it->second.user_name;
						data_length += strlen(it->second.user_name);
						user_name += '/';
						data_length++;
					}
					// 构造消息结构
					char *buff = new char[sizeof(MSG_INFO) + data_length];
					memset(buff, 0, sizeof(buff));
					pMsgInfo msg_info = (pMsgInfo)buff;
					msg_info->type = MT_MULTICASTING_USERINFO;        // 初始化消息类型
					char host_name[128];
					memset(host_name, 0, sizeof(host_name));
					::gethostname(host_name, sizeof(host_name));      // 获取主机名
					hostent *host = ::gethostbyname(host_name);
					msg_info->addr = inet_addr(host->h_addr_list[0]); // 初始化发送者ip地址
					// 初始化发送者名称
					strncpy_s(msg_info->user_name,"server", sizeof(msg_info->user_name));
					msg_info->data_length = data_length;     // 初始化数据字段长度
					strncpy(msg_info->data(), user_name.c_str(), (size_t)user_name.length());
					// 向在线用户主动更新好友列表
					for (it = users.begin(); it != users.end(); ++it)
					{
						::send(it->first, buff, sizeof(MSG_INFO) + data_length, 0);
					}
					delete [] buff;
					buff = NULL;
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
	case MT_CONNECT_USERINFO://连接用户信息
		{
			// 增加用户名
			strncpy_s(users[current_socket].user_name, \
				      recv_message->user_name, \
					  sizeof(recv_message->user_name));
			sockaddr_in *remote = (sockaddr_in *)recv_message->data();
			users[current_socket].addr.sin_port = remote->sin_port;       // 获取用户UDP绑定的端口
			
#ifdef _DEBUG
			cout <<"Current Connect User: " <<recv_message->user_name \
				 <<" length: " << strlen(recv_message->user_name)<<endl;
			cout <<"UDP addr: " <<inet_ntoa(users[current_socket].addr.sin_addr) <<" UDP port: " <<::ntohs(remote->sin_port) <<endl;
#endif
			cout <<endl;
			cout <<users[current_socket].user_name <<" sign in at " <<GetTime() <<endl;
			map<SOCKET, USER_INFO>::const_iterator it;
			std::string user_name;
			int data_length = 0;
			// 格式在线好友名称
			for (it = users.begin(); it != users.end(); ++it) {
				user_name += it->second.user_name;
			    data_length += strlen(it->second.user_name);
				user_name += '/';
			    data_length++;
			}
			// 构造消息结构
		    char *buff = new char[sizeof(MSG_INFO) + data_length];
		    memset(buff, 0, sizeof(buff));
			pMsgInfo msg_info = (pMsgInfo)buff;
			msg_info->type = MT_MULTICASTING_USERINFO;        // 初始化消息类型
			char host_name[128];
			memset(host_name, 0, sizeof(host_name));
			::gethostname(host_name, sizeof(host_name));      // 获取主机名
			hostent *host = ::gethostbyname(host_name);
			msg_info->addr = inet_addr(host->h_addr_list[0]); // 初始化发送者ip地址
			// 初始化发送者名称
			strncpy_s(msg_info->user_name,"server", sizeof("server"));
			msg_info->data_length = data_length;     // 初始化数据字段长度
			strncpy(msg_info->data(), user_name.c_str(), (size_t)user_name.length());
			// 向在线用户主动更新好友列表
			for (it = users.begin(); it != users.end(); ++it)
			{
				::send(it->first, buff, sizeof(MSG_INFO) + data_length, 0);
			}
			delete [] buff;
			buff = NULL;
			/*
			// HIT: 多播在局域网中不能接收数据，将其替换为TCP
			// 多播用户信息
			SOCKET s = ::socket(AF_INET, SOCK_DGRAM, 0);
			sockaddr_in si;
			si.sin_family = AF_INET;
			si.sin_port = ::htons(5000);
			si.sin_addr.S_un.S_addr =::inet_addr("234.5.6.7");  
			sendto(s, buff, sizeof(MSG_INFO) + data_length, 0, (sockaddr *)&si, sizeof(si));
			*/
			
			break;
		}
	case MT_REQUEST_IP: //ip查询请求
		{
			cout<<"接收到IP查询请求:"<<endl;
			map<SOCKET, USER_INFO>::iterator it_user;
			char temp_user_name[64]={0};
			//获取查询ip用户名
			strncpy_s(temp_user_name, recv_message->data(), recv_message->data_length);
			USER_INFO respond_user;
			strncpy_s(respond_user.user_name, temp_user_name, sizeof(respond_user.user_name));
			cout<<"请求查询"<<temp_user_name<<"的ip信息"<<endl;
			bool is_find = false;
			//根据用户名在map中找到该用户
			for(it_user = users.begin(); it_user != users.end(); ++it_user)
			{
				if (string(respond_user.user_name) == it_user->second.user_name)
				{
					is_find = true;
					break;
				}
			}
			if (is_find == false)
			{
				cout<<"没有查找到该用户"<<endl;
				break;
			}
		    respond_user.addr = it_user->second.addr;//获取查询ip用户地址结构
			//构造发送数据，响应客户端的ip查询请求
			char buff[sizeof(MSG_INFO) + 128]={0};
			pMsgInfo respond_msg = (pMsgInfo)buff;
			respond_msg->type = MT_RESPOND_IP;
			strncpy_s(respond_msg->user_name, "server", sizeof("server"));
			respond_msg->data_length = sizeof(USER_INFO);
			memcpy(respond_msg->data(), &respond_user, sizeof(USER_INFO));
#ifdef _DEBUG
			cout <<"USER INFO: " <<respond_msg->data() <<endl;
#endif
			int send_length = ::send(current_socket, buff, sizeof(MSG_INFO) + sizeof(USER_INFO), 0);
			cout<<"给客户端发送ip查询结果的大小："<<send_length<<endl;
			break;
		}
	case MT_REQUEST_ALLUSERINFO: //请求获取在线用户信息
		{
			//TODO:基本与MT_CONNECT_USERINFO同
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