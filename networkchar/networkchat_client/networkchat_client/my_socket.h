#ifndef NETWORKCHAT_CLIENT_MY_SOCKET_H_
#define NETWORKCHAT_CLIENT_MY_SOCKET_H_

#include "err.h"
#include <WinSock2.h>
#pragma comment(lib, "WS2_32")

class MySocket
{
public:
	MySocket();
	~MySocket();
	void InitSocketLib(BYTE minor_version = 2, BYTE major_version = 2);
	void ConnectSever(const char *server_ip, const unsigned short port = 4567);
	inline SOCKET communicate() const;
protected:
	void GetLocalAddress();
	void CreateSocket(bool is_tcp = true);

private:
	in_addr local_ip_;
	sockaddr_in server_addr_;
	SOCKET communicate_;
	bool is_init_lib;
};

SOCKET MySocket::communicate() const
{
	return communicate_;
}
#endif