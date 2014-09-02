#ifndef TAMANDUA_SERVER_HPP
#define TAMANDUA_SERVER_HPP
#include "types.hpp"
#include "logger.hpp"
#include "message.hpp"
#include "server_config.hpp"
#include "user_message_interpreter.hpp"
#include <map>
#include <string>
#include <memory>
#include <chrono>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

using boost::asio::ip::tcp;

namespace tamandua
{
	class participant;
	class group;

	class server
	{
		private:
			boost::asio::io_service &io_service_;
			boost::asio::ssl::context context_;
			tcp::acceptor acceptor_;
			tcp::endpoint endpoint_;
			logger &log_;
			user_message_interpreter interpreter_;

			std::map<id_number_t, std::shared_ptr<participant>> participants_;
			std::map<std::string, id_number_t> participants_ids_;
			std::map<id_number_t, std::shared_ptr<group>> groups_;
			std::map<std::string, id_number_t> groups_ids_;

			id_number_t last_participant_id_, last_group_id_;

			std::chrono::system_clock::time_point start_time_;

		public:
			server(boost::asio::io_service &, tcp::endpoint &, logger &, boost::asio::ssl::context &);
			~server();
		
			void start_server(server_config &);
			void send_startup_data(std::shared_ptr<participant>);
			boost::asio::io_service &get_io_service();
			void process_message(std::shared_ptr<user>, message&);

			void add_participant(std::shared_ptr<participant>);
			void add_group(std::shared_ptr<group>);

			std::shared_ptr<participant> get_participant(id_number_t);
			std::shared_ptr<participant> get_participant(std::string);
			std::shared_ptr<group> get_group(id_number_t);
			std::shared_ptr<group> get_group(std::string);

			bool is_participant_name_available(std::string);
			bool is_group_name_available(std::string);

			bool change_participant_name(std::string, std::string);
			void quit_participant(id_number_t, status = ok);

			id_number_t get_last_participant_id() const;
			id_number_t get_last_group_id() const;

			bool is_username_valid(std::string);
			void set_message_id(message &);
			std::string get_uptime_string();

			logger & get_logger();
			user_message_interpreter & get_interpreter();

		private:
			void add_root_(std::string);
			void add_hall_(std::string);
			void add_new_user_(std::shared_ptr<user>);
			void accept_connection_();
			void send_rooms_list_();
			void send_participants_list_();
			void send_to_all_(message_type, std::string);

			friend class participant;
			id_number_t get_new_participant_id_();
			friend class group;
			id_number_t get_new_group_id_();

			std::string get_default_user_name_(id_number_t);
			std::string get_default_group_name_(id_number_t);
			std::string generate_rooms_list_();
			std::string generate_participants_list_();
	};
}

#endif
