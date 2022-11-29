/**
 * @file emb_algorithm.h
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
#include "emb_pair.h"


namespace emb {
/// @addtogroup emb
/// @{


/**
 * @brief Returns pointer to the first element in range [first; last) that is equal to value.
 * 
 * @tparam It 
 * @tparam T 
 * @param first 
 * @param last 
 * @param value 
 * @return It 
 */
template <class It, class T>
inline It find(It first, It last, const T& value)
{
	for (; first != last; ++first)
	{
		if (*first == value)
		{
			return first;
		}
	}
	return last;
}


/**
 * @brief Returns pointer to the element in range [first; last) that is equal to value.
 * Range must be sorted.
 * 
 * @tparam It 
 * @tparam T 
 * @param first 
 * @param last 
 * @param value 
 * @return It 
 */
template <class It, class T>
inline It binary_find(It first, It last, const T& value)
{
	It notFound = last;

	while (first < last)
	{
		It mid = first + (last - first) / 2;

		if (value == *mid)
		{
			return mid;
		}

		if (value < *mid)
		{
			last = mid;
		}
		else
		{
			first = mid + 1;
		}
	}

	return notFound;
}


/**
 * @brief Assigns the given value to the elements in the range [first, last).
 * 
 * @tparam It 
 * @tparam T 
 * @param first 
 * @param last 
 * @param value 
 */
template <class It, class T>
inline void fill(It first, It last, const T& value)
{
	for (; first != last; ++first)
	{
		*first = value;
	}
}


/**
 * @brief Returns the number of elements in the range [first, last) that is equal to value.
 * 
 * @tparam It 
 * @tparam T 
 * @param first 
 * @param last 
 * @param value 
 * @return size_t 
 */
template <class It, class T>
inline size_t count(It first, It last, const T& value)
{
	size_t ret = 0;
	for (; first != last; ++first)
	{
		if (*first == value)
		{
			++ret;
		}
	}
	return ret;
}


/**
 * @brief Copies the elements in the range, defined by [first, last), to another range beginning at d_first.
 * 
 * @tparam InputIt 
 * @tparam OutputIt 
 * @param first 
 * @param last 
 * @param d_first 
 * @return OutputIt 
 */
template <class InputIt, class OutputIt>
inline OutputIt copy(InputIt first, InputIt last, OutputIt d_first)
{
	for (; first != last; ++first, ++d_first)
	{
		*d_first = *first;
	}
	return d_first;
}


/**
 * @brief Returns true if the range [first1, last1) is equal to the range [first2, first2 + (last1 - first1)), and false otherwise.
 * 
 * @tparam It 
 * @param first1 
 * @param last1 
 * @param first2 
 * @return true 
 * @return false 
 */
template <class It>
inline bool equal(It first1, It last1, It first2)
{
	for (; first1 != last1; ++first1, ++first2)
	{
		if (!(*first1 == *first2))
		{
			return false;
		}
	}
	return true;
}


/**
 * @brief Clamps a value between a pair of boundary values.
 * 
 * @tparam T 
 * @param value 
 * @param lo 
 * @param hi 
 * @return const T& 
 */
template <class T>
inline const T& clamp(const T& value, const T& lo, const T& hi)
{
	return (value < lo) ? lo : (hi < value) ? hi : value;
}


/**
 * @brief Finds the greatest element in the range [first, last).
 * 
 * @tparam It 
 * @param first 
 * @param last 
 * @return It 
 */
template <class It>
inline It max_element(It first, It last)
{
	if (first == last)
	{
		return last;
	}

	It largest = first;
	++first;
	for(; first != last; ++first)
	{
		if (*largest < *first)
		{
			largest = first;
		}
	}
	return largest;
}


/**
 * @brief Finds the smallest element in the range [first, last).
 * 
 * @tparam It 
 * @param first 
 * @param last 
 * @return It 
 */
template <class It>
inline It min_element(It first, It last)
{
	if (first == last)
	{
		return  last;
	}

	It smallest = first;
	++first;
	for (; first != last; ++first)
	{
		if (*first < *smallest)
		{
			smallest = first;
		}
	}
	return smallest;
}


/**
 * @brief Finds the smallest and greatest element in the range [first, last).
 * 
 * @tparam It 
 * @param first 
 * @param last 
 * @return Pair<It, It> 
 */
template <class It>
inline Pair<It, It> minmax_element(It first, It last)
{
	It min = first, max = first;

	if ((first == last) || (++first == last))
	{
		return Pair<It, It>(min, max);
	}

	if (*first < *min)
	{
		min = first;
	}
	else
	{
		max = first;
	}

	while (++first != last)
	{
		It i = first;
		if (++first == last)
		{
			if (*i < *min) min = i;
			else if (!(*i < *max)) max = i;
			break;
		}
		else
		{
			if (*first < *i)
			{
				if (*first < *min) min = first;
				if (!(*i < *max)) max = i;
			}
			else
			{
				if (*i <  *min) min = i;
				if (!(*first < *max)) max = first;
			}
		}
	}
	return Pair<It, It>(min, max);
}


/// @}
} // namespace emb


