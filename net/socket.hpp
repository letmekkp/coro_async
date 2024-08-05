#pragma once

#include <cstddef>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <system_error>
#include <tuple>
#include <unistd.h>

#include "../awaiter/task.hpp"
#include "../uring/uring_ops.hpp"
#include "../executor/executor.hpp"

namespace coro_async
{


class basic_socket
{
public:
	basic_socket()
		: fd_(-1)
	{

	}


	void close()
	{
		if (fd_ != -1)
			::close(fd_);
	}

	virtual ~basic_socket()
	{
		close();
	}

protected:
	basic_socket(int fd)
		: fd_(fd)
	{

	}
	int fd_;
};

template<typename Protocol>
class basic_stream_socket
	: public basic_socket
{
	friend class Promise<basic_stream_socket<Protocol>>;
public:
	using protocol_type = Protocol;

	basic_stream_socket(const any_executor& exec)
		: basic_socket(),
		  exec_(exec)
	{

	}

	basic_stream_socket(executor& exec)
		: basic_stream_socket(any_executor(&exec))
	{

	}

	basic_stream_socket(basic_stream_socket&& sock)
		: basic_socket(sock.fd_),
		  exec_(sock.exec_)
	{
		sock.fd_ = -1;
	}

	basic_stream_socket& operator=(basic_stream_socket&& rhs) noexcept
	{
		fd_ = rhs.fd_;
		rhs.fd_ = -1;
		exec_ = rhs.exec_;
		return *this;
	}

	void assign(int fd)
	{
		fd_ = fd;
	}

	Task<std::tuple<std::size_t, std::error_code>>
	async_send(
			const char* buf, 
			std::size_t buf_siz)
	{
		//auto n = co_await UringOps(exec_).uring_write(fd_, buf, buf_siz);
		auto n = co_await UringOps(exec_).uring_send(fd_, buf, buf_siz);
		co_return {n, n > 0 ? std::error_code() : (n == 0 ? std::error_code(-1, std::system_category()) : std::error_code(-n, std::system_category()))};
	}

	Task<std::tuple<std::size_t, std::error_code>> 
	async_receive(
			char* buf,
			std::size_t buf_siz)
	{
		//auto n = co_await UringOps(exec_).urinig_read(fd_, buf, buf_siz);
		auto n = co_await UringOps(exec_).uring_recv(fd_, buf, buf_siz);
		co_return {n, n > 0 ? std::error_code() : (n == 0 ? std::error_code(-1, std::system_category()) : std::error_code(-n, std::system_category()))};
	}

	Task<void>
	async_connect(Protocol::endpoint end)
	{
		fd_ = socket(end.family(), Protocol::type(), 0);
		co_await UringOps(exec_).uring_connect(fd_, end.data, end.size);
	}

private:
	basic_stream_socket()
		: basic_socket()
	{

	}
	any_executor exec_;

};

template<typename Protocol>
class basic_datagram_socket
	: public basic_socket
{
public:
	using protocol_type = Protocol;
	using endpoint = Protocol::endpoint;
	basic_datagram_socket(const any_executor& exec)
		: basic_socket(),
		  exec_(exec)
	{
	}

	basic_datagram_socket(executor& exec)
		: basic_datagram_socket(any_executor(&exec))
	{

	}

	basic_datagram_socket(basic_datagram_socket&& sock)
		: basic_socket(sock.fd_),
		  exec_(sock.exec_)
	{
		sock.fd_ = -1;
	}

	basic_datagram_socket& operator=(basic_datagram_socket&& rhs) noexcept
	{
		fd_ = rhs.fd_;
		rhs.fd_ = -1;
		exec_ = rhs.exec_;
		return *this;
	}

	void assign(int fd)
	{
		fd_ = fd;
	}

	Task<std::tuple<std::size_t, std::error_code>>
	async_send_to(
			const char* buf, 
			std::size_t buf_siz, 
			const endpoint& end)
	{
		int sock = socket(end.family(), Protocol::type(), 0);
		auto n = co_await UringOps(exec_).uring_write(fd_, buf, buf_siz);
		co_return {n, n > 0 ? std::error_code() : (n == 0 ? std::error_code(-1, std::system_category()) : std::error_code(-n, std::system_category()))};
	}

	Task<std::tuple<std::size_t, std::error_code>> 
	async_receive_from(
			char* buf,
			std::size_t buf_siz,
			const endpoint& end)
	{
		auto n = co_await UringOps(exec_).urinig_read(fd_, buf, buf_siz);
		co_return {n, n > 0 ? std::error_code() : (n == 0 ? std::error_code(-1, std::system_category()) : std::error_code(-n, std::system_category()))};
	}

	Task<std::tuple<std::size_t, std::error_code>>
	async_send(
			const char* buf, 
			std::size_t buf_siz)
	{
		auto n = co_await UringOps(exec_).uring_write(fd_, buf, buf_siz);
		co_return {n, n > 0 ? std::error_code() : (n == 0 ? std::error_code(-1, std::system_category()) : std::error_code(-n, std::system_category()))};
	}

	Task<std::tuple<std::size_t, std::error_code>> 
	async_receive(
			char* buf,
			std::size_t buf_siz)
	{
		auto n = co_await UringOps(exec_).urinig_read(fd_, buf, buf_siz);
		co_return {n, n > 0 ? std::error_code() : (n == 0 ? std::error_code(-1, std::system_category()) : std::error_code(-n, std::system_category()))};
	}

	Task<void>
	async_connect(Protocol::endpoint end)
	{
		fd_ = socket(end.family(), Protocol::type(), 0);
		co_await UringOps(exec_).uring_connect(fd_, end.data, end.size);
	}

private:
	any_executor exec_;
};

template<typename Protocol>
class basic_accept_socket
	: public basic_socket
{
public:
	using protocol_type = Protocol;
	using endpoint = typename Protocol::endpoint;
	basic_accept_socket(const any_executor& exec)
		: basic_socket(),
		  exec_(exec)
	{
	}

	basic_accept_socket(const any_executor& exec, const endpoint& end)
		: basic_socket(),
		  exec_(exec)
	{
		fd_ = socket(end.family(), Protocol::type(), 0);
		int ret = bind(fd_, end.data(), end.size());
		if (ret < 0)
			std::cout << "bind error" << std::endl;
		ret = listen(fd_, 1024);
		if (ret < 0)
			std::cout << "listen error" << std::endl;
	}

	Task<basic_stream_socket<Protocol>>
	async_accept()
	{
		int fd = co_await UringOps(exec_).uring_accept(fd_, nullptr, nullptr);
		basic_stream_socket<Protocol> sock(exec_);
		sock.assign(fd);
		co_return sock;
	}

private:
	any_executor exec_;
};


}
