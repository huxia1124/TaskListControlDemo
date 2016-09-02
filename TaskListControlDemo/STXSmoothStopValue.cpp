#include "StdAfx.h"
#include "STXSmoothStopValue.h"
#define _USE_MATH_DEFINES
#include <math.h>

//////////////////////////////////////////////////////////////////////////
// CSTXAnimationValue

CSTXAnimationValue::CSTXAnimationValue(DOUBLE initialValue, DOUBLE durationMax, CSTXSmoothStopValueEventHandler *pCallbackObject)
: _isActive(FALSE)
, _initialValue(initialValue)
, _finalValue(initialValue)
, _durationMax(durationMax)
, _pCallbackObject(pCallbackObject)
{
	QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER *>(&_qpcFrequency));
}

CSTXAnimationValue::~CSTXAnimationValue()
{
	if(_isActive && _pCallbackObject)
	{
		_pCallbackObject->DecreaseActivate();
	}
}

DOUBLE CSTXAnimationValue::GetValue()
{
	return _finalValue;
}

DOUBLE CSTXAnimationValue::GetFinalValue()
{
	return _finalValue;
}

BOOL CSTXAnimationValue::IsActive()
{
	return _isActive;
}

DOUBLE CSTXAnimationValue::operator=( const DOUBLE val )
{
	DOUBLE _currentTime = GetQueryPerformanceCounter();

	DOUBLE fVelocity = 0.0f;
	InterpolateVelocity(_currentTime - _initialTime, &fVelocity);
	DOUBLE fValue = 0.0f;
	InterpolateValue(_currentTime - _initialTime, &fValue);

	_finalValue = val;
	SetInitialValueAndVelocity(fValue, fVelocity);
	_initialTime = GetQueryPerformanceCounter();
	return _finalValue;
}

DOUBLE CSTXAnimationValue::GetQueryPerformanceCounter()
{
	unsigned __int64 ticks = 0;
	QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER *>(&ticks));

	__int64 seconds = ticks / _qpcFrequency;
	__int64 remainder = ticks % _qpcFrequency;

	return seconds + ((DOUBLE)remainder / _qpcFrequency);
}

void CSTXAnimationValue::InterpolateValue( __in DOUBLE offset, /* Offset to compute value for */ __out DOUBLE *pValue /* Value of variable at given offset */ )
{
	*pValue = _finalValue;
}


void CSTXAnimationValue::InterpolateVelocity( __in DOUBLE offset, /* Offset to compute velocity for */ __out DOUBLE *pVelocity /* Velocity of variable at given offset */ )
{
	*pVelocity = 0;
}

void CSTXAnimationValue::SetInitialValueAndVelocity( __in DOUBLE initialValue, /* Initial value of variable */ __in DOUBLE initialVelocity /* Initial velocity of variable */ )
{
	_initialValue = initialValue;
	_initialVelocity = initialVelocity;
}

//////////////////////////////////////////////////////////////////////////
// CSTXSinValue

CSTXSinValue::CSTXSinValue( DOUBLE value, DOUBLE durationMax, DOUBLE deltaMax, DOUBLE cycles, CSTXSmoothStopValueEventHandler* pCallbackObject)
: CSTXAnimationValue(value, durationMax, pCallbackObject)
, _deltaMax(deltaMax)
, _cycles(cycles)
{

}

CSTXSinValue::~CSTXSinValue()
{

}

DOUBLE CSTXSinValue::GetValue()
{
	DOUBLE _currentTime = GetQueryPerformanceCounter();
	DOUBLE fValue = 0.0f;
	InterpolateValue(_currentTime - _initialTime, &fValue);
	return fValue;
}

void CSTXSinValue::InterpolateValue( __in DOUBLE offset, /* Offset to compute value for */ __out DOUBLE *pValue /* Value of variable at given offset */ )
{
	if(offset >= _durationMax)
	{
		*pValue = _finalValue;
		if(_isActive)
		{
			_isActive = FALSE;
			if(_pCallbackObject)
				_pCallbackObject->DecreaseActivate();
		}
		return;
	}
	else
	{
		if(!_isActive)
		{
			_isActive = TRUE;
			if(_pCallbackObject)
				_pCallbackObject->IncreaseActivate();
		}
	}

	DOUBLE fValue = _deltaMax * sin(2.0 * M_PI * _cycles * offset / _durationMax);
	*pValue = fValue;
}

DOUBLE CSTXSinValue::operator=( const DOUBLE val )
{
	return __super::operator =(val);
}

//////////////////////////////////////////////////////////////////////////
// CSTXConstantValue

CSTXConstantValue::CSTXConstantValue(DOUBLE value, DOUBLE durationMax, CSTXSmoothStopValueEventHandler* pCallbackObject /* = NULL */)
: CSTXAnimationValue(value, durationMax, pCallbackObject)
{

}

CSTXConstantValue::~CSTXConstantValue()
{
	if(_isActive && _pCallbackObject)
	{
		_pCallbackObject->DecreaseActivate();
	}
}

