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
	static bool _initialized;
protected:
	monostate()
	{
		assert(_initialized);
	}

	~monostate() {}

	static void _set_initialized()
	{
		assert(!_initialized);
		_initialized = true;
	}
public:
	static bool initialized() { return _initialized; }
};

template <class T>
bool monostate<T>::_initialized = false;


/// @}
} // namespace emb


