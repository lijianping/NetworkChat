#ifndef NETWORKCHAT_CLIENT_CHAT_H_
#define NETWORKCHAT_CLIENT_CHAT_H_

#include "my_socket.h"
#include "resource.h"
#include <windows.h>
#include <string>
#include <list>
#include <map>
using namespace std;

#define WM_USER_IP (WM_USER + 101)
#define WM_GROUP_TALK (WM_USER + 102)
#define WM_SINGLE_TALK (WM_USER + 103)

BOOL CALLBACK ChatDlgProc(HWND hChatDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
const string GetTime();

/*
 * @ brief: 打开文件
 * @ param: in [in] 文件对象流
 * @ param: file [in] 文件名称
 * @ param: is_in [in] 打开方式，若为false则表示已写入形式打开（默认），否则为读入形式打开
 * @ return: 文件对象流
 **/
fstream& open_file(fstream &in, const string &file, bool is_in = false);

#endif