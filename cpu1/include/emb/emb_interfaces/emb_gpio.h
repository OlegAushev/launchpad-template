///
#pragma once


#include "../emb_core.h"


namespace emb {


/// Pin active states
enum PinActiveState
{
	ActiveLow = 0,
	ActiveHigh = 1
};


/// Pin states
enum PinState
{
	PinInactive = 0,
	PinActive = 1
};


/**
 * @brief GPIO input interface class.
 */
class IGpioInput
{
public:
	IGpioInput() {}
	virtual ~IGpioInput() {}

	virtual emb::PinState read() const = 0;
};


/**
 * @brief GPIO output interface class.
 */
class IGpioOutput
{
public:
	IGpioOutput() {}
	virtual ~IGpioOutput() {}

	virtual emb::PinState read() const = 0;
	virtual void set(emb::PinState state = emb::PinActive) const = 0;
	virtual void reset() const = 0;
	virtual void toggle() const = 0;
};


} // namespace emb


