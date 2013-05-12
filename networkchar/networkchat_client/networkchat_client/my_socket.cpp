#include "my_socket.h"


MySocket::MySocket()
	: is_init_lib_(false),
	  communicate_(INVALID_SOCKET),
	  close_tcp_socket_(false),
	  close_udp_socket_(false),
	  tcp_thread_exit_(false),
	  udp_thread_exit_(false)
{
	InitSocketLib();
//	multi_addr_=inet_addr("234.5.6.7");
	BindUDP();
	//thread_handle = ::CreateThread(NULL, 0, _Recvfrom, this, 0, NULL);
}


MySocket::~MySocket()
{
	if (is_init_lib_)
	{
		::WSACleanup();
	}
	::CloseHandle(TCP_thread_);
	::CloseHandle(UDP_thread_);
}

void MySocket::InitSocketLib(BYTE minor_version /* = 2 */, BYTE major_version /* = 2 */)
{
	is_init_lib_ = true;
	WORD socket_version = MAKEWORD(minor_version, major_version);
	WSADATA wsa_data;
	int error_code = ::WSAStartup(socket_version, &wsa_data);
	if (0 != error_code) 
		LTHROW(ERR_INIT_SOCKET)
}

/*
 * @ brief: 连接服务器，此函数中调用CreateSocket函数创建套接字
 *          在退出时需要显示调用CloseSocket函数关闭套接字
 * @ param: server_ip [in] 服务器ip地址
 * @ param: port [in] 服务器端口
 **/
void MySocket::ConnectSever(const char *server_ip, 
	const unsigned short port /* = 4567 */)
{
	server_addr_.sin_addr.S_un.S_addr = ::inet_addr(server_ip);
	server_addr_.sin_family = AF_INET;
	server_addr_.sin_port = ::htons(port);
	CreateSocket();
	if (-1 == ::connect(communicate_, (sockaddr *)&server_addr_, sizeof(server_addr_)))
		LTHROW(ERR_CONNECT)
	CreateTCPReadThread();
}



/*
 * @ brief: 创建套接字，在创建前首先关闭套接字上的连接
 * @ param: is_tcp [in] 套接字类型，若为true则是流式套接字(默认)，
 *                      否则为数据报套接字
 **/
void MySocket::CreateSocket(bool is_tcp /* = true */)
{
	::closesocket(communicate_);
	if (is_tcp)
	{
		communicate_ = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	}
	else
	{
		communicate_ = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	}
}

void MySocket::CloseSocket()
{
//	SetTCPEvent();
	close_tcp_socket_ = true;
}

void MySocket::CloseUdpSocket()
{
	close_udp_socket_ = true;
}

/*
 * @ brief: 是否所有线程已正常退出
 * @ return: 若是返回true
 **/
bool MySocket::IsThreadClosed() {
	return tcp_thread_exit_;
}

bool MySocket::IsUdpThreadClosed()
{
	return udp_thread_exit_;
}

int MySocket::Send(const char *message, const unsigned int len) 
{
	if (NULL == message)
		LTHROW(ERR_MSG_NULL)
	return send(communicate_, message, len, 0);
}

int MySocket::SendTo(const char *message, const unsigned int len,sockaddr_in *remote_addr)
{
	if (NULL == message)
		LTHROW(ERR_MSG_NULL)
	SOCKET s_send = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(s_send==	INVALID_SOCKET)
		LTHROW(ERR_MSG_NULL)
	
	return sendto(s_send, message, len, 0, (sockaddr *)remote_addr, sizeof(sockaddr_in));
}

void MySocket::RequestUserList() 
{
	if (SOCKET_ERROR != Send(user_name_.c_str(), user_name_.length())) {
//		SetTCPEvent();
	}
	LTHROW(ERR_REQUEST_USER_LIST)
}

