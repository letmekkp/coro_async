#pragma once

#include "promise.hpp"
#include <coroutine>
#include <utility>

namespace coro_async
{


template<typename T = void, typename P = Promise<T>>
class Task 
{
public:
	using promise_type = P;
private:
	struct TaskAwaiter
	{
		auto await_ready() const noexcept { return false; }
		template<typename tP>
		auto await_suspend(std::coroutine_handle<tP> coroutine) noexcept
		{
			coroutine_.promise().set_previous(coroutine);
			coroutine_.promise().set_executor_pointer(static_cast<Promise_Base&>(coroutine.promise()).get_executor_pointer());
			return coroutine_;
		}

		auto await_resume() const noexcept
		{
			return coroutine_.promise().result();
		}

		std::coroutine_handle<promise_type> coroutine_;
	};
public:
	Task(std::coroutine_handle<promise_type> coroutine)
		: coroutine_(coroutine)
	{}

	Task()
		: coroutine_(nullptr)
	{

	}

	Task(Task&& that)
		: coroutine_(that.coroutine_)
	{
		that.coroutine_ = nullptr;
	}

	Task& operator=(Task&& that) noexcept
	{
		std::swap(coroutine_, that.coroutine_);
	}

	~Task()
	{
		if (!coroutine_)
			return;
		coroutine_.destroy();
	}

	operator std::coroutine_handle<>() noexcept
	{
		return std::exchange(coroutine_, nullptr);
	}

	auto get() const noexcept
	{
		return coroutine_;
	}

	auto operator co_await() noexcept
	{
		return TaskAwaiter{coroutine_};
	}


private:
	std::coroutine_handle<promise_type> coroutine_;
};

}
