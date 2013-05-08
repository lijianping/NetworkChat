#include <stdio.h>
#include <iostream>
#include <winsock2.h>
#pragma comment(lib, "WS2_32")
using namespace std;

//用户信息结构体（用户名做map的key值，用户信息做value）
struct USER_INFO
{
	SOCKET user_socket;
	char user_ip[16];
}*pUSER_INFO;

//消息结构体
struct USER_MESSAGE
{
	int message_mark;
	std::string user_name;
	std::string message_info;
}*pUSER_MESSAGE;


bool BroadcastAllUser();

