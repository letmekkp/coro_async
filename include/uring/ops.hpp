#pragma once

#include <coroutine>


namespace coro_async
{

class ops 
{
public:
	virtual ~ops() = default;
	virtual void execute(int res) = 0;
	std::coroutine_handle<> coro_ = nullptr;
	int res_;
};

class resume_ops
	: public ops
{
public:
	void execute(int res) override
	{
		op_completed = true;
		res_ += res;
		coro_.resume();
	}
	bool op_completed = false;
	bool op_registered = false;
};

class detatched_ops
	: public ops
{
public:
	void execute(int res) override
	{
		coro_.resume();
		delete this;
	}
};

class nop_ops
	: public ops
{
public:
	nop_ops() 
	{

	}
	void execute(int res) override
	{

	}
};


}
