#include "save_msg.h"
#include "err.h"

SaveMsg::SaveMsg(const char *file_name)
{
	file_name_ = file_name;
}
SaveMsg::~SaveMsg()
{

}

bool SaveMsg::ReadMsgText()
{
	//	TODO;
	return true;
}

bool SaveMsg::SaveMsgText(const char *msg_text)
{
	save_text.open(file_name_, ios::app);
	if (!save_text.good())
	{
		LTHROW(ERR_FILE_OPEN_FAILD)
	}
	save_text << msg_text;
	save_text << endl;
	save_text.close();
	return true;
}