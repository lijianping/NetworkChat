#ifndef NETWORKCHAT_CLIENT_MY_SOCKET_H_
#define NETWORKCHAT_CLIENT_MY_SOCKET_H_

#include "err.h"
#include <WinSock2.h>
#pragma comment(lib, "WS2_32")

class MySocket
{
public:
	MySocket(BYTE minor_version = 2, BYTE major_version = 2);
	~MySocket();
};

#endif