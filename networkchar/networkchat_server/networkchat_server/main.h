#include <stdio.h>
#include <iostream>
#include <winsock2.h>
#pragma comment(lib, "WS2_32")
using namespace std;

//�û���Ϣ�ṹ�壨�û�����map��keyֵ���û���Ϣ��value��
struct USER_INFO
{
	SOCKET user_socket;
	char user_ip[16];
}*pUSER_INFO;

//��Ϣ�ṹ��
struct USER_MESSAGE
{
	int message_mark;
	std::string user_name;
	std::string message_info;
}*pUSER_MESSAGE;


bool BroadcastAllUser();

