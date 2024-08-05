#pragma once

#include "non_void_helper.hpp"
#include <algorithm>
#include <type_traits>
namespace coro_async
{

template <typename T>
class Utilized
{
public:
	Utilized() = default;
	Utilized(const Utilized& other)
		: data_(other.data_)
	{

	}

	Utilized& operator=(const Utilized& other)
	{
		data_ = other.data_;
	}

	Utilized(const T& other)
		: data_(other)
	{

	}

	Utilized& operator=(const T& other)
	{
		data_ = other;
	}

	Utilized(Utilized&& other)
		: data_(std::move(other.data_))
	{

	}

	Utilized& operator=(Utilized&& other)
	{
		data_ = std::move(other.data_);
	}

	Utilized(T&& other)
		: data_(std::move(other))
	{

	}

	Utilized& operator=(T&& other)
	{
		data_ = std::move(other);
	}

	T get()
	{
		return data_;
	}

private:
	T data_;
};



template<typename T>
class Utilized<const T> : public Utilized<T> {};

template<typename T>
class Utilized<T&> : public Utilized<std::remove_reference<T>> {};


}
