#pragma once

#include <coroutine>

namespace coro_async
{


template<typename A>
concept Awaiter = requires(A a, std::coroutine_handle<> h)
{
	{ a.await_ready() };
	{ a.await_suspend(h) };
	{ a.await_resume() };
};

template<typename A>
concept Awaitable = Awaiter<A> || requires(A a)
{
	{ a.operator co_await() } -> Awaiter;
};

template<typename A>
concept FinAwaiter = Awaiter<A> && requires(std::coroutine_handle<> h)
{
	{ A(h) };
};

}