void CSTXConstantValue::InterpolateValue( __in DOUBLE offset, /* Offset to compute value for */ __out DOUBLE *pValue /* Value of variable at given offset */ )
{
	if(offset >= _durationMax)
	{
		*pValue = _finalValue;
		if(_isActive)
		{
			_isActive = FALSE;
			if(_pCallbackObject)
				_pCallbackObject->DecreaseActivate();
		}
		return;
	}
	else
	{
		if(!_isActive)
		{
			_isActive = TRUE;
			if(_pCallbackObject)
				_pCallbackObject->IncreaseActivate();
		}
	}

	*pValue = _initialValue;
}

DOUBLE CSTXConstantValue::GetValue()
{
	DOUBLE _currentTime = GetQueryPerformanceCounter();
	DOUBLE fValue = 0.0f;
	InterpolateValue(_currentTime - _initialTime, &fValue);
	return fValue;
}

DOUBLE CSTXConstantValue::operator=( const DOUBLE val )
{
	return __super::operator =(val);
}

//////////////////////////////////////////////////////////////////////////
// CSTXSmoothStopValueEventHandler

void CSTXSmoothStopValueEventHandler::DeactivateAnimation()
{
}

void CSTXSmoothStopValueEventHandler::ActivateAnimation()
{
}

void CSTXSmoothStopValueEventHandler::DecreaseActivate()
{
	m_nActivateCount--;
	if(m_nActivateCount == 0)
	{
		DeactivateAnimation();
	}
}

void CSTXSmoothStopValueEventHandler::IncreaseActivate()
{
	
	if(m_nActivateCount == 0)
	{
		ActivateAnimation();
	}
	m_nActivateCount++;
}

CSTXSmoothStopValueEventHandler::CSTXSmoothStopValueEventHandler()
: m_nActivateCount(0)
{

}

CSTXSmoothStopValueEventHandler::~CSTXSmoothStopValueEventHandler()
{

}

//////////////////////////////////////////////////////////////////////////

CSTXSmoothStopValue::CSTXSmoothStopValue(DOUBLE initialValue, DOUBLE durationMax, CSTXSmoothStopValueEventHandler* pCallbackObject)
{
    _duration = durationMax;
    _durationMax = durationMax;
    _finalValue = initialValue;
    _initialValue = initialValue;
    _initialVelocity = 0.0;
    _cubicCoefficient = 0.0;
    _quadraticCoefficient = 0.0;
	_isActive = FALSE;
	_pCallbackObject = pCallbackObject;

	QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER *>(&_qpcFrequency));
	_initialTime = GetQueryPerformanceCounter();
}

CSTXSmoothStopValue::~CSTXSmoothStopValue(void)
{
	if(_isActive && _pCallbackObject)
	{
		_pCallbackObject->DecreaseActivate();
	}
}

DOUBLE CSTXSmoothStopValue::GetQueryPerformanceCounter()
{
    unsigned __int64 ticks = 0;
    QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER *>(&ticks));

    __int64 seconds = ticks / _qpcFrequency;
    __int64 remainder = ticks % _qpcFrequency;
    
    return seconds + ((DOUBLE)remainder / _qpcFrequency);
}

void CSTXSmoothStopValue::SetInitialValueAndVelocityDirect
(
 __in DOUBLE initialValue,                                               // Initial value of variable
 __in DOUBLE initialVelocity                                             // Initial velocity of variable
 )
{
	_initialValue = initialValue;
	_initialVelocity = initialVelocity;

	// First, catch the case of a velocity of zero
	if (_initialVelocity == 0.0)
	{
		// If there's no distance to cover, this is fine
		if (_initialValue == _finalValue)
		{
			_duration = 0.0;
		}
		else
		{
			// Fall through into the cubic polynomial code below
			_duration = -1.0;
		}
	}
	else
	{
		// See if we can use a parabola to reach this time in less than the given maximum duration
		_duration = 2.0 * (_finalValue - _initialValue) / _initialVelocity;
	}

	if (_duration < 0.0 || _duration > _durationMax)
	{
		// It isn't possible to reach this finalValue in the allotted time, so switch to a cubic polynomial instead
		_duration = _durationMax;

		const DOUBLE finalVelocity = 0.0;
		DOUBLE slope = (_finalValue - _initialValue) / _duration;

		_cubicCoefficient = (_initialVelocity + finalVelocity - 2.0 * slope) / (_duration * _duration);
		_quadraticCoefficient = (3.0 * slope - 2.0 * _initialVelocity - finalVelocity) / _duration;
	}
	else
	{
		_cubicCoefficient = 0.0;
		_quadraticCoefficient = -_initialVelocity / (2.0 * _duration);
	}
}

