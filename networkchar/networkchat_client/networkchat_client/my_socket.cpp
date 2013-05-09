#include "my_socket.h"


MySocket::MySocket()
	: is_init_lib_(false),
	  communicate_(INVALID_SOCKET)
{

}


MySocket::~MySocket()
{
	if (is_init_lib_)
	{
		::WSACleanup();
	}
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
	::closesocket(communicate_);
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

void MySocket::UserLogin()
{
	char buf[sizeof(MSG_INFO)];
	memset(buf, 0, sizeof(buf));
	pMsgInfo *msg = (pMsgInfo *)buf;
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
	int ret = ::setsockopt(read_socket_, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&mcast, sizeof(mcast));
	if (SOCKET_ERROR == ret)
		LTHROW(ERR_ADD_CAST)
	return true;
}