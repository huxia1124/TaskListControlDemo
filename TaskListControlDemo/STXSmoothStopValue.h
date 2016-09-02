#pragma once

#include <map>

//////////////////////////////////////////////////////////////////////////

class CSTXSmoothStopValue;
class CSTXSmoothStopValueEventHandler;

//////////////////////////////////////////////////////////////////////////
// CSTXAnimationValue

class CSTXAnimationValue
{
public:
	CSTXAnimationValue(DOUBLE initialValue, DOUBLE durationMax, CSTXSmoothStopValueEventHandler *pCallbackObject);
	virtual ~CSTXAnimationValue();

protected:
	BOOL _isActive;
	DOUBLE _durationMax;                                          // Maximum duration of transition
	DOUBLE _finalValue;
	DOUBLE _initialTime;
	unsigned __int64 _qpcFrequency;                                 // Frequency of QPC
	DOUBLE _initialValue;                                                       // Initial value of transition
	DOUBLE _initialVelocity;                                                    // Initial velocity of transition
	CSTXSmoothStopValueEventHandler* _pCallbackObject;

protected:
	 DOUBLE GetQueryPerformanceCounter();
	virtual void InterpolateValue
		(
		__in DOUBLE offset,                                      // Offset to compute value for
		__out DOUBLE *pValue                                                    // Value of variable at given offset
		);
	virtual void InterpolateVelocity
		(
		__in DOUBLE offset,                                      // Offset to compute velocity for
		__out DOUBLE *pVelocity                                                 // Velocity of variable at given offset
		);
	virtual void SetInitialValueAndVelocity
		(
		__in DOUBLE initialValue,                                               // Initial value of variable
		__in DOUBLE initialVelocity                                             // Initial velocity of variable
		);
public:
	virtual DOUBLE GetValue();
	virtual DOUBLE GetFinalValue();
	virtual BOOL IsActive();
	virtual DOUBLE operator=(const DOUBLE val);
};

//////////////////////////////////////////////////////////////////////////
// CSTXSinValue

class CSTXSinValue : public CSTXAnimationValue
{
public:
	CSTXSinValue(DOUBLE value, DOUBLE durationMax, DOUBLE deltaMax, DOUBLE cycles = 1.0f, CSTXSmoothStopValueEventHandler* pCallbackObject = NULL);
	virtual ~CSTXSinValue();

protected:
	DOUBLE _deltaMax;
	DOUBLE _cycles;

protected:

	virtual void InterpolateValue
		(
		__in DOUBLE offset,                                      // Offset to compute value for
		__out DOUBLE *pValue                                                    // Value of variable at given offset
		);

public:
	virtual DOUBLE GetValue();
	virtual DOUBLE operator=(const DOUBLE val);

};

//////////////////////////////////////////////////////////////////////////
// class CSTXConstantValue

class CSTXConstantValue : public CSTXAnimationValue
{
public:
	CSTXConstantValue(DOUBLE value, DOUBLE durationMax, CSTXSmoothStopValueEventHandler* pCallbackObject = NULL);
	virtual ~CSTXConstantValue();

protected:

	virtual void InterpolateValue
		(
		__in DOUBLE offset,                                      // Offset to compute value for
		__out DOUBLE *pValue                                     // Value of variable at given offset
		);

public:
	virtual DOUBLE GetValue();
	virtual DOUBLE operator=(const DOUBLE val);

};


//////////////////////////////////////////////////////////////////////////
// CSTXSmoothStopValueEventHandler

class CSTXSmoothStopValueEventHandler
{
public:
	CSTXSmoothStopValueEventHandler();
	virtual ~CSTXSmoothStopValueEventHandler();

protected:
	LONG m_nActivateCount;	//For animation

public:
	void IncreaseActivate();
	void DecreaseActivate();

protected:
	virtual void ActivateAnimation() = 0;
	virtual void DeactivateAnimation() = 0;
};

//////////////////////////////////////////////////////////////////////////
// CSTXSmoothStopValue

class CSTXSmoothStopValue
{
private:
	BOOL _isActive;
	unsigned __int64 _qpcFrequency;                                 // Frequency of QPC
	DOUBLE _initialTime;
	DOUBLE _duration;                                             // Duration of transition
    DOUBLE _durationMax;                                          // Maximum duration of transition
    DOUBLE _initialValue;                                                       // Initial value of transition
    DOUBLE _initialVelocity;                                                    // Initial velocity of transition
    DOUBLE _finalValue;                                                         // Final value of transition
    DOUBLE _cubicCoefficient;                                                   // 'a' in a * t^3 + b * t^2 + c * t + d
    DOUBLE _quadraticCoefficient;                                               // 'b' in a * t^3 + b * t^2 + c * t + d
	CSTXSmoothStopValueEventHandler* _pCallbackObject;

private:
	DOUBLE GetQueryPerformanceCounter();

	void CSTXSmoothStopValue::GetFinalValue
		(
		__out DOUBLE *pValue                                                    // Final value of variable at end of transition
		);

	void InterpolateValue
		(
		__in DOUBLE offset,                                      // Offset to compute value for
		__out DOUBLE *pValue                                                    // Value of variable at given offset
		);
	void InterpolateVelocity
		(
		__in DOUBLE offset,                                      // Offset to compute velocity for
		__out DOUBLE *pVelocity                                                 // Velocity of variable at given offset
		);

	void CSTXSmoothStopValue::SetInitialValueAndVelocityDirect
		(
		__in DOUBLE initialValue,                                               // Initial value of variable
		__in DOUBLE initialVelocity                                             // Initial velocity of variable
		);

public:
	CSTXSmoothStopValue(DOUBLE initialValue, DOUBLE durationMax, CSTXSmoothStopValueEventHandler* pCallbackObject);
	~CSTXSmoothStopValue(void);

	void CSTXSmoothStopValue::SetInitialValueAndVelocity
		(
		__in DOUBLE initialValue,                                               // Initial value of variable
		__in DOUBLE initialVelocity                                             // Initial velocity of variable
		);


	DOUBLE GetValue();
	DOUBLE GetFinalValue();
	CSTXSmoothStopValue& operator=(const DOUBLE val);
	BOOL IsActive();
	void SetStatusNotifyCallback(CSTXSmoothStopValueEventHandler* pCallbackObject);

};

