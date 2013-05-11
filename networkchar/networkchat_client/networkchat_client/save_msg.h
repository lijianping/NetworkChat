#ifndef  NETWORKCHAT_CLIENT_SAVE_MSG_H_
#define  NETWORKCHAT_CLIENT_SAVE_MSG_H_

#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
using namespace std;

class SaveMsg
{
public:
	SaveMsg(const char *file_name);
	~SaveMsg();
	bool ReadMsgText();
    bool SaveMsgText(const char *msg_text);
private:
	std::string file_name_;
	ifstream read_text;
	ofstream save_text;
};
#endif
