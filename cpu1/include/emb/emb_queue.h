/**
 * @file emb_queue.h
 * @ingroup emb
 * @author Oleg Aushev (aushevom@protonmail.com)
 * @brief 
 * @version 0.1
 * @date 2022-11-29
 * 
 * @copyright Copyright (c) 2022
 * 
 */


#pragma once


#include <stdint.h>
#include <stddef.h>


namespace emb {
/// @addtogroup emb
/// @{


/**
 * @brief
 */
template <typename T, size_t Capacity>
class Queue
{
private:
	T _data[Capacity];
	size_t _front;
	size_t _back;
	size_t _size;
public:
	Queue()
		: _front(0)
		, _back(0)
		, _size(0)
	{}

	void clear()
	{
		_front = 0;
		_back = 0;
		_size = 0;
	}

	bool empty() const { return _size == 0; }
	bool full() const { return _size == Capacity; }
	size_t capacity() const { return Capacity; }
	size_t size() const { return _size; }

	void push(const T& value)
	{
		assert(!full());

		if (empty())
		{
			_back = _front;
		}
		else
		{
			_back = (_back + 1) % Capacity;
		}
		_data[_back] = value;
		++_size;
	}

	const T& front() const
	{
		assert(!empty());
		return _data[_front];
	}

	const T& back() const
	{
		assert(!empty());
		return _data[_back];
	}

	void pop()
	{
		assert(!empty());
		_front = (_front + 1) % Capacity;
		--_size;
	}
};


/// @}
} // namespace emb


