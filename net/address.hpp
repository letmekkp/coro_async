#pragma once
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string>

namespace coro_async
{

class address_v4 
{
public:
	address_v4()
	{
		addr_.s_addr = 0;
	}
	address_v4(const char* strAddr)
	{
		int ret = inet_pton(AF_INET, strAddr, &addr_);
		if (ret != 1) [[unlikely]]
		{
			addr_.s_addr = 0;
		}
	}
	address_v4(const std::string& strAddr)
		: address_v4(strAddr.data())
	{

	}

	address_v4(const address_v4& rhs)
		: addr_(rhs.addr_)
	{

	}

	address_v4(address_v4&& rhs)
		: addr_(rhs.addr_)
	{

	}

	address_v4& operator=(const address_v4& rhs)
	{
		addr_ = rhs.addr_;
		return *this;
	}

	address_v4& operator=(address_v4&& rhs)
	{
		addr_ = rhs.addr_;
		return *this;
	}

	in_addr to_in_addr() const noexcept
	{
		return addr_;
	}

	friend bool operator==(const address_v4& lhs, const address_v4& rhs)
	{
		return lhs.addr_.s_addr == rhs.addr_.s_addr;
	}

private:
	in_addr addr_;
};

}
