#pragma once

#include <coroutine>


namespace coro_async 
{


class AutoDestroyAwaiter
{
public:
	auto await_ready() const noexcept { return false; }

	void await_suspend(std::coroutine_handle<> coroutine) noexcept
	{
		coroutine.destroy();
	}

	auto await_resume() const noexcept {}
};


}
