#ifndef NETWORKCHAT_CLIENT_ERR_H_
#define NETWORKCHAT_CLIENT_ERR_H_

#include <exception>
enum NETWORKCHAT_ERR
{
	ERR_NOERR = 0,
	ERR_INIT_SOCKET,
	ERR_GET_HOST,
	ERR_IP_NULL,
	ERR_CONNECT,
	ERR_MSG_NULL,
	ERR_REQUEST_USER_LIST,
	ERR_ADD_CAST,
	ERR_USER_NAME_NULL,
	ERR_MY_SOCKET_NULL,
	ERR_FILE_OPEN_FAILD,
	ERR_SET_TIME_OUT,
	ERR_SEND_TCP_DATA,
	ERR_SEND_UDP_DATA,
	ERR_INVALID_SOCKET,
	ERR_BIND_FAILED
};
class Err : public std::exception
{
public:
	public:
        Err(int errcode, const char *file = 0, int line = -1);
        virtual const char *what() const throw();
        /**
         *@brief ��ȡ�������
         *@return �������
         */
        int code() const;
    private:
        int errcode;
        int line;
        const char *file;
};

#ifdef DEBUG
#define LTHROW(err) throw Err(err, (const char*)__LINE__, (int)__FILE__);
#else
#define LTHROW(err) throw Err(err);
#endif

#endif