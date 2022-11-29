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
	T m_data[Capacity];
	size_t m_size;

public:
	Stack()
		: m_size(0)
	{}

	void clear() { m_size = 0; }
	bool empty() const { return m_size == 0; }
	bool full() const { return m_size == Capacity; }
	size_t capacity() const { return Capacity; }
	size_t size() const { return m_size; }

	void push(const T& value)
	{
		assert(!full());
		m_data[m_size] = value;
		++m_size;
	}

	const T& top() const
	{
		assert(!empty());
		return m_data[m_size-1];
	}

	void pop()
	{
		assert(!empty());
		--m_size;
	}
};


/// @}
} // namespace emb


