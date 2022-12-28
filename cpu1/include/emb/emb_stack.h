/**
 * @file emb_stack.h
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
class Stack
{
private:
	T _data[Capacity];
	size_t _size;
public:
	Stack()
		: _size(0)
	{}

	void clear() { _size = 0; }
	bool empty() const { return _size == 0; }
	bool full() const { return _size == Capacity; }
	size_t capacity() const { return Capacity; }
	size_t size() const { return _size; }

	void push(const T& value)
	{
		assert(!full());
		_data[_size] = value;
		++_size;
	}

	const T& top() const
	{
		assert(!empty());
		return _data[_size-1];
	}

	void pop()
	{
		assert(!empty());
		--_size;
	}
};


/// @}
} // namespace emb


