#ifndef NETWORKCHAT_SERVER_H_
#define NETWORKCHAT_SERVER_H_

#include <stdio.h>
#include <iostream>
#include <winsock2.h>
#include <string>
#pragma comment(lib, "WS2_32")
using namespace std;

//用户信息结构体（用户名做map的key值，用户信息做value）
struct USER_INFO
{
	std::string user_name;
	sockaddr_in addr;
}*pUSER_INFO;

//消息类型
const enum MSG_TYPE {
	MT_MULTICASTING_USERINFO = 1,         //多播在线用户信息

	MT_MULTICASTING_TEXT,                 //多播聊天信息

	MT_REQUEST_CONNECT,                   //连接请求

	MT_REQUEST_IP,                   //ip查询请求

	MT_REQUEST_ALLUSERINFO,               //请求获取在线用户信息

	MT_CONNECT_USERINFO                   //连接用户信息
};

// 64 bytes
typedef struct MSG_INFO {
	unsigned char type;            // 消息类型 MSG_TYPE
	unsigned long addr;            // 发送此消息的用户的ip地址
	char user_name[49];            // 发送此消息的用户的用户名
	int data_length;               // 数据长度
	char *data() { return (char *)(this + 1); } // 数据域
}*pMsgInfo;

#define CLIENT_MAX 64
#define WM_SOCKET WM_USER+1 //自定义消息


bool HandleMessage(char* recv_buffer, SOCKET current_socket);
bool ShowOnlineUser(USER_INFO *pOnlineUser);

#endif