#pragma once

#include <algorithm>
#include <coroutine>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>

#include "../awaiter/task.hpp"
#include "../detail/operator.hpp"



namespace coro_async
{


template<typename T1, typename T2>
auto operator&&(Task<T1> t1, Task<T2> t2)
{
	struct Awaiter 
	{
		auto await_ready() const noexcept { return false; }
		auto await_suspend(std::coroutine_handle<> coro) noexcept
		{
			ctl.cnt_ = 2;
			ctl.coro_ = coro;
			std::coroutine_handle<> co1 = when_all(t1, ctl, re1);
			std::coroutine_handle<> co2 = when_all(t2, ctl, re2);
			co1.resume();
			return co2;
		}

		auto await_resume() noexcept
		{
			return std::tuple<std::conditional_t<std::is_void_v<T1>, Void, T1>, std::conditional_t<std::is_void_v<T2>, Void, T2>>{re1, re2};
		}

		Task<T1> t1;
		Task<T2> t2;
		WhenAllCtlBlock ctl;
		std::conditional_t<std::is_void_v<T1>, Void, T1> re1;
		std::conditional_t<std::is_void_v<T2>, Void, T2> re2;
	};

	return Awaiter{std::move(t1), std::move(t2)};
}


template<typename T1, typename T2>
auto operator||(Task<T1> t1, Task<T2> t2)
{
	struct Awaiter
	{
		auto await_ready() const noexcept { return false; }

		auto await_suspend(std::coroutine_handle<> coro) noexcept 
		{
			ctl.index_ = -1;
			ctl.coro_ = coro;
			co1 = When_any(std::move(t1_), ctl, 0, re1);
			co2 = When_any(std::move(t2_), ctl, 1, re2);
			co1.resume();
			return co2;
		}


		auto await_resume() noexcept
		{
			return ctl.index_ == 0 
				? std::variant<std::conditional_t<std::is_void_v<T1>, Void, T1>, std::conditional_t<std::is_void_v<T2>, Void, T2>>(re1) 
				: std::variant<std::conditional_t<std::is_void_v<T1>, Void, T1>, std::conditional_t<std::is_void_v<T2>, Void, T2>>(re2);
		}

		Awaiter(Task<T1> t1, Task<T2> t2)
			: t1_(std::move(t1)),
			  t2_(std::move(t2))
		{

		}

		Awaiter(Awaiter&& other)
			: t1_(std::move(other.t1_)),
			  t2_(std::move(other.t2_))
		{

		}

		~Awaiter()
		{
			static auto func = [](std::coroutine_handle<> coro)
			{
				coro.destroy();
				return true;
			};
			co1 && func(co1);
			co2 && func(co2);
		}


		Task<T1> t1_;
		Task<T2> t2_;
		std::coroutine_handle<> co1 = nullptr;
		std::coroutine_handle<> co2 = nullptr;
		WhenAnyCtlBlock ctl;
		std::conditional_t<std::is_void_v<T1>, Void, T1> re1;
		std::conditional_t<std::is_void_v<T2>, Void, T2> re2;
	};

	return Awaiter{std::move(t1), std::move(t2)};
}


}
