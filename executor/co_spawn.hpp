#include "../awaiter/promise.hpp"
#include "executor.hpp"
#include "../awaiter/task.hpp"
#include "../awaiter/awaiter.hpp"
#include <algorithm>


namespace coro_async
{

template<typename T, typename P>
Task<void, IgnorePromise<AutoDestroyAwaiter>>
coro_detach_helper(Task<T, P> task)
{
	co_await task;
}

template<typename T, typename P>
void
co_spawn(any_executor executor, Task<T, P> task)
{
	auto coro = coro_detach_helper(std::move(task));
	coro.get().promise().set_executor_pointer(executor);
	executor.addTask(coro);
}


template<typename T, typename P>
void
co_spawn(executor& executor, Task<T, P> task)
{
	auto coro = coro_detach_helper(std::move(task));
	coro.get().promise().set_executor_pointer(&executor);
	executor.add_multithread_task(coro);
}


}
