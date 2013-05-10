#ifndef NETWORKCHAT_CLIENT_MY_LIST_BOX_H_
#define NETWORKCHAT_CLIENT_MY_LIST_BOX_H_

#include <Windows.h>

class MyListBox
{
public:
	MyListBox(HWND hwnd, unsigned int id);
	~MyListBox();
	int AddString(const char *data);
	void DeleteAllString();
	int DeleteString(const int &index);
	int GetCount();

protected:

private:
	HWND hwnd_;
	unsigned int id_;
};

#endif
