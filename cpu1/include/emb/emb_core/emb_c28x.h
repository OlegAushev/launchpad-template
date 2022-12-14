/**
 * @file emb_c28x.h
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


namespace c28x {


/**
 * @brief
 */
template <class T>
class interrupt_invoker
{
private:
	static T* _instance;
	static bool _initialized;
protected:
	interrupt_invoker(T* self)
	{
		assert(!_initialized);
		_instance = self;
		_initialized = true;
	}

	~interrupt_invoker()
	{
		_initialized = false;
		_instance = static_cast<T*>(NULL);
	}
public:
	static T* instance()
	{
		assert(_initialized);
		return _instance;
	}

	static bool initialized() { return _initialized; }
};

template <class T>
T* interrupt_invoker<T>::_instance = static_cast<T*>(NULL);
template <class T>
bool interrupt_invoker<T>::_initialized = false;


/**
 * @brief
 */
template <typename T>
void from_bytes(T& dest, const uint16_t* src)
{
	uint16_t c28_byte[sizeof(T)];
	for (size_t i = 0; i < sizeof(T); ++i)
	{
		c28_byte[i] = src[2*i] | src[2*i+1] << 8;
	}
	memcpy (&dest, &c28_byte, sizeof(T));
}


/**
 * @brief
 */
template <typename T>
void to_bytes(uint16_t* dest, const T& src)
{
	uint16_t c28_byte[sizeof(T)];
	memcpy(&c28_byte, &src, sizeof(T));
	for (size_t i = 0; i < sizeof(T); ++i)
	{
		dest[2*i] = c28_byte[i] & 0x00FF;
		dest[2*i+1] = c28_byte[i] >> 8;
	}
}


/**
 * @brief
 */
template <typename T>
bool is_equal(const T& obj1, const T& obj2)
{
	uint16_t obj1_byte8[sizeof(T)*2];
	uint16_t obj2_byte8[sizeof(T)*2];

	to_bytes<T>(obj1_byte8, obj1);
	to_bytes<T>(obj2_byte8, obj2);

	for(size_t i = 0; i < sizeof(T)*2; ++i)
	{
		if (obj1_byte8[i] != obj2_byte8[i])
		{
			return false;
		}
	}
	return true;
}


} // namespace c28x


/// @}
} // namespace emb


