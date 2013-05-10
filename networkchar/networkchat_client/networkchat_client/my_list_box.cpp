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
	return SendMessage(hwnd_, LB_ADDSTRING, 0, (LPARAM)data);
}

void MyListBox::DeleteAllString()
{
	for (int i = 0; i < GetCount(); ++i) 
	{
		DeleteString(i);
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
	return SendMessage(hwnd_, LB_GETTEXT, index, (LPARAM)buff);
}