#pragma once
#include "EventTrigger.h"

class COpenDoorEventTrigger : public CEventTrigger
{
private:
	float m_DoorAngle{};

public:
	COpenDoorEventTrigger(MSG_TYPE Type);
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
	CPowerDownEventTrigger(MSG_TYPE Type);
	virtual ~CPowerDownEventTrigger() = default;

	virtual void InteractEventTrigger();
	virtual void Update(float ElapsedTime);

	bool IsOpened() const;
};

//=========================================================================================================================

class CSirenEventTrigger : public CEventTrigger
{
public:
	CSirenEventTrigger(MSG_TYPE Type);
	virtual ~CSirenEventTrigger() = default;

	virtual void InteractEventTrigger();
};

//=========================================================================================================================

class COpenGateEventTrigger : public CEventTrigger
{
private:
	float m_GateAngle{};

public:
	COpenGateEventTrigger(MSG_TYPE Type);
	virtual ~COpenGateEventTrigger() = default;

	virtual bool CanPassTriggerArea(const XMFLOAT3& Position, const XMFLOAT3& NewPosition);

	virtual void InteractEventTrigger();
	virtual void Update(float ElapsedTime);
};

//=========================================================================================================================

class CGetPistolEventTrigger : public CEventTrigger
{
public:
	CGetPistolEventTrigger(MSG_TYPE Type);
	virtual ~CGetPistolEventTrigger() = default;

	virtual void InteractEventTrigger();
};

//=========================================================================================================================

class CGetKeyEventTrigger : public CEventTrigger
{
public:
	CGetKeyEventTrigger(MSG_TYPE Type);
	virtual ~CGetKeyEventTrigger() = default;

	virtual void InteractEventTrigger();
};
