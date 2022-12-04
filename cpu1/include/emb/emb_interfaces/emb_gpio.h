/**
 * @file emb_gpio.h
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


#include "../emb_core.h"


namespace emb {
/// @addtogroup emb
/// @{


namespace gpio {


/// Pin active states
SCOPED_ENUM_DECLARE_BEGIN(ActiveState)
{
	Low = 0,
	High = 1
}
SCOPED_ENUM_DECLARE_END(ActiveState)


/// Pin states
SCOPED_ENUM_DECLARE_BEGIN(State)
{
	Inactive = 0,
	Active = 1
}
SCOPED_ENUM_DECLARE_END(State)


/**
 * @brief GPIO input interface class.
 */
class IInput
{
public:
	IInput() {}
	virtual ~IInput() {}

	virtual State read() const = 0;
};


/**
 * @brief GPIO output interface class.
 */
class IOutput
{
public:
	IOutput() {}
	virtual ~IOutput() {}

	virtual State read() const = 0;
	virtual void set(State state = State::Active) = 0;
	virtual void reset() = 0;
	virtual void toggle() = 0;
};


} // namespace gpio


/// @}
} // namespace emb


