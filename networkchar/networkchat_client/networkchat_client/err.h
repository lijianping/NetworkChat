#ifndef NETWORKCHAT_CLIENT_ERR_H_
#define NETWORKCHAT_CLIENT_ERR_H_

#include <exception>
enum NETWORKCHAT_ERR
{
	ERR_NOERR = 0,
	ERR_INIT_SOCKET
};
class Err : public std::exception
{
public:
	public:
        Err(int errcode, const char *file = 0, int line = -1);
        virtual const char *what() const throw();
        /**
         *@brief ªÒ»°¥ÌŒÛ±‡¬Î
         *@return ¥ÌŒÛ±‡¬Î
         */
        int code() const;
    private:
        int errcode;
        int line;
        const char *file;
};

#ifdef _DEBUG
/** eSign throw*/
#define LTHROW(err) throw Err(err, (const char*)__LINE__, (int)__FILE__);
#else
#define LTHROW(err) throw Err(err);
#endif

#endif