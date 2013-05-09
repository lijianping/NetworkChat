#ifndef NETWORKCHAT_CLIENT_MY_SOCKET_H_
#define NETWORKCHAT_CLIENT_MY_SOCKET_H_

#include "err.h"
#include <WinSock2.h>
#include <ws2ipdef.h>
#include <string>
#pragma comment(lib, "WS2_32")

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
}pMsgInfo;

class MySocket
{
public:
	MySocket();
	~MySocket();
	void InitSocketLib(BYTE minor_version = 2, BYTE major_version = 2);
	void ConnectSever(const char *server_ip, const unsigned short port = 4567);
	int Send(const char *user, const unsigned int len);
	void CreateSocket(bool is_tcp = true);
	void CloseSocket();
	void RequestUserList();
	void UserLogin();
	inline SOCKET communicate() const;
	inline void set_user_name(const std::string &user_name);

protected:
	void GetLocalAddress();
	bool JoinGroup();

private:
	in_addr local_ip_;
	sockaddr_in server_addr_;
	SOCKET communicate_;          // 通信套接字
	SOCKET read_socket_;           // 接收数据套接字
	bool is_init_lib_;
	unsigned long multi_addr_;    // 多播地址
	std::string user_name_;
};

SOCKET MySocket::communicate() const
{
	return communicate_;
}

void MySocket::set_user_name(const std::string &user_name)
{
	user_name_ = user_name;
}
#endif