#pragma once
#include "EventTrigger.h"

class COpenDoorEventTrigger : public CEventTrigger
{
private:
	float m_DoorAngle{};

public:
	COpenDoorEventTrigger();
	virtual ~COpenDoorEventTrigger() = default;

	virtual bool CanPassTriggerArea(const XMFLOAT3& Position, const XMFLOAT3& NewPosition);

	virtual void InteractEventTrigger();
	virtual void Update(float ElapsedTime);
};

//=========================================================================================================================

class CPowerDownEventTrigger : public CEventTrigger
{
private:
	bool  m_IsOpened{};

	float m_PanelAngle{};

public:
	CPowerDownEventTrigger();
	virtual ~CPowerDownEventTrigger() = default;

	virtual void InteractEventTrigger();
	virtual void Update(float ElapsedTime);

	bool IsOpened() const;
};

//=========================================================================================================================

class CSirenEventTrigger : public CEventTrigger
{
public:
	CSirenEventTrigger();
	virtual ~CSirenEventTrigger() = default;

	virtual void InteractEventTrigger();
};

//=========================================================================================================================

class COpenGateEventTrigger : public CEventTrigger
{
private:
	float m_GateAngle{};

public:
	COpenGateEventTrigger();
	virtual ~COpenGateEventTrigger() = default;

	virtual bool CanPassTriggerArea(const XMFLOAT3& Position, const XMFLOAT3& NewPosition);

	virtual void InteractEventTrigger();
	virtual void Update(float ElapsedTime);
};

//=========================================================================================================================

class CGetPistolEventTrigger : public CEventTrigger
{
public:
	CGetPistolEventTrigger();
	virtual ~CGetPistolEventTrigger() = default;

	virtual void InteractEventTrigger();
};

//=========================================================================================================================

class CGetKeyEventTrigger : public CEventTrigger
{
public:
	CGetKeyEventTrigger();
	virtual ~CGetKeyEventTrigger() = default;

	virtual void InteractEventTrigger();
};
