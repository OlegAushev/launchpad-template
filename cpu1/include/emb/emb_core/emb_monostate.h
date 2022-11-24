///
#pragma once


#include <stdint.h>
#include <stddef.h>
#include <assert.h>


namespace emb {


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


} // namespace emb


