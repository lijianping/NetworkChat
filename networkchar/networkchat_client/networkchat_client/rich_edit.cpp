#include "rich_edit.h"
#include <stdexcept>
using namespace std;
RichEdit::RichEdit(HWND hwnd, unsigned int id)
{
	HMODULE moudle = LoadLibrary("riched20.dll");
	if (!moudle)
		throw runtime_error("Cannot load \"riched20.dll\".");
	hwnd_ = hwnd;
	id_ = id;
}


RichEdit::~RichEdit(void)
{
}

int RichEdit::GetLineCount()
{
	int ret = ::SendMessage(hwnd_, EM_GETLINECOUNT, 0, 0);
	const int start_char_of_last_line = (int)SendMessage(hwnd_, EM_LINEINDEX, ret - 1, 0);
	if (!SendMessage(hwnd_, EM_LINELENGTH, start_char_of_last_line, 9))
	{
		--ret;
	}
	return ret;
}

int RichEdit::GetTail()
{
	const int lines = GetLineCount();
	int ret = (int)SendMessage(hwnd_, EM_LINEINDEX, lines - 1, 0);
	ret += (int)SendMessage(hwnd_, EM_LINELENGTH, ret, 0);
	return ret;
}

void RichEdit::SetSel(int start, int end)
{
	SendMessage(hwnd_, EM_SETSEL, start, end);
}

void RichEdit::AppendText(const std::string &text)
{
	const int tail = GetTail();
	SetSel(tail + 1, tail + 1);
	::SendMessage(hwnd_, EM_REPLACESEL, FALSE, (long)text.c_str());
}