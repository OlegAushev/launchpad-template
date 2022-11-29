/**
 * @file emb_array.h
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
#include <assert.h>


namespace emb {
/// @addtogroup emb
/// @{


/**
 * @brief
 */
template <typename T, size_t Size>
class Array
{
public:
	T data[Size];

	size_t size() const { return Size; }

	T& operator[] (size_t pos)
	{
#ifdef NDEBUG
		return data[pos];
#else
		return at(pos);
#endif
	}

	const T& operator[](size_t pos) const
	{
#ifdef NDEBUG
		return data[pos];
#else
		return at(pos);
#endif
	}

	T& at(size_t pos)
	{
		assert(pos < Size);
		return data[pos];
	}

	const T& at(size_t pos) const
	{
		assert(pos < Size);
		return data[pos];
	}

	T* begin() { return data; }
	T* end() { return data + Size; }
	const T* begin() const { return data; }
	const T* end() const { return data + Size; }

	void fill(const T& value)
	{
		for (size_t i = 0; i < Size; ++i)
		{
			data[i] = value;
		}
	}
};


/// @}
} // namespace emb


