#pragma once

#include <asm-generic/errno.h>
#include <cerrno>
#include <coroutine>
#include <cstring>
#include <liburing.h>
#include <liburing/io_uring.h>
#include <mutex>
#include <vector>
#include <iostream>

#include "../uring/ops.hpp"
#include "executor_abstract_base.hpp"
#include "../detail/lockfree.hpp"


namespace coro_async
{

class executor
	: public executor_abstract_base
{
public:
	executor()
		: stop_(false),
		  running_(false)
	{

	}

	void run() override
	{
		executor_in_single_thread single(this);
		running_ = true;
		single.run();
	}

	io_uring_sqe*
	get_sqe() override 
	{
		return executor_in_single_thread::get_sqe();
	}


	void add_multithread_task(std::coroutine_handle<> coro) override
	{
		multi_thread_coro_queue_.push(coro);

		for (int i = 0; i < multi_thread_task_add_ring_.size(); ++i)
		{
			auto sqe = get_sqe();
			io_uring_sqe_set_data(sqe, &multi_thread_task_add_indicated_);
			io_uring_prep_msg_ring(sqe, multi_thread_task_add_ring_[i], 0, reinterpret_cast<__u64>(&multi_thread_task_add_indicated_), 0);
		}
		if (running_) [[likely]]
			executor_in_single_thread::submit();
	}

private:
	class executor_in_single_thread
	{
	public:
		executor_in_single_thread(executor* ptr)
			: exec_ptr_(ptr)
		{
			io_uring_queue_init(1024, &ring_, 0);
			io_uring_register_ring_fd(&ring_);
			{
				std::lock_guard<std::mutex> lock(exec_ptr_->load_mutex_);
				exec_ptr_->multi_thread_task_add_ring_.push_back(ring_.ring_fd);
			}
		}

		static
		io_uring_sqe*
		get_sqe()
		{
			return io_uring_get_sqe(&ring_);
		}

		static
		void 
		submit()
		{
			io_uring_submit(&ring_);
		}

		void run()
		{
			const auto execute = [](ops* op, int res) -> bool
			{
				op->execute(res);
				return true;
			};
			while (!exec_ptr_->stop_)
			{
				while (!exec_ptr_->multi_thread_coro_queue_.empty())
				{
					std::coroutine_handle<> coro = exec_ptr_->multi_thread_coro_queue_.pop();
					if (coro)
					{
						coro.resume();
					}
				}

				io_uring_cqe* cqe = nullptr;
				io_uring_submit_and_wait(&ring_, 1);
				unsigned int head;
				unsigned int count = 0;

				io_uring_for_each_cqe(&ring_, head, cqe)
				{
					++count;
					auto op = reinterpret_cast<ops*>(cqe->user_data);
					static_cast<int>(cqe->res) != -ECANCELED && !(cqe->flags & IORING_CQE_F_NOTIF) && op && execute(op, cqe->res);
				}
				io_uring_cq_advance(&ring_, count);

			}
		}
	private:
		static thread_local io_uring ring_;
		executor* exec_ptr_;
	};
	bool stop_;
	std::mutex load_mutex_;
//	std::queue<std::coroutine_handle<>> multi_thread_coro_queue_;
	LockFreeQueue<std::coroutine_handle<>> multi_thread_coro_queue_;
	nop_ops multi_thread_task_add_indicated_{};
	std::vector<int> multi_thread_task_add_ring_;
	bool running_;
	friend class executor_in_single_thread;
	friend class ops;
};

thread_local io_uring executor::executor_in_single_thread::ring_;

}