void CSTXSmoothStopValue::SetInitialValueAndVelocity
(
    __in DOUBLE initialValue,                                               // Initial value of variable
    __in DOUBLE initialVelocity                                             // Initial velocity of variable
)
{
	if(_isActive)
	{
		_isActive = FALSE;
		if(_pCallbackObject)
			_pCallbackObject->DecreaseActivate();
	}
	_initialTime = GetQueryPerformanceCounter();

    _initialValue = initialValue;
    _initialVelocity = initialVelocity;

    // First, catch the case of a velocity of zero
    if (_initialVelocity == 0.0)
    {
        // If there's no distance to cover, this is fine
        if (_initialValue == _finalValue)
        {
            _duration = 0.0;
        }
        else
        {
            // Fall through into the cubic polynomial code below
            _duration = -1.0;
        }
    }
    else
    {
        // See if we can use a parabola to reach this time in less than the given maximum duration
        _duration = 2.0 * (_finalValue - _initialValue) / _initialVelocity;
    }

    if (_duration < 0.0 || _duration > _durationMax)
    {
        // It isn't possible to reach this finalValue in the allotted time, so switch to a cubic polynomial instead
        _duration = _durationMax;

        const DOUBLE finalVelocity = 0.0;
        DOUBLE slope = (_finalValue - _initialValue) / _duration;

        _cubicCoefficient = (_initialVelocity + finalVelocity - 2.0 * slope) / (_duration * _duration);
        _quadraticCoefficient = (3.0 * slope - 2.0 * _initialVelocity - finalVelocity) / _duration;
    }
    else
    {
        _cubicCoefficient = 0.0;
        _quadraticCoefficient = -_initialVelocity / (2.0 * _duration);
    }
	//if(fabs((double)_duration) > 0.00001)
	//{
	//	if(!_isActive)
	//	{
	//		_isActive = TRUE;
	//		if(_pfnStatusChange)
	//			_pfnStatusChange(_callbackUserData, _isActive);
	//	}
	//}
	//else
	//{
	//	if(_isActive)
	//	{
	//		_isActive = FALSE;
	//		if(_pfnStatusChange)
	//			_pfnStatusChange(_callbackUserData, _isActive);
	//	}
	//}
}

// Returns the final value of the transition
void CSTXSmoothStopValue::GetFinalValue
(
    __out DOUBLE *pValue                                                    // Final value of variable at end of transition
)
{
    *pValue = _finalValue;
}

// Computes the next value given offset
void CSTXSmoothStopValue::InterpolateValue
(
     __in DOUBLE offset,                                      // Offset to compute value for
    __out DOUBLE *pValue                                                    // Value of variable at given offset
)
{
    //SC_ASSERT(pValue);
    //SC_ASSERT(_duration != 0.0);

	if(offset >= _duration)
	{
		*pValue = _finalValue;
		if(_isActive)
		{
			_isActive = FALSE;
			if(_pCallbackObject)
				_pCallbackObject->DecreaseActivate();
		}
		return;
	}
	else
	{
		if(!_isActive)
		{
			_isActive = TRUE;
			if(_pCallbackObject)
				_pCallbackObject->IncreaseActivate();
		}
	}
	
    DOUBLE offsetSquared = offset * offset;
    
    *pValue =
        _cubicCoefficient * offset * offsetSquared +
        _quadraticCoefficient * offsetSquared +
        _initialVelocity * offset +
        _initialValue;
}

// Computes the next velocity at given offset
void CSTXSmoothStopValue::InterpolateVelocity
(
     __in DOUBLE offset,                                      // Offset to compute velocity for
    __out DOUBLE *pVelocity                                                 // Velocity of variable at given offset
)
{
    //SC_ASSERT(pVelocity);
    //SC_ASSERT(_duration != 0.0);

    if (offset < _duration)
    {
        *pVelocity =
            3.0 * _cubicCoefficient * offset * offset +
            2.0 * _quadraticCoefficient * offset +
            _initialVelocity;
    }
    else
    {
        //SC_ASSERT(offset == _duration);
    
        *pVelocity = 0.0; 
    }
}
DOUBLE CSTXSmoothStopValue::GetValue()
{
	DOUBLE _currentTime = GetQueryPerformanceCounter();
	DOUBLE fValue = 0.0f;
	InterpolateValue(_currentTime - _initialTime, &fValue);
	return fValue;
}

DOUBLE CSTXSmoothStopValue::GetFinalValue()
{
	return  _finalValue;
}

CSTXSmoothStopValue& CSTXSmoothStopValue::operator=(const DOUBLE val)
{
	DOUBLE _currentTime = GetQueryPerformanceCounter();

	DOUBLE fVelocity = 0.0f;
	InterpolateVelocity(_currentTime - _initialTime, &fVelocity);
	DOUBLE fValue = 0.0f;
	InterpolateValue(_currentTime - _initialTime, &fValue);

	_finalValue = val;
	SetInitialValueAndVelocityDirect(fValue, fVelocity);
	_initialTime = GetQueryPerformanceCounter();
	return *this;
}

BOOL CSTXSmoothStopValue::IsActive()
{
	return _isActive;
	//DOUBLE _currentTime = GetQueryPerformanceCounter();
	//return _currentTime - _initialTime < _duration;
}

void CSTXSmoothStopValue::SetStatusNotifyCallback(CSTXSmoothStopValueEventHandler* pCallbackObject)
{
	_pCallbackObject = pCallbackObject;
}


