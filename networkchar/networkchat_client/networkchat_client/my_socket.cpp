#include "my_socket.h"


MySocket::MySocket()
	: is_init_lib_(false),
	  communicate_(INVALID_SOCKET),
	  is_exit_(false)
{
	InitSocketLib();
	multi_addr_=inet_addr("234.5.6.7");
//	thread_handle = ::CreateThread(NULL, 0, _Recvfrom, this, 0, NULL);
}


MySocket::~MySocket()
{
	if (is_init_lib_)
	{
		::WSACleanup();
	}
	::CloseHandle(TCP_thread_);
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
	TCP_thread_ = ::CreateThread(NULL, 0, _Recv, this, 0, NULL);
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
	//::WaitForSingleObject(TCP_thread_, 0);
	is_exit_ = true;

}

int MySocket::Send(const char *message, const unsigned int len) 
{
	if (NULL == message)
		LTHROW(ERR_MSG_NULL)
	return send(communicate_, message, len, 0);
}

void MySocket::RequestUserList() 
{
	if (0 == Send(user_name_.c_str(), user_name_.length()))
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
	Send(buff, sizeof(MSG_INFO) + len);
	delete [] buff;
}

void MySocket::UserLogin()
{
	char buf[sizeof(MSG_INFO)];
	memset(buf, 0, sizeof(buf));
	pMsgInfo msg = (pMsgInfo)buf;
	msg->type = MT_CONNECT_USERINFO;
	msg->addr = local_ip_.S_un.S_addr;
	strncpy_s(msg->user_name, user_name_.c_str(), user_name_.length());
	Send(buf, sizeof(buf));
}

//////////////////////////////////////////////////////////////////
//////////////////      protected function      //////////////////
//////////////////////////////////////////////////////////////////

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

DWORD __stdcall _Recvfrom(LPVOID lpParam)
{
	MySocket *pMySocket=(MySocket*)lpParam;
	pMySocket->read_udp_=socket(AF_INET, SOCK_DGRAM, 0);
	// 允许其它进程使用绑定的地址
	BOOL bReuse = TRUE;
	::setsockopt(pMySocket->read_udp_, SOL_SOCKET, SO_REUSEADDR, (char*)&bReuse, sizeof(BOOL));
	// 绑定端口
	sockaddr_in si;
	si.sin_family = AF_INET;
	si.sin_port = ::ntohs(5000);
	//TODO:端口是否重用
	si.sin_addr.S_un.S_addr = INADDR_ANY;
	int nAddrLen = sizeof(si);
	::bind(pMySocket->read_udp_, (sockaddr*)&si, sizeof(si));
	pMySocket->JoinGroup();
	char buf[4096]={0};
	//TODO:用户缓冲
	while (true)
	{
		int nRet = ::recvfrom(pMySocket->read_udp_, buf, sizeof(buf), 0, (sockaddr*)&si, &nAddrLen);
		if(nRet != SOCKET_ERROR)
		{
			SendMessage(pMySocket->main_hwnd, WM_CHATMSG, 0, (LPARAM)buf);
		//	pMySocket->DispatchMsg(buf, pMySocket->read_udp_);
		}
		else
		{
			MessageBox(NULL, TEXT("接收消息出错"), TEXT("错误"), MB_ICONERROR);
			//int n = ::WSAGetLastError();
			break;
		}

	}
	return 0;
}

/*
 * 说明：
 *     TCP接收数据线程
 * 参数：
 *      lpParam [in] 此处为MySocket对象
 */
DWORD __stdcall _Recv(LPVOID lpParam)
{
	MySocket *my_socket = (MySocket *)lpParam;
	if (my_socket == NULL)
	{
		return -1;
	}
	BOOL bReuse = TRUE;
	::setsockopt(my_socket->communicate_, SOL_SOCKET, SO_REUSEADDR, (char*)&bReuse, sizeof(BOOL));
	unsigned long ul=1;
	::ioctlsocket(my_socket->communicate_, FIONBIO, (unsigned long* )&ul);
	char buff[4096];
	memset(buff, 0, sizeof(buff));
	while (true)
	{
		int ret_len = ::recv(my_socket->communicate_, buff, sizeof(buff), 0);
		if (ret_len > 0)
		{
			SendMessage(my_socket->main_hwnd, WM_CHATMSG, 0, (LPARAM)buff);
		}
		if (my_socket->is_exit_)
		{
			break;
		}
	}
	::closesocket(my_socket->communicate_);
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