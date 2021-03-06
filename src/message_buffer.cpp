#include "message_buffer.hpp"
#include <cstring>

using namespace tamandua;

message_buffer::message_buffer(message_header header, std::string message) : buffer_(nullptr), buffer_size_(0)
{
	compose_(header, message);
}

message_buffer::message_buffer(message_composer &msgc) :
	buffer_(nullptr),
	buffer_size_(0)
{
	message msg = msgc();
	compose_(msg.header, msg.body);
}

std::shared_ptr<char> message_buffer::get_buffer()
{
	return buffer_;
}

size_t message_buffer::get_buffer_size()
{
	return buffer_size_;
}

message_header message_buffer::get_message_header()
{
	message_header header;
	memcpy(reinterpret_cast<char*>(&header), buffer_.get(), sizeof(header));
	return header;
}

std::string message_buffer::get_message()
{
	size_t header_size = sizeof(message_header);
	std::string message(buffer_.get() + header_size, get_buffer_size() - header_size);
	return message;
}

void message_buffer::compose_(message_header header, std::string body)
{
	size_t header_size = sizeof(header);
	size_t size = header_size + body.size();
	buffer_.reset(new char[size]);
	buffer_size_ = size;
	memcpy(buffer_.get(), reinterpret_cast<char*>(&header), header_size);
	memcpy(buffer_.get() + header_size, body.data(), body.size());
}
