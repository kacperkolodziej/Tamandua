#include "main_frame.hpp"
#include "wx/app.h"
#include "tamandua.hpp"
#include <string>
#include "debug_gui.hpp"

wxDECLARE_APP(gui_client);

main_frame::main_frame() :
	wxFrame(0, wxID_ANY, wxT("Tamandua GUI Client"), wxPoint(100,100), wxSize(400,700))
{
	panel = new wxPanel(this);
	msgs = new tamandua_textctrl(panel, MSGS_CTRL);
	msg = new wxTextCtrl(panel, MSG_CTRL, wxEmptyString, wxPoint(0,0), wxDefaultSize, wxTE_MULTILINE | wxTE_PROCESS_ENTER);
	sizer = new wxBoxSizer(wxVERTICAL);
	connect_sizer = new wxBoxSizer(wxHORIZONTAL);
	connect_host = new wxTextCtrl(panel, CON_HOST_TEXT, wxEmptyString, wxPoint(0,0), wxDefaultSize, wxTE_PROCESS_ENTER);
	connect_port = new wxTextCtrl(panel, CON_PORT_TEXT, wxT("5000"), wxPoint(0,0), wxDefaultSize, wxTE_PROCESS_ENTER);
	connect_button = new wxButton(panel, CON_BTN, wxT("Connect"));

	panel->SetSizer(sizer);
	connect_sizer->Add(connect_host,1);
	connect_sizer->Add(connect_port,1);
	connect_sizer->Add(connect_button,1);
	sizer->Add(connect_sizer, 0, wxALL | wxEXPAND, 10);
	sizer->Add(msgs, 3, wxBOTTOM | wxLEFT | wxRIGHT | wxEXPAND, 10);
	sizer->Add(msg, 0, wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND, 10);
}

void main_frame::send_message(wxCommandEvent &event)
{
	wxString data = msg->GetValue();
	msg->Clear();

	tamandua::message msg(tamandua::message_type::standard_message, std::string(data.ToAscii()));
	wxGetApp().get_client()->send_message(msg, [this](tamandua::status s) {
		if (s == tamandua::status::ok)
			message_sent_();
		else
			message_undelivered_();
	});
}

void main_frame::connect(wxCommandEvent &event)
{
	std::string host = connect_host->GetValue().ToStdString();
	std::string port = connect_port->GetValue().ToStdString();
	wxGetApp().get_client()->connect(host, port, [this](tamandua::status s) {
		if (s == tamandua::status::ok)
			connecting_succeeded_();
		else
			connecting_failed_();
	});
	wxGetApp().io_service_run();
}

void main_frame::disconnect(wxCommandEvent &event)
{
	msgs->add_info("Disconnecting...");
	wxGetApp().get_client()->disconnect();
}

void main_frame::connecting_succeeded_()
{
	Debug("Connected to server!");
	msgs->add_info(wxT("Connected to server!"));
}

void main_frame::connecting_failed_()
{
	Debug("Connecting failed!");
	msgs->add_error(wxT("Connecting failed!"));
}

void main_frame::message_sent_()
{
	Debug("Message sent!");
}

void main_frame::message_undelivered_()
{
	msgs->add_error(wxT("Message undelivered!"));
}

BEGIN_EVENT_TABLE(main_frame, wxFrame)
//	EVT_TEXT_ENTER(CON_HOST_TEXT, main_frame::connect)
//	EVT_TEXT_ENTER(CON_PORT_TEXT, main_frame::connect)
	EVT_TEXT_ENTER(MSG_CTRL, main_frame::send_message)
	EVT_BUTTON(CON_BTN, main_frame::connect)
END_EVENT_TABLE()
