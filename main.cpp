#include <cstddef>
#include <cstdio>
#include <system_error>
#include <thread>
#include <tuple>
#include <utility>
#include <variant>

#include "include/awaiter/task.hpp"
#include "include/executor/executor.hpp"
#include "include/executor/co_spawn.hpp"
#include "include/this_coro/this_coro.hpp"
#include "include/net/tcp.hpp"
#include "include/operators/operator.hpp"
#include "include/time/timer.hpp"

using namespace coro_async;

using namespace std::chrono_literals;


Task<> test()
{
	std::cout << "this is a test" << std::endl;
	std::cout << "hello" << std::endl;
	co_return;
}
Task<> world()
{
	std::cout << "world" << std::endl;
	co_return;
}


Task<void> echo(ip::tcp::socket sock)
{
	char buf[BUFSIZ];
	for (;;)
	{
		auto [n, ecr] = co_await sock.async_receive(buf, BUFSIZ);
		if (ecr)
			break;
		auto [nw, ecw] = co_await sock.async_send(buf, n);
		if (ecw)
			break;
	}
}


Task<void> acp()
{
	auto ex = co_await this_coro::executor;
	ip::tcp::acceptor acceptor(ex, ip::tcp::endpoint(ip::tcp::v4(), 9999));

	for (;;)
	{
		auto sock = co_await acceptor.async_accept();
		co_spawn(ex, echo(std::move(sock)));
	}
}


int main()
{
	executor ex;
	co_spawn(ex, acp());
	for (int i = 0; i < 11; ++i)
	{
		std::thread([&]()
				{
					ex.run();
				}).detach();
	}
	ex.run();

	return 0;
}
