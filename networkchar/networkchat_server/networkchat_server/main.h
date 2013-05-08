#include <stdio.h>
#include <iostream>
#include <winsock2.h>
#pragma comment(lib, "WS2_32")

typedef struct USER_INFO
{
	SOCKET user_socket;
	sockaddr_in user_addr;
}*pUSER_INFO;

// the message information
typedef struct USER_MESSAGE
{
	int message_mark;            // the message indentify
	std::string user_name;       // who send the message
	std::string message_data;
}*pUSER_MESSAGE;


bool BroadcastAllUser();

