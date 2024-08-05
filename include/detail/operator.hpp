#pragma once

#include <coroutine>
#include <liburing.h>
#include <utility>
#include <iostream>

#include "../awaiter/task.hpp"
#include "non_void_helper.hpp"
#include "../awaiter/awaiter.hpp"


namespace coro_async
{

class WhenAllCtlBlock
{
public:
	int cnt_;
	std::coroutine_handle<> coro_;
};

template<typename T>
Task<void, IgnorePromise<AutoDestroyAwaiter>> 
when_all(auto&& t, WhenAllCtlBlock& ctl, T& result)
{
	static auto func = [&ctl]() -> bool
	{
		std::cout << "template T" << std::endl;
		ctl.coro_.resume();
		return true;
	};
	result = co_await std::forward<decltype(t)>(t);
	(--ctl.cnt_) == 0 && func();
}

template<>
Task<void, IgnorePromise<AutoDestroyAwaiter>> 
when_all(auto&& t, WhenAllCtlBlock& ctl, Void&)
{
	static auto func = [&ctl]() -> bool
	{
		std::cout << "Void special" << std::endl;
		ctl.coro_.resume();
		return true;
	};
	co_await std::forward<decltype(t)>(t);
	(--ctl.cnt_) == 0 && func();
}

class WhenAnyCtlBlock
{
public:
	int index_ = -1;
	std::coroutine_handle<> coro_ = nullptr;
};

template<typename T>
Task<void, ReturnPreviousPromise>
When_any(Task<T> t, 
		WhenAnyCtlBlock& ctl, 
		int index, 
		T& res)
{
	res = co_await t;
	ctl.index_ = index;
	co_return ctl.coro_;
}

template<typename T>
Task<void, ReturnPreviousPromise> 
When_any(Task<T> t, 
		WhenAnyCtlBlock& ctl, 
		int index, 
		Void& res)
{
	co_await t;
	ctl.index_ = index;
	co_return ctl.coro_;
}



}
