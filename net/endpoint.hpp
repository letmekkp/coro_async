#pragma once

#include "address.hpp"
#include <cstdint>

namespace coro_async
{

class endpoint
{
public:
	using data_type = sockaddr;

	endpoint()
		: data_()
	{
		data_.v4.sin_family = AF_INET;
		data_.v4.sin_addr.s_addr = INADDR_ANY;
		data_.v4.sin_port = 0;
	}

	endpoint(const address_v4& address, std::uint16_t port)
		: data_()
	{
		data_.v4.sin_family = AF_INET;
		data_.v4.sin_addr = address.to_in_addr();
		data_.v4.sin_port = htons(port);
	}

	endpoint(int family, std::uint16_t port)
		: data_()
	{
		if (family == AF_INET)
		{
			data_.v4.sin_family = AF_INET;
			data_.v4.sin_addr.s_addr = INADDR_ANY;
			data_.v4.sin_port = htons(port);
		}
	}

	endpoint(const endpoint& rhs)
		: data_(rhs.data_)
	{

	}

	endpoint(endpoint&& rhs)
		: data_(rhs.data_)
	{

	}

	endpoint& operator=(const endpoint& rhs)
	{
		data_ =  rhs.data_;
		return *this;
	}

	endpoint& operator=(endpoint&& rhs)
	{
		data_ =  rhs.data_;
		return *this;
	}

	bool is_v4() const noexcept
	{
		return data_.base.sa_family == AF_INET;
	}

	friend bool operator==(const endpoint& lhs, const endpoint& rhs) noexcept
	{
		return lhs.data_.base.sa_family == rhs.data_.base.sa_family && lhs.data_.v4.sin_addr.s_addr == rhs.data_.v4.sin_addr.s_addr && 
			lhs.data_.v4.sin_port == rhs.data_.v4.sin_port;
	}

	int family() const noexcept
	{
		return data_.base.sa_family;
	}
	
	data_type* data() noexcept
	{
		return &data_.base;
	}

	const data_type* data() const noexcept
	{
		return &data_.base;
	}

	std::size_t size() const noexcept
	{
		return is_v4() ? sizeof(sockaddr_in) : sizeof(sockaddr_in6);
	}

private:
	union
	{
		sockaddr base;
		sockaddr_in v4;
		sockaddr_in6 v6;
	} data_;
};

template<typename Protocol>
class basic_endpoint
{
public:
	using data_type = sockaddr;


	basic_endpoint()
		: impl_()
	{

	}

	basic_endpoint(const Protocol& protocol, std::uint16_t port)
		: impl_(protocol.family(), port)
	{

	}

	basic_endpoint(const address_v4& addres, std::uint16_t port)
		: impl_(addres, port)
	{

	}

	basic_endpoint(const basic_endpoint& rhs)
		: impl_(rhs.impl_)
	{

	}

	basic_endpoint(basic_endpoint&& rhs)
		: impl_(rhs.impl_)
	{

	}

	int family() const noexcept
	{
		return impl_.family();
	}

	data_type* data() noexcept
	{
		return impl_.data();
	}

	const data_type* data() const noexcept
	{
		return impl_.data();
	}

	std::size_t size() const noexcept
	{
		return impl_.size();
	}
private:
	endpoint impl_;
};
}
