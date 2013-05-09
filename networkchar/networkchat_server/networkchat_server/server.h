#ifndef NETWORKCHAT_SERVER_H_
#define NETWORKCHAT_SERVER_H_

#include <stdio.h>
#include <iostream>
#include <winsock2.h>
#include <map>
#pragma comment(lib, "WS2_32")
using namespace std;

//�û���Ϣ�ṹ�壨�û�����map��keyֵ���û���Ϣ��value��
struct USER_INFO
{
	SOCKET user_socket;
	in_addr user_ip;
}*pUSER_INFO;

//��Ϣ�ṹ��
struct USER_MESSAGE
{
	int message_mark;
	std::string user_name;
	std::string message_info;
}*pUSER_MESSAGE;

#define CLIENT_MAX 64
#define WM_SOCKET WM_USER+1 //�Զ�����Ϣ
//��Ϣ����
#define MT_MULTICASTING_USERINFO 0    //�ಥ�����û���Ϣ
#define MT_MULTICASTING_TEXT 1       //�ಥ������Ϣ
#define MT_REQUEST_CONNECT 2       //��������
#define MT_REQUEST_IP 3            //ip��ѯ����
#define MT_REQUEST_ALLUSERINFO  4      //�����ȡ�����û���Ϣ
#define MT_CONNECT_USERINFO  5      //�����û���Ϣ

bool HandleMessage(char* recv_buffer, SOCKET current_socket);

#endif