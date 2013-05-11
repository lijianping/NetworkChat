#include "my_socket.h"


MySocket::MySocket()
	: is_init_lib_(false),
	  communicate_(INVALID_SOCKET),
	  close_tcp_socket_(false),
	  tcp_thread_exit_(false)
{
	InitSocketLib();
	multi_addr_=inet_addr("234.5.6.7");
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
 * @ brief: ���ӷ��������˺����е���CreateSocket���������׽���
 *          ���˳�ʱ��Ҫ��ʾ����CloseSocket�����ر��׽���
 * @ param: server_ip [in] ������ip��ַ
 * @ param: port [in] �������˿�
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
 * @ brief: �����׽��֣��ڴ���ǰ���ȹر��׽����ϵ�����
 * @ param: is_tcp [in] �׽������ͣ���Ϊtrue������ʽ�׽���(Ĭ��)��
 *                      ����Ϊ���ݱ��׽���
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

/*
 * @ brief: �Ƿ������߳��������˳�
 * @ return: ���Ƿ���true
 **/
bool MySocket::IsThreadClosed() {
	return tcp_thread_exit_;
}

int MySocket::Send(const char *message, const unsigned int len) 
{
	if (NULL == message)
		LTHROW(ERR_MSG_NULL)
	return send(communicate_, message, len, 0);
}

void MySocket::RequestUserList() 
{
	if (SOCKET_ERROR != Send(user_name_.c_str(), user_name_.length())) {
		SetTCPEvent();
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
	// TODO: ������
	if (SOCKET_ERROR != Send(buff, sizeof(MSG_INFO) + len)) {
		SetTCPEvent();
	}
	delete [] buff;
}

/*
 * @ brief: ����TCP�¼�����
 * @ return: ���ɹ�����true�����򷵻�false
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
	addr->sin_port = udp_addr_.sin_port;   // ��ǰ�û�UDP���ݽ��հ󶨵Ķ˿�
	strncpy_s(msg->user_name, user_name_.c_str(), user_name_.length());
	if (SOCKET_ERROR != Send(buf, sizeof(buf))) {
		SetTCPEvent();
	}
}

//////////////////////////////////////////////////////////////////
//////////////////      protected function      //////////////////
//////////////////////////////////////////////////////////////////
void MySocket::BindUDP()
{
	read_udp_=socket(AF_INET, SOCK_DGRAM, 0);
	// ������������ʹ�ð󶨵ĵ�ַ
	BOOL bReuse = TRUE;
	::setsockopt(read_udp_, SOL_SOCKET, SO_REUSEADDR, (char*)&bReuse, sizeof(BOOL));
	// �󶨶˿�
	sockaddr_in si;
	si.sin_family = AF_INET;
	si.sin_port = 0;
	//TODO:�˿��Ƿ�����
	si.sin_addr.S_un.S_addr = INADDR_ANY;
	::bind(read_udp_, (sockaddr*)&si, sizeof(si));
	int len = sizeof(udp_addr_);
	getsockname(read_udp_, (sockaddr *)&udp_addr_, &len);
#ifdef _DEBUG
	char port[64];
	sprintf_s(port, "bind udp -> addr: %s port: %d", inet_ntoa(udp_addr_.sin_addr), ::ntohs(udp_addr_.sin_port));
	MessageBox(NULL, port, "Debug", MB_ICONINFORMATION);
#endif
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
 * @ brief: ����TCP���ݽ����߳�
 **/
void MySocket::CreateTCPReadThread() 
{
	// ����һ���¼����󣬲�����Ϊ�Զ����ã���δ����
	TCP_event_ = CreateEvent(NULL, FALSE, FALSE, NULL);
	TCP_thread_ = ::CreateThread(NULL, 0, _Recv, this, 0, NULL);
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
	MySocket *pMySocket=(MySocket*)lpParam;
	pMySocket->JoinGroup();
	char buf[4096]={0};
	//TODO:�û�����
	sockaddr_in si;
	int nAddrLen = sizeof(si);
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
			MessageBox(NULL, TEXT("������Ϣ����"), TEXT("����"), MB_ICONERROR);
			//int n = ::WSAGetLastError();
			break;
		}
	}
	return 0;
}

/*
 * @ brief: ����TCP����
 * @ param: lpParam [in] MySocket��
 * @ return: �ɹ��˳�����0�����򷵻�-1
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
// 		else
// 		{
// 			my_socket->SetTCPEvent();
// 		}
	}
	::closesocket(my_socket->communicate_);
	my_socket->tcp_thread_exit_ = true;
	return 0;
}

//��ʱδʹ��
bool MySocket::DispatchMsg(char* recv_buffer, SOCKET current_socket)
{
	MSG_INFO *recv_message=(MSG_INFO *)recv_buffer;
	int recv_msg_mark = recv_message->type;
	switch(recv_msg_mark)
	{
	case MT_MULTICASTING_USERINFO: //�ಥ�������û���Ϣ
		{
			int data_length = recv_message->data_length;
			char *user_name = new char[data_length];
			MessageBox(NULL, recv_message->data(), TEXT("�յ���Ϣ"),0);
			SendMessage(main_hwnd, WM_CHATMSG, 0, (LPARAM)recv_buffer);
		    delete [] user_name;
			break;
		}
	case MT_MULTICASTING_TEXT: //�ಥ��������Ϣ
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