#ifndef NETWORKCHAT_SERVER_H_
#define NETWORKCHAT_SERVER_H_

#include <stdio.h>
#include <iostream>
#include <winsock2.h>
#include <string>
#pragma comment(lib, "WS2_32")
using namespace std;

//�û���Ϣ�ṹ�壨�û�����map��keyֵ���û���Ϣ��value��
struct USER_INFO
{
	char user_name[48];
	sockaddr_in addr;
}*pUSER_INFO;

//��Ϣ����
const enum MSG_TYPE {
	MT_MULTICASTING_USERINFO = 1,         //�ಥ�����û���Ϣ

	MT_MULTICASTING_TEXT,                 //�ಥ������Ϣ

	MT_REQUEST_CONNECT,                   //��������

	MT_REQUEST_IP,                   //ip��ѯ����

	MT_REQUEST_ALLUSERINFO,               //�����ȡ�����û���Ϣ

	MT_CONNECT_USERINFO,                   //�����û���Ϣ

	MT_RESPOND_IP                     //����ip��ѯ�����ip��Ϣ
};

// 64 bytes
typedef struct MSG_INFO {
	unsigned char type;            // ��Ϣ���� MSG_TYPE
	unsigned long addr;            // ���ʹ���Ϣ���û���ip��ַ
	char user_name[49];            // ���ʹ���Ϣ���û����û���
	int data_length;               // ���ݳ���
	char *data() { return (char *)(this + 1); } // ������
}*pMsgInfo;

#define CLIENT_MAX 64
#define WM_SOCKET WM_USER+1 //�Զ�����Ϣ


bool HandleMessage(char* recv_buffer, SOCKET current_socket);
bool ShowOnlineUser(USER_INFO *pOnlineUser);

#endif