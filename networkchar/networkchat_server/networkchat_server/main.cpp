#include "main.h"

int main()
{
	WSADATA wsaData;
	WORD sockVersion = MAKEWORD(2, 2);
	if(::WSAStartup(sockVersion, &wsaData)!=0)
	{
		printf("加载WINSOCK库失败！\n");
		exit(0);
	}
	USHORT nPort = 4567;
	//创建监听套接字
	SOCKET sListen = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
//	ioctlsocket(sListen, FIONBIO, (u_long*)1);
	sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(nPort);
	server_addr.sin_addr.S_un.S_addr = INADDR_ANY;
	//绑定套接字到本地机器
	if(::bind(sListen, (sockaddr*)&server_addr, sizeof(server_addr))==SOCKET_ERROR)
	{
		printf("绑定监听套接字失败\n");
		return -1;
	}
	//进入监听模式
	::listen(sListen, 5);
	//select模型处理
	//1)初始化一个套接字集合fdSocket,添加监听套接字句柄到这个集合
	fd_set fdSocket; //所有可用套接字集合
	FD_ZERO (&fdSocket);
	FD_SET (sListen, &fdSocket);
	while(TRUE)
	{
		//2)将fdsocket集合的一个拷贝fdSocket传递给select函数
		fd_set fdRead = fdSocket;
    //	printf("select前\n");
		int nRet = ::select(0, &fdRead, NULL, NULL, NULL);
   //		printf("select后\n");
		if(nRet>0)
		{
			//3)通过将原来fdSocket集合与select处理过的fdRead集合比较
			for(int i = 0; i < (int)fdSocket.fd_count; i++)
			{
				if(FD_ISSET(fdSocket.fd_array[i], &fdRead))
				{
					if(fdSocket.fd_array[i] == sListen)//监听套接字接收到新的连接
					{
						if(fdSocket.fd_count < FD_SETSIZE)//64
						{
							sockaddr_in client_addr;
							int nClientLen = sizeof(client_addr);
							SOCKET sNew = ::accept( sListen, (SOCKADDR*)&client_addr, &nClientLen);
							FD_SET(sNew, &fdSocket);
							printf("接收到新的连接(%s)\n", ::inet_ntoa(client_addr.sin_addr)); 
							//发送在线用户信息
							//char szHello[10]={"hello"};
							//send(sNew, szHello, strlen(szHello), 0);
						}
						else
						{
							printf("连接数量过多，连接失败！\n");
							continue;
						}
					}
					else
					{
						char rec_buffer[256] = {0};
						int rec_len = ::recv(fdSocket.fd_array[i], rec_buffer, sizeof(rec_buffer), 0);//接收数据
						if(rec_len > 0)
						{
							printf("接收到的数据：%s\n", rec_buffer);
							char szHello[10]={"hello"};
							send(fdSocket.fd_array[i], szHello, strlen(szHello), 0);
						}
						else//连接关闭、重启或中断
						{
							::closesocket(fdSocket.fd_array[i]);
							FD_CLR(fdSocket.fd_array[i], &fdSocket);
						}
					}
				}
			}//end for
		}
		else
		{
			printf("选择套接字失败！\n");
			break;
		}
	}
	closesocket(sListen);
	WSACleanup();
	return 0;
}



