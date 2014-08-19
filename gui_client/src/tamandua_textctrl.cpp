#include "tamandua_textctrl.hpp"

void tamandua_textctrl::add_message(wxString author, wxString msg)
{
	wxTextAttr author_attr(wxColour(0, 0, 0xff));
	author_attr.SetFontWeight(wxFONTWEIGHT_BOLD);

	long int start = GetInsertionPoint(), end;
	AppendText(author);
	end = GetInsertionPoint();

	AppendText(": ");
	AppendText(msg);
	AppendText("\n");
	SetStyle(start, end, author_attr);
}

void tamandua_textctrl::add_info(wxString info)
{
	wxTextAttr attr(wxColour(0xff, 0x66, 0));
	long int start = GetInsertionPoint(), end;
	AppendText("[ ");
	AppendText(info);
	AppendText(" ]\n");
	end = GetInsertionPoint();

	SetStyle(start, end, attr);
}

void tamandua_textctrl::add_error(wxString error)
{
	wxTextAttr attr(wxColour(0xcc, 0, 0));
	attr.SetFontWeight(wxFONTWEIGHT_BOLD);
	long int start = GetInsertionPoint(), end;
	AppendText("!!! ");
	AppendText(error);
	AppendText("\n");
	end = GetInsertionPoint();

	SetStyle(start, end, attr);
}