/**
 * @file emb_filter.h
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
#include <algorithm>

#include "emb_core.h"
#include "emb_array.h"
#include "emb_circularbuffer.h"
#include "emb_algorithm.h"


namespace emb {
/// @addtogroup emb
/// @{


/**
 * @brief Filter interface
 */
template <typename T>
class IFilter : public emb::noncopyable
{
public:
	IFilter() {}
	virtual ~IFilter() {}

	virtual void push(T value) = 0;
	virtual T output() const = 0;
	virtual void setOutput(T value) = 0;
	virtual void reset() = 0;
};


/**
 * @brief Moving average filter
 */
template <typename T, size_t WindowSize>
class MovingAvgFilter : public IFilter<T>
{
private:
	size_t _size;
	T* _window;
	size_t _index;
	T _sum;
	bool _heapUsed;
public:
	MovingAvgFilter()
		: _size(WindowSize)
		, _window(new T[WindowSize])
		, _index(0)
		, _sum(0)
		, _heapUsed(true)
	{
		reset();
	}

	MovingAvgFilter(emb::Array<T, WindowSize>& dataArray)
		: _size(WindowSize)
		, _window(dataArray.data)
		, _index(0)
		, _sum(T(0))
		, _heapUsed(false)
	{
		reset();
	}

	~MovingAvgFilter()
	{
		if (_heapUsed == true)
		{
			delete[] _window;
		}
	}

	virtual void push(T value)
	{
		_sum = _sum + value - _window[_index];
		_window[_index] = value;
		_index = (_index + 1) % _size;
	}

	virtual T output() const { return _sum / _size; }

	virtual void setOutput(T value)
	{
		for (size_t i = 0; i < _size; ++i)
		{
			_window[i] = value;
		}
		_index = 0;
		_sum = value * _size;
	}

	virtual void reset() { setOutput(0); }

	int size() const { return _size; }

	void resize(size_t size)
	{
		if (size == 0)
		{
			return;
		}
		if (size > WindowSize)
		{
			_size = WindowSize;
			reset();
			return;
		}
		_size = size;
		reset();
	}
};


/**
 * @brief Median filter
 */
template <typename T, size_t WindowSize>
class MedianFilter : public IFilter<T>
{
private:
	CircularBuffer<T, WindowSize> _window;
	T _out;
public:
	MedianFilter()
	{
		EMB_STATIC_ASSERT((WindowSize % 2) == 1);
		reset();
	}

	virtual void push(T value)
	{
		_window.push(value);
		Array<T, WindowSize> windowSorted;
		emb::copy(_window.begin(), _window.end(), windowSorted.begin());
		std::sort(windowSorted.begin(), windowSorted.end());
		_out = windowSorted[WindowSize/2];
	}

	virtual T output() const { return _out; }

	virtual void setOutput(T value)
	{
		_window.fill(value);
		_out = value;
	}

	virtual void reset() { setOutput(0); }
};


/**
 * @brief Exponential filter
 */
template <typename T>
class ExponentialFilter : public IFilter<T>
{
private:
	float _smoothFactor;
	T _out;
	T _outPrev;
public:
	ExponentialFilter()
		: _smoothFactor(0)
	{
		reset();
	}

	ExponentialFilter(float smoothFactor)
		: _smoothFactor(smoothFactor)
	{
		reset();
	}

	virtual void push(T value)
	{
		_out = _outPrev + _smoothFactor * (value - _outPrev);
		_outPrev = _out;
	}

	virtual T output() const { return _out; }

	virtual void setOutput(T value)
	{
		_out = value;
		_outPrev = value;
	}

	virtual void reset() { setOutput(0); }

	void setSmoothFactor(float smoothFactor) { _smoothFactor = smoothFactor; }
};


/**
 * @brief Exponential + Median filter
 */
template <typename T, size_t WindowSize>
class ExponentialMedianFilter : public IFilter<T>
{
private:
	CircularBuffer<T, WindowSize> _window;
	float _smoothFactor;
	T _out;
	T _outPrev;
public:
	ExponentialMedianFilter()
		: _smoothFactor(0)
	{
		EMB_STATIC_ASSERT((WindowSize % 2) == 1);
		reset();
	}

	ExponentialMedianFilter(float smoothFactor)
		: _smoothFactor(smoothFactor)
	{
		EMB_STATIC_ASSERT((WindowSize % 2) == 1);
		reset();
	}

	virtual void push(T value)
	{
		_window.push(value);
		Array<T, WindowSize> windowSorted;
		emb::copy(_window.begin(), _window.end(), windowSorted.begin());
		std::sort(windowSorted.begin(), windowSorted.end());
		value = windowSorted[WindowSize/2];

		_out = _outPrev + _smoothFactor * (value - _outPrev);
		_outPrev = _out;
	}

	virtual T output() const { return _out; }

	virtual void setOutput(T value)
	{
		_window.fill(value);
		_out = value;
		_outPrev = value;
	}

	virtual void reset() { setOutput(0); }

	void setSmoothFactor(float smoothFactor) { _smoothFactor = smoothFactor; }
};


/// @}
} // namespace emb


