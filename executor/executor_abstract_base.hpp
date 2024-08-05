#pragma once

#include <linux/types.h>
#include <liburing.h>
#include <coroutine>

namespace coro_async
{

class executor_abstract_base
{
public:
	virtual void run() = 0;
	virtual io_uring_sqe* get_sqe() = 0;
	virtual void add_multithread_task(std::coroutine_handle<>) = 0;
};

}
