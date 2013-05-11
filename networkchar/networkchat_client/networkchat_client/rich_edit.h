#ifndef NETWORKCHAT_RICH_EDIT_H_
#define NETWORKCHAT_RICH_EDIT_H_

#include "err.h"
#include <Windows.h>

class RichEdit
{
public:
	RichEdit(HWND hwnd, unsigned int id);
	~RichEdit();
	int GetLineCount();
	int GetTail();
	void SetSel(int start, int end);

private:
	HWND hwnd_;
	unsigned int id_;
};

#endif