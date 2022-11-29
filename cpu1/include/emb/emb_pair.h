/**
 * @file emb_pair.h
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
template <typename T1, typename T2>
class Pair
{
public:
	T1 first;
	T2 second;
	Pair() {}
	Pair(const T1& first_, const T2& second_)
		: first(first_)
		, second(second_) {}

	Pair& operator=(const Pair& other)
	{
		if (this != &other)	// not a self-assignment
		{
			this->first = other.first;
			this->second = other.second;
		}
		return *this;
	}
};


/// @}
} // namespace emb


