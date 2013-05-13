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

/*
 * @ brief: ��ȡ��ǰϵͳʱ��
 * @ return: YYYY-MM-DD hh:mm:ss ��ʽʱ���ַ���
 **/
const string GetTime();

/*
 * @ brief: ���ļ�
 * @ param: in [in] �ļ�������
 * @ param: file [in] �ļ�����
 * @ param: is_in [in] �򿪷�ʽ����Ϊfalse���ʾ��д����ʽ�򿪣�Ĭ�ϣ�������Ϊ������ʽ��
 * @ return: �ļ�������
 **/
fstream& open_file(fstream &in, const string &file, bool is_in = false);

#endif