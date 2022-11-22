///
#pragma once


#include <stdint.h>
#include <stddef.h>
#include <assert.h>


/**
 * @brief
 */
#define EMB_UNUSED(arg) (void)arg;


/**
 * @brief UNIQ_ID implementation
 */
#define EMB_UNIQ_ID_IMPL(line) _a_local_var_##line
#define EMB_UNIQ_ID(line) EMB_UNIQ_ID_IMPL(line)


/**
 * @brief simple static_assert implementation
 */
#define EMB_CAT_(a, b) a ## b
#define EMB_CAT(a, b) EMB_CAT_(a, b)
#define EMB_STATIC_ASSERT(cond) typedef int EMB_CAT(assert, __LINE__)[(cond) ? 1 : -1]


namespace emb {


/**
 * @brief
 */
enum MasterSlaveMode
{
	MODE_MASTER,
	MODE_SLAVE
};


/**
 * @brief
 */
enum ProcessStatus
{
	PROCESS_SUCCESS = 0,
	PROCESS_FAIL = 1,
	PROCESS_IN_PROGRESS = 2,
};


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


namespace c28x {


/**
 * @brief
 */
template <class T>
class interrupt_invoker
{
private:
	static T* s_instance;
	static bool s_initialized;
protected:
	interrupt_invoker(T* self)
	{
		assert(!s_initialized);
		s_instance = self;
		s_initialized = true;
	}
public:
	static T* instance()
	{
		assert(s_initialized);
		return s_instance;
	}

	static bool initialized() { return s_initialized; }

	virtual ~interrupt_invoker()
	{
		s_initialized = false;
		s_instance = static_cast<T*>(NULL);
	}
};

template <class T>
T* interrupt_invoker<T>::s_instance = static_cast<T*>(NULL);
template <class T>
bool interrupt_invoker<T>::s_initialized = false;


/**
 * @brief
 */
template <typename T>
void from_bytes8(T& dest, const uint16_t* src)
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
void to_bytes8(uint16_t* dest, const T& src)
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

	splitIntoBytes<T>(obj1_byte8, obj1);
	splitIntoBytes<T>(obj2_byte8, obj2);

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


} // namespace emb