void MySocket::RequestUserIp(const char *user_name, const int len)
{
	if (user_name == NULL)
		LTHROW(ERR_USER_NAME_NULL)
	
	char *buff = new char[sizeof(MSG_INFO) + len];
	pMsgInfo request_ip = (pMsgInfo)buff;
	request_ip->type = MT_REQUEST_IP;
	strncpy_s(request_ip->user_name, user_name_.c_str(), user_name_.length());
	strncpy(request_ip->data(), user_name, len);
	// TODO: 错误处理
	if (SOCKET_ERROR != Send(buff, sizeof(MSG_INFO) + len)) {
	//	SetTCPEvent();
	}
	delete [] buff;
}

/*
 * @ brief: 触发TCP事件对象
 * @ return: 若成功返回true，否则返回false
 **/
bool MySocket::SetTCPEvent()
{
	return (TRUE == SetEvent(TCP_event_));
}

void MySocket::UserLogin()
{
	char buf[sizeof(MSG_INFO) + sizeof(sockaddr_in)];
	memset(buf, 0, sizeof(buf));
	pMsgInfo msg = (pMsgInfo)buf;
	msg->type = MT_CONNECT_USERINFO;
	msg->addr = local_ip_.S_un.S_addr;
	sockaddr_in *addr = (sockaddr_in *)&buf[sizeof(MSG_INFO)];
	addr->sin_port = udp_addr_.sin_port;   // 当前用户UDP数据接收绑定的端口
	strncpy_s(msg->user_name, user_name_.c_str(), user_name_.length());
	if (SOCKET_ERROR != Send(buf, sizeof(buf))) {
	//	SetTCPEvent();
	}
}

std::string MySocket::UserName()
{
	return user_name_;
}

//////////////////////////////////////////////////////////////////
//////////////////      protected function      //////////////////
//////////////////////////////////////////////////////////////////
void MySocket::BindUDP()
{
	read_udp_=socket(AF_INET, SOCK_DGRAM, 0);
	// 允许其它进程使用绑定的地址
	BOOL bReuse = TRUE;
	::setsockopt(read_udp_, SOL_SOCKET, SO_REUSEADDR, (char*)&bReuse, sizeof(BOOL));
	// 绑定端口
	sockaddr_in si;
	si.sin_family = AF_INET;
	si.sin_port = 0;
	//TODO:端口是否重用
	si.sin_addr.S_un.S_addr = INADDR_ANY;
	::bind(read_udp_, (sockaddr*)&si, sizeof(si));
	int len = sizeof(udp_addr_);
	getsockname(read_udp_, (sockaddr *)&udp_addr_, &len);
#ifdef _DEBUG
	char port[64];
	sprintf_s(port, "bind udp -> addr: %s port: %d", inet_ntoa(udp_addr_.sin_addr), ::ntohs(udp_addr_.sin_port));
	MessageBox(NULL, port, "Debug", MB_ICONINFORMATION);
#endif
	CreateUDPReadThread();
}

void MySocket::GetLocalAddress()
{
	// get local host name
	char host_name[256];
	memset(host_name, 0, sizeof(host_name));
	::gethostname(host_name, sizeof(host_name));
	// get local host ip information
	hostent *host = ::gethostbyname(host_name);
	if (NULL == host) 
		LTHROW(ERR_GET_HOST)
		// get the first ip address
		char *p = host->h_addr_list[0];
	if (NULL == p)
		LTHROW(ERR_IP_NULL)
		memcpy(&local_ip_.S_un.S_addr, p, host->h_length);
}

bool MySocket::JoinGroup()
{
	struct ip_mreq mcast;
	mcast.imr_interface.S_un.S_addr = INADDR_ANY;
	mcast.imr_multiaddr.S_un.S_addr = multi_addr_;
	int ret = ::setsockopt(read_udp_, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&mcast, sizeof(mcast));
	if (SOCKET_ERROR == ret)
		LTHROW(ERR_ADD_CAST)
	return true;
}

/*
 * @ brief: 创建TCP数据接收线程
 **/
void MySocket::CreateTCPReadThread() 
{
	// 创建一个事件对象，并设置为自动重置，且未触发
//	TCP_event_ = CreateEvent(NULL, FALSE, FALSE, NULL);
	TCP_thread_ = ::CreateThread(NULL, 0, _Recv, this, 0, NULL);
}

