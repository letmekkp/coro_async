#pragma once

#include "ops.hpp"
#include <bits/chrono.h>
#include <coroutine>
#include <cstddef>
#include <functional>
#include <liburing.h>
#include <liburing/io_uring.h>
#include <sys/socket.h>
#include <utility>

#include "../executor/any_executor.hpp"


namespace coro_async
{


class UringOps
{
public:
	UringOps(const any_executor& exec)
		: exec_(exec)
	{
	}

	~UringOps()
	{
	}

	auto operator co_await()
	{
		struct Awaiter
		{
			~Awaiter()
			{
				static auto func = [](any_executor& exec, resume_ops* op)
				{
					auto sqe = exec.get_sqe();
					io_uring_prep_cancel(sqe, op, 0);
					io_uring_sqe_set_flags(sqe, IOSQE_CQE_SKIP_SUCCESS);
					return true;
				};
				ops.op_registered && !ops.op_completed && func(exec_, &ops);
			
			}
			auto await_ready() const noexcept { return false; } 

			void await_suspend(std::coroutine_handle<> coroutine) noexcept
			{
				ops.coro_ = coroutine;
				auto sqe = exec_.get_sqe();
				io_uring_sqe_set_data(sqe, &ops);
				func_(sqe);
				ops.op_registered = true;
			}

			auto await_resume() const noexcept
			{
				return ops.res_;
			}
			any_executor exec_;
			std::function<void(io_uring_sqe*)> func_ = nullptr;
			resume_ops ops{};
		};
		return Awaiter{exec_, func_};
	}
private:
	std::function<void(io_uring_sqe*)> func_ = nullptr;
	any_executor exec_;


public:
	UringOps&& urinig_read(int fd, char* buf, std::size_t buf_size) &&
	{
		func_ = [fd, buf, buf_size](io_uring_sqe* sqe)
		{
			io_uring_prep_read(sqe, fd, buf, buf_size, 0);
		};
		return std::move(*this);
	}

	UringOps&& uring_write(int fd, const char* buf, std::size_t buf_size) &&
	{
		func_ = [fd, buf, buf_size](io_uring_sqe* sqe)
		{
			io_uring_prep_write(sqe, fd, buf, buf_size, 0);
		};
		return std::move(*this);
	}

	UringOps&& uring_recv(int fd, char* buf, std::size_t buf_siz) &&
	{
		func_ = [fd, buf, buf_siz](io_uring_sqe* sqe)
		{
			io_uring_prep_recv(sqe, fd, buf, buf_siz, 0);
		};
		return std::move(*this);
	}

	UringOps&& uring_send(int fd, const char* buf, std::size_t buf_siz) &&
	{
		func_ = [fd, buf, buf_siz](io_uring_sqe* sqe)
		{
			io_uring_prep_send_zc(sqe, fd, buf, buf_siz, 0, IOSQE_CQE_SKIP_SUCCESS);
		};
		return std::move(*this);
	}

	UringOps&& uring_timeout(__kernel_timespec* ts) &&
	{
		func_ = [ts](io_uring_sqe* sqe)
		{
			io_uring_prep_timeout(sqe, ts, 0, 0);
		};
		return std::move(*this);
	}

	UringOps&& uring_connect(int fd, const sockaddr* addr, socklen_t addrlen) &&
	{
		func_ = [fd, addr, addrlen](io_uring_sqe* sqe)
		{
			io_uring_prep_connect(sqe, fd, addr, addrlen);
		};
		return std::move(*this);
	}

	UringOps&& uring_accept(int fd, sockaddr* addr, socklen_t* addrlen) &&
	{
		func_ = [fd, addr, addrlen](io_uring_sqe* sqe)
		{
			io_uring_prep_accept(sqe, fd, addr, addrlen, 0);
		};
		return std::move(*this);
	}

	UringOps&& uring_nop(std::coroutine_handle<> coro) &&
	{
		auto sqe = exec_.get_sqe();
		io_uring_prep_nop(sqe);
		detatched_ops* ops = new detatched_ops{};
		ops->coro_ = coro;
		io_uring_sqe_set_data(sqe, ops);
		return std::move(*this);
	}

};


}
