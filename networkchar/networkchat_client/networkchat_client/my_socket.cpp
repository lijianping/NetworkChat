#include "my_socket.h"


MySocket::MySocket(BYTE minor_version /* = 2 */, BYTE major_version /* = 2 */)
{
	WORD socket_version = MAKEWORD(minor_version, major_version);
	WSADATA wsa_data;
	int error_code = ::WSAStartup(socket_version, &wsa_data);
	if (0 != error_code) 
		LTHROW(ERR_INIT_SOCKET)
}


MySocket::~MySocket()
{
	::WSACleanup();
}
