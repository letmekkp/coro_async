#pragma once

#include "executor_abstract_base.hpp"
#include <coroutine>
#include <liburing/io_uring.h>

namespace coro_async
{

class any_executor
{
public:
	any_executor()
		: exec_(nullptr)
	{

	}
	any_executor(executor_abstract_base* exec_ptr)
		: exec_(exec_ptr)
	{

	}

	any_executor(const any_executor& that)
		: exec_(that.exec_)
	{

	}

	any_executor& operator=(const any_executor& that)
	{
		exec_ = that.exec_;
		return *this;
	}

	io_uring_sqe* get_sqe() 
	{
		return exec_->get_sqe();
	}

	void addTask(std::coroutine_handle<> coro)
	{
		exec_->add_multithread_task(coro);
	}


private:
	executor_abstract_base* exec_ = nullptr;
};


}
