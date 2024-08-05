#pragma once

#include <algorithm>
#include <coroutine>
#include <exception>
#include <utility>

#include "../concept.hpp"
#include "../executor/any_executor.hpp"
#include "../this_coro/this_coro.hpp"

namespace coro_async
{

class Promise_Base
{
public:
	auto initial_suspend() const noexcept { return std::suspend_always{}; }
	

	void unhandled_exception()
	{
		exception_ptr_ = std::current_exception();
	}
	void set_previous(std::coroutine_handle<> coroutine) { pre_coroutine_ = coroutine;}

	any_executor get_executor_pointer() const noexcept
	{
		return exec_;
	}

	void set_executor_pointer(executor_abstract_base* exec_ptr)
	{
		exec_ = exec_ptr;
	}

	void set_executor_pointer(const any_executor& exec)
	{
		exec_ = exec;
	}

	template<typename T>
	auto await_transform(T&& expr)
	{
		return std::move(expr);
	}

	auto await_transform(this_coro::executor_t)
	{
		struct Awaiter 
		{
			auto await_ready() const noexcept { return true; }
			void await_suspend(std::coroutine_handle<>) const noexcept {}
			auto await_resume() noexcept
			{
				return exec_;
			}
			any_executor exec_;
		};
		return  Awaiter{exec_};
	}


protected:
	std::coroutine_handle<> pre_coroutine_ = std::noop_coroutine();

private:
	std::exception_ptr exception_ptr_;
	any_executor exec_;
};


class Promise_impl
	: public Promise_Base
{
	class FinalAwaiter
	{
	public:
		auto await_ready() const noexcept { return false; }
		auto await_suspend(std::coroutine_handle<> coroutine) const noexcept { return coroutine_; }
		auto await_resume() const noexcept {}

		std::coroutine_handle<> coroutine_;
	};
public:
	auto final_suspend() const noexcept { return FinalAwaiter{pre_coroutine_}; }
};



template<typename T = void>
class Promise : 
	public Promise_impl 
{
public:
	auto get_return_object() noexcept
	{
		return std::coroutine_handle<Promise>::from_promise(*this);
	}

	void return_value(T&& value)
	{
		value_ = std::move(value);
	}

	void return_value(const T& value)
	{
		value_ = value;
	}

	auto yield_value(T&& value)
	{
		value_ = std::move(value);
		return std::suspend_always{};
	}

	auto yield_value(const T& value)
	{
		value_ = value;
		return std::suspend_always{};
	}

	T result() const noexcept
	{
		return value_;
	}

	T&& result() noexcept
	{
		return std::move(value_);
	}

private:
	T value_;
};

template<>
class Promise<void> : 
	public Promise_impl
{
public:
	auto get_return_object() noexcept
	{
		return std::coroutine_handle<Promise>::from_promise(*this);
	}

	void return_void() const noexcept {}

	auto yield_void() noexcept
	{
		return std::suspend_always{};
	}

	void result() const noexcept {}
};

class ReturnPreviousPromise
	: public Promise_impl
{
public:
	auto get_return_object() noexcept
	{
		return std::coroutine_handle<ReturnPreviousPromise>::from_promise(*this);
	}

	void return_value(std::coroutine_handle<> coroutine)
	{
		this->set_previous(coroutine);
	}


	void result() const noexcept {}
};

template<Awaiter A = std::suspend_never>
class IgnorePromise
	: public Promise_Base
{
public:
	
	auto final_suspend() const noexcept { return A{}; }

	auto get_return_object() noexcept
	{
		return std::coroutine_handle<IgnorePromise>::from_promise(*this);
	}

	void return_void() const noexcept {}

	auto yield_void() noexcept
	{
		return std::suspend_always{};
	}
};

}


























