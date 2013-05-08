#include "err.h"
#include <stdio.h>

static const char *errs[] =
{
	"no error",
	"init socket lib error"     // ��ʼ��winsock��ʧ��
};
Err::Err(int errcode, const char *file, int line)
{
	this->errcode = errcode;
	this->line = line;
	this->file = file;
}

const char* Err::what() const throw()
{
	static char msg[100];

	if(file && line !=-1){
		sprintf(msg, "FILE:%s,LINE:%d : %s", file, line ,errs[errcode]);
		return msg;
	}

	return errs[errcode];
}

int Err::code() const
{
	return errcode;
}
