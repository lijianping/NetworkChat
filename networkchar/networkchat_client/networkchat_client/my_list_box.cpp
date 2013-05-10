#include "my_list_box.h"


MyListBox::MyListBox(HWND hwnd, unsigned int id)
	: hwnd_(hwnd), id_(id)
{
}


MyListBox::~MyListBox()
{
}

int MyListBox::AddString(const char *data) 
{
	return SendMessage(hwnd_, LB_ADDSTRING, 0, (WPARAM)(LPCTSTR)data);
}

void MyListBox::DeleteAllString()
{
	int count = GetCount();
	while (count != 0) 
	{
		DeleteString(0);
		count = GetCount();
	}
}

int MyListBox::DeleteString(const int &index)
{
	return SendMessage(hwnd_, LB_DELETESTRING, index, 0);
}


int MyListBox::GetCount()
{
	return SendMessage(hwnd_, LB_GETCOUNT, 0, 0);
}

int MyListBox::GetSelect()
{
	return SendMessage(hwnd_, LB_GETCURSEL, 0, 0);
}

int MyListBox::GetText(const int index, char *buff)
{
	return SendMessage(hwnd_, LB_GETTEXT, (WPARAM)index, (LPARAM)buff);
}