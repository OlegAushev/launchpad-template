///
#pragma once


#include "../emb_core.h"


namespace emb {


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
	virtual void set(State state = State::Active) const = 0;
	virtual void reset() const = 0;
	virtual void toggle() const = 0;
};


} // namespace gpio


} // namespace emb


