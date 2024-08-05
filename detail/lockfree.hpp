#pragma once

#include <atomic>
namespace coro_async
{

template<typename T>
class LockFreeQueue
{
	class FifoNode
	{
	public:
		T value_;
		FifoNode* next_ = nullptr;
		bool can_free = false;
	};
public:
	LockFreeQueue()
		: first_(new FifoNode()),
		  last_(first_.load())
	{
		first_.load()->can_free = true;
	}


	void push(const T& value)
	{
		FifoNode* node = new FifoNode(value);
		FifoNode* last = last_.exchange(node);
		last->next_ = node;
	}

	T pop()
	{
		FifoNode* head = nullptr;
		FifoNode* next = nullptr;
		do 
		{
			head = first_.load();
			next = head->next_;
			if (!next)
				return nullptr;
		} while(!first_.compare_exchange_strong(head, next));
		T value = next->value_;
		next->can_free = true;
		while (!head->can_free);
		delete head;
		return value;
	}

	bool empty()
	{
		return !first_.load()->next_;
	}
private:
	std::atomic<FifoNode*> first_;
	std::atomic<FifoNode*> last_;

};

}
