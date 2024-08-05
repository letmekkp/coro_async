#pragma once

#include "endpoint.hpp"
#include "socket.hpp"

namespace coro_async
{

namespace ip
{

class tcp
{
public:

	using endpoint = basic_endpoint<tcp>;
	using socket = basic_stream_socket<tcp>;
	using acceptor = basic_accept_socket<tcp>;

	int family() const noexcept
	{
		return family_;
	}

	static int type()
	{
		return SOCK_STREAM;
	}

	static tcp v4()
	{
		return tcp(AF_INET);
	}

	static tcp v6()
	{
		return tcp(AF_INET6);
	}

private:

	tcp(int family)
		: family_(family)
	{

	}

	int family_;
};
}

}
