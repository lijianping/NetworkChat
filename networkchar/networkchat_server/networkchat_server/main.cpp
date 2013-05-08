#include "main.h"

int main()
{
	WSADATA wsaData;
	WORD sockVersion = MAKEWORD(2, 2);
	if(::WSAStartup(sockVersion, &wsaData)!=0)
	{
		printf("����WINSOCK��ʧ�ܣ�\n");
		exit(0);
	}
	USHORT nPort = 4567;
	//���������׽���
	SOCKET sListen = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
//	ioctlsocket(sListen, FIONBIO, (u_long*)1);

	char host_name[256];
	memset(host_name, 0, sizeof(host_name));
	::gethostname(host_name, sizeof(host_name));
	// get local host ip address
	hostent *host_ip = ::gethostbyname(host_name);
	// use the first ip
	char *p = host_ip->h_addr_list[0];
	if (NULL == p) 
	{
		printf("Get local host ip address failed!\n");
		return -1;
	}
	in_addr addr;
	memcpy(&addr.S_un.S_addr, p, host_ip->h_length);

	sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(nPort);
	server_addr.sin_addr = addr;
	//���׽��ֵ����ػ���
	if(::bind(sListen, (sockaddr*)&server_addr, sizeof(server_addr))==SOCKET_ERROR)
	{
		printf("�󶨼����׽���ʧ��\n");
		return -1;
	}
	//�������ģʽ
	::listen(sListen, 5);
	//selectģ�ʹ���
	//1)��ʼ��һ���׽��ּ���fdSocket,��Ӽ����׽��־�����������
	fd_set fdSocket; //���п����׽��ּ���
	FD_ZERO (&fdSocket);
	FD_SET (sListen, &fdSocket);
	while(TRUE)
	{
		//2)��fdsocket���ϵ�һ������fdSocket���ݸ�select����
		fd_set fdRead = fdSocket;
    //	printf("selectǰ\n");
		int nRet = ::select(0, &fdRead, NULL, NULL, NULL);
   //		printf("select��\n");
		if(nRet>0)
		{
			//3)ͨ����ԭ��fdSocket������select�������fdRead���ϱȽ�
			for(int i = 0; i < (int)fdSocket.fd_count; i++)
			{
				if(FD_ISSET(fdSocket.fd_array[i], &fdRead))
				{
					if(fdSocket.fd_array[i] == sListen)//�����׽��ֽ��յ��µ�����
					{
						if(fdSocket.fd_count < FD_SETSIZE)//64
						{
							sockaddr_in client_addr;
							int nClientLen = sizeof(client_addr);
							SOCKET sNew = ::accept( sListen, (SOCKADDR*)&client_addr, &nClientLen);
							FD_SET(sNew, &fdSocket);
							printf("���յ��µ�����(%s)\n", ::inet_ntoa(client_addr.sin_addr)); 
							//todp:�㲥�����������û���Ϣ
							//char szHello[10]={"hello"};
							//send(sNew, szHello, strlen(szHello), 0);
						}
						else
						{
							printf("�����������࣬����ʧ�ܣ�\n");
							continue;
						}
					}
					else
					{
						char rec_buffer[256] = {0};
						int rec_len = ::recv(fdSocket.fd_array[i], rec_buffer, sizeof(rec_buffer), 0);//��������
						if(rec_len > 0)
						{
							printf("���յ������ݣ�%s\n", rec_buffer);
							char szHello[10]={"hello"};
							send(fdSocket.fd_array[i], szHello, strlen(szHello), 0);
						}
						else//���ӹرա��������ж�
						{
#ifdef _DEBUG
							printf("Close %d", i);
#endif
							::closesocket(fdSocket.fd_array[i]);
							FD_CLR(fdSocket.fd_array[i], &fdSocket);
						}
					}
				}
			}//end for
		}
		else
		{
			printf("ѡ���׽���ʧ�ܣ�\n");
			break;
		}
	}
	closesocket(sListen);
	WSACleanup();
	return 0;
}


bool BroadcastAllUser()
{
   return false;
}


