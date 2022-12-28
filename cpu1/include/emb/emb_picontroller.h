/**
 * @file emb_picontroller.h
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
#include "emb_algorithm.h"
#include "float.h"


namespace emb {
/// @addtogroup emb
/// @{


/// Controller logic
SCOPED_ENUM_DECLARE_BEGIN(ControllerLogic)
{
	Direct,
	Inverse
}
SCOPED_ENUM_DECLARE_END(ControllerLogic)


/*
 * @brief PI controller interface
 */
template <ControllerLogic::enum_type Logic>
class IPiController : public emb::noncopyable
{
protected:
	float _kP;		// proportional gain
	float _kI;		// integral gain
	float _dt;		// time slice
	float _sumI;		// integrator sum;
	float _outMin;		// PI output minimum limit
	float _outMax;		// PI output maximum limit
	float _out;		// PI output;

	static float _error(float ref, float meas);
public:
	IPiController(float kP, float kI, float dt, float outMin, float outMax)
		: _kP(kP)
		, _kI(kI)
		, _dt(dt)
		, _sumI(0)
		, _outMin(outMin)
		, _outMax(outMax)
		, _out(0)
	{}

	virtual ~IPiController() {}
	virtual void update(float ref, float meas) = 0;
	virtual void reset()
	{
		_sumI = 0;
		_out = 0;
	}
	float output() const { return _out; }
	void setOutputMin(float value) { _outMin = value; }
	void setOutputMax(float value) { _outMax = value; }
	float outputMin() const { return _outMin; }
	float outputMax() const { return _outMax; }

	void setKp(float value) { _kP = value; }
	void setKi(float value) { _kI = value; }
	float kP() const { return _kP; }
	float kI() const { return _kI; }
	float sumI() const { return _sumI; }
};


inline float IPiController<ControllerLogic::Direct>::_error(float ref, float meas) { return ref - meas; }
inline float IPiController<ControllerLogic::Inverse>::_error(float ref, float meas) { return meas - ref; }


/*
 * @brief PI controller with back-calculation
 */
template <ControllerLogic::enum_type Logic>
class PiControllerBC : public IPiController<Logic>
{
protected:
	float _kC;	// anti-windup gain
public:
	PiControllerBC(float kP, float kI, float dt, float kC, float outMin, float outMax)
		: IPiController<Logic>(kP, kI, dt, outMin, outMax)
		, _kC(kC)
	{}

	virtual void update(float ref, float meas)
	{
		float error = IPiController<Logic>::_error(ref, meas);
		float out = emb::clamp(error * this->_kP + this->_sumI, -FLT_MAX, FLT_MAX);

		if (out > this->_outMax)
		{
			this->_out = this->_outMax;
		}
		else if (out < this->_outMin)
		{
			this->_out = this->_outMin;
		}
		else
		{
			this->_out = out;
		}

		this->_sumI = emb::clamp(this->_sumI + this->_kI * this->_dt * error - _kC * (out - this->_out),
				-FLT_MAX, FLT_MAX);
	}
};


/*
 * @brief PI controller with clamping
 */
template <ControllerLogic::enum_type Logic>
class PiControllerCl : public IPiController<Logic>
{
protected:
	float _error;
public:
	PiControllerCl(float kP, float kI, float dt, float outMin, float outMax)
		: IPiController<Logic>(kP, kI, dt, outMin, outMax)
		, _error(0)
	{}

	virtual void update(float ref, float meas)
	{
		float error = IPiController<Logic>::_error(ref, meas);
		float outp = error * this->_kP;
		float sumI = (error + _error) * 0.5f * this->_kI * this->_dt + this->_sumI;
		_error = error;
		float out = outp + sumI;

		if (out > this->_outMax)
		{
			this->_out = this->_outMax;
			if (outp < this->_outMax)
			{
				this->_sumI = this->_outMax - outp;
			}
		}
		else if (out < this->_outMin)
		{
			this->_out = this->_outMin;
			if (outp > this->_outMin)
			{
				this->_sumI = this->_outMin - outp;
			}
		}
		else
		{
			this->_out = out;
			this->_sumI = sumI;
		}
	}

	virtual void reset()
	{
		this->_sumI = 0;
		_error = 0;
		this->_out = 0;
	}
};


/// @}
} // namespace emb


