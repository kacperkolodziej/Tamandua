#include "main_frame.hpp"
#include "wx/app.h"
#include "tamandua.hpp"
#include "tamandua_box.hpp"
#include "debug_gui.hpp"
#include <string>
#include <functional>

wxDECLARE_APP(gui_client);

main_frame::main_frame() :
	wxFrame(0, wxID_ANY, wxT("Tamandua GUI Client"), wxPoint(100,100), wxSize(400,700))
{
	panel = new wxPanel(this);
	msgs = new tamandua_textctrl(panel, MSGS_CTRL);
	msg = new wxTextCtrl(panel, MSG_CTRL, wxEmptyString, wxPoint(0,0), wxDefaultSize, wxTE_MULTILINE | wxTE_PROCESS_ENTER);
	sizer = new wxBoxSizer(wxVERTICAL);
	connect_sizer = new wxBoxSizer(wxHORIZONTAL);
	connect_host = new wxTextCtrl(panel, CON_HOST_TEXT, wxT("localhost"), wxPoint(0,0), wxDefaultSize, wxTE_PROCESS_ENTER);
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
	tamandua::message msg(tamandua::message_type::standard_message, std::string(data.utf8_str()));
	tb->client.send_message(msg);
}

void main_frame::connect(wxCommandEvent &event)
{
	std::string host(connect_host->GetValue().utf8_str());
	std::string port(connect_port->GetValue().utf8_str());

	tb = new tamandua_box;
	tb->client.add_event_handler(tamandua::event_type::connecting_succeeded,
		std::bind(&main_frame::connect_callback_, this, std::placeholders::_1));
	tb->client.add_event_handler(tamandua::event_type::connecting_failed,
		std::bind(&main_frame::connect_callback_, this, std::placeholders::_1));
	tb->client.add_event_handler(tamandua::event_type::message_received,
		std::bind(&main_frame::message_received_callback_, this, std::placeholders::_1));

	tb->client.connect(host, port);
	tb->io_service_thread = std::thread([this]() {
		tb->io_service.run();
	});
	tb->reader_thread = std::thread([this]() {
		tamandua::client &cl = tb->client;
		bool local_running = true;
		do {
			// start
			auto msg_pair = cl.get_next_message();
			wxString author = wxString::FromUTF8(msg_pair.first.data());
			wxString msg_body = wxString::FromUTF8(msg_pair.second.body.data());
			tamandua::message &msg = msg_pair.second;
			switch (msg.header.type)
			{
				case tamandua::message_type::info_message:
					Debug("info: ", msg_body);
					wxTheApp->GetTopWindow()->GetEventHandler()->CallAfter(std::bind(&tamandua_textctrl::add_info, msgs, msg_body));
					break;

				case tamandua::message_type::error_message:
					Debug("error: ", msg_body);
					wxTheApp->GetTopWindow()->GetEventHandler()->CallAfter(std::bind(&tamandua_textctrl::add_error, msgs, msg_body));
					break;

				case tamandua::message_type::warning_message:
					Debug("warning: ", msg_body);
					wxTheApp->GetTopWindow()->GetEventHandler()->CallAfter(std::bind(&tamandua_textctrl::add_warning, msgs, msg_body));
					break;

				case tamandua::message_type::private_message:
					Debug("@", author, ": ", msg_body);
					wxTheApp->GetTopWindow()->GetEventHandler()->CallAfter(std::bind(&tamandua_textctrl::add_private_message, msgs, author, msg_body));
					break;
				
				case tamandua::message_type::standard_message:
					Debug(author, ": ", msg_body);
					wxTheApp->GetTopWindow()->GetEventHandler()->CallAfter(std::bind(&tamandua_textctrl::add_message, msgs, author, msg_body));
					break;
			}

			tb->running_lock.lock();
			local_running = tb->running;
			tb->running_lock.unlock();
		} while (local_running);
	});
}

void main_frame::disconnect(wxCommandEvent &event)
{
	if (tb->client.is_connected())
	{
		msgs->add_info(wxT("Disconnecting..."));
		tb->turn_off();
		tb->reader_thread.join();
		tb->client.disconnect();
		tb->io_service_thread.join();
		msgs->add_info(wxT("Disconnected!"));
	}
}

void main_frame::msgs_url(wxTextUrlEvent &event)
{
	if (event.GetMouseEvent().Button(wxMOUSE_BTN_LEFT))
		wxMessageBox(wxT("You clicked URL address, but you didn't specified your web browser in Tamandua Client options.\nIt was probably because Tamandua Client has not options :D"), wxT("You clicked URL address"), wxICON_INFORMATION);
}

void main_frame::connecting_succeeded_()
{
	connected = true;
	Debug("Connected to server!");
	wxString info(wxT("Connected to server!"), wxMBConvUTF8());
	msgs->add_info(info);
}

void main_frame::connecting_failed_()
{
	Debug("Connecting failed!");
	wxString error(wxT("Connecting failed!"), wxMBConvUTF8());
	msgs->add_error(error);
}

void main_frame::message_sent_()
{
	Debug("Message sent!");
}

void main_frame::message_undelivered_()
{
	wxString error(wxT("Message undelivered!"), wxMBConvUTF8());
	msgs->add_error(error);
}

void main_frame::connect_callback_(tamandua::status st)
{
	if (st == tamandua::status::ok)
	{
		msgs->add_info(wxT("Connected to server!"));
		connect_button->Unbind(wxEVT_BUTTON, &main_frame::connect, this);
		connect_button->Bind(wxEVT_BUTTON, &main_frame::disconnect, this);
		connect_button->SetLabel(wxT("Disconnect"));
	} else
	{
		msgs->add_error(wxT("Connecting failed!"));
	}
}

void main_frame::disconnect_callback_(tamandua::status st)
{
	if (st == tamandua::status::ok)
	{
		msgs->add_info(wxT("Disconnected"));
		connect_button->Unbind(wxEVT_BUTTON, &main_frame::disconnect, this);
		connect_button->Bind(wxEVT_BUTTON, &main_frame::connect, this);
		connect_button->SetLabel(wxString(wxT("Connect")));
	}
}

void main_frame::message_received_callback_(tamandua::status st)
{
}

BEGIN_EVENT_TABLE(main_frame, wxFrame)
//	EVT_TEXT_ENTER(CON_HOST_TEXT, main_frame::connect)
//	EVT_TEXT_ENTER(CON_PORT_TEXT, main_frame::connect)
	EVT_TEXT_ENTER(MSG_CTRL, main_frame::send_message)
	EVT_TEXT_URL(MSGS_CTRL, main_frame::msgs_url)
	EVT_BUTTON(CON_BTN, main_frame::connect)
END_EVENT_TABLE()
