/**
 * @file emb_bitset.h
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
#include "emb_core.h"
#include "emb_array.h"
#include <bitset>


namespace emb {
/// @addtogroup emb
/// @{


/**
 * @brief
 */
template <size_t BitCount>
class Bitset
{
	EMB_STATIC_ASSERT(BitCount > 0);
private:
	static const size_t _byteCount = (BitCount + CHAR_BIT - 1) / CHAR_BIT;
	static const size_t _extraBits = BitCount % CHAR_BIT;

	unsigned int _data[_byteCount];

	static size_t _whichByte(size_t pos) { return pos / CHAR_BIT; }
	static size_t _whichBit(size_t pos) { return pos % CHAR_BIT; }
	unsigned int& _hiByte() { return _data[_byteCount-1]; }
	unsigned int _hiByte() const { return _data[_byteCount-1]; }
	static void _resetUnusedBits(unsigned int& hiByte) { if (_extraBits) hiByte &= ~(~0U << _extraBits); }

public:
	class Reference;

	Bitset()
	{
		for (size_t i = 0; i < _byteCount; ++i)
		{
			_data[i] = 0U;
		}
	}

	Bitset(const emb::Array<unsigned int, _byteCount>& words)
	{
		for (size_t i = 0; i < _byteCount; ++i)
		{
			_data[i] = words[i];
		}
		_resetUnusedBits(_hiByte());
	}

	Bitset(uint64_t value)
	{
		EMB_STATIC_ASSERT(_byteCount <= 4);
		uint64_t val = value;	// protection against implicit conversion and wrong memcpy()
		memcpy(_data, &val, _byteCount);
		_resetUnusedBits(_hiByte());
	}

	size_t size() const { return BitCount; }

	bool operator[](size_t pos) const
	{
#ifdef NDEBUG
		return _data[_whichByte(pos)] & (1U << _whichBit(pos));
#else
		return test(pos);
#endif
	}

	Reference operator[](size_t pos)
	{
		return Reference(this, pos);
	}

	bool test(size_t pos) const
	{
		assert(pos < BitCount);
		return _data[_whichByte(pos)] & (1U << _whichBit(pos));
	}

	bool all() const
	{
		// int index is used to suppress "pointless comparison of unsigned integer with zero" warning
		for (int i = 0; i < _byteCount-1; ++i)
		{
			if (_data[i] != 0xFFFF) return false;
		}

		if (_extraBits)
		{
			if (_hiByte() != ~(~0U << _extraBits)) return false;
		}
		else
		{
			if (_hiByte() != 0xFFFF) return false;
		}
		return true;
	}

	bool any() const
	{
		for (size_t i = 0; i < _byteCount; ++i)
		{
			if (_data[i] != 0) return true;
		}
		return false;
	}

	bool none() const
	{
		return !any();
	}

	size_t count() const
	{
		size_t ret = 0;
		for (size_t i = 0; i < BitCount; ++i)
		{
			if ((*this)[i]) ++ret;
		}
		return ret;
	}

	void set()
	{
		for (size_t i = 0; i < _byteCount; ++i)
		{
			_data[i] = 0xFFFF;
		}
		_resetUnusedBits(_hiByte());
	}

	void set(size_t pos, bool value = true)
	{
		assert(pos < BitCount);
		if (value)
			_data[_whichByte(pos)] |= 1U << _whichBit(pos);
		else
			_data[_whichByte(pos)] &= ~(1U << _whichBit(pos));
	}

	void reset()
	{
		for (size_t i = 0; i < _byteCount; ++i)
		{
			_data[i] = 0U;
		}
	}

	void reset(size_t pos)
	{
		assert(pos < BitCount);
		_data[_whichByte(pos)] &= ~(1U << _whichBit(pos));
	}

	void flip()
	{
		for (size_t i = 0; i < _byteCount; ++i)
		{
			_data[i] = ~_data[i];
		}
		_resetUnusedBits(_hiByte());
	}

	void flip(size_t pos)
	{
		assert(pos < BitCount);
		_data[_whichByte(pos)] ^= 1U << _whichBit(pos);
	}

	bool operator==(const Bitset& rhs) const
	{
		for (size_t i = 0; i < _byteCount; ++i)
		{
			if (_data[i] != rhs._data[i]) return false;
		}
		return true;
	}

	bool operator!=(const Bitset& rhs) const
	{
		return !((*this) == rhs);
	}

	struct Reference
	{
		Bitset* self;
		size_t pos;
		Reference(Bitset* self_, size_t pos_) : self(self_), pos(pos_) {}
		operator bool() const { return self->test(pos); }
		Reference& operator=(bool value)
		{
			self->set(pos, value);
			return *this;
		}
	};
};


/// @}
} // namespace emb