void MySocket::CreateUDPReadThread()
{
	UDP_thread_ = ::CreateThread(NULL, 0, _Recvfrom, this, 0, NULL);
}

bool MySocket::SetTimeOut(SOCKET sock, int time_out, bool is_receive /* = true */)
{
	int ret = ::setsockopt(sock, SOL_SOCKET, is_receive ? SO_RCVTIMEO : SO_SNDTIMEO,\
		                   (char *)&time_out, sizeof(int));
	if (ret == SOCKET_ERROR)
		LTHROW(ERR_SET_TIME_OUT)
	return true;
}
DWORD __stdcall _Recvfrom(LPVOID lpParam)
{
	MySocket *my_socket = (MySocket *)lpParam;
	if (my_socket == NULL)
	{
		return -1;
	}
	my_socket->SetTimeOut(my_socket->read_udp_, 2 * 1000);
	char buff[4096];
	my_socket->udp_thread_exit_ = false;
	sockaddr_in remote_addr;
	int remote_length = sizeof(remote_addr);
	while (!my_socket->close_udp_socket_)
	{
		//	WaitForSingleObject(my_socket->TCP_event_, INFINITE);
		memset(buff, 0, sizeof(buff));
	//	MessageBeep(0);
		int ret_len = ::recvfrom(my_socket->read_udp_, buff, sizeof(buff), 0, (sockaddr *)&remote_addr, &remote_length );
		if (ret_len != SOCKET_ERROR && ret_len != 0)
		{
			MessageBox(NULL, TEXT("收到udp"),TEXT("DEBUG"),0);
			SendMessage(my_socket->main_hwnd, WM_CHATMSG, 0, (LPARAM)buff);
		} 
		// 		else
		// 		{
		// 			my_socket->SetTCPEvent();
		// 		}
	}
	::closesocket(my_socket->read_udp_);
	my_socket->udp_thread_exit_ = true;
	return 0;
}

/*
 * @ brief: 接收TCP数据
 * @ param: lpParam [in] MySocket类
 * @ return: 成功退出返回0，否则返回-1
 **/
DWORD __stdcall _Recv(LPVOID lpParam)
{
	MySocket *my_socket = (MySocket *)lpParam;
	if (my_socket == NULL)
	{
		return -1;
	}
	BOOL bReuse = TRUE;
	::setsockopt(my_socket->communicate_, SOL_SOCKET, SO_REUSEADDR, (char*)&bReuse, sizeof(BOOL));
    my_socket->SetTimeOut(my_socket->communicate_, 2 * 1000);
	char buff[4096];
	my_socket->tcp_thread_exit_ = false;
	while (!my_socket->close_tcp_socket_)
	{
	//	WaitForSingleObject(my_socket->TCP_event_, INFINITE);
		memset(buff, 0, sizeof(buff));
		int ret_len = ::recv(my_socket->communicate_, buff, sizeof(buff), 0);
		if (ret_len > 0)
		{
			SendMessage(my_socket->main_hwnd, WM_CHATMSG, 0, (LPARAM)buff);
		} 
	}
	::closesocket(my_socket->communicate_);
	my_socket->tcp_thread_exit_ = true;
	return 0;
}

//暂时未使用
bool MySocket::DispatchMsg(char* recv_buffer, SOCKET current_socket)
{
	MSG_INFO *recv_message=(MSG_INFO *)recv_buffer;
	int recv_msg_mark = recv_message->type;
	switch(recv_msg_mark)
	{
	case MT_MULTICASTING_USERINFO: //多播的在线用户信息
		{
			int data_length = recv_message->data_length;
			char *user_name = new char[data_length];
			MessageBox(NULL, recv_message->data(), TEXT("收到信息"),0);
			SendMessage(main_hwnd, WM_CHATMSG, 0, (LPARAM)recv_buffer);
		    delete [] user_name;
			break;
		}
	case MT_MULTICASTING_TEXT: //多播的聊天信息
		{
			break; 
		}
	}
	return false;
}

bool ShowChatWindow()
{
	return false;
}