#pragma once

#include "../executor/executor.hpp"
#include "../awaiter/task.hpp"
#include "../uring/uring_ops.hpp"
#include <chrono>
namespace coro_async
{


class Timer 
{
public:

	Timer(const any_executor& exec)
		: exec_(exec)
	{

	}

	template<typename Rep, typename Period>
	Task<> async_wait(std::chrono::duration<Rep, Period> duration)
	{
		__kernel_timespec ts;
		auto sec = std::chrono::duration_cast<std::chrono::seconds>(duration);
		auto nsec = std::chrono::duration_cast<std::chrono::nanoseconds>((duration- sec));
		ts.tv_sec = sec.count();
		ts.tv_nsec = nsec.count();
		auto start = std::chrono::system_clock::now();
		co_await UringOps(exec_).uring_timeout(&ts);
		auto end = std::chrono::system_clock::now();
		std::cout << std::chrono::duration_cast<std::chrono::seconds>(end - start) << std::endl;
	}


private:
	any_executor exec_;
};


}
