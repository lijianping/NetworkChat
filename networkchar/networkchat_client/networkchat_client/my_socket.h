#ifndef NETWORKCHAT_CLIENT_MY_SOCKET_H_
#define NETWORKCHAT_CLIENT_MY_SOCKET_H_

#include "err.h"
#include <WinSock2.h>
#include <ws2ipdef.h>
#include <string>
#pragma comment(lib, "WS2_32")

#define WM_CHATMSG (WM_USER+100)
//��Ϣ����
const enum MSG_TYPE {
	MT_MULTICASTING_USERINFO = 1,         //�ಥ�����û���Ϣ

	MT_MULTICASTING_TEXT,                 //�ಥ������Ϣ


	MT_REQUEST_CONNECT,                   //��������
	
	MT_REQUEST_IP,                   //ip��ѯ����
	
	MT_REQUEST_ALLUSERINFO,               //�����ȡ�����û���Ϣ
	
	MT_CONNECT_USERINFO,                   //�����û���Ϣ

	MT_RESPOND_IP                        //��ӦIp����
};
// 64 bytes
typedef struct MSG_INFO {
	unsigned char type;            // ��Ϣ���� MSG_TYPE
	unsigned long addr;            // ���ʹ���Ϣ���û���ip��ַ
	char user_name[49];            // ���ʹ���Ϣ���û����û���
	int data_length;               // ���ݳ���
	char *data() { return (char *)(this + 1); } // ������
}*pMsgInfo;

//�û���Ϣ�ṹ�壨�û�����map��keyֵ���û���Ϣ��value��
typedef struct USER_INFO
{
	char user_name[48];
	sockaddr_in addr;
}*pUserInfo;

class MySocket
{
public:
	MySocket();
	~MySocket();
	void InitSocketLib(BYTE minor_version = 2, BYTE major_version = 2);
	void ConnectSever(const char *server_ip, const unsigned short port = 4567);
	int Send(const char *message, const unsigned int len);
	void CreateSocket(bool is_tcp = true);
	
	void CloseSocket();
	void RequestUserList();
	void RequestUserIp(const char *user_name,  const int len);
	bool SetTCPEvent();
	void UserLogin();
	inline SOCKET communicate() const;
	inline void set_user_name(const std::string &user_name);
	inline void set_main_hwnd(HWND hwnd);

protected:
	void BindUDP();
	void GetLocalAddress();
	bool JoinGroup();
	void CreateTCPReadThread();

private:
	in_addr local_ip_;
	sockaddr_in server_addr_;
	SOCKET communicate_;          // ͨ���׽���
	SOCKET read_udp_;             // ���������׽���
	sockaddr_in udp_addr_;        // ����UDP���ݵĵ�ַ
	bool is_init_lib_;
	unsigned long multi_addr_;    // �ಥ��ַ
	unsigned short multi_port;    //�ಥ�˿�
	std::string user_name_;
	HWND main_hwnd;               //
	HANDLE thread_handle;            //�߳̾��
	HANDLE TCP_thread_;
	HANDLE TCP_event_;
	bool is_exit_;         
	friend DWORD __stdcall _Recvfrom(LPVOID lpParam);
	friend DWORD __stdcall _Recv(LPVOID lpParam);
	bool DispatchMsg(char* recv_buffer, SOCKET current_socket);
};

SOCKET MySocket::communicate() const
{
	return communicate_;
}

void MySocket::set_user_name(const std::string &user_name)
{
	user_name_ = user_name;
}

void  MySocket:: set_main_hwnd(HWND hwnd)
{
	main_hwnd = hwnd;
}
#endif