#ifndef NETWORKCHAT_SERVER_H_
#define NETWORKCHAT_SERVER_H_

#include <stdio.h>
#include <iostream>
#include <winsock2.h>
#include <map>
#pragma comment(lib, "WS2_32")
using namespace std;

//用户信息结构体（用户名做map的key值，用户信息做value）
struct USER_INFO
{
	SOCKET user_socket;
	in_addr user_ip;
}*pUSER_INFO;

//消息结构体
struct USER_MESSAGE
{
	int message_mark;
	std::string user_name;
	std::string message_info;
}*pUSER_MESSAGE;

#define CLIENT_MAX 64
#define WM_SOCKET WM_USER+1 //自定义消息
//消息类型
#define MT_MULTICASTING_USERINFO 0    //多播在线用户信息
#define MT_MULTICASTING_TEXT 1       //多播聊天信息
#define MT_REQUEST_CONNECT 2       //连接请求
#define MT_REQUEST_IP 3            //ip查询请求
#define MT_REQUEST_ALLUSERINFO  4      //请求获取在线用户信息
#define MT_CONNECT_USERINFO  5      //连接用户信息

bool HandleMessage(char* recv_buffer, SOCKET current_socket);

#endif