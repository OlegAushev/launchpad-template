/**
 * @file emb_monostate.h
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
template <class T>
class monostate
{
private:
	static bool s_initialized;
protected:
	monostate()
	{
		assert(s_initialized);
	}

	~monostate() {}

	static void set_initialized()
	{
		assert(!s_initialized);
		s_initialized = true;
	}
public:
	static bool initialized() { return s_initialized; }
};

template <class T>
bool monostate<T>::s_initialized = false;


/// @}
} // namespace emb


